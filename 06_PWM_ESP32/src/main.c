#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define LED_GPIO            GPIO_NUM_2

// Configuration du PWM
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT // Résolution 13 bits (Valeurs de 0 à 8191)
#define LEDC_FREQUENCY      5000               // Fréquence en Hz (5 kHz)

static const char *TAG = "PWM_EXAMPLE";

void app_main(void)
{
    // 1. Configuration du Timer PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2. Configuration du Canal PWM
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_GPIO,
        .duty           = 0, // Rapport cyclique initial (0%)
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "PWM configuré sur GPIO %d à %d Hz", LED_GPIO, LEDC_FREQUENCY);

    // Boucle principale : variation progressive de la luminosité (Fading)
    while (1) {
        // Augmentation progressive de la luminosité (0 à 100%)
        for (int duty = 0; duty <= 8191; duty += 100) {
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // Diminution progressive de la luminosité (100% à 0%)
        for (int duty = 8191; duty >= 0; duty -= 100) {
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}