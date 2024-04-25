#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "stim.h"
#include "driver/dac_continuous.h"

#define HOST_IP_ADDR        "192.168.8.198"
#define PORT                8000
#define PAYLOAD_LENGTH      405 

static const char *TAG = "UDP";

uint8_t wave[WAVE_LENGTH];

void udp_client_task(void *pvParameters){

    int addr_family = 0;
    int ip_protocol = IPPROTO_IP;


    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if(sock < 0){
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    //ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);


    int err = sendto(sock, pvParameters, PAYLOAD_LENGTH, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if(err < 0){
        ESP_LOGE(TAG, "Error occurred while sending: errno %d", errno);
    }
    //ESP_LOGI(TAG, "Message sent");   
             

    shutdown(sock, 0);
    close(sock);
    
    

    vTaskDelete(NULL);
}

void udp_recieve_task(void *pvParameters){

    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = IPPROTO_IP;

    while(true){

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if(sock < 0){
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        }

        struct timeval timeout;
        timeout.tv_sec = 1000;
        timeout.tv_usec = 0;
        
        int err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        if (err < 0){
            ESP_LOGE(TAG, "Failed to set sock options: errno %d", errno);
        }
        

        // Initialize dac
        err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0){
            ESP_LOGE(TAG, "unable to bind socket: errno %d", errno);
        }


        ESP_LOGI(TAG, "CONNECTED TO PORT: %d", PORT);
        ESP_LOGI(TAG, "CONNECTED TO SOCKET     %d", sock);

        dac_continuous_handle_t stim_handle;
        err = stim_dma(&stim_handle);
        if (err < 0){
            ESP_LOGE(TAG, "Unable to create dac handle: errno %d", errno);
        }

        // start DMA with no wave
        uint32_t wave_len = WAVE_LENGTH;
        generate_wave(0, 255, wave, wave_len);
        dac_continuous_write_cyclically(stim_handle, (uint8_t *)wave, wave_len, NULL);

        while(true){

            ESP_LOGI(TAG,"THIS TASK IS ALIVE");

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                //if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                //    ESP_LOGI(TAG, "Received expected message, reconnecting");
                //    break;
                //}

                if(rx_buffer[0] == 'S'){
                    ESP_LOGI(TAG, "GOOD MESSAGE RECIEVED");////////////
                    generate_wave(rx_buffer[1], rx_buffer[2], wave, wave_len);
                    dac_continuous_write_cyclically(stim_handle, (uint8_t *)wave, wave_len, NULL);
                }


            }


            vTaskDelay(100 / portTICK_PERIOD_MS);
            

        }

             
        if(sock != -1){
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            dac_continuous_del_channels(stim_handle);
        }
    
    
    }
    vTaskDelete(NULL);
}