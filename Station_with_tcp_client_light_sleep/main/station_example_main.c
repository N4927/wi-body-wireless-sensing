#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "sdkconfig.h"


#define TAG                  "STA"
#define TIME_SYNCH_ENABLE    0
#define LIGHT_SLEEP_TIME_MS  5000  
#define EXAMPLE_ESP_WIFI_SSID  "YOUR_WIFI_SSID"
#define EXAMPLE_ESP_WIFI_PASS  "YOUR_WIFI_PASSWORD"
#define ESP_MAXIMUM_RETRY      5

// Definizione delle latenze per i diversi scenari
#define LATENCY_HMI_MS       100   // 100 ms per HMI
#define LATENCY_SPORT_MS     1000  // 1 s per monitoraggio sportivo
#define LATENCY_BIO_MS       100000 // 100 s per storage biologico

// Variabile globale per il tempo di sleep
static int sleep_time_ms = LIGHT_SLEEP_TIME_MS;

// Flag per indicare se il dispositivo è in light sleep
static volatile bool is_in_light_sleep = false;
extern volatile bool is_tcp_active;
extern EventGroupHandle_t s_tcp_event_group;

// Inizializzazione della variabile in base alla configurazione
static void init_sleep_time(void) {
#ifdef CONFIG_EXAMPLE_LATENCY_HMI
    sleep_time_ms = LATENCY_HMI_MS;
#elif defined(CONFIG_EXAMPLE_LATENCY_SPORT)
    sleep_time_ms = LATENCY_SPORT_MS;
#elif defined(CONFIG_EXAMPLE_LATENCY_BIO)
    sleep_time_ms = LATENCY_BIO_MS;
#endif
    ESP_LOGI(TAG, "Sleep time initialized to %d ms", sleep_time_ms);
}

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

#define TCP_ACTIVE_BIT     BIT0
#define TCP_INACTIVE_BIT   BIT1
// Dichiarazione anticipata della funzione tcp_client
extern void tcp_client(void *pvParameters);
extern void udp_time_synch(void *pvParameters);

// Modified event handler for better connection management
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } 
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (s_retry_num < ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(TAG, "Retry connecting to AP (%d/%d)", s_retry_num, ESP_MAXIMUM_RETRY);
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            }
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Improved Wi-Fi initialization with error checking
void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
            .listen_interval = sleep_time_ms / 100, // Adjust based on sleep time and beacon interval
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                        pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP. Setting power save mode");
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));  // Better for light sleep
    } else {
        ESP_LOGE(TAG, "Failed to connect to AP");
    }
}

// Task dedicato per il light sleep con controllo manuale
void light_sleep_task(void *pvParameters)
{
    // Wait for TCP event group to be initialized
    int retry_count = 0;
    while (s_tcp_event_group == NULL && retry_count < 10) {
        ESP_LOGI(TAG, "Waiting for TCP event group to be initialized...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry_count++;
    }
    
    if (s_tcp_event_group == NULL) {
        ESP_LOGE(TAG, "TCP event group not initialized after timeout, task will exit");
        vTaskDelete(NULL);
        return;
    }
    
    while(1) {
        // 1. Attendi che il TCP sia inattivo PRIMA del sleep
        EventBits_t tcp_bits = xEventGroupWaitBits(
            s_tcp_event_group,
            TCP_INACTIVE_BIT,
            pdTRUE,       // Clear bits on exit
            pdTRUE,       // Wait for ALL bits
            pdMS_TO_TICKS(sleep_time_ms)
        );

        // 2. Controllo connessione WiFi migliorato
        EventBits_t wifi_bits = xEventGroupGetBits(s_wifi_event_group);
        if (!(wifi_bits & WIFI_CONNECTED_BIT)) {
            ESP_LOGW(TAG, "WiFi disconnected, attempting reconnect...");
            esp_wifi_connect();
            
            // Attendi con timeout per la riconnessione
            wifi_bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE, pdFALSE,
                                          pdMS_TO_TICKS(2000));
            
            if (!(wifi_bits & WIFI_CONNECTED_BIT)) {
                ESP_LOGE(TAG, "Reconnection failed, retrying...");
                continue;
            }
        }

        // 3. Configurazione corretta del wakeup source
        esp_sleep_enable_timer_wakeup(sleep_time_ms * 1000);
        
        // 4. Sincronizzazione stati con event group
        xEventGroupClearBits(s_tcp_event_group, TCP_ACTIVE_BIT);
        xEventGroupSetBits(s_tcp_event_group, TCP_INACTIVE_BIT);

        // 5. Single sleep call con timing corretto
        int64_t sleep_start = esp_timer_get_time();
        esp_light_sleep_start();
        int64_t sleep_duration = (esp_timer_get_time() - sleep_start) / 1000;

        ESP_LOGI(TAG, "Woke up after %.2f ms", (float)sleep_duration);

        // 6. Notifica al task TCP dopo il risveglio
        xEventGroupSetBits(s_tcp_event_group, TCP_ACTIVE_BIT);
        
        // 7. Delay ottimizzato basato sul tempo residuo
        vTaskDelay(pdMS_TO_TICKS(10)); // Breve delay per yield
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inizializza il tempo di sleep in base alla configurazione
    init_sleep_time();

    // Configure power management
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 40,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);

    // Initialize TCP event group
    s_tcp_event_group = xEventGroupCreate();
    if (s_tcp_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create TCP event group");
        return;
    }
    // Set initial state to active
    xEventGroupSetBits(s_tcp_event_group, TCP_ACTIVE_BIT);

    // Initialize TCP/IP adapter
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialize WiFi
    wifi_init_sta();

    // Create tasks
    xTaskCreate(tcp_client, "tcp_client", 4096, NULL, 5, NULL);
    xTaskCreate(udp_time_synch, "udp_time_synch", 4096, NULL, 5, NULL);
    xTaskCreate(light_sleep_task, "light_sleep", 4096, NULL, 5, NULL);
}