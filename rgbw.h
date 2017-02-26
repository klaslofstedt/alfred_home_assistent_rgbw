#ifndef RGBW_H
#define RGBW_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


typedef struct
{
    uint8_t status;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
    // raw data before conversion
    uint8_t saturation;
    uint8_t brightness;
    uint8_t random;
    uint8_t speed;
    uint16_t color;
} rgbw_t;

xSemaphoreHandle random_rgb;
void rainbow_task(void *delay);
void rgbw_parse_mqtt(char *data, uint8_t len);
void rgbw_init(void);

#endif

