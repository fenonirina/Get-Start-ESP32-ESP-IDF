#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Définition des broches SPI
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_18
#define PIN_NUM_CS   GPIO_NUM_5

static const char *TAG = "SPI_MASTER";

void app_main(void)
{
    esp_err_t ret;

    // 1. Configuration du bus SPI principal (Horloge + Données)
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1, // Non utilisé
        .quadhd_io_num = -1, // Non utilisé
        .max_transfer_sz = 32, // Taille max du transfert en octets
    };

    // Initialisation du bus SPI2 (SPI2_HOST)
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // 2. Configuration du périphérique esclave attaché au bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // Fréquence de l'horloge : 1 MHz
        .mode = 0,                          // Mode SPI 0 (CPOL=0, CPHA=0)
        .spics_io_num = PIN_NUM_CS,         // Broche Chip Select
        .queue_size = 7,                    // Nombre de transactions simultanées en file d'attente
    };

    // Enregistrement du périphérique sur le bus
    spi_device_handle_t spi_device;
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Bus SPI Master initialisé avec succès !");

    uint8_t tx_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t rx_data[4] = {0};

    // 3. Préparation et exécution de la transaction SPI
    spi_transaction_t t = {
        .length = 8 * sizeof(tx_data), // Longueur de la transaction en BITS (4 octets = 32 bits)
        .tx_buffer = tx_data,          // Pointeur vers les données à envoyer
        .rx_buffer = rx_data,          // Pointeur vers le buffer de réception
    };

    while (1) {
        // Transfert synchrone (Emmission et Réception simultanées)
        ret = spi_device_transmit(spi_device, &t);
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(TAG, "Envoyé : [0x%02X, 0x%02X, 0x%02X, 0x%02X]", 
                 tx_data[0], tx_data[1], tx_data[2], tx_data[3]);
        ESP_LOGI(TAG, "Reçu   : [0x%02X, 0x%02X, 0x%02X, 0x%02X]", 
                 rx_data[0], rx_data[1], rx_data[2], rx_data[3]);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}