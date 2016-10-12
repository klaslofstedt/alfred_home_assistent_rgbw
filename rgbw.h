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
} rgbw_t;

//void pwm_setup(void);
//void pwm_task(void *pvParameters);
//void HW_init(void);
void rgbw_init(void);
void rgbw_pins_init(void);
void blue_task(void *duty_cycle);
void red_task(void *duty_cycle);
void rgb_calc(uint16_t value, float *r, float *g, float *b);
void saturation_calc(uint16_t value, float *color, float *white);
void mqtt_message_process(char **data, uint8_t len);
void pwm_values_calc(uint16_t *data_array);
void rainbow_task(void *delay);

#endif

