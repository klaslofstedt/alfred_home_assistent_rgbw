#include <stdlib.h> 
#include <stdio.h> 
#include "poor_mans_pwm.h"

/* Poor mans PWM
 * The routine fires frequency*resolution times a second.
 * Verified with Saleae logic analyzer.
 */
static void frc1_interrupt_handler(void);
static void pmp_pwm_set(uint8_t pin, uint8_t duty);

static uint8_t *pmp_pwm_pins_g = NULL;
static uint8_t *pmp_pwm_duty_g = NULL;
static uint8_t pmp_pwm_pins_size_g = 0;
static volatile uint8_t resolution_g = 0;
static volatile uint16_t frequency_g = 0;

static volatile uint32_t frc1_interrupt_count_g;

static void pmp_pwm_set(uint8_t pin, uint8_t duty)
{
    if(frc1_interrupt_count_g <= duty){
        gpio_write(pin, 1);
    }
    else{
        gpio_write(pin, 0);
    }
}

uint8_t toggle = 0;
static void frc1_interrupt_handler(void)
{
    frc1_interrupt_count_g++;
    if(toggle == 1){
        gpio_write(16, toggle);
        toggle = 0;
    }
    else{
        gpio_write(16, toggle);
        toggle = 1;
    }
    uint8_t i;
    for(i = 0; i < pmp_pwm_pins_size_g; i++){
        pmp_pwm_set(pmp_pwm_pins_g[i], pmp_pwm_duty_g[i]);
    }
    if(frc1_interrupt_count_g >= resolution_g){
        frc1_interrupt_count_g = 0;
    }
}

void pmp_pwm_set_duty(uint8_t *duty, uint8_t size)
{
    uint8_t i;
    for(i = 0; i < size; i++){
        pmp_pwm_duty_g[i] = duty[i];
    }
}


void pmp_pwm_pins_init(uint8_t *pins, uint8_t size)
{
    pmp_pwm_pins_g = malloc(sizeof(uint8_t) * size);
    pmp_pwm_duty_g = malloc(sizeof(uint8_t) * size);
    pmp_pwm_pins_size_g = size;
    uint8_t i;
    for(i = 0; i < size; i++){
        gpio_enable(pins[i], GPIO_OUTPUT);
        pmp_pwm_pins_g[i] = pins[i];
        pmp_pwm_duty_g[i] = 0;
    }
    // for debugging interrupt frequency
    gpio_enable(16, GPIO_OUTPUT);
}

void pmp_pwm_init(uint16_t frequency, uint8_t resolution)
{
    frequency_g = frequency;
    resolution_g = resolution;
    // pause interrupt
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);
    // setup interrupt
    _xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler);
    timer_set_frequency(FRC1, frequency * resolution);
    // start interrupt
    timer_set_interrupts(FRC1, true);
    timer_set_run(FRC1, true);
}
