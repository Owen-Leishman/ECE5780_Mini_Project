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

#define HOST_IP_ADDR        "192.168.8.198"
#define PORT                8000
#define PAYLOAD_LENGTH      205 


static const char *TAG = "UDP";

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

    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);


    int err = sendto(sock, pvParameters, PAYLOAD_LENGTH, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if(err < 0){
        ESP_LOGE(TAG, "Error occurred while sending: errno %d", errno);
    }
    ESP_LOGI(TAG, "Message sent");   
            


        
        

    shutdown(sock, 0);
    close(sock);
    
    

    vTaskDelete(NULL);
}