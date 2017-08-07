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
    uint8_t white; // white/color
    uint8_t color; // white/color - don't mix with rgb
    // raw data before conversion
    uint8_t saturation;
    uint8_t brightness;
    uint8_t rainbow;
    uint8_t speed;
    uint16_t rgb;
} rgbw_t;

//SemaphoreHandle_t random_rgb;

//void rainbow_task(void *delay);
//void rgbw_task(void *pvParameters);

void rgbw_init(void);
void rgbw_start_lamp();

void rgbw_status(uint8_t value);
void rgbw_color(uint16_t value);
void rgbw_brightness(uint8_t value);
void rgbw_saturation(uint8_t value);



#endif
