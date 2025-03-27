#include <stdint.h>
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"

#define SHTC3_I2C_ADDRESS 0x70

// Power, Sleep, Wake up
#define SHTC3_SLEEP 0xB098
#define SHTC3_WAKE_UP 0x3517

// Measurements (normal mode)
#define SHTC3_TEMP_FIRST_N 0x7866
#define SHTC3_RH_FIRST_N 0x58E0
#define SHTC3_TEMP_FIRST_N_CS 0x7CA2
#define SHTC3_RH_FIRST_N_CS 0x5C24

// Measurements (low power mode)
#define SHTC3_TEMP_FIRST_LP 0x609C
#define SHTC3_RH_FIRST_LP 0x401A
#define SHTC3_TEMP_FIRST_LP_CS 0x6458
#define SHTC3_RH_FIRST_LP_CS 0x44DE

typedef struct {
  int _address;
	float _temp;
	float _rh;
  i2c_port_t _i2c_num;
  i2c_master_bus_handle_t i2c_bus_handle;
  i2c_master_dev_handle_t i2c_dev_handle;
} SHTC3_t;

esp_err_t SHTC3_sendCMD(SHTC3_t *dev, uint16_t cmd);
void SHTC3_readTRH(SHTC3_t *dev);
float SHTC3_getTEMP(SHTC3_t *dev);
float SHTC3_getRH(SHTC3_t *dev);
void SHTC3_softReset(uint16_t cmd);
