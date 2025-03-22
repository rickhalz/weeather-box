#include "mq135.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MQ135_MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Starting MQ-135 project...");
    mq135_init();

    while (1) {
        int air_quality = mq135_read();
        if (air_quality) {
            ESP_LOGI(TAG, "Air Quality: GOOD");
        } else {
            ESP_LOGW(TAG, "Air Quality: BAD");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // Read every 1 second
    }
}

