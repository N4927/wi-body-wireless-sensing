#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_timer.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_mac.h"
#include "esp_attr.h"
#include <sys/select.h>
#include <netinet/in.h>
//enable sleep mode
#include "esp_sleep.h"

#define TRUE                       1
#define TAG                        "AP"
#define EXAMPLE_ESP_WIFI_SSID      "myssid"
#define EXAMPLE_ESP_WIFI_PASS      "mypassword"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       8
#define PORT                       3333
#define KEEPALIVE_IDLE             CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL         CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT            CONFIG_EXAMPLE_KEEPALIVE_COUNT
#define LOG_OUTPUT                 0
#define BUFFER_SIZE                2048
#define MICRO_S_CONVERSION         1000000
#define TIMER_PERIOD               (MICRO_S_CONVERSION * 1)
#define ACTIVE_TIME                10      // 10 seconds active
#define SLEEP_TIME                 5       // 5 seconds sleep
static int connected_stations = 0;
#define BEACON_INTERVAL 300       // 300 * 100ms = 30 seconds
#define MODEM_SLEEP_ACTIVE_TIME 30

static int listen_sock = -1;
static int client_sockets[EXAMPLE_MAX_STA_CONN] = {0};
TaskHandle_t tcp_task_handle = NULL;
esp_timer_handle_t throughput_timer;
esp_timer_handle_t sleep_timer;

// Timer callback prototypes
static void periodic_timer_callback(void* arg);
static void sleep_timer_callback(void* arg);

static void close_all_connections() {
    // Close all client sockets
    for (int i = 0; i < EXAMPLE_MAX_STA_CONN; i++) {
        if (client_sockets[i] != 0) {
            shutdown(client_sockets[i], 0);
            close(client_sockets[i]);
            client_sockets[i] = 0;
        }
    }

    // Close listening socket
    if (listen_sock != -1) {
        shutdown(listen_sock, 0);
        close(listen_sock);
        listen_sock = -1;
    }
}

static void enter_sleep_mode() {
    if (connected_stations > 0) {
        ESP_LOGI(TAG, "Skipping sleep - active clients: %d", connected_stations);
        ESP_ERROR_CHECK(esp_timer_restart(sleep_timer, ACTIVE_TIME * MICRO_S_CONVERSION));
        return;
    }

    ESP_LOGI(TAG, "Preparing for sleep...");
    
    // Stop throughput timer
    esp_timer_stop(throughput_timer);
    
    // Close all network connections
    close_all_connections();
    
    // Delete TCP server task
    if (tcp_task_handle != NULL) {
        vTaskDelete(tcp_task_handle);
        tcp_task_handle = NULL;
    }

    // Stop WiFi
    ESP_ERROR_CHECK(esp_wifi_stop());
    
    // Configure wakeup timer
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * MICRO_S_CONVERSION);
    ESP_LOGI(TAG, "Entering light sleep for %d seconds", SLEEP_TIME);
    esp_light_sleep_start();

    // After wakeup
    ESP_LOGI(TAG, "Waking up from sleep");
    
    // Re-start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Re-create TCP server task
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, &tcp_task_handle);
    
    // Restart throughput timer
    esp_timer_start_periodic(throughput_timer, TIMER_PERIOD);
    
    // Restart sleep timer
    ESP_ERROR_CHECK(esp_timer_restart(sleep_timer, ACTIVE_TIME * MICRO_S_CONVERSION));
}

static void sleep_timer_callback(void* arg) {
    enter_sleep_mode();
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    // Existing wifi event handler code...
}

void wifi_init_softap(void) {
    // Existing WiFi init code...
}

static void do_retransmit(void *pvParameters) {
    // Existing retransmit code...
}

static void tcp_server_task(void *pvParameters) {
    // Modified to use global listen_sock
    char addr_str[128];
    int addr_family = (int)pvParameters;
    struct sockaddr_storage dest_addr;

    // IPv4 setup
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket");
        vTaskDelete(NULL);
        return;
    }

    int flag = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
        ESP_LOGE(TAG, "Socket bind failed");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, 1) != 0) {
        ESP_LOGE(TAG, "Listen failed");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection");
            break;
        }

        // Store client socket
        for (int i = 0; i < EXAMPLE_MAX_STA_CONN; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = sock;
                break;
            }
        }

        xTaskCreate(do_retransmit, "TCP_Receiver", 4096, &sock, 5, NULL);
    }

    vTaskDelete(NULL);
}

static void periodic_timer_callback(void* arg) {
    // Existing throughput calculation code...
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create timers
    const esp_timer_create_args_t throughput_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "throughput_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&throughput_timer_args, &throughput_timer));

    const esp_timer_create_args_t sleep_timer_args = {
        .callback = &sleep_timer_callback,
        .name = "sleep_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&sleep_timer_args, &sleep_timer));

    // Initialize WiFi
    wifi_init_softap();
    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MCS7_SGI);

    // Start timers
    esp_timer_start_periodic(throughput_timer, TIMER_PERIOD);
    ESP_ERROR_CHECK(esp_timer_start_once(sleep_timer, ACTIVE_TIME * MICRO_S_CONVERSION));

    // Start TCP server task
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, &tcp_task_handle);
}