#include "rgbw.h"
//#include "mqtt.h"
#include "math_lite.h"
#include "poor_mans_pwm.h"
#include <stdio.h>
#include <FreeRTOSConfig.h>


#define RED 13
#define GREEN 12
#define BLUE 14
#define WHITE 2
#define SWITCH 15

#define FREQUENCY 150
#define RESOLUTION 0xff
#define SIZE 4

#define SATURATION_MIN 0
#define SATURATION_MAX 200
#define COLOR_MAX 1530

#define DIMMER_TIME_MS 30

static uint8_t pwm_pin_g[] = {RED, GREEN, BLUE, WHITE};
static uint8_t pwm_duty_g[] = {0, 0, 0, 0};

//uint8_t prev_status = 0;
//uint8_t prev_brightness = 0;
//uint8_t prev_saturation = 0;
//uint16_t prev_rgb = 0;

//UBaseType_t stack_size_rgbw;


static void rgbw_calc_rgb(int32_t value);
static void rgbw_calc_saturation(uint8_t value);
static void rgbw_corner_case();

//static void constrain8(uint8_t *in, uint8_t min, uint8_t max);
//static void constrain16(uint16_t *in, uint16_t min, uint16_t max);
static void constrain32(int32_t *in, int32_t min, int32_t max);

rgbw_t lamp_g = {
    .status = 0,
    .rgb = 0,
    .red = 0,
    .green = 0,
    .blue = 0,
    .rainbow = 0,
    .speed = 20,
    .color = 0,
    .white = 100,
    .saturation = 0,
    .brightness = 100,
};

/*void constrain8(uint8_t *in, uint8_t min, uint8_t max)
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
}*/

void constrain32(int32_t *in, int32_t min, int32_t max)
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

/*static void rgbw_calc_saturation(uint8_t value)
{
    int32_t temp = 0;
    //constrain32(&value, 0, 200);
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
    temp = (int32_t)lamp_g.color;
    constrain32(&temp, 0, 100);
    lamp_g.color = (uint8_t)temp;

    temp = (int32_t)lamp_g.white;
    constrain32(&temp, 0, 100);
    lamp_g.white = (uint8_t)temp;
    //printf("color1: %d\n", lamp_g.color);
    //printf("white1: %d\n", lamp_g.white);
}*/

/*void rgbw_calc_rgb(int32_t value)
{
    int32_t temp;
    constrain32(&value, 0, 1530);
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
    temp = (int32_t)lamp_g.red;
    constrain32(&temp, 0, 0xff);
    lamp_g.red = (uint8_t)temp;

    temp = (int32_t)lamp_g.green;
    constrain32(&temp, 0, 0xff);
    lamp_g.green = (uint8_t)temp;

    temp = (int32_t)lamp_g.blue;
    constrain32(&temp, 0, 0xff);
    lamp_g.blue = (uint8_t)temp;

    //printf("red: %d\n", lamp_g.red);
    //printf("green: %d\n", lamp_g.green);
    //printf("blue: %d\n", lamp_g.blue);
}*/

/*void rgbw_corner_case()
{
    if(lamp_g.saturation == 0){
        pwm_duty_g[0] = 0;
        pwm_duty_g[1] = 0;
        pwm_duty_g[2] = 0;
    }
    if(lamp_g.saturation == 200){
        pwm_duty_g[3] = 0;
    }
    if((lamp_g.status == 0) || (lamp_g.brightness == 0)){
        pwm_duty_g[0] = 0;
        pwm_duty_g[1] = 0;
        pwm_duty_g[2] = 0;
        pwm_duty_g[3] = 0;
    }
    pmp_pwm_set_duty(pwm_duty_g, SIZE);
}*/


