#include "display_gui.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui.h"

static const char *TAG = "display_gui";

#define LCD_HOST            SPI2_HOST
#define PIN_NUM_LCD_MOSI    45
#define PIN_NUM_LCD_CLK     40
#define PIN_NUM_LCD_CS      42
#define PIN_NUM_LCD_DC      41
#define PIN_NUM_LCD_RST     39
#define PIN_NUM_LCD_BK      46

#define LCD_H_RES           320
#define LCD_V_RES           172
#define LCD_X_GAP           0
#define LCD_Y_GAP           34
#define LCD_PIXEL_CLOCK_HZ  (20 * 1000 * 1000)

static lv_disp_t *lvgl_disp = NULL;

esp_err_t display_gui_init(void)
{
    // 1. Backlight
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_LCD_BK
    };
    gpio_config(&bk_gpio_config);
    gpio_set_level(PIN_NUM_LCD_BK, 1);

    // 2. Bus SPI
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_LCD_CLK,
        .mosi_io_num = PIN_NUM_LCD_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 3. Panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_LCD_DC,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    // 4. ST7789
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_X_GAP, LCD_Y_GAP));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Limpieza
    uint16_t *black_buf = calloc(LCD_H_RES * LCD_V_RES, sizeof(uint16_t));
    if (black_buf) {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, black_buf);
        free(black_buf);
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    // 5. LVGL Port
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * 40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    // 6. UI SquareLine
    if (lvgl_port_lock(0)) {
        ui_init();
        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "Display + SquareLine UI listo");
    return ESP_OK;
}

void display_gui_update_data(uint8_t temp, uint8_t hum, bool valid)
{
    if (lvgl_port_lock(0)) {
        if (valid) {
            lv_label_set_text_fmt(ui_lbltempval, "%d °C", temp);
            lv_label_set_text_fmt(ui_lblhumval,  "%d %%", hum);
        } else {
            lv_label_set_text(ui_lbltempval, "-- °C");
            lv_label_set_text(ui_lblhumval,  "-- %");
        }
        lvgl_port_unlock();
    }
}