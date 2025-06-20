#include "bmp280.h"
#include "esp_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Register address of temperature fraction significant byte.
#define BMP280_REG_TEMP_XLSB 0xFC
// Register address of temperature least significant byte.
#define BMP280_REG_TEMP_LSB 0xFB
// Register address of temperature most significant byte.
#define BMP280_REG_TEMP_MSB 0xFA
// Register address of pressure fraction significant byte.
#define BMP280_REG_PRES_XLSB 0xF9
// Register address of pressure least significant byte.
#define BMP280_REG_PRES_LSB 0xF8
// Register address of pressure most significant byte.
#define BMP280_REG_PRES_MSB 0xF7
// Register address of sensor configuration.
#define BMP280_REG_CONFIG 0xF5
// Register address of sensor measurement control.
#define BMP280_REG_MESCTL 0xF4
// Register address of sensor status.
#define BMP280_REG_STATUS 0xF3
// Register address of calibration constants. (high bank)
#define BMP280_REG_CAL_HI 0xE1
// Register address of calibration constants. (low bank)
#define BMP280_REG_CAL_LO 0x88
// Register address for sensor reset.
#define BMP280_REG_RESET 0xE0
// Chip reset vector.
#define BMP280_RESET_VEC 0xB6
// Register address for chip identification number.
#define BMP280_REG_CHPID 0xD0
// Value of REG_CHPID for BMP280 (Engineering Sample 1)
#define BMP280_ID0 0x56
// Value of REG_CHPID for BMP280 (Engineering Sample 2)
#define BMP280_ID1 0x57
// Value of REG_CHPID for BMP280 (Production)
#define BMP280_ID2 0x58

struct bmp280_t{
    // I2C master handle via port with configuration
    i2c_master_dev_handle_t i2c_dev;
    // I2C master configuration
    i2c_device_config_t dev_cfg;
    // I2C master handle via port
    i2c_master_bus_handle_t bus_handle;
    // Chip ID of sensor
    uint8_t chip_id;
    // Compensation data
    struct {
        uint16_t T1;
        int16_t T2;
        int16_t T3;
        uint16_t P1;
        int16_t P2;
        int16_t P3;
        int16_t P4;
        int16_t P5;
        int16_t P6;
        int16_t P7;
        int16_t P8;
        int16_t P9;
    } cmps;
    // Storage for a variable proportional to temperature.
    int32_t t_fine;
};
//get_id 
#define bmp280_verify(chip_id) ( ((chip_id) == BMP280_ID2) || ((chip_id) == BMP280_ID1) || ((chip_id) == BMP280_ID0))
#define bmp280_validate(bmp280) (!(bmp280->i2c_dev == NULL && bmp280->chip_id == 0xAD))
//device create 

static esp_err_t bmp280_device_create(bmp280_t *bmp280, const uint16_t dev_addr)
{
    ESP_LOGI("bmp280", "device_create for BMP280 sensors on ADDR %X", dev_addr);
    bmp280->dev_cfg.device_address = dev_addr;
    // Add device to the I2C bus
    esp_err_t err = i2c_master_bus_add_device(bmp280->bus_handle, &bmp280->dev_cfg, &bmp280->i2c_dev);
    if (err == ESP_OK)
    {
        ESP_LOGI("bmp280", "device_create success on 0x%x", dev_addr);
    }
    return err;
}

//read data 
#define BMP280_TIMEOUT 50
static esp_err_t bmp280_read(bmp280_t *bmp280, uint8_t addr, uint8_t *dout, size_t size)
{
    return i2c_master_transmit_receive(bmp280->i2c_dev, &addr, sizeof(addr), dout, size, BMP280_TIMEOUT);
}

static esp_err_t bmp280_write(bmp280_t* bmp280, uint8_t addr, const uint8_t *din, size_t size)
{
    esp_err_t err;
    for(uint8_t i = 0; i < size; i++)
    {
        uint8_t dat[2] = {(addr + i), din[i]};
        if ((err = i2c_master_transmit(bmp280->i2c_dev, dat, 2, BMP280_TIMEOUT)) != ESP_OK)
            return err;
    }
    return ESP_OK;
}

//probe address 
static esp_err_t bmp280_probe_address(bmp280_t *bmp280)
{
    esp_err_t err = bmp280_read(bmp280, BMP280_REG_CHPID, &bmp280->chip_id, sizeof bmp280->chip_id);
    if (err == ESP_OK)
    {
        if (
        #if CONFIG_BMP280_EXPECT_BMP280
            bmp280->chip_id == BMP280_ID2 || bmp280->chip_id == BMP280_ID1 || bmp280->chip_id == BMP280_ID0
        #else
            bmp280_verify(bmp280->chip_id)
        #endif
        )
        { 
            ESP_LOGI("bmp280", "Probe success: address=%hhx, id=%hhx", bmp280->dev_cfg.device_address, bmp280->chip_id);
            return ESP_OK;
        }
        else
        {
            ESP_LOGE("bmp280", "Sensor model may be incorrect. Please check the sensor model configuration.");
            err = ESP_ERR_NOT_FOUND;
        }
    } 
    ESP_LOGW("bmp280", "Probe failure: address=%hhx, id=%hhx, reason=%s", bmp280->dev_cfg.device_address, bmp280->chip_id, esp_err_to_name(err));
    return err;
}

