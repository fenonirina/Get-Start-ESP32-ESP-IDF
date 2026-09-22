#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
#include "esp_log.h"

#define DAC_CHANNEL     DAC_CHAN_0 // DAC_CHAN_0 = GPIO 25 (DAC_CHAN_1 = GPIO 26)

static const char *TAG = "DAC_EXAMPLE";

void app_main(void)
{
    // 1. Configuration et initialisation du canal DAC
    dac_oneshot_handle_t dac_handle;
    dac_oneshot_config_t dac_cfg = {
        .chan_id = DAC_CHANNEL,
    };

    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_cfg, &dac_handle));

    ESP_LOGI(TAG, "DAC initialisé sur le GPIO 25");

    uint8_t dac_value = 0;

    while (1) {
        // 2. Écriture de la valeur sur le DAC (résolution 8 bits : 0 à 255)
        // 0   -> 0V
        // 128 -> ~1.65V
        // 255 -> ~3.3V
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac_handle, dac_value));

        // Calcul indicatif de la tension en millivolts
        uint32_t voltage_mv = (dac_value * 3300) / 255;
        ESP_LOGI(TAG, "Valeur DAC : %d | Tension sortie : %" PRIu32 " mV", dac_value, voltage_mv);

        // Incrémentation pour créer un signal en dent de scie
        dac_value += 25; 

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
}