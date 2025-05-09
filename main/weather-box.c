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
#include "wifi_sta.h"

adc_oneshot_unit_handle_t adc_handle;

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
void app_main(void) {
  SH1106_t dev;
  SHTC3_t dev1;
  // initialize peripherals

  mq135_init(&adc_handle);
  i2c_master_init(&dev, &dev1, GPIO_NUM_21, GPIO_NUM_22);
  sh1106_init(&dev, 128, 64);
  sh1106_clearScreen(&dev, false);
  sh1106_setContrast(&dev, 0xFF);
  sh1106_displayText(&dev, 0, " weather data", 13, false);
  char dataAQ[16], dataTemp[16], dataRH[16];
  int lenAQ = 0, lenTemp = 0, lenRH = 0;

    // initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();

  while (1) {
		// display datas
    sh1106_drawCircle(&dev, 93, 29, 2, false);
    sh1106_showBuffer(&dev);
    int air_quality = mq135_read(adc_handle);
    SHTC3_readTRH(&dev1);
    snprintf(dataAQ, sizeof(dataAQ), " AQ: %d", air_quality);
    lenAQ = strlen(dataAQ);
    snprintf(dataTemp, sizeof(dataTemp), " Temp: %.1f C", dev1._temp);
    lenTemp = strlen(dataTemp);
    snprintf(dataRH, sizeof(dataRH), " RH: %.1f", dev1._rh);
    lenRH = strlen(dataRH);
    sh1106_displayText(&dev, 3, dataAQ, lenAQ, false);
    sh1106_displayText(&dev, 4, dataTemp, lenTemp, false);
    sh1106_displayText(&dev, 5, dataRH, lenRH, false);

		// MQTT part


    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}
