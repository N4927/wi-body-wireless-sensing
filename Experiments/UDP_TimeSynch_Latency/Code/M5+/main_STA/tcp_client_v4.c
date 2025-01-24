#include "sdkconfig.h"
#include "config.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_attr.h"         //IRAM ATTR
#include "esp_timer.h"        //Timer
#if defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#include "addr_from_stdin.h"
#endif
//this could also be deleted
#define CONFIG_EXAMPLE_IPV4 1
#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#define HOST_IP_ADDR ""
#endif
#define TAG "STA"
#define PORT 3333
#define TIMER_PERIOD                (1000000)//microseconds

int sock_local = 0;

#include "driver/gpio.h"
#define GPIO_ENABLE                 0
#define GPIO_OUTPUT_PIN_SEL         (1ULL<<18)  

const char *signal_generation(char c[2048]){
    //ESP_LOGI(TAG, "In signal_gen: %s", sensorBuffer);
    //ESP_LOGI(TAG, "In signal_generation: %s", sensorBuffer);

    char newData[] = " aaaaaaaaa";
    //char newData[] = " bbbbbbbbb";

    strcat(c, newData);
    return c;
}

uint64_t timepoint = 0;

static void tcp_sending(char *load){
    //The following could be put into a do-while loop (?)
    int err = send(sock_local, load, strlen(load), 0);
    if (err < 0) ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
}

void tcp_client(void)
{

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

    ESP_LOGI(TAG, "BEGINNING OF tcp_client");
    char rx_buffer[2048];
    char host_ip[] = "192.168.4.1";
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT); //3333
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif

//CREATE:
        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        //sock_local = sock;
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);
        int buffer_size = 32768; // or larger
        setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d, Socket nr: %d", errno, sock);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

       //timepoint = esp_timer_get_time();
       while (1) {
            char sensorBuffer[2048] = " ";
            //ESP_LOGI(TAG, "Sensorbuffer NOW: %s", sensorBuffer);
            while (1)
            {    
                //ESP_LOGI(TAG,"In Loop");
                //int time = esp_timer_get_time();

                //const char *temp = signal_generation(sensorBuffer);
                const char *temp = payload;
                //uint64_t time = esp_timer_get_time();

                //ESP_LOGI(TAG, "Buffer: %s", temp);
                //const char *temp = t;
                //ESP_LOGI(TAG,"DATA:%s, Stringlength: %d, Time:%d", temp, strlen(temp), time);
                if ( /*strlen(temp) >= 1430*/ 1 ){
                        //ESP_LOGI(TAG,"DATA: %s \n", temp);
                        //tcp_sending(&sensorBuffer);
                        int err = send(sock, temp, strlen(temp), 0);
                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            goto CLEAN;
                        }

                        #if GPIO_ENABLE
                        gpio_set_level(18, 1);
                        vTaskDelay(10 / portTICK_PERIOD_MS);
                        gpio_set_level(18, 0);
                        #endif

                        //break;
                        ESP_LOGI(TAG, "TCP DATA sent");
                }

                //Inducing Delay to match sampling period of a sensor
                //while (toggle_GPIO + 10000 > esp_timer_get_time()){}

                //vTaskDelay(90/portTICK_PERIOD_MS);
                //This call causes problems!
                //sys_delay_ms(1); //when break is executed there won't be a delay !!!
            }
       }
       CLEAN:
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}