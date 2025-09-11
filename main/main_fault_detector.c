#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_adc/adc_oneshot.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_sntp.h"

// Configuration
#define WIFI_SSID           "....."
#define WIFI_PASS           "#12345678"
#define FAULT_API_URL       "http://10.230.25.216:3000/api/fault/esp32"
#define API_KEY             "sexophobia69"
#define LIGHT_ID            "CB-420 light#2"           // Unique light identifier
#define DEVICE_ID           "ESP32-001"                // Device identifier

// Fault detection thresholds (voltage levels)
#define NORMAL_MIN_VOLTAGE      1.0     // Minimum normal operating voltage
#define NORMAL_MAX_VOLTAGE      3.0     // Maximum normal operating voltage
#define SHORT_CIRCUIT_VOLTAGE   0.3     // Below this = short circuit
#define OPEN_CIRCUIT_VOLTAGE    3.2     // Above this = open circuit
#define OVERVOLTAGE_THRESHOLD   3.1     // Overvoltage detection

static const char *TAG = "FAULT_DETECTOR";
static bool wifi_connected = false;
static float last_voltage = 0.0;
static char current_fault[32] = "normal";

// Fault types
typedef enum {
    FAULT_NORMAL = 0,
    FAULT_SHORT_CIRCUIT,
    FAULT_OPEN_CIRCUIT,
    FAULT_OVERVOLTAGE,
    FAULT_UNDERVOLTAGE,
    FAULT_FLUCTUATION
} fault_type_t;

// --- Get current timestamp in ISO 8601 format ---
void get_iso_timestamp(char *timestamp, size_t size) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    gmtime_r(&now, &timeinfo);
    strftime(timestamp, size, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
}

// --- Detect fault type based on voltage reading ---
fault_type_t detect_fault_type(float voltage, float prev_voltage) {
    // Short circuit detection
    if (voltage < SHORT_CIRCUIT_VOLTAGE) {
        return FAULT_SHORT_CIRCUIT;
    }
    
    // Open circuit detection
    if (voltage > OPEN_CIRCUIT_VOLTAGE) {
        return FAULT_OPEN_CIRCUIT;
    }
    
    // Overvoltage detection
    if (voltage > OVERVOLTAGE_THRESHOLD && voltage <= OPEN_CIRCUIT_VOLTAGE) {
        return FAULT_OVERVOLTAGE;
    }
    
    // Undervoltage detection
    if (voltage < NORMAL_MIN_VOLTAGE && voltage >= SHORT_CIRCUIT_VOLTAGE) {
        return FAULT_UNDERVOLTAGE;
    }
    
    // Voltage fluctuation detection (sudden change > 0.5V)
    if (prev_voltage > 0 && fabs(voltage - prev_voltage) > 0.5) {
        return FAULT_FLUCTUATION;
    }
    
    // Normal operation
    if (voltage >= NORMAL_MIN_VOLTAGE && voltage <= NORMAL_MAX_VOLTAGE) {
        return FAULT_NORMAL;
    }
    
    return FAULT_NORMAL;
}

// --- Convert fault type to string ---
const char* fault_type_to_string(fault_type_t fault) {
    switch (fault) {
        case FAULT_SHORT_CIRCUIT: return "short_circuit";
        case FAULT_OPEN_CIRCUIT: return "open_circuit";
        case FAULT_OVERVOLTAGE: return "overvoltage";
        case FAULT_UNDERVOLTAGE: return "undervoltage";
        case FAULT_FLUCTUATION: return "voltage_fluctuation";
        case FAULT_NORMAL: return "normal";
        default: return "unknown";
    }
}

// --- HTTP response handler ---
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP Response: %.*s", evt->data_len, (char*)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

