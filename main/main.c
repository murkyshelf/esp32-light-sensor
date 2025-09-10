#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/adc.h"
#include "cJSON.h"
#include "esp_websocket_client.h"

#define WIFI_SSID      "Ce4"
#define WIFI_PASS      "p4msujax"
#define WEBSOCKET_URI  "ws://10.230.25.216:8080"   // Change to your WebSocket server

static const char *TAG = "ADC_WS";
static esp_websocket_client_handle_t ws_client = NULL;

// --- WiFi event handler ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Connected to WiFi");
    }
}

// --- Connect WiFi ---
static void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

// --- WebSocket event handler ---
static void websocket_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket connected");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket disconnected");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "Received data from server");
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket error");
            break;
    }
}

// --- Send ADC voltage over WebSocket ---
static void send_adc_data(int adc_val) {
    float voltage = (adc_val / 4095.0) * 3.3;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "voltage", voltage);
    char *json_str = cJSON_PrintUnformatted(root);

    ESP_LOGI(TAG, "Sending JSON: %s", json_str);

    if (esp_websocket_client_is_connected(ws_client)) {
        esp_websocket_client_send_text(ws_client, json_str, strlen(json_str), portMAX_DELAY);
    } else {
        ESP_LOGW(TAG, "WebSocket not connected, skipping send");
    }

    cJSON_Delete(root);
    free(json_str);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    // Configure ADC (example: ADC1 channel 6 -> GPIO34)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    // WebSocket config
    esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_URI,
    };

    ws_client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
    esp_websocket_client_start(ws_client);

    while (1) {
        int adc_val = adc1_get_raw(ADC1_CHANNEL_6);
        send_adc_data(adc_val);
        vTaskDelay(pdMS_TO_TICKS(5000)); // every 5s
    }
}
