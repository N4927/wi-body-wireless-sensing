//Julian Caredda 18.11.2024
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

#define TRUE                       1
#define TAG                        "AP"
#define EXAMPLE_ESP_WIFI_SSID      "myssid"
#define EXAMPLE_ESP_WIFI_PASS      "mypassword"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       8
#define PORT                        3333
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT
#define LOG_OUTPUT                  0       //For displaying every log statement: 1, for max performance: 0
#define BUFFER_SIZE                 2048    //Size of RX_Buffer in bytes
#define MICRO_S_CONVERSION          1000000 //1 second in microseconds
#define TIMER_PERIOD                (MICRO_S_CONVERSION * 1) //Timer period until callback function is called

static int sock;
static int client_sockets[EXAMPLE_MAX_STA_CONN] = {0};
fd_set read_fds;
struct timeval timeout;

esp_timer_handle_t timer;
static void periodic_timer_callback(void* arg);
uint64_t time1 = 0;
uint64_t time2 = 0;
uint64_t timeDifference = 0;
uint64_t totalBytesSent = 0;
uint64_t totalBytesSent_OLD = 0;
uint64_t totalBytesSentDifference = 0;
double currentThroughput = 0;
double average = 0;
uint64_t printCounter = 0;
double addedUpThroughput = 0;


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


static void do_retransmit(void *pvParameters)
{
    const int sock = *(int *)pvParameters;
    ESP_LOGI(TAG, "Socket active: %d", sock);
    int len;
    char rx_buffer[BUFFER_SIZE];

    do {
        //select();
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            shutdown(sock, 0);
            close(sock);
        } else if (len == 0) {
            //ESP_LOGW(TAG, "Connection closed");
            shutdown(sock, 0);
            close(sock);
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            if (LOG_OUTPUT) ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            if (!LOG_OUTPUT) totalBytesSent += len;
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128]; //IP Adress
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr; //File descriptor (?)

#ifdef CONFIG_EXAMPLE_IPV4
    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);//3333
        ip_protocol = IPPROTO_IP;
    }
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    /*
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));*/
    /*int buffer_size = 32768; // or larger
    setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));*/

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGE(TAG, "Socket created: %d", listen_sock);
    /*
    ESP_LOGE(TAG, "SENDING MESSAGE");
    const char *payload = "Hello from ESP32-C6!";
    int bytes_sent = send(listen_sock, payload, strlen(payload), 0);
    if (bytes_sent < 0) {
        ESP_LOGE("TCP", "Error occurred during sending: errno %d", errno);
    } else {
        ESP_LOGI("TCP", "Message sent successfully");
    }
    */
    int flag = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }
    
    int sock;
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    
    while (1)
    {
        ESP_LOGI(TAG, "Socket listening");
        sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

// Convert ip address to string
#ifdef CONFIG_EXAMPLE_IPV4
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
#ifdef CONFIG_EXAMPLE_IPV6
        if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted IP ADDRESS: %s, SOCK: %d", addr_str, sock);

        int *temp = &sock;
        xTaskCreate(do_retransmit, "TCP_Receiver", 4096, (int*)temp, 5, NULL);
        //do_retransmit(sock);

        //break;
    }

    while (1) {

        //ESP_LOGI(TAG, "Socket listening");

        /*sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }*/

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
#ifdef CONFIG_EXAMPLE_IPV4
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
#ifdef CONFIG_EXAMPLE_IPV6
        if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted ip address: %s, SOCK: %d", addr_str, sock);
        //select();
        //xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
        //xTaskCreate(do_retransmit, "Receiver", 4096, (void*)AF_INET, 5, NULL);
        //do_retransmit(sock);

        
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

static void periodic_timer_callback(void* arg)
{ //INTERRUPT: Gets called after each TIMERPERIOD
    time1 = time2;
    time2 = esp_timer_get_time();
    timeDifference = time2 - time1;
    totalBytesSentDifference = totalBytesSent - totalBytesSent_OLD;
    totalBytesSent_OLD = totalBytesSent;

    //Computing the throughput in [Bytes/second]
    currentThroughput = ((double)totalBytesSentDifference / timeDifference) * MICRO_S_CONVERSION;
    
    //Computing the average throughput
    ++printCounter;
    addedUpThroughput += currentThroughput;
    average = (double)addedUpThroughput / printCounter;

    ESP_LOGI(TAG,
    "Runtime: %lld [us], #Bytes: %lld, TP_Interval: %lld [Bytes/PERIOD], Throughput: %.0f[B/s], Avg TP: %.2f[B/s]",
    time2, totalBytesSent, totalBytesSentDifference, currentThroughput, average);
}


void app_main(void)
{
    //ESP_LOGE(TAG, "+++++++++++++++++++START+++++++++++++++++++++");
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };
    
    if(esp_timer_create(&periodic_timer_args, &timer) != ESP_OK){
        ESP_LOGE(TAG, "ERROR WHILE CREATING TIMER");
    }

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    //wifi_phy_rate_t rate = WIFI_PHY_RATE_MCS7_SGI;
    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MCS7_SGI);
    ESP_LOGI(TAG, "wifi_init completed");

    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    if (!LOG_OUTPUT /*&& strcmp(rx_buffer, "start_timer") == 0*/) {
        //Timer will be started
        if(esp_timer_start_periodic(timer, TIMER_PERIOD) != ESP_OK) ESP_LOGE(TAG, "ERROR STARTING TIMER");
    }

    #ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL); //Creates a Thread(?) for handling TCP server
    #endif
    #ifdef CONFIG_EXAMPLE_IPV6
        xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
    #endif
}