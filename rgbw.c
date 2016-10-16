#include "rgbw.h"
#include "math_lite.h"
#include "poor_mans_pwm.h"

//#define DEBUG
#define RED 13
#define GREEN 12
#define BLUE 14
#define WHITE 2
#define SWITCH 15

#define FREQENCY 0xff
#define RESOLUTION 0xff
#define SIZE 4

uint8_t pwm_pin_g[] = {RED, GREEN, BLUE, WHITE};
uint8_t pwm_duty_g[] = {0, 0, 0, 0};
uint16_t previous_state = 0;

static void rgbw_dim_light(uint16_t *data_array);
static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat);
static void rgbw_pwm_set_duty(void);
static void rgbw_calc_rgb(uint16_t value, float *r, float *g, float *b);
static void rgbw_calc_saturation(uint16_t value, float *color, float *white);
static void rgbw_calc_pwm(uint16_t *data_array);

rgbw_t lamp_g = {
    .status = 0,
    .red = 0,
    .green = 0,
    .blue = 0,
    .white = 0
};


void rainbow_task(void *delay)
{
    uint16_t data_array[4] = {1, 100, 200, 0};
    while(1){
#ifdef DEBUG
        printf("color: %d\n", data_array[3]);
#endif
        rgbw_calc_pwm(data_array);
        vTaskDelay(10 / portTICK_RATE_MS);
        data_array[3]++;
        if(data_array[3] > 1530){
            data_array[3] = 0;
        }
    }
}

static void rgbw_pwm_set_duty(void)
{
    pwm_duty_g[0] = lamp_g.red;
    pwm_duty_g[1]= lamp_g.green;
    pwm_duty_g[2] = lamp_g.blue;
    pwm_duty_g[3]= lamp_g.white;

    pmp_pwm_set_duty(pwm_duty_g, SIZE);
}

void rgbw_init(void)
{
    // pmp - poor mans pwm
    pmp_pwm_init(FREQENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g, SIZE);
    pmp_pwm_set_duty(pwm_duty_g, SIZE);
}

static void rgbw_calc_rgb(uint16_t value, float *r, float *g, float *b)
{
    if((float)value <= 255){
        *r = 255;
        *g = (float)value;
        *b = 0;
    }
    else if(((float)value > 255) && ((float)value <= 510)){
        *r = 255 - ((float)value - 255);
        *g = 255;
        *b = 0;
    }
    else if(((float)value > 510) && ((float)value <= 765)){
        *r = 0;
        *g = 255;
        *b = 255 - (765 - (float)value);
    }
    else if(((float)value > 765) && ((float)value <= 1020)){
        *r = 0;
        *g = 255 - ((float)value - 765);
        *b = 255;
    }
    else if(((float)value > 1020) && ((float)value <= 1275)){
        *r = 255 - (1275 - (float)value);
        *g = 0;
        *b = 255;
    }
    else if(((float)value > 1275) && ((float)value <= 1530)){
        *r = 255;
        *g = 0;
        *b = 1530 - (float)value;
    }
    else{
#ifdef DEBUG
        printf("color error");
#endif
    }
}

static void rgbw_calc_saturation(uint16_t value, float *color, float *white)
{
    if((float)value >= 0 && (float)value <= 100){
        *white = 100;
        *color = (float)value;
    }
    else if((float)value > 100 && (float)value <= 200){
        *color = 100;
        *white = 200 - (float)value;
    }
    else{
#ifdef DEBUG
        printf("saturation error%f\n", (float)value);
#endif
    } 
}
/*****************************************
 * Parse mqtt message. 
 * In: string ("x:yyy:zzz:www"), string lenght
 * Out: uint8_t [x, yyy, zzz, www], where
 * x = on/off (0 - 1)
 * y = brightness (0 - 100)
 * z = saturation (0 - 200)
 * w = color (0 - 1530)
 ****************************************/

