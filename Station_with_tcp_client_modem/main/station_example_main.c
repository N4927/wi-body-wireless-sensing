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

#define TAG                  "STA"
#define DATA_STREAM_ENABLE   1  // Abilita il task tcp_client
#define TIME_SYNCH_ENABLE    0

// --- Config Wi-Fi ---
#define EXAMPLE_ESP_WIFI_SSID  "YOUR_WIFI_SSID"

#define EXAMPLE_ESP_WIFI_PASS  "YOUR_WIFI_PASSWORD"
#define ESP_MAXIMUM_RETRY      5

// Event group e bit usati per segnalare la connessione
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

// Dichiarazioni di eventuali task esterni
extern void tcp_client(void);
extern void udp_time_synch(void *pvParameters);

// --- Gestore eventi Wi-Fi / IP ---
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry connecting to AP (%d/%d)", s_retry_num, ESP_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Connection to AP failed");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// --- Inizializzazione Wi-Fi (STA) ---
void wifi_init_sta(void)
{
    // Inizializza interfaccia di rete
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Crea event group per segnalare connessione/fallimento
    s_wifi_event_group = xEventGroupCreate();

    // Inizializza driver Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Registra event handler
    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    // Config Wi-Fi STA
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
        },
    };

    // Setta modalità STA, configura e avvia
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Imposta la modalità di risparmio energetico minima (modem sleep)
    ESP_LOGI(TAG, "Setting WiFi Power Save mode to WIFI_PS_MIN_MODEM");
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));

    ESP_LOGI(TAG, "Wi-Fi initialization complete. Connecting to: %s", EXAMPLE_ESP_WIFI_SSID);

    // Attendi di essere connessi o di aver fallito
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP: %s", EXAMPLE_ESP_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to AP: %s", EXAMPLE_ESP_WIFI_SSID);
    }
}

void app_main(void)
{
    // Inizializza NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Avvia Wi-Fi in STA
    wifi_init_sta();

#if DATA_STREAM_ENABLE
    xTaskCreate(tcp_client, "TCP_ClientTask", 4096, NULL, 5, NULL);
#endif

#if TIME_SYNCH_ENABLE
    xTaskCreate(udp_time_synch, "Time_SyncTask", 4096, (void*)AF_INET, 5, NULL);
#endif

}
