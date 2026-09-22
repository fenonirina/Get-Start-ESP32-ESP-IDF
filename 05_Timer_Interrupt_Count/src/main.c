#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_log.h"

static const char *TAG = "TIMER_EXAMPLE";

// Compteur incrémenté par l'interruption (doit être volatile)
static volatile uint32_t timer_count = 0;

// 1. Routine d'interruption du Timer (ISR)
static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    // Incrémentation du compteur
    timer_count++;

    // Retourne false car aucune tâche FreeRTOS à haute priorité n'est réveillée depuis cette ISR
    return false;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initialisation du Timer...");

    // 2. Configuration du Timer
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Source d'horloge par défaut (80 MHz)
        .direction = GPTIMER_COUNT_UP,      // Compte vers le haut
        .resolution_hz = 1000000,          // Résolution de 1 MHz (1 tick = 1 microseconde)
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // 3. Définition de la fonction de rappel (Callback ISR)
    gptimer_event_callbacks_t cbs = {
        .on_alarm = on_timer_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // 4. Configuration de l'alarme (Interruption toutes les 1 seconde = 1 000 000 µs)
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,                 // Redémarre à 0 après déclenchement
        .alarm_count = 1000000,            // Déclenche l'alarme à 1 000 000 ticks (1s)
        .flags.auto_reload_on_alarm = true,// Rechargement automatique en boucle
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // 5. Activation et démarrage du Timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    ESP_LOGI(TAG, "Timer démarré.");

    // Boucle principale : Affiche la valeur du compteur incrémenté par le timer
    while (1) {
        ESP_LOGI(TAG, "Compteur de l'interruption : %" PRIu32, timer_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}