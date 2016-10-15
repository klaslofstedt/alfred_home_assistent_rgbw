#ifndef RGBW_H
#define RGBW_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


typedef struct
{
    float status;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
} rgbw_t;

xSemaphoreHandle toggle_lamp;

void rgbw_slow_toggle_task(void *pvParameters);
void rainbow_task(void *delay);
void rgbw_parse_mqtt(char **data, uint8_t len);
void rgbw_init(void);

#endif

