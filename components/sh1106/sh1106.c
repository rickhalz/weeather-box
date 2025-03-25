#include "sh1106.h"
#include <stdio.h>

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "font8x8_basic.h"
#include "sh1106.h"

#define PACK8 __attribute__((packed))

typedef union {
  uint32_t u32;
  uint8_t u8[4];
} PACK8 out_of_column;

void sh1106_init(SH1106_t *dev, int width, int height) {
  i2c_init(dev, width, height);
  // Initialize internal buffer
  for (int i = 0; i < dev->_pages; i++) {
    memset(dev->_page[i].segs, 0, dev->_width);
  };
}

int sh1106_getWidth(SH1106_t *dev) { return dev->_width; }

int sh1106_getHeight(SH1106_t *dev) { return dev->_height; }

int sh1106_getPages(SH1106_t *dev) { return dev->_pages; }

void sh1106_displayImage(SH1106_t *dev, int page, int seg, uint8_t *images, int width) {
	i2c_display_image(dev, page, seg, images, width);
	memcpy(&dev->_page[page].segs[seg], images, width);
}

void sh1106_displayText(SH1106_t *dev, int page, char *text, int text_len, bool invert) {
	if (page >= dev->_pages) return;
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;

	int seg = 0;
	uint8_t image[8];
	for (int i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) sh1106_invert(image, 8);
		if (dev->_flip) sh1106_flip(image,8);
		sh1106_displayImage(dev, page, seg, image, 8);
		seg += 8;
	}
}

void sh1106_flip(uint8_t *buf, size_t blen)
{
	for(int i=0; i<blen; i++){
		buf[i] = sh1106_rotate_byte(buf[i]);
	}
}

uint8_t sh1106_rotate_byte(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}

void sh1106_invert(uint8_t *buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}
