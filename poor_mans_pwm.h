#ifndef RGBW_H
#define RGBW_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//xSemaphoreHandle pwm_sem;

//void rgbw_task(void *arg);
void frc1_interrupt_handler(void);
void poor_mans(uint8_t pin, uint8_t duty);
void poor_mans_pwm_exec(void);
void poor_mans_pwm_init(uint8_t frequency, uint8_t resolution);
#endif
