#ifndef _BMP280_H_
#define _BMP280_H_
#endif
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "sdkconfig.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BMPAPI extern

typedef enum bmp280_tsmpl_t {
    BMP280_TEMPERATURE_OVERSAMPLING_NONE = 0x0,
    BMP280_TEMPERATURE_OVERSAMPLING_X1,
    BMP280_TEMPERATURE_OVERSAMPLING_X2,
    BMP280_TEMPERATURE_OVERSAMPLING_X4,
    BMP280_TEMPERATURE_OVERSAMPLING_X8,
    BMP280_TEMPERATURE_OVERSAMPLING_X16,
} bmp280_tsmpl_t;

typedef enum bmp280_psmpl_t {
    BMP280_PRESSURE_OVERSAMPLING_NONE = 0x0,
    BMP280_PRESSURE_OVERSAMPLING_X1,
    BMP280_PRESSURE_OVERSAMPLING_X2,
    BMP280_PRESSURE_OVERSAMPLING_X4,
    BMP280_PRESSURE_OVERSAMPLING_X8,
    BMP280_PRESSURE_OVERSAMPLING_X16,
} bmp280_psmpl_t;

typedef enum bmp280_tstby_t {
    BMP280_STANDBY_0M5 = 0x0,
    BMP280_STANDBY_62M5,
    BMP280_STANDBY_125M,
    BMP280_STANDBY_250M,
    BMP280_STANDBY_500M,
    BMP280_STANDBY_1000M,
    BMP280_STANDBY_2000M, 
    BMP280_STANDBY_4000M, 
} bmp280_tstby_t;

typedef enum bmp280_iirf_t {
    BMP280_IIR_NONE = 0x0,
    BMP280_IIR_X1,
    BMP280_IIR_X2,
    BMP280_IIR_X4,
    BMP280_IIR_X8,
    BMP280_IIR_X16,
} bmp280_iirf_t;

typedef enum bmp280_mode_t {
    /** Sensor does no measurements. */
    BMP280_MODE_SLEEP = 0,
    /** Sensor is in a forced measurement cycle. Sleeps after finishing. */
    BMP280_MODE_FORCE = 1,
    /** Sensor does measurements. Never sleeps. */
    BMP280_MODE_CYCLE = 3,
} bmp280_mode_t;

typedef struct bmp280_config_t {
    bmp280_tsmpl_t t_sampling;
    bmp280_psmpl_t p_sampling;
    bmp280_tstby_t t_standby;
    bmp280_iirf_t iir_filter;
} bmp280_config_t;

#if (CONFIG_BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING_NONE)
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_NONE
#elif (CONFIG_BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING_X1)
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_X1
#elif (CONFIG_BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING_X2)
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_X2
#elif (CONFIG_BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING_X4)
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_X4
#elif (CONFIG_BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING_X8)
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_X8
#else
#define BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING BMP280_TEMPERATURE_OVERSAMPLING_X16
#endif

#if CONFIG_BMP280_DEFAULT_PRESSURE_OVERSAMPLING_NONE
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_NONE
#elif CONFIG_BMP280_DEFAULT_PRESSURE_OVERSAMPLING_X1
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_X1
#elif CONFIG_BMP280_DEFAULT_PRESSURE_OVERSAMPLING_X2
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_X2
#elif CONFIG_BMP280_DEFAULT_PRESSURE_OVERSAMPLING_X4
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_X4
#elif CONFIG_BMP280_DEFAULT_PRESSURE_OVERSAMPLING_X8
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_X8
#else
#define BMP280_DEFAULT_PRESSURE_OVERSAMPLING BMP280_PRESSURE_OVERSAMPLING_X16
#endif

#if (CONFIG_BMP280_DEFAULT_STANDBY_0M5)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_0M5
#elif (CONFIG_BMP280_DEFAULT_STANDBY_62M5)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_62M5
#elif (CONFIG_BMP280_DEFAULT_STANDBY_125M)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_125M
#elif (CONFIG_BMP280_DEFAULT_STANDBY_250M)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_250M
#elif (CONFIG_BMP280_DEFAULT_STANDBY_500M)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_500M
#elif (CONFIG_BMP280_DEFAULT_STANDBY_1000M)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_1000M
#elif (CONFIG_BMP280_DEFAULT_STANDBY_2000M)
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_2000M
#else
#define BMP280_DEFAULT_STANDBY BMP280_STANDBY_4000M
#endif

#if (CONFIG_BMP280_DEFAULT_IIR_NONE)
#define BMP280_DEFAULT_IIR BMP280_IIR_NONE
#elif (CONFIG_BMP280_DEFAULT_IIR_X2)
#define BMP280_DEFAULT_IIR BMP280_IIR_X2
#elif (CONFIG_BMP280_DEFAULT_IIR_X4)
#define BMP280_DEFAULT_IIR BMP280_IIR_X4
#elif (CONFIG_BMP280_DEFAULT_IIR_X8)
#define BMP280_DEFAULT_IIR BMP280_IIR_X8
#else
#define BMP280_DEFAULT_IIR BMP280_IIR_X16
#endif
#define BMP280_DEFAULT_CONFIG ((bmp280_config_t) { BMP280_DEFAULT_TEMPERATURE_OVERSAMPLING, BMP280_DEFAULT_PRESSURE_OVERSAMPLING, BMP280_DEFAULT_STANDBY, BMP280_DEFAULT_IIR})

//End define bits

typedef struct bmp280_t bmp280_t;

BMPAPI bmp280_t* bmp280_create_master(i2c_master_bus_handle_t bus_handle);
// legacy define for existing code bases
#define bmp280_create(port) bmp280_create_legacy(port)
#define bmp280_create_legacy(port) static_assert(0, "You have the wrong driver configuration for using the legacy I2C driver.")


/* Destroy your the instance.*/
BMPAPI void bmp280_close(bmp280_t* bmp280);
/*Probe for the sensor and read calibration data. */
BMPAPI esp_err_t bmp280_init(bmp280_t* bmp280);
/* Configure the sensor with the given parameters.*/
BMPAPI esp_err_t bmp280_configure(bmp280_t* bmp280, bmp280_config_t *cfg);
/* Set the sensor mode of operation. */
BMPAPI esp_err_t bmp280_setMode(bmp280_t* bmp280, bmp280_mode_t mode);
/* Get the sensor current mode of operation. */
BMPAPI esp_err_t bmp280_getMode(bmp280_t* bmp280, bmp280_mode_t* mode);
/* Returns true if sensor is currently sampling environment conditions. */
BMPAPI bool bmp280_isSampling(bmp280_t* bmp280);

BMPAPI esp_err_t bmp280_readout(bmp280_t *bmp280, int32_t *temperature, uint32_t *pressure);

/**
 * Convert sensor readout to floating point values.
 * @param tin Input temperature.
 * @param pin Input pressure.
 * @param tout Output temperature. (C)
 * @param pout Output pressure. (Pa)
 */
static inline void bmp280_readout2float(int32_t* tin, uint32_t *pin, float *tout, float *pout)
{
    if (tin && tout)
        *tout = (float)*tin * 0.01f;
    if (pin && pout)
        *pout = (float)*pin * (1.0f/256.0f);
   }
 
static inline esp_err_t bmp280_readoutFloat(bmp280_t *bmp280, float* temperature, float* pressure)
{
    int32_t t; uint32_t p;
    esp_err_t err = bmp280_readout(bmp280, &t, &p);

    if (err == ESP_OK)
    {
        bmp280_readout2float(&t, &p, temperature, pressure);
    }

    return err;
}

void func(void);