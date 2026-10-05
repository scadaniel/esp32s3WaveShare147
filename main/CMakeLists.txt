idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES display_gui dht11 esp_timer esp_lcd esp_driver_gpio esp_driver_spi esp_driver_ledc
    )  
    
