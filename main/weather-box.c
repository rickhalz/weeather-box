#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_system.h"
#include "font8x8_basic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include "sh1106.h"
#include "mq135.h"
#include "shtc3.h"

adc_oneshot_unit_handle_t adc_handle;

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

  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &oled_config, &oled_dev_handle));
	ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &shtc3_config, &shtc3_dev_handle));

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

void app_main(void) {
  SH1106_t dev;
	SHTC3_t dev1;
// 	mq135_init(&adc_handle);
  i2c_master_init(&dev, &dev1, GPIO_NUM_21, GPIO_NUM_22);
  sh1106_init(&dev, 128, 64);
  sh1106_clearScreen(&dev, false);
  sh1106_setContrast(&dev, 0xff);

	sh1106_displayText(&dev, 3, "LUONG NGU", 9, false);
	while (1) {
		SHTC3_readTRH(&dev1);
		printf("Temperature: %.2f°C, Humidity: %.2f%%\n", dev1._temp, dev1._rh);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	};
// 		char data[13];
//
// 	while (0) {
// 		int air_quality = mq135_read(adc_handle);
// 		snprintf(data, sizeof(data), "   %d", air_quality);
// 		sh1106_displayText(&dev, 3, data, 12, false);
// 		vTaskDelay(pdMS_TO_TICKS(1000));
// 	}
}
