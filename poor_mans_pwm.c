#include "poor_mans_pwm.h"

/* Poor mans PWM
 * The routine fires 255*255 = 65025 times a second, which gives
 * a PWM signal of 255 Hz and a resolution of 255. 
 * Verified with Saleae logic analysator.
 */
uint8_t pwm_pin1, pwm_pin2, pwm_pin3;
uint8_t *duty1;
uint8_t *duty2;
uint8_t *duty3;
uint8_t *duty4;

static volatile uint32_t frc1_interrupt_count;
void frc1_interrupt_handler(void)
{
    frc1_interrupt_count++;
    //xSemaphoreGive(pwm_sem);
    poor_mans_pwm_exec();
}

void poor_mans(uint8_t pin, uint8_t duty)
{
    uint8_t resolution = 255;
    if(frc1_interrupt_count < duty){
        gpio_write(pin, 1);
    }
    else{
        gpio_write(pin, 0);
    }
    if(frc1_interrupt_count == resolution){
        frc1_interrupt_count = 0;
    }
}

void poor_mans_pwm_exec(void)
{
    poor_mans(pwm_pin1, duty1);
    poor_mans(pwm_pin2, duty2);
    poor_mans(pwm_pin3, duty3);
    poor_mans(pwm_pin4, duty4);
    /*
    poor_mans(RED, lamp_g.red);
    poor_mans(GREEN, lamp_g.green);
    poor_mans(BLUE, lamp_g.blue);
    */
} 

void poor_mans_pwm_init(uint8_t frequency, uint8_t resolution)
{
    // pause interrupt
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);
    // setup interrupt
    _xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler);
    timer_set_frequency(FRC1, frequency*resolution);
    // start interrupt
    timer_set_interrupts(FRC1, true);
    timer_set_run(FRC1, true);
}
