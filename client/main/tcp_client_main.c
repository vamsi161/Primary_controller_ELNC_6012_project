/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_event.h"

// bring in your LCD driver
#include "lcd20x4_driver.h"

extern void tcp_client(void);
extern void init(void);

void app_main(void)
{
    // 1) Initialize NVS, netif, event loop (these are cheap and required by both Wi-Fi & I²C)
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 2) **First** fire up your LCD driver and print something you can see in the log
    lcd20x4_init(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 0x27);
    lcd20x4_set_cursor(0, 0);
    lcd20x4_print(">>> LCD is alive <<<");

    // Give it a moment to flush
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 3) Now bring up the network
    ESP_ERROR_CHECK(example_connect());

    // 4) And finally your application logic
    init();
    tcp_client();
}

