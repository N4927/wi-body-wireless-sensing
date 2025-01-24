//Julian Caredda 06.12.2024
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
#include "esp_attr.h"                          //for IRAM, might not be needed
#include <sys/select.h>
#include <netinet/in.h>

//#include "esp_sntp.h"

#define TRUE                        1          // Boolean True value
#define TAG                         "AP"       // Tag for logging
#define WIFI_SSID                   "myssid"   // Wi-Fi SSID
#define WIFI_PASSWORD               "mypassword" // Wi-Fi password
#define ESP_WIFI_CHANNEL            1          // Wi-Fi channel
#define MAX_STA_CONN                8          // Max number of simultaneous station connections
#define PORT                        3333       // TCP connection port
#define BUFFER_SIZE                 2048       // RX buffer size in bytes
#define KEEPALIVE_IDLE              5          // Keep-alive idle time (in seconds)
#define KEEPALIVE_INTERVAL          5          // Keep-alive interval (in seconds)
#define KEEPALIVE_COUNT             3          // Keep-alive count
#define MICRO_S_CONVERSION          1000000    // Microseconds in one second
#define TIMER_PERIOD                (MICRO_S_CONVERSION * 10) // Timer period for callback (in microseconds)
#define LOG_OUTPUT                  1          // Log output control (1 for logs, 0 for performance)
#define DATA_STREAM_ENABLE          0
#define TIME_SYNCH_ENABLE           1          // Enable time synchronization

//GPIO Pin for testing TCP latency
#include "driver/gpio.h"
#define GPIO_ENABLE                 0
#define GPIO_OUTPUT_PIN_SEL         (1ULL<<18)  


extern void udp_time_master(void *pvParameters);

//static int sock;
//static int client_sockets[MAX_STA_CONN] = {0};
//fd_set read_fds;
//struct timeval timeout;


//Timer for Logging Throughput
//#define BENCHMARKING                1
esp_timer_handle_t TP_logging_timer;
static void throughput_logging_callback(void* arg);
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

//Timer for Network Synchronization
esp_timer_handle_t Net_synch_timer;
//static void Network_synching_callback(void* arg);


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
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = WIFI_PASSWORD,
            .max_connection = MAX_STA_CONN,
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
    if (strlen(WIFI_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(0); //TEMPORARY!!!

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASSWORD, ESP_WIFI_CHANNEL);
}


static void do_retransmit(void *pvParameters)
{
    const int sock = *(int *)pvParameters;
    ESP_LOGI(TAG, "Socket active: %d", sock);
    int len;
    char rx_buffer[BUFFER_SIZE];

    do {
        char rx_buffer[BUFFER_SIZE];
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
#if GPIO_ENABLE
            gpio_set_level(18, 1);
#endif
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes", len);
            //if (LOG_OUTPUT) ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            if (!LOG_OUTPUT) totalBytesSent += len;
            //vTaskDelay(20 / portTICK_PERIOD_MS);
#if GPIO_ENABLE
            gpio_set_level(18, 0);
#endif
            //vTaskDelay(10/portTICK_PERIOD_MS);
        }
    } while (len > 0);
    shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL);
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
    struct sockaddr_storage dest_addr; //File descriptor of connection(?)

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
    int buffer_size = 32768; // or larger
    setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));

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

        //Create new Thread for each new connection
        int *temp = &sock; //do I need this?
        xTaskCreate(do_retransmit, "TCP_Receiver", 4096, (int*)temp, 5, NULL);
    }

    CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

static void throughput_logging_callback(void* arg)
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
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
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

    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &throughput_logging_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };
    
    if(esp_timer_create(&periodic_timer_args, &TP_logging_timer) != ESP_OK){
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
        if(esp_timer_start_periodic(TP_logging_timer, TIMER_PERIOD) != ESP_OK) ESP_LOGE(TAG, "ERROR STARTING TIMER");
    }
    
    //TCP Data Servers
    #if DATA_STREAM_ENABLE
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL); //Creates a Thread(?) for handling TCP server
    #endif

    //ESP_LOGI(TAG, "BETWEEN UDP AND TCP");

    //UDP Time-synchronization Server
    //For some reason does creating this Thread not yield in parent-thread, 
    //therefore it should be created last
    #if TIME_SYNCH_ENABLE
    xTaskCreate(udp_time_master, "udp_server", 4096, (void*)AF_INET, 2, NULL); 
    #endif
}