#ifndef MQ135_H
#define MQ135_H

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

#define MQ135_ADC_CHANNEL ADC_CHANNEL_0
#define ADC_ATTEN ADC_ATTEN_DB_11

float mq135_adc_to_ppm(int adc_raw);
void mq135_init(adc_oneshot_unit_handle_t *adc_handle);
int mq135_read(adc_oneshot_unit_handle_t adc_handle);

#endif

