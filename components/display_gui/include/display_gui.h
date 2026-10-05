#ifndef DISPLAY_GUI_H
#define DISPLAY_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el panel LCD ST7789 y el sistema gráfico LVGL.
 */
esp_err_t display_gui_init(void);

/**
 * @brief Actualiza las etiquetas de temperatura y humedad en la interfaz de LVGL.
 */
void display_gui_update_data(uint8_t temp, uint8_t hum, bool valid);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_GUI_H