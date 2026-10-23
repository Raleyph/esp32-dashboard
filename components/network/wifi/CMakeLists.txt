idf_component_register(
    SRCS
        "src/wifi.cpp"

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
)