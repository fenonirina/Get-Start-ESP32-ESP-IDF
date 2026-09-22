#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SDA_IO      GPIO_NUM_21
#define I2C_MASTER_SCL_IO      GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ     100000 // 100 kHz
#define DS3231_I2C_ADDR        0x68   // Adresse I2C fixe du DS3231

static const char *TAG = "DS3231_RTC";

// Structure de représentation du temps
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week; // 1 = Dimanche, 2 = Lundi, ...
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// --- Conversion BCD <-> Décimal ---
static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

// --- Écriture de l'heure dans le DS3231 ---
esp_err_t ds3231_set_time(i2c_master_dev_handle_t dev_handle, const rtc_time_t *time) {
    uint8_t write_buf[8];
    write_buf[0] = 0x00; // Adresse du registre de départ (Secondes)
    write_buf[1] = dec_to_bcd(time->seconds);
    write_buf[2] = dec_to_bcd(time->minutes);
    write_buf[3] = dec_to_bcd(time->hours);
    write_buf[4] = dec_to_bcd(time->day_of_week);
    write_buf[5] = dec_to_bcd(time->day);
    write_buf[6] = dec_to_bcd(time->month);
    write_buf[7] = dec_to_bcd((uint8_t)(time->year % 100)); // Année sur 2 chiffres (ex: 26 pour 2026)

    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

// --- Lecture de l'heure depuis le DS3231 ---
esp_err_t ds3231_get_time(i2c_master_dev_handle_t dev_handle, rtc_time_t *time) {
    uint8_t reg_addr = 0x00;
    uint8_t data[7] = {0};

    esp_err_t err = i2c_master_transmit_receive(
        dev_handle,
        &reg_addr, sizeof(reg_addr),
        data, sizeof(data),
        pdMS_TO_TICKS(1000)
    );

    if (err == ESP_OK) {
        time->seconds     = bcd_to_dec(data[0] & 0x7F);
        time->minutes     = bcd_to_dec(data[1]);
        time->hours       = bcd_to_dec(data[2] & 0x3F); // Format 24h
        time->day_of_week = bcd_to_dec(data[3]);
        time->day         = bcd_to_dec(data[4]);
        time->month       = bcd_to_dec(data[5] & 0x1F);
        time->year        = 2000 + bcd_to_dec(data[6]);
    }

    return err;
}

void app_main(void)
{
    // 1. Initialisation du bus I2C
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    // 2. Configuration de l'esclave DS3231
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DS3231_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    i2c_master_dev_handle_t rtc_dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &rtc_dev_handle));

    ESP_LOGI(TAG, "RTC DS3231 initialisé à l'adresse 0x%02X", DS3231_I2C_ADDR);

    // 3. (OPTIONNEL) Régler l'heure initiale (À décommenter une première fois si besoin)
    /*
    rtc_time_t initial_time = {
        .seconds = 0,
        .minutes = 30,
        .hours = 14,
        .day_of_week = 3, // Mardi
        .day = 22,
        .month = 9,
        .year = 2026
    };
    ds3231_set_time(rtc_dev_handle, &initial_time);
    ESP_LOGI(TAG, "Nouvelle heure configurée !");
    */

    // 4. Boucle de lecture continue
    while (1) {
        rtc_time_t current_time;

        if (ds3231_get_time(rtc_dev_handle, &current_time) == ESP_OK) {
            ESP_LOGI(TAG, "Date/Heure : %02d/%02d/%04d - %02d:%02d:%02d",
                     current_time.day, current_time.month, current_time.year,
                     current_time.hours, current_time.minutes, current_time.seconds);
        } else {
            ESP_LOGE(TAG, "Erreur de lecture RTC DS3231");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}