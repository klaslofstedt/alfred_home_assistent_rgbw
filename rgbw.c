#include "rgbw.h"
#include "math_lite.h"
#include "poor_mans_pwm.h"
#include <stdio.h>
#include <FreeRTOSConfig.h>


#define RED 13
#define GREEN 12
#define BLUE 14
#define WHITE 2
#define SWITCH 15

#define FREQUENCY 100
#define RESOLUTION 0xff
#define SIZE 4

#define SATURATION_MIN 0
#define SATURATION_MAX 200
#define COLOR_MAX 1530

#define DIMMER_TIME_MS 30

uint8_t pwm_pin_g[] = {RED, GREEN, BLUE, WHITE};
uint8_t pwm_duty_g[] = {0, 0, 0, 0};

uint8_t prev_status = 0;
uint8_t prev_brightness = 0;
uint8_t prev_saturation = 0;
uint16_t prev_rgb = 0;


static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat);
static void rgbw_calc_pwm();
static void rgbw_calc_rgb(uint16_t value);
static void rgbw_calc_saturation(uint8_t value);

static void constrain8(uint8_t *in, uint8_t min, uint8_t max);
static void constrain16(uint16_t *in, uint16_t min, uint16_t max);

rgbw_t lamp_g = {
    .status = 0,
    .rgb = 0,
    .red = 0,
    .green = 0,
    .blue = 0,
    .random = 0,
    .speed = 0,
    .color = 100,
    .white = 100,
    .saturation = 100,
    .brightness = 100,
};

void constrain8(uint8_t *in, uint8_t min, uint8_t max)
{
    if(*in < min){
        *in = min;
    }else if(*in > max){
        *in = max;
    }
}

void constrain16(uint16_t *in, uint16_t min, uint16_t max)
{
    if(*in < min){
        *in = min;
    }else if(*in > max){
        *in = max;
    }
}

void rgbw_status(uint8_t value)
{
    lamp_g.status = value;
}

void rgbw_color(uint16_t value)//, float *r, float *g, float rgbw_g.blue)
{
    lamp_g.rgb = value;
}

void rgbw_brightness(uint8_t value)
{
    lamp_g.brightness = value;
}

void rgbw_saturation(uint8_t value)
{
    lamp_g.saturation = value;
}

void rgbw_calc_saturation(uint8_t value)
{
    constrain8(&value, 0, 200);
    if((float)value >= SATURATION_MIN && (float)value <= SATURATION_MAX / 2){
        lamp_g.white = SATURATION_MAX / 2;
        lamp_g.color = (float)value;
    }
    else if((float)value > SATURATION_MAX / 2 && (float)value <= SATURATION_MAX){
        lamp_g.color = SATURATION_MAX / 2;
        lamp_g.white = SATURATION_MAX - (float)value;
    }
    else{
        printf("saturation error%f\n", (float)value);
    }
    printf("color: %d\n", lamp_g.color);
    printf("white: %d\n", lamp_g.white);
    constrain8(&lamp_g.color, 0, 100);
    constrain8(&lamp_g.color, 0, 100);
}

