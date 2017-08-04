#ifndef POOR_MANS_PWM_H
#define POOR_MANS_PWM_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

void pmp_pwm_set_duty(uint8_t *duty, uint8_t size);
void pmp_pwm_pins_init(uint8_t *pins, uint8_t size);
void pmp_pwm_init(uint16_t frequency, uint8_t resolution);
#endif