// --- Send fault data to server ---
void send_fault_data(const char* fault_type, const char* light_id, const char* timestamp) {
    if (!wifi_connected) {
        ESP_LOGW(TAG, "WiFi not connected, skipping fault report");
        return;
    }

    // Create JSON payload
    cJSON *json = cJSON_CreateObject();
    cJSON *fault_type_json = cJSON_CreateString(fault_type);
    cJSON *light_id_json = cJSON_CreateString(light_id);
    cJSON *timestamp_json = cJSON_CreateString(timestamp);
    
    cJSON_AddItemToObject(json, "fault-type", fault_type_json);
    cJSON_AddItemToObject(json, "light-ID", light_id_json);
    cJSON_AddItemToObject(json, "timestamp", timestamp_json);
    
    char *json_string = cJSON_Print(json);
    
    ESP_LOGI(TAG, "Sending fault data: %s", json_string);
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = FAULT_API_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // Set headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "x-api-key", API_KEY);
    
    // Set POST data
    esp_http_client_set_post_field(client, json_string, strlen(json_string));
    
    // Perform request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d", status_code);
        if (status_code == 200 || status_code == 201) {
            ESP_LOGI(TAG, "Fault report sent successfully");
        } else {
            ESP_LOGW(TAG, "Server returned status code: %d", status_code);
        }
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    
    // Cleanup
    esp_http_client_cleanup(client);
    free(json_string);
    cJSON_Delete(json);
}

// --- Read ADC and detect faults ---
void adc_fault_detection_task(void *pvParameters) {
    // Configure ADC using new API
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));
    
    ESP_LOGI(TAG, "Starting fault detection on GPIO34 (ADC1_CH6)");
    ESP_LOGI(TAG, "Light ID: %s", LIGHT_ID);
    ESP_LOGI(TAG, "Device ID: %s", DEVICE_ID);
    
    while (1) {
        // Read ADC value
        int adc_reading = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_reading));
        float voltage = (adc_reading / 4095.0) * 3.3;  // Convert to voltage
        
        // Detect fault type
        fault_type_t current_fault_type = detect_fault_type(voltage, last_voltage);
        const char* fault_string = fault_type_to_string(current_fault_type);
        
        // Only send data when fault status changes or every 30 seconds for normal operation
        static TickType_t last_report_time = 0;
        TickType_t current_time = xTaskGetTickCount();
        bool should_report = false;
        
        // Check if fault status changed
        if (strcmp(current_fault, fault_string) != 0) {
            strcpy(current_fault, fault_string);
            should_report = true;
            ESP_LOGI(TAG, "Fault status changed to: %s (%.3fV)", fault_string, voltage);
        }
        // Or report every 30 seconds if normal
        else if (current_fault_type == FAULT_NORMAL && 
                 (current_time - last_report_time) > (30000 / portTICK_PERIOD_MS)) {
            should_report = true;
        }
        // Or report every 5 seconds for any fault condition
        else if (current_fault_type != FAULT_NORMAL && 
                 (current_time - last_report_time) > (5000 / portTICK_PERIOD_MS)) {
            should_report = true;
        }
        
        if (should_report) {
            // Get timestamp
            char timestamp[32];
            get_iso_timestamp(timestamp, sizeof(timestamp));
            
            // Send fault data
            send_fault_data(fault_string, LIGHT_ID, timestamp);
            last_report_time = current_time;
            
            // Log the reading
            ESP_LOGI(TAG, "ADC: %d, Voltage: %.3fV, Fault: %s, Time: %s", 
                     adc_reading, voltage, fault_string, timestamp);
        }
        
        last_voltage = voltage;
        vTaskDelay(pdMS_TO_TICKS(1000));  // Check every second
    }
}

// --- WiFi event handler ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        ESP_LOGI(TAG, "Connected to WiFi - Ready for fault detection");
        
        // Initialize SNTP for accurate timestamps
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();
    }
}

// --- Initialize WiFi ---
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

// --- Main application ---
void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP32 Fault Detection System Starting...");
    ESP_LOGI(TAG, "Target API: %s", FAULT_API_URL);
    ESP_LOGI(TAG, "Light ID: %s", LIGHT_ID);
    
    // Initialize WiFi
    wifi_init();
    
    // Wait for WiFi connection
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Start fault detection task
    xTaskCreatePinnedToCore(adc_fault_detection_task, "adc_fault_task", 4096, NULL, 5, NULL, 1);
    
    ESP_LOGI(TAG, "Fault detection system initialized");
}
