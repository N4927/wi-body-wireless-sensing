#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"

#define TAG              "TCP_CLIENT"
#define SERVER_PORT      3333
static const char *SERVER_IP = "192.168.4.1";
static const char *payload = "1819202122232425262728929012345678910112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314164684684645168721135468784354151617181920212223242526272829012345678910111213141516171819202122232425262728297181920212223242526272829012345678910111213141516171819202122232425262728211213141516171819202122232425218192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213138607200";

#define LATENCY_HMI_MS       100   
#define LATENCY_SPORT_MS     1000  
#define LATENCY_BIO_MS       100000
#define LATENCY_250          250
#define PERCENTAGE           100
#define DUTY_CYCLE_US   (LATENCY_BIO_MS * 1000)     


static uint32_t ACTIVE_TIME_US = (uint32_t)(((uint64_t)PERCENTAGE * LATENCY_BIO_MS * 1000) / 100);



void tcp_client(void)
{
    ESP_LOGI(TAG, "Starting tcp_client task");

    while (1)
    {
        struct sockaddr_in dest_addr = { 0 };
        dest_addr.sin_family      = AF_INET;
        dest_addr.sin_port        = htons(SERVER_PORT);
        dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket() failed: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
            ESP_LOGE(TAG, "connect() failed: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        ESP_LOGI(TAG, "Connected to %s:%d", SERVER_IP, SERVER_PORT);

        while (1)
        {
            //active phase
            int64_t cycle_start = esp_timer_get_time();      // t₀ in µs
            esp_wifi_set_ps(WIFI_PS_NONE);                   // radio sempre ON

            while (esp_timer_get_time() - cycle_start < ACTIVE_TIME_US)
            {
                ssize_t written = send(sock, payload, strlen(payload), 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "send() failed: errno %d", errno);
                    goto sock_error;
                }
            }

            // sleep phase
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);              
            int64_t deadline = cycle_start + DUTY_CYCLE_US;

            // wait to the end of the 100 ms
            while (esp_timer_get_time() < deadline) {
                vTaskDelay(pdMS_TO_TICKS(1));
                //reset watchdog 
            }
        }

    sock_error:
        shutdown(sock, 0);
        close(sock);
        ESP_LOGI(TAG, "Socket closed, retrying in 5 s");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    vTaskDelete(NULL);
}
