```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 

extern QueueHandle_t trigger_queue;

#define TAG             "TCP_TASK"
#define HOST_IP         "192.168.4.1"
#define HOST_PORT       3333
#define BOARD_ID        2

#define SAMPLE_BYTES    3
#define NUM_CHANNELS    32
#define PAYLOAD_SIZE    (SAMPLE_BYTES * NUM_CHANNELS)  // 96 bytes

#pragma pack(push,1)
typedef struct {
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t tsf_us;
} PacketHeader;
#pragma pack(pop)

static uint32_t seq = 0;
#define htonll(x) __builtin_bswap64(x)

static uint8_t pkt_hdr[sizeof(PacketHeader)] = {
    BOARD_ID,               // board_id
    0,0,0,0,                // sequence (filled later)
    (PAYLOAD_SIZE >> 8),    // length (network order)
    (PAYLOAD_SIZE & 0xFF),
    0,0,0,0,0,0,0,0         // tsf_us (filled later)
};

static inline void build_packet(uint8_t *dst, const uint8_t *payload)
{
    /* copy the immutable template */
    memcpy(dst, pkt_hdr, sizeof(pkt_hdr));

    /* sequence */
    uint32_t s = htonl(seq++);
    memcpy(dst + 1, &s, 4);

    /* tsf */
    uint64_t tsf = htonll(esp_wifi_get_tsf_time(WIFI_IF_STA));
    memcpy(dst + 7, &tsf, 8);

    memcpy(dst + sizeof(PacketHeader), payload, PAYLOAD_SIZE);
}

void tcp_task(void *arg)
{
    static uint8_t payload_one [PAYLOAD_SIZE];
    static uint8_t payload_zero[PAYLOAD_SIZE];
    static uint8_t packet[sizeof(PacketHeader) + PAYLOAD_SIZE];

    memset(payload_one , 0x01, PAYLOAD_SIZE);
    memset(payload_zero, 0x00, PAYLOAD_SIZE);

    for (;;)
    {
        
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) { vTaskDelay(pdMS_TO_TICKS(1000)); continue; }

        /* disable Nagle for low‑latency */
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int));

        struct sockaddr_in dest = {
            .sin_family      = AF_INET,
            .sin_port        = htons(HOST_PORT),
            .sin_addr.s_addr = inet_addr(HOST_IP)
        };
        if (connect(sock, (void *)&dest, sizeof(dest)) < 0) {
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ESP_LOGI(TAG, "Connected to %s:%d", HOST_IP, HOST_PORT);

        
        bool send_full = false;
        TickType_t wait = pdMS_TO_TICKS(50);    
        while (1) {
            uint8_t tok;
            if (xQueueReceive(trigger_queue, &tok, wait) == pdTRUE) {
                send_full = true;
            }

            const uint8_t *pl = send_full ? payload_one : payload_zero;
            build_packet(packet, pl);
            send_full = false;   

            int sent = send(sock, packet, sizeof(packet), 0);
            if (sent < 0) {
                ESP_LOGE(TAG, "send errno %d – reconnecting", errno);
                break;          
            }
        }

        
        shutdown(sock, 0);
        close(sock);
        vTaskDelay(pdMS_TO_TICKS(1000));   
    }
}



taske main :

trigger_queue = xQueueCreate(4, sizeof(uint8_t));
gpio_init_trigger();

xTaskCreate(tcp_task, "tcp_task", 4096, NULL, 5, NULL);
```