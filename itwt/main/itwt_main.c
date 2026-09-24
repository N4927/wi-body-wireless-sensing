/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* itwt Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   this example shows how to use itwt
   set a router or a AP using the same SSID&PASSWORD as configuration of this example.
   start esp32c6 and when it connected to AP it will setup itwt.
*/
#include <netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_console.h"
#include "cmd_system.h"
#include "esp_wifi_he.h"
#include "esp_pm.h"
#include "esp_timer.h"
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include "freertos/event_groups.h"  

#include "esp_rom_sys.h"   /* per ets_delay_us / esp_rom_delay_us */

#include "wifi_cmd.h"

/*******************************************************
 *                Constants
 *******************************************************/
 // apri il terminale con control shift + perchè ti serve esp v5.4.1
static const char *TAG = "itwt";
#define MAX_PAYLOAD_SIZE 1640
#define PORT            3333
/* Configure Wi-Fi credentials through menuconfig before running this example. */
static const char *host_ip = "192.168.4.1";
static const char *board_static_ip = "192.168.4.2";
#define BOARD_ID        1
static uint32_t sequence = 0; 
bool CHAR_SELECTED = 1;
#define TCP_STACK_SIZE  6144
#define TCP_PRIO  10
#define TCP_DELAY_MS  1000

#define LATENCY_HMI_MS       100   
#define LATENCY_SPORT_MS     1000  
#define LATENCY_BIO_MS       100000

static EventGroupHandle_t twt_evt = NULL;   // <<< NEW
#define TWT_WAKE_BIT  BIT0                 // <<< NEW

/*******************************************************
 *                Structures
 *******************************************************/

/*******************************************************
 *                Variable Definitions
 *******************************************************/
/*set the ssid and password via "idf.py menuconfig"*/
#define DEFAULT_SSID CONFIG_EXAMPLE_WIFI_SSID
#define DEFAULT_PWD CONFIG_EXAMPLE_WIFI_PASSWORD
#define ITWT_SETUP_SUCCESS 1

#if CONFIG_EXAMPLE_TWT_ENABLE_KEEP_ALIVE_QOS_NULL
bool keep_alive_enabled = true;
#else
bool keep_alive_enabled = true;
#endif

#if CONFIG_EXAMPLE_ITWT_TRIGGER_ENABLE
uint8_t trigger_enabled = 1;
#else
uint8_t trigger_enabled = 0;
#endif

#if CONFIG_EXAMPLE_ITWT_ANNOUNCED
uint8_t flow_type_announced = 1;
#else
uint8_t flow_type_announced = 0;
#endif

esp_netif_t *netif_sta = NULL;
const int CONNECTED_BIT = BIT0;
const int DISCONNECTED_BIT = BIT1;
EventGroupHandle_t wifi_event_group;



typedef struct PacketHeader {
    uint8_t board_id;
    uint32_t sequence;
    uint16_t length;
    uint16_t checksum;
} PacketHeader;

char* encode_str(const char* payload, uint8_t board_id, uint32_t* sequence) {
    size_t msg_len = strlen(payload);
    uint16_t checksum = 0;

    for (size_t i = 0; i < msg_len; i++) {
        checksum += (uint8_t)payload[i];
    }

    char* packet = (char*)malloc(sizeof(PacketHeader) + msg_len);
    if (!packet) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return NULL;
    }

    PacketHeader* header = (PacketHeader*)packet;
    header->board_id = board_id;
    header->sequence = htonl((*sequence)++);
    header->length = htons(msg_len);
    header->checksum = htons(checksum);

    memcpy(packet + sizeof(PacketHeader), payload, msg_len);
    return packet;
}

uint8_t* encode_arr(const uint8_t* data, size_t data_len, uint32_t* sequence) {
    if (data == NULL || data_len > MAX_PAYLOAD_SIZE) {
        ESP_LOGE(TAG, "Invalid encode_arr parameters");
        return NULL;
    }
    size_t total_len = sizeof(PacketHeader) + data_len;
    uint8_t* packet = (uint8_t*)malloc(sizeof(PacketHeader) + data_len);
    if (!packet) { 
        ESP_LOGE(TAG, "Failed to allocate memory for packet");
        return NULL;
    }

    uint16_t checksum = 0;

    for (size_t i = 0; i < data_len; i++) {
        checksum += data[i];
    }

    PacketHeader* header = (PacketHeader*)packet;
    header->board_id = BOARD_ID;
    header->sequence = htonl((*sequence)++);
    header->length = htons(data_len);
    header->checksum = htons(checksum);

    memcpy(packet + sizeof(PacketHeader), data, data_len);
    return packet;
}

