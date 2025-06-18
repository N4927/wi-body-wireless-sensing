idf_component_register(SRCS "power_server_tcp.c"
                            "get_ap_info.c"
                    PRIV_REQUIRES esp_driver_uart vfs esp_wifi esp_pm nvs_flash
                    INCLUDE_DIRS ".")