void rgbw_parse_mqtt(char **data, uint8_t len)
{
    // Totally fucking shit code. Rewrite
    uint8_t i;
    uint8_t array_size = 4;
    char char_num_arr[4] = {0};
    uint16_t data_array[4] = {0};
    uint8_t num_lenght = 0;
    uint8_t arr_ptr = 0;
    uint8_t arr_ptr_copy = 0;
    for(i = 0; i < array_size; i++){
        arr_ptr = arr_ptr_copy;
        uint8_t j = 0;
        for(j = arr_ptr; data[j] != ':' && j < len; j++){
            char_num_arr[j - arr_ptr] = data[j];
            num_lenght++;
            arr_ptr_copy++;
        }
        arr_ptr_copy++;
        uint8_t num_lenght_copy = num_lenght;
        for(j = 0; j < num_lenght_copy; j++)
        {
            uint16_t number = (power(10, num_lenght -1) * (char_num_arr[j] - '0'));
            num_lenght--;
            data_array[i] += number;
        }
    }
#ifdef DEBUG
    for(i = 0; i < array_size; i++){
        printf("data_array%d\n", data_array[i]);
    }
#endif
    if(previous_state == data_array[0]){
        rgbw_calc_pwm(data_array);
    }
    else{
        rgbw_dim_light(&data_array);
    }
}

static void rgbw_dim_light(uint16_t *data_array)
{
    uint16_t temp_brightness = data_array[1];
    if(previous_state == 0){
        lamp_g.status = 1;
        previous_state = 1;
        data_array[1] = 0;
        while(data_array[1] < temp_brightness){
            data_array[1] = (data_array[1] + 1);
#ifdef DEBUG
            printf("upp data_array[1]: %d\n", data_array[1]);
#endif
            rgbw_calc_pwm(data_array);
            vTaskDelay(25 / portTICK_RATE_MS);
        }
    }
    else{
        lamp_g.status = 1;
        previous_state = 0;
        while(data_array[1] > 0){
            data_array[1] = (data_array[1] - 1);
#ifdef DEBUG
            printf("ner data_array[1]: %d\n", data_array[1]);
#endif
            rgbw_calc_pwm(data_array);
            vTaskDelay(18 / portTICK_RATE_MS);
        }
        lamp_g.status = 0;
        rgbw_calc_pwm(data_array);
    }
}

static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat)
{
    *c = (col * ((float)bri / 100) * (sat / 100) * stat);
}


static void rgbw_calc_pwm(uint16_t *data_array)
{
    // Not too shitty. Make a separate function for this.
    float temp_r = 0, temp_g = 0, temp_b = 0, temp_color = 0, temp_white = 0;

    rgbw_calc_saturation(data_array[2], &temp_color, &temp_white);
    rgbw_calc_rgb(data_array[3], &temp_r, &temp_g, &temp_b); 
    /*
    if(data_array[1] != 0 && previous_state == 0){
        lamp_g.status = 1;
    }
    else{
        lamp_g.status = data_array[0];
    }*/

    rgbw_set_lamp(&lamp_g.red, temp_r, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.green, temp_g, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.blue, temp_b, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.white, 0xff, data_array[1], temp_white, lamp_g.status);
    /*
       lamp_g.red = (temp_r * ((float)data_array[1] / 100) * (temp_color / 100) * lamp_g.status);
       lamp_g.green = (temp_g * ((float)data_array[1] / 100) * (temp_color / 100) * lamp_g.status);
       lamp_g.blue = (temp_b * ((float)data_array[1] / 100) * (temp_color / 100) * lamp_g.status);
       lamp_g.white = (0xff * ((float)data_array[1] / 100) * (temp_white / 100) * lamp_g.status);
       */
#ifdef DEBUG
    printf("status%d\n", lamp_g.status);
    printf("red%d\n", lamp_g.red);
    printf("green%d\n", lamp_g.green);
    printf("blue%d\n", lamp_g.blue);
    printf("white%d\n", lamp_g.white);
#endif

    rgbw_pwm_set_duty();
}
