#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "display_gui.h"
#include "dht11.h"

static const char *TAG = "main";

#define DHT_PIN GPIO_NUM_4

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema...");

    // 1. Inicializar pantalla + UI de SquareLine
    ESP_ERROR_CHECK(display_gui_init());

    // 2. Inicializar sensor DHT11
    dht11_config_t dht_cfg = {
        .gpio_num = DHT_PIN
    };
    ESP_ERROR_CHECK(dht11_init(&dht_cfg));

    uint8_t temp = 0;
    uint8_t hum  = 0;

    // 3. Bucle de lectura y actualización de la pantalla
    while (1) {
        esp_err_t ret = dht11_read(&hum, &temp);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "DHT11 -> Temp: %d °C, Hum: %d %%", temp, hum);
            display_gui_update_data(temp, hum, true);
        } else {
            ESP_LOGE(TAG, "Error al leer DHT11 (%s)", esp_err_to_name(ret));
            display_gui_update_data(0, 0, false);
        }

        // El DHT11 necesita mínimo 2 segundos entre lecturas
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}