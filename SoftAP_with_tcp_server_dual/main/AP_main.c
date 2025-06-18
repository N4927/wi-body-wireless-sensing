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
#include "esp_timer.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "esp_mac.h"
#include "driver/gpio.h"
#include <netinet/in.h>

#define TAG "AP"
#define WIFI_SSID "myssid"
#define WIFI_PASSWORD "mypassword"
#define ESP_WIFI_CHANNEL 1
#define MAX_STA_CONN 8
#define PORT 3333
#define BUFFER_SIZE 2048*2
#define AVG_PERIOD_US (10ULL * 1000000ULL)

typedef struct __attribute__((packed)){
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t ts_us;
} PacketHeader;

typedef struct{
    uint64_t bytes_total;
    uint64_t bytes_prev10;
} board_stats_t;

static board_stats_t b1 = {0}, b2 = {0};
static esp_timer_handle_t avg_timer;

#ifndef ntohll
static inline uint64_t ntohll(uint64_t v){ return __builtin_bswap64(v); }
#endif

static void wifi_event_handler(void *arg, esp_event_base_t base,int32_t id,void *data){
    ESP_LOGI(TAG,"WIFI EVENT id=%ld",id);
}

static void wifi_init_softap(void){
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,NULL);
    wifi_config_t wc = { .ap = {.ssid=WIFI_SSID,.ssid_len=strlen(WIFI_SSID),.channel=ESP_WIFI_CHANNEL,.password=WIFI_PASSWORD,.max_connection=MAX_STA_CONN,.authmode=WIFI_AUTH_WPA2_PSK,.pmf_cfg={.required=true}}};
    if(strlen(WIFI_PASSWORD)==0) wc.ap.authmode = WIFI_AUTH_OPEN;
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP,&wc);
    esp_wifi_start();
}

static void tcp_receive(void *arg){
    int sock = *(int*)arg;
    uint8_t buf[BUFFER_SIZE];
    size_t resid = 0;
    while(1){
        int n = recv(sock, buf+resid, sizeof(buf)-resid, 0);
        if(n<=0) break;
        n += resid;
        size_t off = 0;
        while(off + sizeof(PacketHeader) <= (size_t)n){
            PacketHeader *h = (PacketHeader*)(buf+off);
            uint16_t plen = ntohs(h->length);
            size_t pkt = sizeof(PacketHeader)+plen;
            if(off + pkt > (size_t)n) break;
            board_stats_t *st = (h->board_id==1) ? &b1 : (h->board_id==2) ? &b2 : NULL;
            if(st) st->bytes_total += pkt;
            off += pkt;
        }
        resid = n - off;
        if(resid) memmove(buf, buf+off, resid);
    }
    shutdown(sock,0);
    close(sock);
    vTaskDelete(NULL);
}

static void tcp_server_task(void *param){
    int af = (int)param;
    int ip_proto = (af==AF_INET)?IPPROTO_IP:IPPROTO_IPV6;
    struct sockaddr_storage addr = {0};
    if(af==AF_INET){
        struct sockaddr_in *a = (struct sockaddr_in*)&addr;
        a->sin_family = AF_INET;
        a->sin_addr.s_addr = htonl(INADDR_ANY);
        a->sin_port = htons(PORT);
    }else{
        struct sockaddr_in6 *a = (struct sockaddr_in6*)&addr;
        a->sin6_family = AF_INET6;
        bzero(&a->sin6_addr.un, sizeof(a->sin6_addr.un));
        a->sin6_port = htons(PORT);
    }
    int ls = socket(af, SOCK_STREAM, ip_proto);
    int buf_sz = 32768;  
    setsockopt(ls, SOL_SOCKET, SO_RCVBUF, &buf_sz, sizeof(buf_sz));
    setsockopt(ls, SOL_SOCKET, SO_SNDBUF, &buf_sz, sizeof(buf_sz));
    int flag = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    bind(ls, (struct sockaddr*)&addr, sizeof(addr));
    listen(ls, 4);
    while(1){
        struct sockaddr_storage src;
        socklen_t len = sizeof(src);
        int sock = accept(ls, (struct sockaddr*)&src, &len);
        if(sock>=0) xTaskCreate(tcp_receive, "tcp_rx", 4096*2, &sock, 5, NULL);
    }
}

static void avg_cb(void *arg){
    uint64_t diff1 = b1.bytes_total - b1.bytes_prev10;
    uint64_t diff2 = b2.bytes_total - b2.bytes_prev10;
    b1.bytes_prev10 = b1.bytes_total;
    b2.bytes_prev10 = b2.bytes_total;
    double tp1 = diff1 / 10.0;
    double tp2 = diff2 / 10.0;
    double avg = (tp1 + tp2) / 2.0;
    ESP_LOGI(TAG,"[AVG 10s] B1=%.0f B/s B2=%.0f B/s AVG=%.0f B/s",tp1,tp2,avg);
}

void app_main(void){
    if(nvs_flash_init()!=ESP_OK){ nvs_flash_erase(); nvs_flash_init(); }
    wifi_init_softap();
    esp_timer_create_args_t t = {.callback=&avg_cb,.name="avg"};
    esp_timer_create(&t, &avg_timer);
    esp_timer_start_periodic(avg_timer, AVG_PERIOD_US);
    xTaskCreate(tcp_server_task, "srv", 4096*2, (void*)AF_INET, 5, NULL);
}