volatile uint32_t sp_us   = 100000;   
volatile uint32_t awake_us = 62800;  
static const char *payload_str = "1819202122232425262728929012345678910112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213141516171819202122232425262728290123456789101112131415161718192021222324252627282901234567891011121314164684684645168721135468784354151617181920212223242526272829012345678910111213141516171819202122232425262728297181920212223242526272829012345678910111213141516171819202122232425262728211213141516171819202122232425218192021222324252627282901234567891011121314151617181920212223242526272829012345678910111213138607200";


/* -------------------------------------------------------------------- */
/*  Task TCP client                                                     */
/* -------------------------------------------------------------------- */
/* ---------------- CONFIG LOCALE ----------------------------- */
#define TCP_RETRY_DELAY_MS   1000        // ritenta il socket dopo 1 s
#define PHY_RATE_BPS         10e6        // stima prudente: 10 Mb/s effettivi
#define WAIT_TIMEOUT_MS      100         // max attesa del TWT_WAKE_BIT (ms)


/*1.5 255
1.3  195
1.1  117
0.9  78
0.7  39	
*/



#define INCREASE  0.85
#define wake_value  1



static void tcp_client_task(void *arg)
{
    

    const TickType_t retry_delay = pdMS_TO_TICKS(TCP_RETRY_DELAY_MS);

    const struct sockaddr_in peer = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT),
        .sin_addr.s_addr = inet_addr(host_ip)
    };

    while (true) {


        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            pdFALSE, pdTRUE, portMAX_DELAY);


                            int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket() errno %d", errno);
            vTaskDelay(retry_delay);
            continue;
        }

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        int one = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

        if (connect(sock, (struct sockaddr *)&peer, sizeof(peer)) < 0 &&
            errno != EINPROGRESS) {
            ESP_LOGE(TAG, "connect() errno %d", errno);
            close(sock);
            vTaskDelay(retry_delay);
            continue;
        }

        while (xEventGroupGetBits(wifi_event_group) & CONNECTED_BIT) {

            EventBits_t res = xEventGroupWaitBits(
                                  twt_evt,
                                  TWT_WAKE_BIT,
                                  pdTRUE,        
                                  pdTRUE,        
                                  pdMS_TO_TICKS(WAIT_TIMEOUT_MS));

            if (!(res & TWT_WAKE_BIT)) {

                const char ka = 0;
                send(sock, &ka, 1, 0);            
                continue;                         
            }

            int64_t start_us = esp_timer_get_time();
            int64_t deadline = start_us + awake_us*INCREASE;


            while (esp_timer_get_time() < deadline) {

                ssize_t wr = send(sock, payload_str, strlen(payload_str), 0);

                if (wr > 0) {

                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                          
                  vTaskDelay(1);
                  continue;                     
                } else {
                    ESP_LOGE(TAG, "send() errno %d", errno);
                    goto restart_socket;              
                }
            }

            if (esp_timer_get_time() < deadline) {
                vTaskDelay(1);                        
            }
        }

restart_socket:
        shutdown(sock, SHUT_RDWR);
        close(sock);
        ESP_LOGW(TAG, "TCP closed, retry in %d ms", TCP_RETRY_DELAY_MS);
        vTaskDelay(retry_delay);
    }
}



void tcp_client_start(void)
{
    xTaskCreate(tcp_client_task, "tcp_cli",
                            TCP_STACK_SIZE, NULL, TCP_PRIO, NULL);
}


/*******************************************************
 *                Function Declarations
 *******************************************************/

/*******************************************************
 *                Function Definitions
 *******************************************************/

