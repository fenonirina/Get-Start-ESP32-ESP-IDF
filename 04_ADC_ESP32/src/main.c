#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "ADC_EXAMPLE";

void app_main(void) {

    // 1. Configuration du Handle de l'unité ADC1
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0, // Par défaut
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // 2. Configuration du canal ADC (Exemple: Canal 0)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Résolution maximale (ex: 12 bits)
        .atten = ADC_ATTEN_DB_12,         // Atténuation pour lire jusqu'à ~3.3V
    };
    // Note: Vérifiez à quel GPIO correspond ADC_CHANNEL_0 sur votre puce (ESP32, ESP32-S3, etc.)
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));

    while (1) {
        int adc_raw;
        // 3. Lecture de la valeur brute
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw));
        ESP_LOGI(TAG, "Valeur brute ADC: %d", adc_raw);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}
