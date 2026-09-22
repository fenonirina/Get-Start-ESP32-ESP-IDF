#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SDA_IO      GPIO_NUM_21
#define I2C_MASTER_SCL_IO      GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ     100000 // 100 kHz (Standard Mode)

// REMPLACER par l'adresse I2C de votre esclave (ex: 0x68 pour MPU6050, 0x3C pour SSD1306)
#define SLAVE_DEV_ADDR         0x68   

static const char *TAG = "I2C_MASTER_SLAVE";

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

    // 2. Ajout du périphérique Esclave sur le Bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SLAVE_DEV_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    ESP_LOGI(TAG, "Bus I2C prêt. Esclave configuré à l'adresse 0x%02X", SLAVE_DEV_ADDR);

    // --- A. ÉCRITURE D'UN REGISTRE DE L'ESCLAVE ---
    uint8_t reg_addr = 0x6B;  // Exemple : Adresse du registre Power Management (MPU6050)
    uint8_t data_to_write = 0x00; // Exemple : Réveiller le composant

    uint8_t write_buf[2] = {reg_addr, data_to_write};
    esp_err_t err = i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Écriture réussie : Registre 0x%02X = 0x%02X", reg_addr, data_to_write);
    } else {
        ESP_LOGE(TAG, "Échec d'écriture : %s", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    // --- B. LECTURE D'UN REGISTRE DE L'ESCLAVE (Transmit-Receive Combined) ---
    uint8_t read_reg = 0x75; // Exemple : Registre WHO_AM_I (MPU6050)
    uint8_t rx_buffer[1] = {0};

    // La fonction transmit_receive envoie l'adresse du registre puis lit directement la réponse
    err = i2c_master_transmit_receive(
        dev_handle, 
        &read_reg, sizeof(read_reg), 
        rx_buffer, sizeof(rx_buffer), 
        pdMS_TO_TICKS(1000)
    );

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Lecture réussie : Registre 0x%02X -> Valeur lue = 0x%02X", read_reg, rx_buffer[0]);
    } else {
        ESP_LOGE(TAG, "Échec de lecture : %s", esp_err_to_name(err));
    }

    // --- C. BOUCLE DE LECTURE EN CONTINU ---
    while (1) {
        uint8_t sensor_data[2] = {0};
        uint8_t start_reg = 0x3B; // Exemple : Début des données accéléromètre

        err = i2c_master_transmit_receive(
            dev_handle, 
            &start_reg, sizeof(start_reg), 
            sensor_data, sizeof(sensor_data), 
            pdMS_TO_TICKS(500)
        );

        if (err == ESP_OK) {
            uint16_t raw_value = (sensor_data[0] << 8) | sensor_data[1];
            ESP_LOGI(TAG, "Données brutes lues (2 octets) : 0x%04X (%d)", raw_value, raw_value);
        } else {
            ESP_LOGW(TAG, "Erreur de communication I2C");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}