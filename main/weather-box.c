#include "mq135.h"
#include "shtc3.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG_1 = "MQ135_MAIN";

#define SHTC3_SDA_GPIO           21  /*!< gpio number for I2C master data  */
#define SHTC3_SCL_GPIO           22  /*!< gpio number for I2C master clock */

static const char *TAG_2 = "SHTC3";

i2c_master_dev_handle_t shtc3_handle;

// Task to read the sensor data
void shtc3_read_task(void *pvParameters)
{
    float temperature, humidity;
    esp_err_t err = ESP_OK;
    shtc3_register_rw_t reg = SHTC3_REG_T_CSE_NM;

    while (1) {
        err = shtc3_get_th(shtc3_handle, reg, &temperature, &humidity);
        if(err != ESP_OK) {
            ESP_LOGE(TAG_2, "Failed to read data from SHTC3 sensor");
        } else {
            ESP_LOGI(TAG_2, "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

i2c_master_bus_handle_t i2c_bus_init(uint8_t sda_io, uint8_t scl_io)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = 0,
        .sda_io_num = sda_io,
        .scl_io_num = scl_io,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
    ESP_LOGI(TAG_2, "I2C master bus created");

    return bus_handle;
}

//SHTC3 setup part end

void app_main(void) {
    ESP_LOGI(TAG_1, "Starting MQ-135 project...");
    mq135_init();
    
    i2c_master_bus_handle_t bus_handle = i2c_bus_init(SHTC3_SDA_GPIO, SHTC3_SCL_GPIO);
    shtc3_handle = shtc3_device_create(bus_handle, SHTC3_I2C_ADDR, 100000);
    ESP_LOGI(TAG_2, "Sensor initialization success");

    // Probe the sensor to check if it is connected to the bus with a 10ms timeout
    esp_err_t err = i2c_master_probe(bus_handle, SHTC3_I2C_ADDR, 200);

  
    if(err == ESP_OK) {
        ESP_LOGI(TAG_2, "SHTC3 sensor found");
        uint8_t sensor_id[2];
        err = shtc3_get_id(shtc3_handle, sensor_id);
        ESP_LOGI(TAG_2, "Sensor ID: 0x%02x%02x", sensor_id[0], sensor_id[1]);

        if(err == ESP_OK) {
            ESP_LOGI(TAG_2, "SHTC3 ID read successfully");
            xTaskCreate(shtc3_read_task, "shtc3_read_task", 4096, NULL, 5, NULL);
        } else {
            ESP_LOGE(TAG_2, "Failed to read SHTC3 ID");
        }

    } else {
        ESP_LOGE(TAG_2, "SHTC3 sensor not found");
        shtc3_device_delete(shtc3_handle);
    }

    while (1) {
        //MQ135 
        int air_quality = mq135_read();
        if (air_quality) {
            ESP_LOGI(TAG_1, "Air Quality: GOOD");
        } else {
            ESP_LOGW(TAG_1, "Air Quality: BAD");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // Read every 1 second

        //SHTC3
    }  
}