static esp_err_t bmp280_probe(bmp280_t *bmp280)
{
    ESP_LOGI("bmp280", "Probing for BMP280/BME280 sensors on I2C");
    esp_err_t err;
    #if CONFIG_BMP280_ADDRESS_HI
    err = bmp280_device_create(bmp280, 0x77);
    if (err != ESP_OK) return err;
    err = bmp280_probe_address(bmp280);
    if (err != ESP_OK) ESP_LOGE("bmp280", "Sensor not found at 0x77 , Please check the address.");
    return err;
    #elif CONFIG_BMP280_ADDRESS_LO
    err = bmp280_device_create(bmp280, 0x76);
    if (err != ESP_OK) return err;
    err = bmp280_probe_address(bmp280);
    if (err != ESP_OK) ESP_LOGE("bmp280", "Sensor not found at 0x76 , Please check the address.");
    return err;
    #else
    err = bmp280_device_create(bmp280, 0x76);
    if (err != ESP_OK) return err;
    if ((err = bmp280_probe_address(bmp280)) != ESP_OK)
    {
        err = bmp280_device_create(bmp280, 0x77);
        if (err != ESP_OK) return err;
        if ((err = bmp280_probe_address(bmp280)) != ESP_OK)
        {
            ESP_LOGE("bmp280", "Sensor not found.");
            bmp280->i2c_dev = NULL;
            bmp280->chip_id = 0xAD;
        }
    }
    return err;
    #endif
}

static esp_err_t bmp280_reset(bmp280_t *bmp280)
{
    const static uint8_t din[] = { BMP280_RESET_VEC };
    return bmp280_write(bmp280, BMP280_REG_RESET, din, sizeof din);
}

static esp_err_t bmp280_calibrate(bmp280_t *bmp280)
{
    // Honestly, the best course of action is to read the high and low banks
    // into a buffer, then put them in the calibration values. Makes code
    // endian agnostic, and overcomes struct packing issues.
    // Also the BME280 high bank is weird.
    //
    // Write and pray to optimizations is my new motto.

    ESP_LOGI("bmp280", "Reading out calibration values...");

    esp_err_t err;
    uint8_t buf[26];

    // Low Bank
    err = bmp280_read(bmp280, BMP280_REG_CAL_LO, buf, sizeof buf);

    if (err != ESP_OK) return err;

    ESP_LOGI("bmp280", "Read Low Bank.");
    bmp280->cmps.T1 = buf[0] | (buf[1] << 8);
    bmp280->cmps.T2 = buf[2] | (buf[3] << 8);
    bmp280->cmps.T3 = buf[4] | (buf[5] << 8);
    bmp280->cmps.P1 = buf[6] | (buf[7] << 8);
    bmp280->cmps.P2 = buf[8] | (buf[9] << 8);
    bmp280->cmps.P3 = buf[10] | (buf[11] << 8);
    bmp280->cmps.P4 = buf[12] | (buf[13] << 8);
    bmp280->cmps.P5 = buf[14] | (buf[15] << 8);
    bmp280->cmps.P6 = buf[16] | (buf[17] << 8);
    bmp280->cmps.P7 = buf[18] | (buf[19] << 8);
    bmp280->cmps.P8 = buf[20] | (buf[21] << 8);
    bmp280->cmps.P9 = buf[22] | (buf[23] << 8);
    return ESP_OK;
}
#define CONFIG_BMP280_I2C_CLK_SPEED_HZ 1000000
bmp280_t* bmp280_create_master(i2c_master_bus_handle_t bus_handle)
{
    bmp280_t* bmp280 = malloc(sizeof(bmp280_t));
    if (bmp280)
    {
        memset(bmp280, 0, sizeof(bmp280_t));
        bmp280->bus_handle = bus_handle;
        bmp280->dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        bmp280->dev_cfg.device_address = 0xDE;
        bmp280->dev_cfg.scl_speed_hz = CONFIG_BMP280_I2C_CLK_SPEED_HZ;
        bmp280->i2c_dev = NULL;
        bmp280->chip_id = 0xAD;
    }
    else
    {
        ESP_LOGE("bmp280", "Failed to allocate memory for bmp280.");
        bmp280_close(bmp280);
        return NULL;
    }
    return bmp280;
}


void bmp280_close(bmp280_t *bmp280)
{
    if(bmp280 != NULL && bmp280->i2c_dev != NULL)
        i2c_master_bus_rm_device(bmp280->i2c_dev);
    free(bmp280);
}

esp_err_t bmp280_init(bmp280_t* bmp280)
{
    if (bmp280 == NULL) return ESP_ERR_INVALID_ARG;

    esp_err_t error = bmp280_probe(bmp280) || bmp280_reset(bmp280);

    if (error == ESP_OK)
    {
        // Give the sensor 10 ms delay to reset.
        vTaskDelay(pdMS_TO_TICKS(10));
        // Read calibration data.
        bmp280_calibrate(bmp280);
        ESP_LOGI("bmp280", "Dumping calibration...");
        ESP_LOG_BUFFER_HEX("bmp280", &bmp280->cmps, sizeof(bmp280->cmps));
    }

    return error;
}

