#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_timer.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define CONFIG_EXAMPLE_IPV4 1
#ifdef CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR "192.168.4.255"
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR "192.168.4.1"
#endif

#define PORT 3333
#define LOG_OUTPUT                  0

#include "driver/gpio.h"
#define GPIO_ENABLE                 1
#define GPIO_OUTPUT_PIN_SEL         (1ULL<<18)  

static const char *TAG = "example";
//37 bytes
//static const char *payload = "testing now again ... xyz yadda yadda";
static const char *start = "start_timer";
static const char *stop = "stop_timer";
int broadcast_enable = 1;
struct sockaddr_in broadcast_addr;

//108 bytes
//static const char *payload = "181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021";
//120 bytes
//static const char *payload = "181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627";
//216 bytes
//static const char *payload = "181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425";
//428 bytes
static const char *payload = "18192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123";
//1970 bytes
//static const char *payload = "18192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627";

static void udp_time_synch(void *pvParameters)
{
    char rx_buffer[2048];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1) {
        /*struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl("0.0.0.0");
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;*/

        //Init specific target address
        struct sockaddr_in target;
        target.sin_addr.s_addr = inet_addr("192.168.4.2");
        target.sin_family = AF_INET;
        target.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        //"Listening/Host" -Socket
        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");
        
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("Failed to set socket option SO_BROADCAST");
        close(sock);
        exit(EXIT_FAILURE);
        }
        int enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof enable);
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", "192.168.4.1", PORT);

        // Configure broadcast address
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = 2;
        broadcast_addr.sin_port = htons(PORT);
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        int enable = 1;
        lwip_setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int buffer_size = 32768;
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsgtmp;
        u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

        iov.iov_base = rx_buffer;
        iov.iov_len = sizeof(rx_buffer);
        msg.msg_control = cmsg_buf;
        msg.msg_controllen = sizeof(cmsg_buf);
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (struct sockaddr *)&source_addr;
        msg.msg_namelen = socklen;
#endif

        while (1) {
#if 1
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
            int len = recvmsg(sock, &msg, 0);
#else       
            //ESP_LOGI(TAG, "Time Before recv: %lld", esp_timer_get_time());
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            //ESP_LOGI(TAG, "Time after recv: %lld", esp_timer_get_time());

            gpio_set_level(18, 1);
#endif
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            
                // Get the sender's ip address as string
                /*if (source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
                    for ( cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL; cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp) ) {
                        if ( cmsgtmp->cmsg_level == IPPROTO_IP && cmsgtmp->cmsg_type == IP_PKTINFO ) {
                            struct in_pktinfo *pktinfo;
                            pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgtmp);
                            ESP_LOGI(TAG, "dest ip: %s", inet_ntoa(pktinfo->ipi_addr));
                        }
                    }
#endif
                } else if (source_addr.ss_family == PF_INET6) {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
                }*/

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                #if LOG_OUTPUT 
                ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
                #endif
                vTaskDelay(40 / portTICK_PERIOD_MS);
                gpio_set_level(18, 0);
                vTaskDelay(360 / portTICK_PERIOD_MS);

                /*int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
                ESP_LOGI(TAG, "SENT");*/
#endif
#if 0
            int err = sendto(sock, start, strlen(start), 0, (struct sockaddr *)&target, sizeof(target));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "SENT");
            vTaskDelay(500/portTICK_PERIOD_MS);
#endif
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
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
    ESP_ERROR_CHECK(example_connect());
    esp_wifi_set_ps(0);

    //xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
    xTaskCreate(udp_time_synch, "udp_server", 4096, (void*)AF_INET, 5, NULL);
}
