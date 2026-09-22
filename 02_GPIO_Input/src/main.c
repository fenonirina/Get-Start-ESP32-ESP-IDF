#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GPIO    GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_5

static const char *TAG = "main";

void app_main(void)
{
    // Configuration de la LED (Sortie)
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // Configuration du Bouton (Entrée avec Pull-Up)
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Système prêt. Appuyez sur le bouton du GPIO %d pour basculer la LED sur GPIO %d.", BUTTON_GPIO, LED_GPIO);

    int last_button_state = 1; // État initial (Relâché = HIGH grâce au Pull-Up)
    int led_state = 0;          // État initial de la LED (Éteinte = LOW)

    // S'assurer que la LED est bien éteinte au démarrage
    gpio_set_level(LED_GPIO, led_state);

    while (1) {
        int current_button_state = gpio_get_level(BUTTON_GPIO);

        // Détection du front descendant : le bouton passe de HIGH (1) à LOW (0) -> Appui sur le bouton
        if (last_button_state == 1 && current_button_state == 0) {
            
            // Inversement de l'état de la LED
            led_state = !led_state;
            gpio_set_level(LED_GPIO, led_state);

            ESP_LOGI(TAG, "Bouton appuyé ! Nouvel état de la LED : %s", led_state ? "ALLUMÉE" : "ÉTEINTE");

            // Anti-rebond (debounce) : pause de 50 ms pour ignorer les parasites mécaniques
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        // Mettre à jour le dernier état connu du bouton
        last_button_state = current_button_state;

        // Pause de boucle pour laisser la main à FreeRTOS
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}