void rgbw_calc_rgb(uint16_t value)
{
    constrain16(&value, 0, 1530);
    if(((float)value >= 0) && ((float)value <= 1*COLOR_MAX/6)){
        lamp_g.red = 1*COLOR_MAX/6;
        lamp_g.green = (float)value;
        lamp_g.blue = 0;
    }
    else if(((float)value > 1*COLOR_MAX/6) && ((float)value <= 2*COLOR_MAX/6)){
        lamp_g.red = 1*COLOR_MAX/6 - ((float)value - 1*COLOR_MAX/6);
        lamp_g.green = 1*COLOR_MAX/6;
        lamp_g.blue = 0;
    }
    else if(((float)value > 2*COLOR_MAX/6) && ((float)value <= 3*COLOR_MAX/6)){
        lamp_g.red = 0;
        lamp_g.green = 1*COLOR_MAX/6;
        lamp_g.blue = 1*COLOR_MAX/6 - (3*COLOR_MAX/6 - (float)value);
    }
    else if(((float)value > 3*COLOR_MAX/6) && ((float)value <= 4*COLOR_MAX/6)){
        lamp_g.red = 0;
        lamp_g.green = 1*COLOR_MAX/6 - ((float)value - 3*COLOR_MAX/6);
        lamp_g.blue = 1*COLOR_MAX/6;
    }
    else if(((float)value > 4*COLOR_MAX/6) && ((float)value <= 5*COLOR_MAX/6)){
        lamp_g.red = 1*COLOR_MAX/6 - (5*COLOR_MAX/6 - (float)value);
        lamp_g.green = 0;
        lamp_g.blue = 1*COLOR_MAX/6;
    }
    else if(((float)value > 5*COLOR_MAX/6) && ((float)value <= COLOR_MAX)){
        lamp_g.red = 1*COLOR_MAX/6;
        lamp_g.green = 0;
        lamp_g.blue = COLOR_MAX - (float)value;
    }
    else{
        printf("color error%d\n", value);
    }
    constrain8(&lamp_g.red, 0, 0xff);
    constrain8(&lamp_g.green, 0, 0xff);
    constrain8(&lamp_g.blue, 0, 0xff);
    /*printf("red: %d\n", lamp_g.red);
    printf("green: %d\n", lamp_g.green);
    printf("blue: %d\n", lamp_g.blue);*/
}

