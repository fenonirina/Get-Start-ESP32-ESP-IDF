#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SDA_IO      GPIO_NUM_21
#define I2C_MASTER_SCL_IO      GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ     100000 // 100 kHz
#define LCD_I2C_ADDR           0x27   // Modifier par 0x3F si l'écran n'affiche rien

// Masques de contrôle du PCF8574
#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00
#define ENABLE                 0x04
#define RS_CMD                 0x00
#define RS_DATA                0x01

static const char *TAG = "LCD_I2C";
static i2c_master_dev_handle_t lcd_dev_handle = NULL;
static uint8_t backlight_state = LCD_BACKLIGHT;

// --- Fonctions de bas niveau pour l'écran LCD ---

static esp_err_t lcd_send_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | backlight_state;
    uint8_t buf[2] = {
        data | ENABLE,  // Enable HIGH
        data & ~ENABLE  // Enable LOW
    };
    return i2c_master_transmit(lcd_dev_handle, buf, sizeof(buf), pdMS_TO_TICKS(100));
}

static void lcd_send_byte(uint8_t byte, uint8_t mode) {
    lcd_send_nibble(byte & 0xF0, mode);
    lcd_send_nibble((byte << 4) & 0xF0, mode);
}

void lcd_send_cmd(uint8_t cmd) {
    lcd_send_byte(cmd, RS_CMD);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_send_data(uint8_t data) {
    lcd_send_byte(data, RS_DATA);
}

// --- Fonctions utilisateur ---

void lcd_clear(void) {
    lcd_send_cmd(0x01); // Commande Clear Display
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_send_cmd(0x80 | (col + row_offsets[row]));
}

void lcd_write_string(const char *str) {
    while (*str) {
        lcd_send_data((uint8_t)(*str++));
    }
}

void lcd_init(void) {
    vTaskDelay(pdMS_TO_TICKS(50)); // Attente de stabilisation à l'allumage

    // Séquence d'initialisation en mode 4-bits
    lcd_send_nibble(0x30, RS_CMD);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_send_nibble(0x30, RS_CMD);
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_nibble(0x30, RS_CMD);
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_nibble(0x20, RS_CMD); // Passer en mode 4 bits

    // Configuration LCD : 2 lignes, police 5x8
    lcd_send_cmd(0x28);
    // Affichage actif, curseur masqué, pas de clignotement
    lcd_send_cmd(0x0C);
    // Effacer l'écran
    lcd_clear();
    // Mode d'incrémentation automatique
    lcd_send_cmd(0x06);
}

// --- Programme Principal ---

void app_main(void)
{
    // 1. Initialisation du Bus I2C Maître
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    // 2. Configuration de l'esclave LCD
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &lcd_dev_handle));

    ESP_LOGI(TAG, "Initialisation de l'écran LCD...");
    lcd_init();

    // 3. Affichage de texte fixe
    lcd_set_cursor(0, 0);
    lcd_write_string("ESP32 + ESP-IDF");
    lcd_set_cursor(0, 1);
    lcd_write_string("Compteur: ");

    int counter = 0;
    char buffer[16];

    // 4. Boucle de mise à jour du compteur
    while (1) {
        snprintf(buffer, sizeof(buffer), "%-6d", counter++);
        lcd_set_cursor(10, 1);
        lcd_write_string(buffer);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}