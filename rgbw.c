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

#define SATURATION_MIN 0
#define SATURATION_MAX 200
#define COLOR_MAX 1530

#define DIMMER_TIME_MS 18

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
    .status = 1,
    .red = 0,
    .green = 0,
    .blue = 0,
    .white = 255
};


void rainbow_task(void *delay)
{
    uint16_t data_array[SIZE] = {1, 100, 200, 0};
    while(1){
#ifdef DEBUG
        printf("color: %d\n", data_array[3]);
#endif
        rgbw_calc_pwm(data_array);
        vTaskDelay(10 / portTICK_RATE_MS);
        data_array[3]++;
        if(data_array[3] > COLOR_MAX){
            data_array[3] = 0;
        }
    }
}


static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat)
{
    *c = (col * ((float)bri / 100) * (sat / 100) * stat);
}


static void rgbw_calc_rgb(uint16_t value, float *r, float *g, float *b)
{
    if(((float)value >= 0) && ((float)value <= 1*COLOR_MAX/6)){
        *r = 1*COLOR_MAX/6;
        *g = (float)value;
        *b = 0;
    }
    else if(((float)value > 1*COLOR_MAX/6) && ((float)value <= 2*COLOR_MAX/6)){
        *r = 1*COLOR_MAX/6 - ((float)value - 1*COLOR_MAX/6);
        *g = 1*COLOR_MAX/6;
        *b = 0;
    }
    else if(((float)value > 2*COLOR_MAX/6) && ((float)value <= 3*COLOR_MAX/6)){
        *r = 0;
        *g = 1*COLOR_MAX/6;
        *b = 1*COLOR_MAX/6 - (3*COLOR_MAX/6 - (float)value);
    }
    else if(((float)value > 3*COLOR_MAX/6) && ((float)value <= 4*COLOR_MAX/6)){
        *r = 0;
        *g = 1*COLOR_MAX/6 - ((float)value - 3*COLOR_MAX/6);
        *b = 1*COLOR_MAX/6;
    }
    else if(((float)value > 4*COLOR_MAX/6) && ((float)value <= 5*COLOR_MAX/6)){
        *r = 1*COLOR_MAX/6 - (5*COLOR_MAX/6 - (float)value);
        *g = 0;
        *b = 1*COLOR_MAX/6;
    }
    else if(((float)value > 5*COLOR_MAX/6) && ((float)value <= COLOR_MAX)){
        *r = 1*COLOR_MAX/6;
        *g = 0;
        *b = COLOR_MAX - (float)value;
    }
    else{
#ifdef DEBUG
        printf("color error");
#endif
    }
}

static void rgbw_calc_saturation(uint16_t value, float *color, float *white)
{
    if((float)value >= SATURATION_MIN && (float)value <= SATURATION_MAX / 2){
        *white = SATURATION_MAX / 2;
        *color = (float)value;
    }
    else if((float)value > SATURATION_MAX / 2 && (float)value <= SATURATION_MAX){
        *color = SATURATION_MAX / 2;
        *white = SATURATION_MAX - (float)value;
    }
    else{
#ifdef DEBUG
        printf("saturation error%f\n", (float)value);
#endif
    } 
}

static void rgbw_calc_pwm(uint16_t *data_array)
{
    float temp_r = 0, temp_g = 0, temp_b = 0, temp_color = 0, temp_white = 0;

    rgbw_calc_saturation(data_array[2], &temp_color, &temp_white);
    rgbw_calc_rgb(data_array[3], &temp_r, &temp_g, &temp_b); 

    rgbw_set_lamp(&lamp_g.red, temp_r, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.green, temp_g, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.blue, temp_b, data_array[1], temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.white, 0xff, data_array[1], temp_white, lamp_g.status);

#ifdef DEBUG
    printf("status%d\n", lamp_g.status);
    printf("red%d\n", lamp_g.red);
    printf("green%d\n", lamp_g.green);
    printf("blue%d\n", lamp_g.blue);
    printf("white%d\n", lamp_g.white);
#endif

    rgbw_pwm_set_duty();
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
            vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
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
            vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
        }
        lamp_g.status = 0;
        rgbw_calc_pwm(data_array);
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
    uint8_t array_size = SIZE;
    char char_num_arr[SIZE] = {0};
    uint16_t data_array[SIZE] = {0};
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
    gpio_enable(SWITCH, GPIO_OUTPUT);
    gpio_write(SWITCH, 1);
    // pmp - poor mans pwm
    pmp_pwm_init(FREQENCY, RESOLUTION);
    pmp_pwm_pins_init(pwm_pin_g, SIZE);
    rgbw_pwm_set_duty();
}

