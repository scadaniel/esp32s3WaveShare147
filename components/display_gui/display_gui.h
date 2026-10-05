#ifndef DISPLAY_GUI_H
#define DISPLAY_GUI_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa la pantalla ST7789 + LVGL + UI de SquareLine
 * @return ESP_OK si todo salió bien
 */
esp_err_t display_gui_init(void);

/**
 * @brief Actualiza los valores de temperatura y humedad en la UI
 * @param temp  Temperatura en °C
 * @param hum   Humedad en %
 * @param valid true si la lectura del DHT11 fue correcta
 */
void display_gui_update_data(uint8_t temp, uint8_t hum, bool valid);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_GUI_H