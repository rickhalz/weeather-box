#include "shtc3.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

float SHTC3_getTEMP(SHTC3_t *dev) { return dev->_temp; }
float SHTC3_getRH(SHTC3_t *dev) { return dev->_rh; }

esp_err_t SHTC3_sendCMD(SHTC3_t *dev, uint16_t cmd) {
	uint8_t buffer[2] = { cmd >> 8, cmd & 0x00FF };
	esp_err_t err = i2c_master_transmit(dev->i2c_dev_handle, buffer, 2, 100);
	return err;
};

void SHTC3_readTRH(SHTC3_t *dev) {
  uint8_t data[6];
  uint16_t temp, rh;

	ESP_ERROR_CHECK(SHTC3_sendCMD(dev, SHTC3_WAKE_UP));
	    vTaskDelay(pdMS_TO_TICKS(10));

	ESP_ERROR_CHECK(SHTC3_sendCMD(dev, SHTC3_TEMP_FIRST_N));
	vTaskDelay(pdMS_TO_TICKS(100));

	ESP_ERROR_CHECK(i2c_master_receive(dev->i2c_dev_handle, data, 6, 100));

	ESP_ERROR_CHECK(SHTC3_sendCMD(dev, SHTC3_SLEEP));

	temp = (data[0] << 8) | data[1];
	rh = (data[3] << 8) | data[4];

	dev->_temp = 175.0 * ((float)temp / 65535.0) - 45.0;
	dev->_rh = 100.0 * ((float)rh / 65535.0);
}
