#include "mq135.h"
#include "esp_log.h"
#include "hal/adc_types.h"

static const char *TAG = "MQ135";

float mq135_adc_to_ppm(int adc_raw) {
    float voltage = ((float)adc_raw / 4095.0f) * 3.3f;
    
    if (voltage <= 1.5f) return 0;
    if (voltage <= 2.5f) return (voltage - 1.5f) * 200.0f;
    if (voltage <= 3.0f) return 200.0f + (voltage - 2.5f) * 800.0f;
    if (voltage <= 3.3f) return 600.0f + (voltage - 3.0f) * 1333.0f;

    return 1000.0f; 
}

void mq135_init(adc_oneshot_unit_handle_t *adc_handle) {
		esp_err_t res;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
				.ulp_mode = ADC_ULP_MODE_DISABLE
    };
    res = adc_oneshot_new_unit(&init_config, adc_handle);
		if (res == ESP_OK) {
			ESP_LOGI(TAG, "Unit initialized successfully");
		} else {
			ESP_LOGE(TAG, "Unit fails to initialize");
		}

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    res = adc_oneshot_config_channel(*adc_handle, MQ135_ADC_CHANNEL, &chan_config);
		if (res == ESP_OK) {
			ESP_LOGI(TAG, "Channel configured successfully");
		} else {
			ESP_LOGE(TAG, "Could not configure");
		}
}

int mq135_read(adc_oneshot_unit_handle_t adc_handle) {
    int adc_raw = 0;
    adc_oneshot_read(adc_handle, MQ135_ADC_CHANNEL, &adc_raw);
    ESP_LOGI(TAG, "MQ135 ADC Value: %d", adc_raw);
    return adc_raw;
}
