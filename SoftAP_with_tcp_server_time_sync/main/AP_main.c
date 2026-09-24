#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <netinet/in.h>
#include <inttypes.h>

#define TAG                 "AP"
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"
#define ESP_WIFI_CHANNEL    1
#define MAX_STA_CONN        8
#define PORT                3333

#define RX_BUF_SIZE         2048
#define MICRO_S_CONVERSION  1000000ULL
#define TIMER_PERIOD_US     (MICRO_S_CONVERSION * 10)

#define TRIG_GPIO           18

static TaskHandle_t tcp_task_hdl;

static uint64_t totalBytes = 0, totalBytesOld = 0, tPrev = 0;
static uint64_t last_tsf_board1 = 0;
static uint64_t last_tsf_board2 = 0;
static uint64_t counter1 = 0;
static uint64_t counter2 = 0;
static volatile uint64_t last_tsf_ap = 0;


typedef struct __attribute__((packed)) {
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t synced_timestamp_us;
} PacketHeader;

#ifndef ntohll
static inline uint64_t ntohll(uint64_t v) { return __builtin_bswap64(v); }
#endif

static void IRAM_ATTR gpio_isr(void *arg)
{
    last_tsf_ap = esp_wifi_get_tsf_time(WIFI_IF_AP);
    //xTaskNotifyFromISR(tcp_task_hdl, 0, eIncrement, NULL);
}

static void gpio_trigger_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(TRIG_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&cfg);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(TRIG_GPIO, gpio_isr, NULL);
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *e = (wifi_event_ap_staconnected_t *)data;
        ESP_LOGI(TAG, "STA %02x:%02x:%02x:%02x:%02x:%02x joined, AID=%d",
                 e->mac[0],e->mac[1],e->mac[2],e->mac[3],e->mac[4],e->mac[5], e->aid);
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *e = (wifi_event_ap_stadisconnected_t *)data;
        ESP_LOGI(TAG, "STA %02x:%02x:%02x:%02x:%02x:%02x left, AID=%d, reason=%d",
                 e->mac[0],e->mac[1],e->mac[2],e->mac[3],e->mac[4],e->mac[5], e->aid, e->reason);
    }
}

static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        wifi_event_handler, NULL, NULL));

    wifi_config_t ap = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASSWORD,
            .channel = ESP_WIFI_CHANNEL,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.required = true},
            .beacon_interval = 60000,
        }
    };
    if (strlen(WIFI_PASSWORD) == 0) ap.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_LOGI(TAG, "SoftAP started: SSID=%s, chan=%d", WIFI_SSID, ESP_WIFI_CHANNEL);
}

static void tcp_receiver(void *pv)
{
    int sock = (int)(intptr_t)pv;
    uint8_t rxbuf[RX_BUF_SIZE];
    size_t buffered = 0;

    tcp_task_hdl = xTaskGetCurrentTaskHandle();

    while (1) {
        //if (ulTaskNotifyTake(pdTRUE, 0)) {
            //ESP_LOGI(TAG, "GPIO Triggered — AP timestamp = %llu", (unsigned long long)last_tsf_ap);
        //}

        int n = recv(sock, rxbuf + buffered, sizeof(rxbuf) - buffered, 0);
        if (n <= 0) { ESP_LOGW(TAG, "client closed/err"); break; }

        buffered += (size_t)n;
        size_t off = 0;

        while (buffered - off >= sizeof(PacketHeader)) {
            PacketHeader *h = (PacketHeader *)(rxbuf + off);
            uint16_t pay_len = ntohs(h->length);
            size_t pkt_len = sizeof(PacketHeader) + pay_len;

            if (buffered - off < pkt_len) break;

            uint8_t *pl = rxbuf + off + sizeof(PacketHeader);
            bool isAll1 = (pl[20] == 0x01);

            if (isAll1) {
                uint64_t tsf = ntohll(h->synced_timestamp_us);
                //ESP_LOGI(TAG, "=== TRIGGER board %u — TSF %llu µs ===", h->board_id, (unsigned long long)tsf);
                //ESP_LOGI(TAG, "=== TRIGGER AP — TSF %llu µs ===", last_tsf_ap);


                if (h->board_id == 1) {
                    last_tsf_board1 = tsf;
                }
                else if (h->board_id == 2) last_tsf_board2 = tsf;

                if (last_tsf_board1 || last_tsf_board2) {
                    printf("%llu,%llu,%llu\n",
                        (unsigned long long)last_tsf_board1,
                        (unsigned long long)last_tsf_board2,
                        (unsigned long long)last_tsf_ap);

                    last_tsf_board1 = 0;
                    last_tsf_board2 = 0;
                }
            }

            if (h->board_id == 1) {
                ESP_LOGI(TAG, "counter 1: %" PRIu32, ntohl(h->sequence));
                    
                }
                else if (h->board_id == 2){
                     ESP_LOGI(TAG, "counter 2: %" PRIu32, ntohl(h->sequence));
                }

            

            totalBytes += pkt_len;
            off += pkt_len;
        }

        if (off) {
            memmove(rxbuf, rxbuf + off, buffered - off);
            buffered -= off;
        }
    }

    shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL);
}

static void tcp_server(void *pv)
{
    int addr_family = (int)pv;
    int ip_proto = (addr_family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;

    struct sockaddr_storage addr = {0};
    if (addr_family == AF_INET) {
        struct sockaddr_in *a = (struct sockaddr_in *)&addr;
        a->sin_family = AF_INET;
        a->sin_port   = htons(PORT);
        a->sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)&addr;
        a6->sin6_family = AF_INET6;
        a6->sin6_port   = htons(PORT);
        bzero(&a6->sin6_addr.un, sizeof(a6->sin6_addr.un));
    }

    int lsock = socket(addr_family, SOCK_STREAM, ip_proto);
    if (lsock < 0) { ESP_LOGE(TAG, "socket errno %d", errno); vTaskDelete(NULL); }

    int buf = 32768;
    setsockopt(lsock, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));
    setsockopt(lsock, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(buf));
    int flag = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    ESP_ERROR_CHECK(bind(lsock, (struct sockaddr *)&addr, sizeof(addr)));
    ESP_ERROR_CHECK(listen(lsock, MAX_STA_CONN));
    ESP_LOGI(TAG, "TCP server listening on port %d", PORT);

    struct sockaddr_storage src;
    socklen_t slen = sizeof(src);
    while (1) {
        int csock = accept(lsock, (struct sockaddr *)&src, &slen);
        if (csock < 0) { ESP_LOGE(TAG, "accept errno %d", errno); continue; }

        xTaskCreate(tcp_receiver, "tcp_rx", 4096, (void *)(intptr_t)csock, 5, NULL);
    }
}

static void tp_cb(void *arg)
{
    uint64_t now = esp_timer_get_time();
    uint64_t diff = now - tPrev;
    uint64_t bytes = totalBytes - totalBytesOld;
    tPrev = now; totalBytesOld = totalBytes;

    double tp = ((double)bytes / diff) * MICRO_S_CONVERSION;
    // ESP_LOGI(TAG, "Throughput %.1f kB/s", tp / 1024.0);
}

void app_main(void)
{
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    wifi_init_softap();
    gpio_trigger_init();
    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MCS7_SGI);



    xTaskCreate(tcp_server, "tcp_server", 4096 * 2, (void *)AF_INET, 5, NULL);
}
