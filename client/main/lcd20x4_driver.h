#ifndef LCD20X4_DRIVER_H
#define LCD20X4_DRIVER_H

#include "driver/i2c.h"
#include <stdbool.h>

/**
 * @brief Initialize the 20×4 LCD over I²C.
 *
 * @param port      I2C port number (e.g. I2C_NUM_0)
 * @param sda_pin   GPIO number for SDA line
 * @param scl_pin   GPIO number for SCL line
 * @param i2c_addr  7-bit I2C address of the PCF8574 (usually 0x27 or 0x3F)
 */
void lcd20x4_init(i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin, uint8_t i2c_addr);

/**
 * @brief Position the cursor on the display.
 *
 * @param row  Zero-based row (0..3)
 * @param col  Zero-based column (0..19)
 */
void lcd20x4_set_cursor(uint8_t row, uint8_t col);

/**
 * @brief Print a NUL-terminated string at the current cursor.
 *
 * @param str  The C-string to display
 */
void lcd20x4_print(const char *str);

#endif // LCD20X4_DRIVER_H
