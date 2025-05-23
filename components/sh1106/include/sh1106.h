#ifndef SH1106_H
#define SH1106_H

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define OLED_CONTROL_CMD 0x00
#define OLED_CONTROL_DATA 0x40

// NOTE: follow with is for double bytes command

// fundamental commands
#define OLED_SET_CONTRAST 0x81 // follow with 0x00 -> 0xFF to set contrast
#define OLED_DISPLAY_RAM 0xA4  // displays output according to GDDRAM contents
#define OLED_DISPLAY_ALL_ON                                                    \
  0xA5 // forces the display to be ON regardless of GDDRAM contents
#define OLED_DISPLAY_NORMAL 0xA6
#define OLED_DISPLAY_INVERSE 0xA7
#define OLED_DISPLAY_ON 0xAF
#define OLED_DISPLAY_OFF 0xAE

// address table
#define OLED_SET_PAGE_ADDR 0xB0;
#define OLED_SET_LOWER_COLUMN_ADDR 0x00
#define OLED_SET_HIGHER_COLUMN_ADDR 0x10

// hardware config
#define OLED_SET_DISPLAY_START_LINE 0x40
#define OLED_SET_SEGMENT_REMAP_0 0xA0
#define OLED_SET_SEGMENT_REMAP_1 0xA1
#define OLED_SET_MUX_RATIO 0xA8 // follow with 0x3F = 64
#define OLED_SET_COM_SCAN_MODE 0xC8
#define OLED_SET_DISPLAY_OFFSET 0xD3 // follow with 0x00 -> 0x3F
#define OLED_SET_COM_PADS_MAP 0xDA // 0x02 for sequential - 0x12 for alternative
#define OLED_NOP 0xE3

// display timing & power control
#define OLED_SET_DISPLAY_CLK_DIV 0xD5 // follow with 0x50
#define OLED_SET_PRECHARGE 0xD9       // follow with 0x22
#define OLED_VCOM_DESELECT 0xDB       // follow with 0x30
#define OLED_SET_CHARGE_PUMP 0x8B

#define OLED_I2C_ADDRESS 0x3C

typedef struct {
  uint8_t segs[128];
} PAGE_t;

typedef struct {
  int _address;
  int _width;
  int _height;
  int _pages;
  int _dc;
  PAGE_t _page[8];
  bool _flip;
  i2c_port_t _i2c_num;
  i2c_master_bus_handle_t i2c_bus_handle;
  i2c_master_dev_handle_t i2c_dev_handle;
} SH1106_t;

void sh1106_init(SH1106_t *dev, int width, int height);
int sh1106_getWidth(SH1106_t *dev);
int sh1106_getHeight(SH1106_t *dev);
int sh1106_getPages(SH1106_t *dev);
void sh1106_showBuffer(SH1106_t *dev);
void sh1106_setBuffer(SH1106_t *dev, uint8_t *buffer);
void sh1106_getBuffer(SH1106_t *dev, uint8_t *buffer);
void sh1106_setPage(SH1106_t *dev, int page, uint8_t *buffer);
void sh1106_getPage(SH1106_t *dev, int page, uint8_t *buffer);
void sh1106_displayImage(SH1106_t *dev, int page, int seg, uint8_t *images,
                         int width);
void sh1106_displayText(SH1106_t *dev, int page, char *text, int text_len,
                        bool invert);
void sh1106_clearScreen(SH1106_t *dev, bool invert);
void sh1106_clearLine(SH1106_t *dev, int page, bool invert);
void sh1106_setContrast(SH1106_t *dev, int contrast);
void sh1106_bitmaps(SH1106_t *dev, int xpos, int ypos, uint8_t *bitmap,
                    int width, int height, bool invert);
void sh1106_drawPixel(SH1106_t *dev, int xpos, int ypos, bool invert);
void sh1106_line(SH1106_t *dev, int x1, int y1, int x2, int y2, bool invert);
void sh1106_drawCircle(SH1106_t *dev, int x0, int y0, int r, bool invert);
void sh1106_cursor(SH1106_t *dev, int x0, int y0, int r, bool invert);
void sh1106_invert(uint8_t *buf, size_t blen);
void sh1106_flip(uint8_t *buf, size_t blen);
uint8_t sh1106_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits);
uint8_t sh1106_rotate_byte(uint8_t ch1);
void sh1106_fadeout(SH1106_t *dev);
void sh1106_rotate_image(uint8_t *image, bool flip);
void sh1106_display_rotate_text(SH1106_t *dev, int seg, char *text,
                                int text_len, bool invert);
void sh1106_dump(SH1106_t dev);
void sh1106_dump_page(SH1106_t *dev, int page, int seg);

void i2c_device_add(SH1106_t *dev, i2c_port_t i2c_num, uint16_t i2c_address);
void i2c_init(SH1106_t *dev, int width, int height);
void i2c_display_image(SH1106_t *dev, int page, int seg, uint8_t *images,
                       int width);
void i2c_contrast(SH1106_t *dev, int contrast);

#endif
