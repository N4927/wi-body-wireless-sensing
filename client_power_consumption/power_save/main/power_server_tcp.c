#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "get_ap_info.h"
#include "esp_pm.h"

#define DEFAULT_LISTEN_INTERVAL CONFIG_EXAMPLE_WIFI_LISTEN_INTERVAL
#define DEFAULT_BEACON_TIMEOUT  CONFIG_EXAMPLE_WIFI_BEACON_TIMEOUT


//stay in light sleep
#if CONFIG_EXAMPLE_POWER_SAVE_MIN_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_EXAMPLE_POWER_SAVE_MAX_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MAX_MODEM
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif

#define HOST_IP_ADDR "192.168.4.1"
#define PORT 3333
static const char *TAG = "tcp_client";
static const char *payload = "1819202122232425262728929012345678910112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314164684684645168721135468784354151617181920212223242526272829012345678910111213141516171819202122232425262728297181920212223242526272829012345678910111213141516171819202122232425262728211213141516171819202122232425218192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213138607200";

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void wifi_power_save(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .listen_interval = DEFAULT_LISTEN_INTERVAL,
        },
    };

    strcpy((char *)wifi_config.sta.ssid, get_ap_ssid());
    strcpy((char *)wifi_config.sta.password, get_ap_password());

    ESP_LOGI(TAG, "Connecting to AP: %s", wifi_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_inactive_time(WIFI_IF_STA, DEFAULT_BEACON_TIMEOUT));
    ESP_ERROR_CHECK(esp_wifi_set_ps(DEFAULT_PS_MODE));
}

void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, HOST_IP_ADDR, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    while (1) {
        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Connecting to %s:%d", HOST_IP_ADDR, PORT);
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            close(sock);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Successfully connected");

        while (1) {
            int err = send(sock, payload, strlen(payload), 0);
            ESP_LOGI(TAG, "data sended");
            //ESP_LOGE(TAG, "Send problem: errno %d", err);
            if (err < 0) {
                ESP_LOGE(TAG, "Send failed: errno %d", errno);
                break;
            }


            // int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // ESP_LOGI(TAG, "recived %d",len);
            // if (len < 0) {
            //     ESP_LOGE(TAG, "Receive failed: errno %d", errno);
            //     break;
            // } else if (len == 0) {
            //     ESP_LOGW(TAG, "Connection closed");
            //     break;
            // } else {
            //     rx_buffer[len] = 0;
            //     ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            // }

           // vTaskDelay(1000 / portTICK_PERIOD_MS);  // optional delay
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Closing socket...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Reconnect after delay
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

#if CONFIG_EXAMPLE_GET_AP_INFO_FROM_STDIN
    get_ap_info_from_stdin();
#endif

#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = {
        .max_freq_mhz = CONFIG_EXAMPLE_MAX_CPU_FREQ_MHZ,
        .min_freq_mhz = CONFIG_EXAMPLE_MIN_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif

    // once called this function will operate autonmoally 
    // when there is no transmition will outmaticly switch to the type selected in Kconfig
    wifi_power_save();

    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
