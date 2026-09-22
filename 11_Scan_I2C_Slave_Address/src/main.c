#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SDA_IO      GPIO_NUM_21
#define I2C_MASTER_SCL_IO      GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ     100000     // 100 kHz

static const char *TAG = "I2C_SCANNER";

void app_main(void)
{
    // 1. Configuration du bus I2C Maître
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true, // Pull-up interne activé
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    ESP_LOGI(TAG, "Démarrage du scan I2C sur SDA: GPIO %d, SCL: GPIO %d...", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);

    while (1) {
        printf("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("00:          ");

        int devices_found = 0;

        for (uint16_t addr = 0x01; addr < 0x7F; addr++) {
            if (addr % 16 == 0) {
                printf("\n%02x:", addr);
            }

            // Sondage du périphérique à l'adresse spécifiée (délai d'attente court)
            esp_err_t ret = i2c_master_probe(bus_handle, addr, 50);

            if (ret == ESP_OK) {
                printf(" %02x", addr);
                devices_found++;
            } else {
                printf(" --");
            }
        }

        printf("\n\nScan terminé : %d périphérique(s) trouvé(s).\n", devices_found);

        // Relance un scan toutes les 5 secondes
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}