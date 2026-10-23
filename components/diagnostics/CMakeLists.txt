idf_component_register(
    SRCS
        "src/Diagnostics.cpp"
        "src/DiagnosticsSerializer.cpp"

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_timer
        esp_wifi
        esp_netif
        spi_flash
        app_update
		utils
		logging

    PRIV_REQUIRES
        efuse
		espressif__cjson
)