void rgbw_start_lamp()
{
    //int32_t red, green, blue, white;

    /*uint8_t rainbow = lamp_g.rainbow;
    lamp_g.rainbow = 0;
    // If smooth color
    if(lamp_g.rgb != prev_rgb){
        printf("smooth color\n");
        int32_t i;
        if(lamp_g.rgb > prev_rgb){ //
            printf("higher\n");
            for(i = prev_rgb; i < lamp_g.rgb; (i = i+5)){ // 51 whole steps (51*30 = 1530)
                constrain32(&i, 0, 1530);
                rgbw_calc_rgb(i);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = 0;
                }
                else{
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);

                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        else{ //
            printf("lower\n");
            for(i = prev_rgb; i >= (lamp_g.rgb); (i = i-5)){
                constrain32(&i, 0, 1530);
                rgbw_calc_rgb(i);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = 0;
                }
                else{
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
    }
    // If smooth saturation
    if(lamp_g.saturation != prev_saturation){
        printf("smooth saturation\n");
        int32_t i;
        if(lamp_g.saturation > prev_saturation){ // white to color
            printf("colorful\n");
            for(i = prev_saturation; i <= lamp_g.saturation; (i = i+5)){
                constrain32(&i, 0, 200);
                rgbw_calc_saturation(i);
                red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
        else{ // color to white
            printf("whiter\n");
            for(i = prev_saturation; i > lamp_g.saturation; (i = i-5)){
                constrain32(&i, 0, 200);
                rgbw_calc_saturation(i);
                red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100)  * prev_status);
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
    }
    // If smooth brightness
    if(lamp_g.brightness != prev_brightness){
        printf("smooth brightness\n");
        int32_t i;
        if(lamp_g.brightness > prev_brightness){ // brighter
            printf("brighter\n");
            for(i = prev_brightness; i <= lamp_g.brightness; (i = i+5)){
                constrain32(&i, 0, 100);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = 0;
                }else{
                    red = (int32_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = (int32_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * prev_status);
                }

                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
        else{ // darker
            printf("darker\n");
            for(i = prev_brightness; i > lamp_g.brightness; (i = i-5)){
                constrain32(&i, 0, 100);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = 0;
                }else{
                    red = (int32_t)((float)lamp_g.red * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    green = (int32_t)((float)lamp_g.green * ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    blue = (int32_t)((float)lamp_g.blue* ((float) i / 100) * ((float)lamp_g.color/ 100) * prev_status);
                    white = (int32_t)((float)0xff * ((float) i / 100) * ((float)lamp_g.white / 100) * prev_status);
                }
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
    }
    // if smooth status (onoff)
    if(lamp_g.status != prev_status){
        printf("smooth status\n");
        int32_t i;
        if(lamp_g.status == 1){ // from 0 to 1
            printf("0 to 1\n");
            for(i = 0; i <= 100; (i = i+5)){
                constrain32(&i, 0, 100);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    blue = (int32_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    white = 0;
                }else{
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    blue = (int32_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                }
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
        else{ // from 1 to 0
            printf("1 to 0\n");
            for(i = 100; i > 0; (i = i-5)){
                constrain32(&i, 0, 100);
                // because of a shitton of float leaking current we check a saturation corner case
                if(lamp_g.saturation == 0){
                    red = 0;
                    green = 0;
                    blue = 0;
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                }
                else if(lamp_g.saturation == 200){
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    blue = (int32_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    white = 0;
                }else{
                    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    blue = (int32_t)((float)lamp_g.blue* ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * ((float)(i)/100));
                    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * ((float)(i)/100));
                }
                constrain32(&red, 0, 0xff);
                constrain32(&green, 0, 0xff);
                constrain32(&blue, 0, 0xff);
                constrain32(&white, 0, 0xff);
                pwm_duty_g[0] = (uint8_t)red;
                pwm_duty_g[1] = (uint8_t)green;
                pwm_duty_g[2] = (uint8_t)blue;
                pwm_duty_g[3] = (uint8_t)white;

                pmp_pwm_set_duty(pwm_duty_g, SIZE);

                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
        }
    }*/
    // No smooth nothing
    //printf("nothing smooth\n");
    /*rgbw_calc_saturation(lamp_g.saturation);
    printf("white: %d\n", lamp_g.white);
    printf("color: %d\n", lamp_g.color);
    rgbw_calc_rgb(lamp_g.rgb);
    printf("red: %d\n", lamp_g.red);
    printf("green: %d\n", lamp_g.green);
    printf("blue: %d\n", lamp_g.blue);
    red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
    green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
    blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * lamp_g.status);
    white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * lamp_g.status);
    //rgbw_corner_case();

    constrain32(&red, 0, 0xff);
    constrain32(&green, 0, 0xff);
    constrain32(&blue, 0, 0xff);
    constrain32(&white, 0, 0xff);

    printf("pwm_red: %d\n", red);
    printf("pwm_green: %d\n", green);
    printf("pwm_blue: %d\n", blue);
    printf("pwm_white: %d\n", white);*/

    /*pwm_duty_g[0] = (uint8_t)red;
    pwm_duty_g[1] = (uint8_t)(lamp_g.status * 100);
    pwm_duty_g[2] = (uint8_t)blue;
    pwm_duty_g[3] = (uint8_t)white;*/

    //pmp_pwm_set_duty(pwm_duty_g, SIZE);

    //prev_status = lamp_g.status;
    //prev_rgb = lamp_g.rgb;
    //prev_saturation = lamp_g.saturation;
    //prev_brightness =  lamp_g.brightness;

    /*lamp_g.rainbow = rainbow;
    if(lamp_g.rainbow == 1){
        xSemaphoreGive(sem_activate_rainbow);
    }*/
    //printf("red: %d\n", red);
    //printf("green: %d\n", green);
    //printf("blue: %d\n", blue);
    //printf("white: %d\n", white);

    /*gpio_write(12, lamp_g.status);
    gpio_write(13, lamp_g.saturation);
    gpio_write(14, lamp_g.brightness);
    gpio_write(2, 0);*/
    pwm_duty_g[0] = lamp_g.status * 0xff;
    pwm_duty_g[1] = lamp_g.saturation;
    pwm_duty_g[2] = lamp_g.brightness;
    pwm_duty_g[3] = 0;
    pmp_pwm_set_duty(pwm_duty_g);
}

