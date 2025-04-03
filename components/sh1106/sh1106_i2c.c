#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/i2c_types.h"
#include "sh1106.h"

#define TAG "SSD1306"

void i2c_device_add(SH1106_t *dev, i2c_port_t i2c_num, uint16_t i2c_address) {
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = i2c_address,
      .scl_speed_hz = 100000,
  };
  i2c_master_dev_handle_t i2c_dev_handle;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(dev->i2c_bus_handle, &dev_cfg,
                                            &i2c_dev_handle));

  dev->_address = i2c_address;
  dev->_flip = false;
  dev->_i2c_num = i2c_num;
  dev->i2c_dev_handle = i2c_dev_handle;
}

void i2c_init(SH1106_t *dev, int width, int height) {
  dev->_width = width;
  dev->_height = height;
  dev->_pages = 8;
  uint8_t buffer[24];
  int index = 0;
  buffer[index++] = OLED_CONTROL_CMD;
  buffer[index++] = OLED_DISPLAY_OFF;
  buffer[index++] = OLED_SET_MUX_RATIO;
  buffer[index++] = 0x3F;
  buffer[index++] = OLED_SET_DISPLAY_OFFSET;
  buffer[index++] = 0x00;
  buffer[index++] = OLED_CONTROL_DATA;
  buffer[index++] = OLED_SET_DISPLAY_START_LINE;
  if (dev->_flip) {
    buffer[index++] = OLED_SET_SEGMENT_REMAP_0;
  } else {
    buffer[index++] = OLED_SET_SEGMENT_REMAP_1;
  }
  buffer[index++] = OLED_SET_COM_SCAN_MODE;
  buffer[index++] = OLED_SET_DISPLAY_CLK_DIV;
  buffer[index++] = 0x50;
  buffer[index++] = OLED_SET_COM_PADS_MAP;
  buffer[index++] = 0x12;
  buffer[index++] = OLED_SET_CONTRAST;
  buffer[index++] = OLED_DISPLAY_RAM;
  buffer[index++] = OLED_VCOM_DESELECT;
  buffer[index++] = 0x30;
  buffer[index++] = OLED_SET_PAGE_ADDR;
  buffer[index++] = OLED_SET_LOWER_COLUMN_ADDR;
  buffer[index++] = OLED_SET_HIGHER_COLUMN_ADDR;
  buffer[index++] = OLED_SET_CHARGE_PUMP;
  buffer[index++] = OLED_DISPLAY_NORMAL;
  buffer[index++] = OLED_DISPLAY_ON;

  esp_err_t res;
  res = i2c_master_transmit(dev->i2c_dev_handle, buffer, index, 100);
  if (res == ESP_OK) {
    ESP_LOGI(TAG, "OLED configured successfully");
  } else {
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             dev->_address, dev->_i2c_num, res, esp_err_to_name(res));
  }
}

void i2c_display_image(SH1106_t *dev, int page, int seg, uint8_t *images,
                       int width) {
  if (page >= dev->_pages)
    return;
  if (seg >= dev->_width)
    return;
  int _seg = seg;
  uint8_t columnLower = _seg & 0x0F;
  uint8_t columnHigher = (_seg >> 4) & 0x0F;

  int _page = page;
  if (dev->_flip)
    _page = (dev->_pages - page) - 1;

  uint8_t *buffer;
  int index = 0;
  buffer = malloc(width < 4 ? 4 : width + 1);

  buffer[index++] = OLED_CONTROL_CMD;
  buffer[index++] = (0x00 + columnLower);
  buffer[index++] = (0x10 + columnHigher);
  buffer[index++] = 0xB0 | _page;

  esp_err_t res;
  res = i2c_master_transmit(dev->i2c_dev_handle, buffer, index, 100);
  if (res != ESP_OK)
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             dev->_address, dev->_i2c_num, res, esp_err_to_name(res));

  buffer[0] = OLED_CONTROL_DATA;
  memcpy(&buffer[1], images, width);

  res = i2c_master_transmit(dev->i2c_dev_handle, buffer, width + 1, 100);
  if (res != ESP_OK)
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             dev->_address, dev->_i2c_num, res, esp_err_to_name(res));
  free(buffer);
}

void i2c_contrast(SH1106_t *dev, int contrast) {
  uint8_t _contrast = contrast;
  if (contrast < 0x0)
    _contrast = 0;
  if (contrast > 0xFF)
    _contrast = 0xFF;

  uint8_t out_buf[3];
  int out_index = 0;
  out_buf[out_index++] = OLED_CONTROL_CMD;  // 00
  out_buf[out_index++] = OLED_SET_CONTRAST; // 81
  out_buf[out_index++] = _contrast;

  esp_err_t res = i2c_master_transmit(dev->i2c_dev_handle, out_buf, 3, 100);
  if (res != ESP_OK)
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             dev->_address, dev->_i2c_num, res, esp_err_to_name(res));
}
