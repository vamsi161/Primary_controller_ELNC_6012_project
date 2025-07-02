#include "lcd20x4_driver.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "LCD20x4";

// PCF8574 pin bits
#define PCF_BACKLIGHT  (1<<3)
#define PCF_ENABLE     (1<<2)
#define PCF_READ_WRITE (1<<1)  // always 0 for write
#define PCF_REGISTER   (1<<0)  // 0=command, 1=data

// I²C handle
static i2c_port_t s_i2c_port;
static uint8_t    s_i2c_addr;

/** @brief Low-level write to the PCF8574 expander */
static void pcf8574_write(uint8_t data)
{
    i2c_master_write_to_device(
        s_i2c_port,
        s_i2c_addr,
        &data, 1,
        pdMS_TO_TICKS(100)
    );
}

/** @brief Pulse the Enable line to latch data/command */
static void lcd_strobe(uint8_t data)
{
    pcf8574_write(data | PCF_ENABLE);
    vTaskDelay(pdMS_TO_TICKS(1));
    pcf8574_write(data & ~PCF_ENABLE);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/** @brief Send a 4-bit nibble (upper half of a byte) */
static void lcd_send_nibble(uint8_t nibble, bool is_data)
{
    uint8_t out = (nibble << 4)        // D7..D4 → P7..P4
                  | PCF_BACKLIGHT      // keep backlight on
                  | (is_data ? PCF_REGISTER : 0);
    pcf8574_write(out);
    lcd_strobe(out);
}

/** @brief Send a full 8-bit command */
static void lcd_send_command(uint8_t cmd)
{
    lcd_send_nibble(cmd >> 4, false);
    lcd_send_nibble(cmd & 0x0F, false);
    vTaskDelay(pdMS_TO_TICKS(2));
}

/** @brief Send a single character as data */
static void lcd_send_data(uint8_t d)
{
    lcd_send_nibble(d >> 4, true);
    lcd_send_nibble(d & 0x0F, true);
    vTaskDelay(pdMS_TO_TICKS(1));
}

void lcd20x4_init(i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin, uint8_t i2c_addr)
{
    // Save for internal use
    s_i2c_port = port;
    s_i2c_addr = i2c_addr;

    // Configure I2C bus
    i2c_config_t cfg = {
        .mode              = I2C_MODE_MASTER,
        .sda_io_num        = sda_pin,
        .scl_io_num        = scl_pin,
        .sda_pullup_en     = GPIO_PULLUP_ENABLE,
        .scl_pullup_en     = GPIO_PULLUP_ENABLE,
        .master.clk_speed  = 100000
    };
    ESP_ERROR_CHECK(i2c_param_config(port, &cfg));
    ESP_ERROR_CHECK(i2c_driver_install(port, cfg.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C init on SDA=%d SCL=%d @0x%02X", sda_pin, scl_pin, i2c_addr);

    // According to HD44780 datasheet reset procedure
    vTaskDelay(pdMS_TO_TICKS(50));
    for (int i = 0; i < 3; i++) {
        lcd_send_nibble(0x03, false);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    // Switch to 4-bit mode
    lcd_send_nibble(0x02, false);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Function set: 4-bit interface, 2 lines (works for 4 lines), 5×8 font
    lcd_send_command(0x28);
    // Display ON, cursor OFF, blink OFF
    lcd_send_command(0x0C);
    // Entry mode: auto-increment, no shift
    lcd_send_command(0x06);
    // Clear display
    lcd_send_command(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));

    ESP_LOGI(TAG, "LCD initialized");
}

void lcd20x4_set_cursor(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[4] = { 0x00, 0x40, 0x14, 0x54 };
    if (row > 3) row = 3;
    if (col > 19) col = 19;
    lcd_send_command(0x80 | (row_offsets[row] + col));
}

void lcd20x4_print(const char *str)
{
    while (*str) {
        lcd_send_data((uint8_t)*str++);
    }
}
