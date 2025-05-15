// #include "bmp280.h"
#include <stdio.h>
#include <string.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_system.h"
#include "font8x8_basic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "mq135.h"
#include "portmacro.h"
#include "sh1106.h"
#include "shtc3.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "wifi_sta.h"

#define MQTT_BROKER_URI	"mqtt://mqtt3.thingspeak.com"

esp_mqtt_client_handle_t client = NULL;
static const char *TAG = "MQTT";

adc_oneshot_unit_handle_t adc_handle;
char dataAQ[16], dataTemp[16], dataRH[16];
int lenAQ = 0, lenTemp = 0, lenRH = 0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
	esp_mqtt_event_handle_t event = event_data;

	switch((esp_mqtt_event_id_t)event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "Connected to MQTT broker");
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGE(TAG, "MQTT error occurred");
      break;

    default:
      break;
	}
}

static void mqtt_app_start(void) {
		esp_mqtt_client_config_t mqtt_cfg = {
			.broker.address.uri = MQTT_BROKER_URI,
			.credentials = {
				.username = CONFIG_MQTT_USERNAME,
				.client_id = CONFIG_MQTT_CLIENT_ID,
				.authentication.password = CONFIG_MQTT_PASSWORD,
			}

		};
	 client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
		    ESP_LOGI(TAG, "MQTT client started");
}

void send_to_thingspeak(float temperature, float humidity) {
    char topic[128], payload[128];
    snprintf(topic, sizeof(topic), "channels/%s/publish", CONFIG_MQTT_CHANNEL_ID);
    snprintf(payload, sizeof(payload), "field1=%.1f&field2=%.1f", temperature, humidity);
    esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "Sent to MQTT: %s -> %s", topic, payload);
}

i2c_master_bus_handle_t i2c_bus_init(uint8_t sda_io, uint8_t scl_io) {
  i2c_master_bus_config_t i2c_bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = sda_io,
      .scl_io_num = scl_io,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
  ESP_LOGI("test", "I2C master bus created");
  return bus_handle;
}

void i2c_master_init(SH1106_t *oled, SHTC3_t *shtc3, uint8_t sda, uint8_t scl) {
  i2c_master_bus_config_t i2c_config = {.clk_source = I2C_CLK_SRC_DEFAULT,
                                        .glitch_ignore_cnt = 7,
                                        .i2c_port = I2C_NUM_0,
                                        .scl_io_num = scl,
                                        .sda_io_num = sda,
                                        .flags.enable_internal_pullup = true};

  i2c_master_bus_handle_t i2c_bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &i2c_bus_handle));

  i2c_device_config_t oled_config = {
      .dev_addr_length = I2C_ADDR_BIT_7,
      .device_address = OLED_I2C_ADDRESS,
      .scl_speed_hz = 100000,
  };

  i2c_device_config_t shtc3_config = {
      .dev_addr_length = I2C_ADDR_BIT_7,
      .device_address = SHTC3_I2C_ADDRESS,
      .scl_speed_hz = 100000,
  };

  i2c_master_dev_handle_t shtc3_dev_handle;
  i2c_master_dev_handle_t oled_dev_handle;

  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &oled_config,
                                            &oled_dev_handle));
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &shtc3_config,
                                            &shtc3_dev_handle));

  oled->_address = OLED_I2C_ADDRESS;
  oled->_flip = false;
  oled->_i2c_num = I2C_NUM_0;
  oled->i2c_bus_handle = i2c_bus_handle;
  oled->i2c_dev_handle = oled_dev_handle;

  shtc3->_address = SHTC3_I2C_ADDRESS;
  shtc3->_i2c_num = I2C_NUM_0;
  shtc3->i2c_bus_handle = i2c_bus_handle;
  shtc3->i2c_dev_handle = shtc3_dev_handle;
}

void esp_task(void *arg) {
    SH1106_t dev;
    SHTC3_t dev1;
    adc_oneshot_unit_handle_t adc_handle;

    char dataAQ[16], dataTemp[16], dataRH[16];
    int lenAQ = 0, lenTemp = 0, lenRH = 0;

    // === Init all peripherals ===
    mq135_init(&adc_handle);
    i2c_master_init(&dev, &dev1, GPIO_NUM_21, GPIO_NUM_22);
    sh1106_init(&dev, 128, 64);
    sh1106_clearScreen(&dev, false);
    sh1106_setContrast(&dev, 0xFF);
    sh1106_displayText(&dev, 0, " weather data", 13, false);

    // === Main loop ===
    while (1) {
        // Draw circle as marker
        sh1106_drawCircle(&dev, 93, 29, 2, false);
        sh1106_showBuffer(&dev);

        // Read sensors
        int air_quality = mq135_read(adc_handle);
        SHTC3_readTRH(&dev1);  // dev1._temp and dev1._rh

        // Format text
        snprintf(dataAQ, sizeof(dataAQ), " AQ: %d", air_quality);
        lenAQ = strlen(dataAQ);
        snprintf(dataTemp, sizeof(dataTemp), " Temp: %.1f C", dev1._temp);
        lenTemp = strlen(dataTemp);
        snprintf(dataRH, sizeof(dataRH), " RH: %.1f", dev1._rh);
        lenRH = strlen(dataRH);

        // Display text on OLED
        sh1106_displayText(&dev, 3, dataAQ, lenAQ, false);
        sh1106_displayText(&dev, 4, dataTemp, lenTemp, false);
        sh1106_displayText(&dev, 5, dataRH, lenRH, false);

        // Publish to MQTT
				send_to_thingspeak(dev1._temp, dev1._rh);

        vTaskDelay(pdMS_TO_TICKS(10000));  // 3 seconds delay
    }
}

void app_main(void) {
    // initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();
		mqtt_app_start();

		xTaskCreate(esp_task, "ESP Task", 4096, NULL, 5, NULL);
}
