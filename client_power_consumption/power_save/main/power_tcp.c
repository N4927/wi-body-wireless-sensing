/*
   Esempio di ESP32 "client" che:
   1. Si collega a un AP con SSID/PW
   2. Apre una connessione TCP a 192.168.4.1:3333
   3. Invia un messaggio di test e riceve risposta
*/

#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/uart.h"

// Configura qui l'SSID e la Password del tuo AP
#define EXAMPLE_ESP_WIFI_SSID      "myssid"
#define EXAMPLE_ESP_WIFI_PASS      "mypassword"

// IP e Porta del server TCP (il tuo ESP32 in modalità SoftAP)
#define HOST_IP_ADDR              "192.168.4.1"
#define HOST_PORT                 3333

static const char *TAG = "CLIENT_EXAMPLE";

// Event group per la gestione della connessione Wi-Fi
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

// Handler eventi Wi-Fi
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Avvia la connessione Wi-Fi
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Se disconnesso, ritenta
        ESP_LOGW(TAG, "Wi-Fi disconnesso, ritento...");
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Quando ottiene IP, setta il bit di "connessione riuscita"
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Ottenuto IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Inizializza Wi-Fi in STA mode
static void wifi_init_sta(void)
{
    // Inizializza lo stack di rete
    ESP_ERROR_CHECK(esp_netif_init());
    // Crea loop di eventi di default
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Crea interfaccia Wi-Fi station
    esp_netif_create_default_wifi_sta();

    // Config Wi-Fi di base
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Registra gli handler degli eventi Wi-Fi
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));

    // Configurazione credenziali
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            // Se la rete non trasmette SSID, usa .scan_method = WIFI_ALL_CHANNEL_SCAN, ...
        },
    };

    // Imposta modalità STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // Applica config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // Avvia Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// Task che gestisce la connessione TCP al server
static void tcp_client_task(void *pvParameters)
{
    // Buffer da inviare
    const char *payload = "Hello from ESP32-C6 client!\n";

    while (1) {
        // Aspetta finché non siamo connessi al Wi-Fi
        xEventGroupWaitBits(s_wifi_event_group,
                            WIFI_CONNECTED_BIT,
                            pdFALSE,
                            pdTRUE,
                            portMAX_DELAY);

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(HOST_PORT);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            ESP_LOGE(TAG, "Impossibile creare socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        ESP_LOGI(TAG, "Socket creato, connessione a %s:%d...",
                 HOST_IP_ADDR, HOST_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Connessione fallita: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        ESP_LOGI(TAG, "Connesso con successo al server!");

        // Invia il payload
        int sent = send(sock, payload, strlen(payload), 0);
        if (sent < 0) {
            ESP_LOGE(TAG, "Errore durante l'invio: errno %d", errno);
        } else {
            ESP_LOGI(TAG, "Inviato payload di %d byte al server", sent);
        }

        // Ricevi eventuale risposta
        char rx_buffer[128];
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Errore in recv(): errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Server ha chiuso la connessione.");
        } else {
            rx_buffer[len] = 0; // Terminatore di stringa
            ESP_LOGI(TAG, "Ricevuti %d byte dal server: %s", len, rx_buffer);
        }

        // Chiude il socket
        ESP_LOGI(TAG, "Chiudo socket e riprovo fra 5s...");
        shutdown(sock, 0);
        close(sock);

        // Attendi qualche secondo prima di riconnetterti
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    // Inizializza NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Crea event group per la connessione Wi-Fi
    s_wifi_event_group = xEventGroupCreate();

    // Avvia Wi-Fi in STA mode
    wifi_init_sta();

    // Crea un task per connettersi via TCP al server
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
}