static void example_set_static_ip(esp_netif_t *netif)
{
#if CONFIG_EXAMPLE_ENABLE_STATIC_IP
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(CONFIG_EXAMPLE_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(CONFIG_EXAMPLE_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(CONFIG_EXAMPLE_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGI(TAG, "Success to set static ip: %s, netmask: %s, gw: %s",
             CONFIG_EXAMPLE_STATIC_IP_ADDR, CONFIG_EXAMPLE_STATIC_NETMASK_ADDR, CONFIG_EXAMPLE_STATIC_GW_ADDR);
#endif
}

static const char *itwt_probe_status_to_str(wifi_itwt_probe_status_t status)
{
    switch (status) {
    case ITWT_PROBE_FAIL:                 return "itwt probe fail";
    case ITWT_PROBE_SUCCESS:              return "itwt probe success";
    case ITWT_PROBE_TIMEOUT:              return "itwt probe timeout";
    case ITWT_PROBE_STA_DISCONNECTED:     return "Sta disconnected";
    default:                              return "Unknown status";
    }
}

static void got_ip_handler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
    xEventGroupClearBits(wifi_event_group, DISCONNECTED_BIT);
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    if (!twt_evt) {                         // <<< NEW
        twt_evt = xEventGroupCreate();      //    crea una sola volta
    }
    static bool tcp_started = false;
    if(!tcp_started){
        tcp_client_start();
        tcp_started = true;
    }

    /* setup a trigger-based announce individual TWT agreement. */
    wifi_phy_mode_t phymode;
    wifi_config_t sta_cfg = { 0, };
    esp_wifi_get_config(WIFI_IF_STA, &sta_cfg);
    esp_wifi_sta_get_negotiated_phymode(&phymode);
    if (phymode == WIFI_PHY_MODE_HE20) {
        esp_err_t err = ESP_OK;
        wifi_itwt_setup_config_t setup_config = {
            .setup_cmd         = TWT_REQUEST,
            .flow_id           = 0,
            .twt_id            = CONFIG_EXAMPLE_ITWT_ID,
        
            .flow_type         = 0,       
            .trigger           = 1,       
        
            .min_wake_dura     = wake_value,
            .wake_duration_unit= 0,
            .wake_invl_mant    = 98, //98 -> 100ms  238 -> 250ms
            .wake_invl_expn    = 10,
        
            .timeout_time_ms   = CONFIG_EXAMPLE_ITWT_SETUP_TIMEOUT_TIME_MS,
        };
        
        
        err = esp_wifi_sta_itwt_setup(&setup_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "itwt setup failed, err:0x%x", err);
        }
    } else {
        ESP_LOGE(TAG, "Must be in 11ax mode to support itwt");
    }
}

static void start_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "sta connect to %s", DEFAULT_SSID);
    esp_wifi_connect();
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "sta disconnect, reconnect...");
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    esp_wifi_connect();
}

static void itwt_wakeup_handler(void *arg, esp_event_base_t base,
    int32_t id, void *data)
{
    if (twt_evt) {
    xEventGroupSetBits(twt_evt, TWT_WAKE_BIT);
    }
}

static void itwt_setup_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_setup_t *setup = (wifi_event_sta_itwt_setup_t *) event_data;
    if (setup->status == ITWT_SETUP_SUCCESS) {
        /* TWT Wake Interval = TWT Wake Interval Mantissa * (2 ^ TWT Wake Interval Exponent) */
        ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, flow_id:%d, %s, %s, wake_dura:%d, wake_dura_unit:%d, wake_invl_e:%d, wake_invl_m:%d", setup->config.twt_id,
                setup->config.flow_id, setup->config.trigger ? "trigger-enabled" : "non-trigger-enabled", setup->config.flow_type ? "unannounced" : "announced",
                setup->config.min_wake_dura, setup->config.wake_duration_unit, setup->config.wake_invl_expn, setup->config.wake_invl_mant);
        ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SETUP>target wake time:%lld, wake duration:%d us, service period:%d us", setup->target_wake_time, setup->config.min_wake_dura << (setup->config.wake_duration_unit == 1 ? 10 : 8),
                setup->config.wake_invl_mant << setup->config.wake_invl_expn);
                /* calcola le durate */                                  // <<< NEW
        sp_us  = (setup->config.wake_invl_mant << setup->config.wake_invl_expn) * 1024U;
        awake_us = setup->config.min_wake_dura << (setup->config.wake_duration_unit ? 10 : 8);

    } else {
        if (setup->status == ESP_ERR_WIFI_TWT_SETUP_TIMEOUT) {
            ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, timeout of receiving twt setup response frame", setup->config.twt_id);
        } else if (setup->status == ESP_ERR_WIFI_TWT_SETUP_TXFAIL) {
            ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, twt setup frame tx failed, reason: %d", setup->config.twt_id, setup->reason);
        } else if (setup->status == ESP_ERR_WIFI_TWT_SETUP_REJECT) {
            ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SETUP>target wake time:%lld, wake duration:%d us, service period:%d us", setup->target_wake_time, setup->config.min_wake_dura << (setup->config.wake_duration_unit == 1 ? 10 : 8),
                setup->config.wake_invl_mant << setup->config.wake_invl_expn);
            ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, twt setup request was rejected, setup cmd: %d", setup->config.twt_id, setup->config.setup_cmd);
        } else {
            ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, twt setup failed, status: %d", setup->config.twt_id, setup->status);
        }
    }
}

