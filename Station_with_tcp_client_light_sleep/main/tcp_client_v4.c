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
 #include "esp_sleep.h"
 
 #define TAG           "TCP_CLIENT"
 #define SERVER_IP     "192.168.4.1"//"192.168.0.170"//"192.168.50.147"//"192.168.4.1"
 #define SERVER_PORT   3333
 #define BOARD_ID       1
 #define PAYLOAD_SIZE   1440
 

 #define LATENCY_HMI_MS       100   
 #define LATENCY_250          250
 #define LATENCY_SPORT_MS     1000  
 #define LATENCY_BIO_MS       100000
 #define ACTIVE_PERCENTAGE    	100
 #define DUTY_CYCLE_US   (LATENCY_HMI_MS * 1000)     
 
 
 static uint32_t ACTIVE_TIME_US = (uint32_t)(((uint64_t)ACTIVE_PERCENTAGE * LATENCY_HMI_MS * 1000) / 100);

 
 
 static const char *payload = "1819202122232425262728929012345678910112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314164684684645168721135468784354151617181920212223242526272829012345678910111213141516171819202122232425262728297181920212223242526272829012345678910111213141516171819202122232425262728211213141516171819202122232425218192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213138607200";

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

static inline void build_packet(uint8_t *dst, const uint8_t *payload)
{
    /* copy the immutable template */
    memcpy(dst, pkt_hdr, sizeof(pkt_hdr));

    /* sequence */
    uint32_t s = htonl(seq++);
    memcpy(dst + 1, &s, 4);

    uint64_t tsf = htonll(esp_timer_get_time());
    memcpy(dst + 7, &tsf, 8);

    memcpy(dst + sizeof(PacketHeader), payload, PAYLOAD_SIZE);
}

void tcp_client(void *arg)
{
    static uint8_t payload_one [PAYLOAD_SIZE];
    static uint8_t packet[sizeof(PacketHeader) + PAYLOAD_SIZE];

    memset(payload_one , 0xA1, PAYLOAD_SIZE);

    ESP_LOGI(TAG, "TCP client task started");
 
     struct sockaddr_in dest_addr = {
         .sin_family      = AF_INET,
         .sin_port        = htons(SERVER_PORT),
         .sin_addr.s_addr = inet_addr(SERVER_IP),
     };
 
     while (1)
     {
         int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
         if (sock < 0) {
             ESP_LOGE(TAG, "socket() failed: %d", errno);
             vTaskDelay(pdMS_TO_TICKS(2000));
             continue;
         }
         if (connect(sock, (struct sockaddr *)&dest_addr,
                     sizeof(dest_addr)) != 0) {
             ESP_LOGE(TAG, "connect() failed: %d", errno);
             close(sock);
             vTaskDelay(pdMS_TO_TICKS(2000));
             continue;
         }
         ESP_LOGI(TAG, "Connected to %s:%d", SERVER_IP, SERVER_PORT);
 
         while (1)
         {
             const int64_t cycle_start = esp_timer_get_time();
 
             //ACTIVE 
             esp_wifi_set_ps(WIFI_PS_NONE);              
             while (esp_timer_get_time() - cycle_start < ACTIVE_TIME_US) {
                build_packet(packet, payload_one);
                //size_t wr = send(sock, packet, sizeof(packet), 0);
                ssize_t wr = send(sock, payload, strlen(payload), 0);

                 if (wr < 0) { 
                    ESP_LOGE(TAG, "send() err: %d", errno); 
                    goto sock_err; 
                }
             }
 
             esp_wifi_set_ps(WIFI_PS_MIN_MODEM); 
             const uint32_t idle_us = DUTY_CYCLE_US - ACTIVE_TIME_US;
             if (idle_us > 0) {
 
                 vTaskDelay(pdMS_TO_TICKS(idle_us / 1000));   
             }
         }
 
     sock_err:
         shutdown(sock, 0);
         close(sock);
         ESP_LOGI(TAG, "Socket closed, retrying in 2 s");
         vTaskDelay(pdMS_TO_TICKS(2000));
     }
 }
 