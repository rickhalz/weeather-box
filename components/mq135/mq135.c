#include "mq135.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MQ135";

void mq135_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MQ135_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "MQ-135 initialized on GPIO %d", MQ135_GPIO);
}

int mq135_read(void) {
    return gpio_get_level(MQ135_GPIO);  // 1 = Clean air, 0 = Poor air quality
}

