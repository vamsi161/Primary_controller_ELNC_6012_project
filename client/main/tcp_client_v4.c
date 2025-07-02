/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "lcd20x4_driver.h"
#if defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#define HOST_IP_ADDR ""
#endif
static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)


#define PORT CONFIG_EXAMPLE_PORT
#define TRUE 1
#define FALSE 0
#define WEIGHT_1 2
#define WEIGHT_2 5
#define TYPE_1 1
#define TYPE_2 2
#define TENTHPLACE 10
static const char *TAG = "example";
static const char *dummyPtr = "dummy";
static const char *payload = "Message from ESP32 Vamsi ";
static char messageRecv[1024];
static char bufId[20];
static char emptySlots[10];
char prodID[10][10] = {"12345671","12345672" ,"12345673" ,"12345674" ,"12345675", "12345676","12345677","12345678" };
typedef struct 
{
    char weight;
    char type;
    char frozen;
    char id[10];
    char slot;
} product_t;

product_t products[8];





void sortData(char *msgPtr)
{
    int msgid = (msgPtr[0] - '0') * TENTHPLACE;
    msgid += (msgPtr[1] - '0');



    switch(msgid)
    {
        case 01:
            // display empty slot data here
            ESP_LOGI(TAG, "EMPTY SLOTS :");
            
            for(int index = 2; msgPtr[index] != '\0'; index++)
            {
                emptySlots[index - 2] = msgPtr[index] - '0';
                ESP_LOGI(TAG, "%d", emptySlots[index - 2] );
            }
            break;
        case 02:
            // display spill ALERT HERE
            lcd20x4_print(dummyPtr);
           ESP_LOGI(TAG, "ALERT:SPILL DETECTED");
            break;
        case 03:
            //display TEMP ALERT HERE
           // ESP_LOGI(TAG, "ALERT:TEMPERATURE EXCEEDING");
            break;
        default:
            //DO NOTHING
            break;

    }
}


void initProducts(void)
{
    products[0].weight = WEIGHT_1;
    products[0].type = TYPE_1;
    products[0].frozen = TRUE;
    strcpy(products[0].id,prodID[0]);
    products[0].slot = 1;

    products[1].weight = WEIGHT_1;
    products[1].type = TYPE_2;
    products[1].frozen = TRUE;
    strcpy(products[1].id,prodID[1]);
    products[1].slot = 2;

    products[2].weight = WEIGHT_1;
    products[2].type = TYPE_1;
    products[2].frozen = FALSE;
    strcpy(products[2].id,prodID[2]);
    products[2].slot = 3;

    products[3].weight = WEIGHT_1;
    products[3].type = TYPE_2;
    products[3].frozen = FALSE;
    strcpy(products[3].id,prodID[3]);
    products[3].slot = 4;

    products[4].weight = WEIGHT_2;
    products[4].type = TYPE_1;
    products[4].frozen = TRUE;
    strcpy(products[4].id,prodID[4]);
    products[4].slot = 5;

    products[5].weight = WEIGHT_2;
    products[5].type = TYPE_2;
    products[5].frozen = TRUE;
    strcpy(products[5].id,prodID[5]);
    products[5].slot = 6;

    products[6].weight = WEIGHT_2;
    products[6].type = TYPE_1;
    products[6].frozen = FALSE;
    strcpy(products[6].id,prodID[6]);
    products[6].slot = 7;

    products[7].weight = WEIGHT_2;
    products[7].type = TYPE_2;
    products[7].frozen = FALSE;
    strcpy(products[7].id,prodID[7]);
    products[7].slot = 8;
}


void init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
void getSlot(char *buf)
{
    buf[strcspn(buf, "\r")] = '\0';
    //ESP_LOGI(TAG, "data received: %s", buf);
    for(int index = 0; index<8;index++)
    {
        if(strcmp(buf,prodID[index]) == FALSE)
        {
            //ESP_LOGI(RX_TASK_TAG, "product weight: %d product type: %d frozen: %d ID: '%s' slot: %d", products[index].weight,products[index].type,products[index].frozen,products[index].id,products[index].slot);
            //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, products[index].weight,products[index].type,products[index].frozen,products[index].id,products[index].slot, ESP_LOG_INFO);
            // -------------------send data to lcd here------------------------------------------------//
            ESP_LOGI(TAG, "PLACE PROD: %d SLOT", products[index].slot);

        }
    }

}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    char* data = (char*) malloc(RX_BUF_SIZE + 1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            strcpy(bufId,data);
            getSlot(bufId);
            //ESP_LOGI(RX_TASK_TAG, " stored bytes: %s Read bytes: %s ",prodID[7],  bufId );
            //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, prodID[7],bufId, ESP_LOG_INFO);
            
        }
    }
    free(data);
}
void tcp_client(void)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    initProducts();
    lcd20x4_init(I2C_NUM_0, GPIO_NUM_21,GPIO_NUM_22, 0x01);
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    while (1) {
#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif

        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1) 
        {
            int err = send(sock, payload, strlen(payload), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
               // ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
               // ESP_LOGI(TAG, "%s", rx_buffer);
                strcpy(messageRecv,rx_buffer);
                sortData(messageRecv);

            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
}