esp_err_t bmp280_configure(bmp280_t* bmp280, bmp280_config_t *cfg)
{
    if (bmp280 == NULL || cfg == NULL) return ESP_ERR_INVALID_ARG;
    if (!bmp280_validate(bmp280)) return ESP_ERR_INVALID_STATE;

    // Always set ctrl_meas first.
    uint8_t num = (cfg->t_sampling << 5) | (cfg->p_sampling << 2) | BMP280_MODE_SLEEP;
    esp_err_t err = bmp280_write(bmp280, BMP280_REG_MESCTL, &num, sizeof num);

    if (err) return err;

    // We can set cfg now.
    num = (cfg->t_standby << 5) | (cfg->iir_filter << 2);
    err = bmp280_write(bmp280, BMP280_REG_CONFIG, &num, sizeof num);

    if (err) return err;
    // f = 0; 
    return ESP_OK;
}

esp_err_t bmp280_setMode(bmp280_t* bmp280, bmp280_mode_t mode)
{
    uint8_t ctrl_mes;
    esp_err_t err;

    if ((err = bmp280_read(bmp280, BMP280_REG_MESCTL, &ctrl_mes, 1)) != ESP_OK)
        return err;

    ctrl_mes = (ctrl_mes & (~3)) | mode;

    return bmp280_write(bmp280, BMP280_REG_MESCTL, &ctrl_mes, 1);
}

esp_err_t bmp280_getMode(bmp280_t* bmp280, bmp280_mode_t* mode)
{
    uint8_t ctrl_mes;
    esp_err_t err;

    if ((err = bmp280_read(bmp280, BMP280_REG_MESCTL, &ctrl_mes, 1)) != ESP_OK)
        return err;

    ctrl_mes &= 3;

    switch (ctrl_mes)
    {
    default:
        *mode = ctrl_mes; break;
    case (BMP280_MODE_FORCE + 1):
        *mode = BMP280_MODE_FORCE; break;
    }

    return ESP_OK;
}

bool bmp280_isSampling(bmp280_t* bmp280)
{
    uint8_t status;
    if (bmp280_read(bmp280, BMP280_REG_STATUS, &status, 1) == ESP_OK)
        return (status & (1 << 3)) != 0;
    else
        return false;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t BMP280_compensate_T_int32(bmp280_t *bmp280, int32_t adc_T)
{
    int32_t var1, var2, T; 
    var1 = ((((adc_T>>3) -((int32_t)bmp280->cmps.T1<<1))) * ((int32_t)bmp280->cmps.T2)) >> 11;
    var2  =(((((adc_T>>4) -((int32_t)bmp280->cmps.T1)) * ((adc_T>>4) -((int32_t)bmp280->cmps.T1))) >> 12) * ((int32_t)bmp280->cmps.T3)) >> 14;
    bmp280->t_fine = var1 + var2;
    T  = (bmp280->t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BMP280_compensate_P_int64(bmp280_t *bmp280, int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)bmp280->t_fine) -128000;
    var2 = var1 * var1 * (int64_t)bmp280->cmps.P6;
    var2 = var2 + ((var1*(int64_t)bmp280->cmps.P5)<<17);
    var2 = var2 + (((int64_t)bmp280->cmps.P4)<<35);
    var1 = ((var1 * var1 * (int64_t)bmp280->cmps.P3)>>8) + ((var1 * (int64_t)bmp280->cmps.P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)bmp280->cmps.P1)>>33;
    if(var1 == 0){ 
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)bmp280->cmps.P9) * (p>>13) * (p>>13)) >> 25;
    var2 =(((int64_t)bmp280->cmps.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)bmp280->cmps.P7)<<4);
    return (uint32_t)p;
}

esp_err_t bmp280_readout(bmp280_t *bmp280, int32_t *temperature, uint32_t *pressure)
{
    if (bmp280 == NULL) return ESP_ERR_INVALID_ARG;
    if (!bmp280_validate(bmp280)) return ESP_ERR_INVALID_STATE;

    uint8_t buffer[3];
    esp_err_t error;

    if (temperature)
    {
        if ((error = bmp280_read(bmp280, BMP280_REG_TEMP_MSB, buffer, 3)) != ESP_OK)
            return error;

        *temperature = BMP280_compensate_T_int32(bmp280,(buffer[0] << 12) | (buffer[1] << 4) | (buffer[0] >> 4));
    }

    if (pressure)
    {
        if ((error = bmp280_read(bmp280, BMP280_REG_PRES_MSB, buffer, 3)) != ESP_OK)
            return error;

        *pressure = BMP280_compensate_P_int64(bmp280,(buffer[0] << 12) | (buffer[1] << 4) | (buffer[0] >> 4));
    }
    return ESP_OK;
}

void func(void)
{

}