void rgbw_start_lamp()
{
    uint8_t red, green, blue, white;
    // if smooth status (onoff)
    if(lamp_g.status != prev_status){
        printf("smooth status\n");
        uint8_t i;
        if(lamp_g.status == 1){ // from 0 to 1
            printf("0 to 1\n");
            for(i = 0; i <= 100; (i = i+2)){
                constrain8(&i, 0, 100);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                blue = (uint8_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
            }
        }
        else{ // from 1 to 0
            printf("1 to 0\n");
            for(i = 100; i > 0; (i = i-2)){
                constrain8(&i, 0, 100);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                blue = (uint8_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
            }
            //Do it one more time to make sure "status" i actually 0 and not leak light because of float
            pwm_duty_g[0] = 0;
            pwm_duty_g[1] = 0;
            pwm_duty_g[2] = 0;
            pwm_duty_g[3] = 0;

            pmp_pwm_set_duty(pwm_duty_g, SIZE);
        }

    }
    // If smooth color
    else if(lamp_g.rgb != prev_rgb){
        printf("smooth color\n");
        uint16_t i;
        if(lamp_g.rgb > prev_rgb){ //
            printf("higher\n");
            for(i = prev_rgb; i < lamp_g.rgb; (i = i+25)){
                constrain16(&i, 0, 1530);
                rgbw_calc_rgb(i);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
        }
        else{ //
            printf("lower\n");
            for(i = prev_rgb; i > lamp_g.rgb; (i = i-25)){
                constrain16(&i, 0, 1530);
                rgbw_calc_rgb(i);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
        }
    }
    // If smooth saturation
    else if(lamp_g.saturation != prev_saturation){
        printf("smooth saturation\n");
        uint8_t i;
        if(lamp_g.saturation > prev_saturation){ // white to color
            printf("colorful\n");
            for(i = prev_saturation; i <= lamp_g.saturation; (i = i+2)){
                constrain8(&i, 0, 200);
                rgbw_calc_saturation(i);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
            // In case of floting rounding errors makes white leak
            if(lamp_g.saturation == 200){
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * lamp_g.status);
                white = 0;
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);
            }
        }
        else{ // color to white
            printf("whiter\n");
            for(i = prev_saturation; i > lamp_g.saturation; (i = i-2)){
                constrain8(&i, 0, 200);
                rgbw_calc_saturation(i);
                red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
            // In case of floting rounding errors makes RGB leak
            if(lamp_g.saturation == 0){
                red = 0;
                green = 0;
                blue = 0;
                white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);
            }
        }
    }
    // If smooth brightness
    else if(lamp_g.brightness != prev_brightness){
        printf("smooth brightness\n");
        uint8_t i;
        if(lamp_g.brightness > prev_brightness){ // brighter
            printf("brighter\n");
            for(i = prev_brightness; i <= lamp_g.brightness; (i = i+5)){
                constrain8(&i, 0, 100);
                red = (uint8_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
            // In case of floating rounding errors makes light leak
            if(lamp_g.brightness == 100){
                red = (uint8_t)((float)lamp_g.red * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);
            }
        }
        else{ // darker
            printf("darker\n");
            for(i = prev_brightness; i > lamp_g.brightness; (i = i-5)){
                constrain8(&i, 0, 100);
                red = (uint8_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                green = (uint8_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                blue = (uint8_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
                white = (uint8_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
                constrain8(&red, 0, 0xff);
                constrain8(&green, 0, 0xff);
                constrain8(&blue, 0, 0xff);
                constrain8(&white, 0, 0xff);
                pwm_duty_g[0] = red;
                pwm_duty_g[1] = green;
                pwm_duty_g[2] = blue;
                pwm_duty_g[3] = white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(20 / portTICK_RATE_MS);
            }
            // In case of floting rounding errors makes light leak
            if(lamp_g.brightness == 0){
                pwm_duty_g[0] = 0;
                pwm_duty_g[1] = 0;
                pwm_duty_g[2] = 0;
                pwm_duty_g[3] = 0;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);
            }
        }
    }
    else{ // No smooth nothing
        printf("nothing smooth\n");
        red = (uint8_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
        green = (uint8_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
        blue = (uint8_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
        white = (uint8_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);

        constrain8(&red, 0, 0xff);
        constrain8(&green, 0, 0xff);
        constrain8(&blue, 0, 0xff);
        constrain8(&white, 0, 0xff);

        printf("pwm_red: %d\n", red);
        printf("pwm_green: %d\n", green);
        printf("pwm_blue: %d\n", blue);
        printf("pwm_white: %d\n", white);

        pwm_duty_g[0] = red;
        pwm_duty_g[1] = green;
        pwm_duty_g[2] = blue;
        pwm_duty_g[3] = white;

        pmp_pwm_set_duty(pwm_duty_g, SIZE);
    }
    prev_status = lamp_g.status;
    prev_rgb = lamp_g.rgb;
    prev_saturation = lamp_g.saturation;
    prev_brightness =  lamp_g.brightness;
}

void rainbow_task(void *delay) // don't use the in parameter
{
    //uint16_t data_array[SIZE] = {1, 100, 200, 0};
    while(1){
        if(xSemaphoreTake(random_rgb, portMAX_DELAY)){
            //printf("take2\r\n");
            xSemaphoreGive(random_rgb);
            //printf("give2\r\n");
            printf("color: %d\n", lamp_g.color);
            rgbw_calc_pwm();
            vTaskDelay(50 / portTICK_RATE_MS);
            if(lamp_g.speed == 0){
                lamp_g.color++;
            }
            else{
                lamp_g.color = lamp_g.color + (2 * lamp_g.speed);
            }
            if(lamp_g.color> COLOR_MAX){
                lamp_g.color= 0;
            }
        }
    }
}

static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat)
{
    *c = (col * ((float)bri / 100) * (sat / 100) * stat);
}

static void rgbw_calc_pwm()
{
    float temp_r = 0, temp_g = 0, temp_b = 0, temp_color = 0, temp_white = 0;

    rgbw_set_lamp(&lamp_g.red, temp_r, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.green, temp_g, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.blue, temp_b, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.white, 0xff, lamp_g.brightness, temp_white, lamp_g.status);
}


void rgbw_init(void)
{
    gpio_enable(SWITCH, GPIO_OUTPUT);
    gpio_write(SWITCH, 1);
    // pmp - poor mans pwm
    pmp_pwm_init(FREQUENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g, SIZE);
    rgbw_start_lamp();
}

void rgbw_task(mqtt_message_data_t *md)
{
    // some inits
    gpio_enable(SWITCH, GPIO_OUTPUT);
    gpio_write(SWITCH, 1);
    // pmp - poor mans pwm
    pmp_pwm_init(FREQUENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g, SIZE);
    while(1) {
      xSemaphoreTake(wifi_alive, portMAX_DELAY);
}
