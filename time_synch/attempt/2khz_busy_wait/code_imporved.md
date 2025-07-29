```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "driver/gpio.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 
#include <rom/ets_sys.h>


extern QueueHandle_t trigger_queue;
//after this code test also the synch with the ap
// the two board with the ap

#define TAG             "TCP_TASK"
#define HOST_IP         "192.168.4.1"
#define HOST_PORT       3333
#define BOARD_ID        2

#define SAMPLE_BYTES    3
#define NUM_CHANNELS    32
#define PAYLOAD_SIZE    (SAMPLE_BYTES * NUM_CHANNELS)  // 96 bytes

#define NOTIF_TRIG   (1 << 0)  
#define TRIG_GPIO    18

static TaskHandle_t tcp_task_hdl;

volatile uint64_t last_trigger_tsf = 0;


static void IRAM_ATTR gpio_isr(void *arg)
{

    last_trigger_tsf = esp_wifi_get_tsf_time(WIFI_IF_STA);

    xTaskNotifyFromISR(tcp_task_hdl, 0, eIncrement, NULL);
}

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

uint64_t trig_time = 0;

static uint8_t pkt_hdr[sizeof(PacketHeader)] = {
    BOARD_ID,               // board_id
    0,0,0,0,                // sequence (filled later)
    (PAYLOAD_SIZE >> 8),    // length (network order)
    (PAYLOAD_SIZE & 0xFF),
    0,0,0,0,0,0,0,0         // tsf_us (filled later)
};

static inline void build_packet(uint8_t *dst, const uint8_t *payload, uint64_t timestamp)
{
    /* copy the immutable template */
    memcpy(dst, pkt_hdr, sizeof(pkt_hdr));

    /* sequence */
    uint32_t s = htonl(seq++);
    memcpy(dst + 1, &s, 4);

    /* tsf */
    uint64_t tsf = htonll(timestamp);
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

    tcp_task_hdl = xTaskGetCurrentTaskHandle();

    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(TRIG_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,   /* pulsante verso GND   */
        .intr_type    = GPIO_INTR_POSEDGE
    };
    gpio_config(&cfg);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(TRIG_GPIO, gpio_isr, NULL);

    for (;;)
    {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) { 
            vTaskDelay(pdMS_TO_TICKS(1000));
            ESP_LOGE(TAG, "quiiiiiiiiiiiiiiiiii");
            continue; 
        }

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
        while (1) {
            if (ulTaskNotifyTake(pdTRUE, 0)) {
                send_full = true;          
                trig_time = last_trigger_tsf;
                ESP_LOGI(TAG, "triggeredddddddddddddddddd");
            }
            else{
                trig_time = esp_wifi_get_tsf_time(WIFI_IF_STA);
            }

            const uint8_t *pl = send_full ? payload_one : payload_zero;
            build_packet(packet, pl,trig_time);
            send_full = false;   

            int sent = send(sock, packet, sizeof(packet), 0);
            ESP_LOGE(TAG, "packet packettttttttttt %d packet", packet[20]);
            //taskYIELD();
            if (sent < 0) {
                ESP_LOGE(TAG, "send errno %d reconnecting", errno);
                break;          
            }

            esp_rom_delay_us(500);

        }

        shutdown(sock, 0);
        close(sock);
        vTaskDelay(pdMS_TO_TICKS(1000));   
    }
}
```