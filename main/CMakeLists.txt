idf_component_register(SRCS "main_fault_detector.c"
                    INCLUDE_DIRS "."
                    REQUIRES json esp_wifi esp_netif esp_event nvs_flash driver esp_http_client esp_adc)