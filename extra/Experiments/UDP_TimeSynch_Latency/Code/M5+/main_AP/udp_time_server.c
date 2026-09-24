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

//#define TAG "Access Point"
#define EXAMPLE_ESP_WIFI_SSID      "YOUR_WIFI_SSID"
#define EXAMPLE_ESP_WIFI_PASS      "YOUR_WIFI_PASSWORD"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4
#define CONFIG_EXAMPLE_IPV4 1

#define HOST_IP_ADDR                "192.168.4.255"
#define PORT                        1234
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *payload = "18192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123";

#define LOG_OUTPUT                  1       //For displaying every log statement: 1, for max performance: 0
#define BUFFER_SIZE                 2048

//GPIO Pin for testing TCP latency
#include "driver/gpio.h"
#define GPIO_ENABLE                 1
#define GPIO_OUTPUT_PIN_SEL         (1ULL<<18)  

#define TAG                         "UDP"
//static const char *TAG = "UDP";
static const char *start = "start_timer";

struct sockaddr_in broadcast_addr;
int broadcast_enable = 1;

void udp_time_master(void *pvParameters)
{
    #if GPIO_ENABLE
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    #endif

    char rx_buffer[2048];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    int broadcast_enable = 1;

    while (1) {

#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Enable broadcast option
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
            perror("Failed to set socket option SO_BROADCAST");
            close(sock);
            exit(EXIT_FAILURE);
        }

        // Set timeout
        /*struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
*/
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", "192.168.4.1", PORT);


        uint64_t time;
        uint64_t toggle_GPIO;
        char temp[128];
        //Loop periodically sends timestamps
        while (1) {

            //if-conditional for enabling sendings
            #if 1       
            time = esp_timer_get_time();
            sprintf(temp,"time %lld",time);
            int err = sendto(sock, temp, strlen(temp), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

            #if GPIO_ENABLE
            gpio_set_level(18, 1);
            toggle_GPIO = esp_timer_get_time();
            #endif

            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");
            #endif

            #if 0
            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                /*if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }*/
            }
            #endif

            #if GPIO_ENABLE
            while (toggle_GPIO + 10000 > esp_timer_get_time()){
            }
            gpio_set_level(18, 0);
            toggle_GPIO = esp_timer_get_time();
            while (toggle_GPIO + 20000 > esp_timer_get_time()){
            }
            #endif
            //vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        //Socket Error; shutting down and restarting
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}