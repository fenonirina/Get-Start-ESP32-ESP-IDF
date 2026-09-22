#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define HALL_SENSOR_GPIO  GPIO_NUM_5

static const char *TAG = "HALL_SENSOR";

void app_main(void)
{
    // Configure the Hall sensor pin as input with pull-up
    gpio_reset_pin(HALL_SENSOR_GPIO);
    gpio_set_direction(HALL_SENSOR_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HALL_SENSOR_GPIO, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Digital Hall Sensor initialized on GPIO %d", HALL_SENSOR_GPIO);

    while (1) {
        // Most digital Hall sensors pull LOW when a magnet is detected
        int hall_state = gpio_get_level(HALL_SENSOR_GPIO);

        if (hall_state == 0) {
            ESP_LOGI(TAG, "Magnet Detected! [LOW]");
        } else {
            ESP_LOGI(TAG, "No Magnet Field. [HIGH]");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}