//Julian Caredda 19.11.2024
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
#define PORT 3333
#define TIMER_PERIOD                (1000000)//microseconds

int sock_local = 0;


const char *signal_generation(char c[2048]){
    //ESP_LOGI(TAG, "In signal_gen: %s", sensorBuffer);
    //ESP_LOGI(TAG, "In signal_generation: %s", sensorBuffer);
    
    //char newData[] = " aaaaaaaaa";
    char newData[] = " bbbbbbbbb";

    strcat(c, newData);
    return c;
}

uint64_t timepoint = 0;

void tcp_sending(char *load){
    //The following could be put into a do-while loop (?)
    int err = send(sock_local, load, strlen(load), 0);
    if (err < 0) ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
}

void tcp_client(void)
{
    ESP_LOGI(TAG, "BEGINNING OF tcp_client");
    char rx_buffer[2048];
    char host_ip[] = "192.168.4.1";//HOST_IP_ADDR; //192.168.0.165 //PROBLEM
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

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d, Socket nr: %d", errno, sock);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");
        
        /*
        while(1){
            if ( strlen(&sensorBuffer) >= 1459){
                ESP_LOGI(TAG,"DATA: %s \n", &sensorBuffer);
                int err = send(sock, &sensorBuffer, strlen(&sensorBuffer), 0);
                if (err < 0) ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);

            }
        }*/

       //Start Sequence
       /*
        do {
            int err = send(sock, start, strlen(start), 0);
            ESP_LOGI(TAG, "Start sent");
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
        } while (0);*/

       //timepoint = esp_timer_get_time();
       while (1) {
            char sensorBuffer[2048] = " ";
            //ESP_LOGI(TAG, "Sensorbuffer NOW: %s", sensorBuffer);
            while (1)
            {    
                //ESP_LOGI(TAG,"In Loop");
                //int time = esp_timer_get_time();

                const char *temp = signal_generation(sensorBuffer);
                //ESP_LOGI(TAG, "Buffer: %s", temp);
                //const char *temp = t;
                //ESP_LOGI(TAG,"DATA:%s, Stringlength: %d, Time:%d", temp, strlen(temp), time);
                if ( strlen(temp) >= 1430 ){
                        //ESP_LOGI(TAG,"DATA: %s \n", temp);
                        //tcp_sending(&sensorBuffer);
                        int err = send(sock, temp, strlen(temp), 0);
                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            goto CLEAN;
                        }
                        //ESP_LOGI(TAG,"sent");
                        //timepoint = esp_timer_get_time();
                        //free(sensorBuffer);
                        //sensorBuffer[2048] = " ";
                        /*
                        shutdown(sock, 0);
                        close(sock);
                        goto CREATE;*/
                        break;
                }
                //This call causes problems!
                sys_delay_ms(1); //when break is executed there won't be a delay !!!
            }
       }
       CLEAN:
        shutdown(sock, 0);
        close(sock);
    }
    
}