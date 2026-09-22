#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Définitions de la configuration UART
#define UART_PORT_NUM      UART_NUM_1
#define TX_PIN             GPIO_NUM_17
#define RX_PIN             GPIO_NUM_16
#define BUF_SIZE           1024

static const char *TAG = "UART_EXAMPLE";

// Tâche dédiée à la réception UART
static void rx_task(void *arg)
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    
    while (1) {
        // Lecture des données reçues dans le buffer UART
        int len = uart_read_bytes(UART_PORT_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(100));
        
        if (len > 0) {
            data[len] = '\0'; // Ajouter le caractère de fin de chaîne
            ESP_LOGI(TAG, "Reçu (%d octets) : %s", len, (char *) data);
        }
    }
    free(data);
    vTaskDelete(NULL);
}

void app_main(void)
{
    // 1. Configuration des paramètres du port UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 2. Application de la configuration
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

    // 3. Attribution des broches GPIO pour TX et RX
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // 4. Installation du pilote UART avec un buffer de réception
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));

    ESP_LOGI(TAG, "UART%d initialisé sur TX: GPIO%d, RX: GPIO%d", UART_PORT_NUM, TX_PIN, RX_PIN);

    // 5. Création de la tâche de réception en arrière-plan
    xTaskCreate(rx_task, "uart_rx_task", 3072, NULL, 10, NULL);

    // 6. Boucle d'émission périodique
    int count = 0;
    while (1) {
        char tx_message[64];
        snprintf(tx_message, sizeof(tx_message), "Message ESP32 #%d\r\n", count++);

        // Émission du message sur le port UART
        uart_write_bytes(UART_PORT_NUM, tx_message, strlen(tx_message));
        ESP_LOGI(TAG, "Envoyé : %s", tx_message);

        vTaskDelay(pdMS_TO_TICKS(2000)); // Envoie un message toutes les 2 secondes
    }
}