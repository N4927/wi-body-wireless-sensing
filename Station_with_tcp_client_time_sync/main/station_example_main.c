
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_netif.h"
#include <string.h>

#define TAG                 "WIFI_SETUP"
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASS           "YOUR_WIFI_PASSWORD"
#define ESP_MAX_RETRY       50

#define GPIO_TEST_PIN       GPIO_NUM_18
#define GPIO_INPUT_PIN_SEL  (1ULL << GPIO_TEST_PIN)
#define ESP_INTR_FLAG_DEF   0

/*  Log TSF ogni secondo (facoltativo) */
#define TSF_DEBUG_INTERVAL_MS  1000

/*  Event group per connessione Wi‑Fi */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

/*  Queue visibile dal task TCP */
QueueHandle_t trigger_queue = NULL;

/*  Timer per leggere il TSF */
static esp_timer_handle_t tsf_timer = NULL;

/* ───────────── callback timer TSF ───────────── */
static void tsf_timer_cb(void *arg)
{
    uint64_t tsf = esp_wifi_get_tsf_time(WIFI_IF_STA);
    ESP_LOGI(TAG, "TSF = %llu µs", (unsigned long long)tsf);
}

/* ───────────── event handler Wi‑Fi/IP ───────────── */
static void event_handler(void *arg, esp_event_base_t base,
                          int32_t id, void *data)
{
    static int retry = 0;

    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry++ < ESP_MAX_RETRY) {
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        if (tsf_timer) esp_timer_stop(tsf_timer);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_CONNECTED) {
        retry = 0;
        if (!tsf_timer) {
            const esp_timer_create_args_t t_args = {
                .callback = tsf_timer_cb,
                .name     = "tsf_log"
            };
            ESP_ERROR_CHECK(esp_timer_create(&t_args, &tsf_timer));
        }
        ESP_ERROR_CHECK(
            esp_timer_start_periodic(tsf_timer,
                                     TSF_DEBUG_INTERVAL_MS * 1000));
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL, NULL));

    wifi_config_t sta_cfg = {
        .sta = {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASS,
            .pmf_cfg  = { .capable = true, .required = true },
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));


    xEventGroupWaitBits(s_wifi_event_group,
                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                        pdFALSE, pdFALSE, portMAX_DELAY);
}


static void IRAM_ATTR gpio_isr(void *arg)
{
    uint8_t tok = 1;
    xQueueSendFromISR(trigger_queue, &tok, NULL);
}

static void gpio_init_trigger(void)
{
    gpio_config_t io = {
        .intr_type    = GPIO_INTR_POSEDGE,
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = GPIO_INPUT_PIN_SEL,
        .pull_down_en = 1
    };
    gpio_config(&io);
    gpio_install_isr_service(ESP_INTR_FLAG_DEF);
    gpio_isr_handler_add(GPIO_TEST_PIN, gpio_isr, NULL);
}


void tcp_task(void *arg);


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_sta();         
    //trigger_queue = xQueueCreate(4, sizeof(uint8_t));
    //gpio_init_trigger();    
    xTaskCreate(tcp_task, "tcp_task", 4096, NULL, 10, NULL);
}
