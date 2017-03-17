#include <stdlib.h>
#include <stdio.h>
#include "poor_mans_pwm.h"

/* Poor mans PWM
 * The routine fires 255*255 = 65025 times a second, which gives
 * a PWM signal of 255 Hz and a resolution of 255. 
 * Verified with Saleae logic analysator.
 */
static void frc1_interrupt_handler(void);
static void pmp_pwm_set(uint8_t pin, uint8_t duty);
static void pmp_pwm_pins_update(void);

static uint8_t *pmp_pwm_pins_g = NULL;
static uint8_t *pmp_pwm_duty_g = NULL;
static uint8_t pmp_pwm_pins_size_g = 0;
static uint8_t resolution_g = 0;
static uint8_t frequency_g = 0;

static volatile uint32_t frc1_interrupt_count_g;

static void pmp_pwm_set(uint8_t pin, uint8_t duty)
{
    if(frc1_interrupt_count_g < duty){
        gpio_write(pin, 1);
    }
    else{
        gpio_write(pin, 0);
    }
    if(frc1_interrupt_count_g == resolution_g){
        frc1_interrupt_count_g = 0;
    }
}

static void pmp_pwm_pins_update(void)
{
    uint8_t i;
    for(i = 0; i < pmp_pwm_pins_size_g; i++){
        pmp_pwm_set(pmp_pwm_pins_g[i], pmp_pwm_duty_g[i]);
    }
} 
uint8_t toggle = 0;
static void frc1_interrupt_handler(void)
{
    /*
       uint32_t ticks = 0, prev_ticks = 0;
       ticks = xTaskGetTickCountFromISR();
       printf("ticks: %d\n\r", ticks - prev_ticks);
       prev_ticks = ticks;
       */
    /*if(toggle == 1){
        gpio_write(16, toggle);
        toggle = 0;
    }
    else{
        gpio_write(16, toggle);
        toggle = 1;
    }*/

    frc1_interrupt_count_g++;
    pmp_pwm_pins_update();
}

void pmp_pwm_set_duty(uint8_t *temp, uint8_t size)
{
    uint8_t i;
    for(i = 0; i < size; i++){
        pmp_pwm_duty_g[i] = temp[i];
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
    gpio_enable(16, GPIO_OUTPUT);
}

void pmp_pwm_init(uint8_t frequency, uint8_t resolution)
{
    frequency_g = frequency;
    resolution_g = resolution;
    // pause interrupt
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);
    // setup interrupt
    _xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler);
    timer_set_frequency(FRC1, frequency_g*resolution_g);
    // start interrupt
    timer_set_interrupts(FRC1, true);
    timer_set_run(FRC1, true);
}

