#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GPIO    GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_5

static const char *TAG = "main";

// État partagé de la LED (doit être volatile car modifié dans une ISR)
static volatile int led_state = 0;

// Gestionnaire d'interruption (ISR) exécuté directement lors de l'appui
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // Inversion de l'état de la LED
    led_state = !led_state;
    gpio_set_level(LED_GPIO, led_state);
}

void app_main(void)
{
    // Configuration de la LED (Sortie)
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, led_state);

    // Configuration du Bouton (Entrée avec Pull-Up et Interruption sur front descendant)
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);

    // Installation du service d'interruption global pour les GPIO
    gpio_install_isr_service(0);
    
    // Association de notre fonction ISR à la broche du bouton
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "Interruption GPIO configurée sur le bouton (GPIO %d)", BUTTON_GPIO);

    // app_main peut simplement se terminer ou tourner au ralenti, 
    // l'interruption gère tout en arrière-plan.
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}