static void itwt_teardown_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_teardown_t *teardown = (wifi_event_sta_itwt_teardown_t *) event_data;
    if (teardown->status == ITWT_TEARDOWN_FAIL) {
        ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_TEARDOWN>flow_id %d%s, twt teardown frame tx failed", teardown->flow_id, (teardown->flow_id == 8) ? "(all twt)" : "");
    } else {
        ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_TEARDOWN>flow_id %d%s", teardown->flow_id, (teardown->flow_id == 8) ? "(all twt)" : "");
    }
}

static void itwt_suspend_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_suspend_t *suspend = (wifi_event_sta_itwt_suspend_t *) event_data;
    ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SUSPEND>status:%d, flow_id_bitmap:0x%x, actual_suspend_time_ms:[%lu %lu %lu %lu %lu %lu %lu %lu]",
             suspend->status, suspend->flow_id_bitmap,
             suspend->actual_suspend_time_ms[0], suspend->actual_suspend_time_ms[1], suspend->actual_suspend_time_ms[2], suspend->actual_suspend_time_ms[3],
             suspend->actual_suspend_time_ms[4], suspend->actual_suspend_time_ms[5], suspend->actual_suspend_time_ms[6], suspend->actual_suspend_time_ms[7]);
}

static void itwt_probe_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data)
{
/* 1. fai il cast subito  */
    wifi_event_sta_itwt_probe_t *probe =
        (wifi_event_sta_itwt_probe_t *) event_data;

/* 2. ora puoi usarlo senza errori */
    if (probe->status == ITWT_PROBE_SUCCESS && twt_evt) {
    xEventGroupSetBits(twt_evt, TWT_WAKE_BIT);
    }

/* 3. log di debug */
    ESP_LOGI(TAG,
    "<WIFI_EVENT_ITWT_PROBE>status:%s, reason:0x%x",
    itwt_probe_status_to_str(probe->status),
    probe->reason);
}



static void wifi_itwt(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    netif_sta = esp_netif_create_default_wifi_sta();
    assert(netif_sta);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_TWT_WAKEUP,      //  <‑‑ questo!
        &itwt_wakeup_handler,
        NULL,
        NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_STA_START,
                    &start_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_STA_DISCONNECTED,
                    &disconnect_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &got_ip_handler,
                    NULL,
                    NULL));
    /* itwt */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_ITWT_SETUP,
                    &itwt_setup_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_ITWT_TEARDOWN,
                    &itwt_teardown_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_ITWT_SUSPEND,
                    &itwt_suspend_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    WIFI_EVENT_ITWT_PROBE,
                    &itwt_probe_handler,
                    NULL,
                    NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    wifi_twt_config_t wifi_twt_config = {
        .post_wakeup_event = true,
        .twt_enable_keep_alive = true,
    };
    ESP_ERROR_CHECK(esp_wifi_sta_twt_config(&wifi_twt_config));

    //esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

#if CONFIG_EXAMPLE_ENABLE_STATIC_IP
    example_set_static_ip(netif_sta);
#endif

    ESP_ERROR_CHECK(esp_wifi_start());

#if CONFIG_ESP_WIFI_ENABLE_WIFI_RX_STATS
#if CONFIG_ESP_WIFI_ENABLE_WIFI_RX_MU_STATS
    esp_wifi_enable_rx_statistics(true, true);
#else
    esp_wifi_enable_rx_statistics(true, false);
#endif
#endif
#if CONFIG_ESP_WIFI_ENABLE_WIFI_TX_STATS
    esp_wifi_enable_tx_statistics(ESP_WIFI_ACI_VO, true); //VO, mgmt
    esp_wifi_enable_tx_statistics(ESP_WIFI_ACI_BE, true); //BE, data
#endif

}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 40,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
    ESP_ERROR_CHECK( ret );

#else
    printf("\n =================================================\n");
    printf(" |                   Test WiFi 6 itwt             |\n");
    printf(" =================================================\n\n");

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "itwt>";

    // init console REPL environment
#if CONFIG_ESP_CONSOLE_UART
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
#elif CONFIG_ESP_CONSOLE_USB_CDC
    esp_console_dev_usb_cdc_config_t cdc_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&cdc_config, &repl_config, &repl));
#elif CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl));
#endif

    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
#endif

    //start wifi
    wifi_itwt();
    // register commands
    register_system();
    register_wifi_itwt();
    register_wifi_stats();

}
