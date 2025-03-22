#ifndef MQ135_H
#define MQ135_H

#include "esp_err.h"

#define MQ135_GPIO  4  // Change this to the actual GPIO pin you use

void mq135_init(void);
int mq135_read(void);

#endif