void rgbw_init()
{
    // some inits
    gpio_enable(SWITCH, GPIO_OUTPUT);
    gpio_write(SWITCH, 1);
    // pmp - poor mans pwm
    pmp_pwm_init(FREQUENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g);

    // init lamp off
    pwm_duty_g[0] = 0xff;
    pwm_duty_g[1] = 0;
    pwm_duty_g[2] = 0;
    pwm_duty_g[3] = 0;
    pmp_pwm_set_duty(pwm_duty_g);
}

/*void rgbw_task(void *pvParameters)
{
    // some inits
    gpio_enable(SWITCH, GPIO_OUTPUT);
    gpio_write(SWITCH, 1);
    // pmp - poor mans pwm
    pmp_pwm_init(FREQUENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g, SIZE);

    // init lamp off
    pwm_duty_g[0] = 0;
    pwm_duty_g[1] = 0;
    pwm_duty_g[2] = 0;
    pwm_duty_g[3] = 0;
    pmp_pwm_set_duty(pwm_duty_g, SIZE);

    uint8_t status, brightness, saturation, speed;
    uint16_t color;

    while(1) {
        if(xSemaphoreTake(sem_mqtt_new, portMAX_DELAY) == pdTRUE){
            printf("sem_mqtt_new\n");
            if(xQueueReceive(queue_mqtt_status, &status, 0)){
                if(status == 0 || status == 1){
                    lamp_g.status = status;
                }
                else if(status == 2 || status == 3){
                    lamp_g.rainbow = status - 2;
                    printf("rainbow: %d\n", lamp_g.rainbow);
                    if(lamp_g.rainbow == 1){
                      xSemaphoreGive(sem_activate_rainbow);
                    }
                }
                else{
                    printf("status error\n");
                }
                //printf("status\n");
            }
            if(xQueueReceive(queue_mqtt_color, &color, 0)){
                lamp_g.rgb = color;
                //printf("color\n");
            }
            if(xQueueReceive(queue_mqtt_brightness, &brightness, 0)){
                lamp_g.brightness = brightness;
                //printf("brightness\n");
            }
            if(xQueueReceive(queue_mqtt_saturation, &saturation, 0)){
                lamp_g.saturation = saturation;
                //printf("saturation\n");
            }
            if(xQueueReceive(queue_mqtt_speed, &speed, 0)){
                lamp_g.speed = speed;
                printf("speed\n");
            }/*
            if(xQueueReceive(queue_mqtt_rainbow, &rainbow, 0)){
                printf("rainbow\n");
                lamp_g.rainbow = rainbow;


            }
            rgbw_start_lamp();
        }
    }
}*/

/*void rainbow_task(void *pvParameters)
{
    int32_t red, green, blue, white;
    int32_t color = 0, delay;

    while(1){
        if(xSemaphoreTake(sem_activate_rainbow, portMAX_DELAY) == pdTRUE){
            // Blink
            rgbw_calc_rgb(color);
            if(lamp_g.saturation == 0){
                red = 0;
                green = 0;
                blue = 0;
                white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
            }
            else if(lamp_g.saturation == 200){
                red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                white = 0;
            }
            else{
                red = (int32_t)((float)lamp_g.red * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                green = (int32_t)((float)lamp_g.green * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                blue = (int32_t)((float)lamp_g.blue * ((float) lamp_g.brightness / 100) * ((float)lamp_g.color/ 100) * prev_status);
                white = (int32_t)((float)0xff * ((float) lamp_g.brightness / 100) * ((float)lamp_g.white / 100) * prev_status);
            }

            constrain32(&red, 0, 0xff);
            constrain32(&green, 0, 0xff);
            constrain32(&blue, 0, 0xff);
            constrain32(&white, 0, 0xff);

            pwm_duty_g[0] = (uint8_t)red;
            pwm_duty_g[1] = (uint8_t)green;
            pwm_duty_g[2] = (uint8_t)blue;
            pwm_duty_g[3] = (uint8_t)white;

            pmp_pwm_set_duty(pwm_duty_g, SIZE);

            color = color + lamp_g.speed +1;
            delay = (100 - lamp_g.speed) + 10;

            if(color >= 1530){
                color = 0;
            }

            lamp_g.rgb = color;
            vTaskDelay(delay / portTICK_PERIOD_MS); //

            if(lamp_g.rainbow == 1){
                xSemaphoreGive(sem_activate_rainbow);
            }
        }
        //stack_size_rgbw = uxTaskGetStackHighWaterMark(NULL);
        //printf("rgbw size: %d", stack_size_rgbw);
    }
}*/
