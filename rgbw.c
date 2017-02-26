#include "rgbw.h"
#include "math_lite.h"
#include "poor_mans_pwm.h"
#include <stdio.h>
#include <FreeRTOSConfig.h>

//#define DEBUG
#define RED 13
#define GREEN 12
#define BLUE 14
#define WHITE 2
#define SWITCH 15

#define FREQENCY 100//0xff
#define RESOLUTION 100//0xff
#define SIZE 5

#define SATURATION_MIN 0
#define SATURATION_MAX 200
#define COLOR_MAX 1530

#define DIMMER_TIME_MS 50

uint8_t pwm_pin_g[] = {RED, GREEN, BLUE, WHITE};
uint8_t pwm_duty_g[] = {0, 0, 0, 0};
uint16_t previous_state = 0;

static void rgbw_dim_light();
static void rgbw_set_lamp(uint8_t *c, float col, uint16_t bri, float sat, uint8_t stat);
static void rgbw_pwm_set_duty(void);
static void rgbw_calc_rgb(uint16_t value, float *r, float *g, float *b);
static void rgbw_calc_saturation(uint16_t value, float *color, float *white);
static void rgbw_calc_pwm();

rgbw_t lamp_g = {
    .status = 1,
    .red = 0,
    .green = 0,
    .blue = 0,
    .white = 255,
    .random = 0,
    .speed = 0,
    .color = 0,
    .saturation = 0,
    .brightness = 0,
};


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

static void rgbw_calc_pwm()
{
    float temp_r = 0, temp_g = 0, temp_b = 0, temp_color = 0, temp_white = 0;

    rgbw_calc_saturation(lamp_g.saturation, &temp_color, &temp_white);
    rgbw_calc_rgb(lamp_g.color, &temp_r, &temp_g, &temp_b); 

    rgbw_set_lamp(&lamp_g.red, temp_r, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.green, temp_g, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.blue, temp_b, lamp_g.brightness, temp_color, lamp_g.status);
    rgbw_set_lamp(&lamp_g.white, 0xff, lamp_g.brightness, temp_white, lamp_g.status);

#ifdef DEBUG
    printf("status%d\n", lamp_g.status);
    printf("red%d\n", lamp_g.red);
    printf("green%d\n", lamp_g.green);
    printf("blue%d\n", lamp_g.blue);
    printf("white%d\n", lamp_g.white);
#endif

    rgbw_pwm_set_duty();
}

static void rgbw_dim_light()
{
    uint8_t temp_brightness = lamp_g.brightness;
    if(previous_state == 0){
        lamp_g.status = 1;
        previous_state = 1;
        lamp_g.brightness = 0;
        while(lamp_g.brightness < temp_brightness){
            lamp_g.brightness = lamp_g.brightness + 2;
#ifdef DEBUG
            printf("upp brightness: %d\n", lamp_g.brightness);
#endif
            rgbw_calc_pwm();
            vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
        }
    }
    else{
        lamp_g.status = 1;
        previous_state = 0;
        while(lamp_g.brightness > 0){
            lamp_g.brightness = (lamp_g.brightness - 2);
            if(lamp_g.brightness < 0){
                lamp_g.brightness = 0;
            }
#ifdef DEBUG
            printf("ner brightness: %d\n",lamp_g.brightness);
#endif
            rgbw_calc_pwm();
            vTaskDelay(DIMMER_TIME_MS / portTICK_RATE_MS);
        }
        lamp_g.status = 0;
        rgbw_calc_pwm();
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
void rgbw_parse_mqtt(char *data, uint8_t len)
{
    printf("len: %d\n", len);
    int n;
    printf("data[n]: ");
    for(n = 0; n < len; n++){
        printf(" %c", data[n]);
    }
    printf("\n");
    uint8_t i;
    uint8_t array_size = SIZE;
    char char_num_arr[SIZE] = {0};
    uint16_t data_array[SIZE] = {0};
    uint8_t num_lenght = 0;
    uint8_t arr_ptr = 0;
    uint8_t arr_ptr_copy = 0;
    printf("char_num_arr: ");
    for(i = 0; i < array_size; i++){
        arr_ptr = arr_ptr_copy;
        uint8_t j = 0;
        for(j = arr_ptr; data[j] != ':' && j < len; j++){
            char_num_arr[j - arr_ptr] = data[j];
            printf(" %c", char_num_arr[j - arr_ptr]);
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
            //printf("array: %d", data_array[i]);
        }
    }
    printf("\n");
    printf("data_array[i]: ");
    for(i = 0; i < array_size; i++){
        printf(" %d", data_array[i]);
    }
    printf("\n");
    // Here we check if the random task should start or not.
    if(data_array[0] == 0){
        lamp_g.status = 0;
        lamp_g.random = 0;
    }
    if(data_array[0] == 1){
        lamp_g.status =  1;
        lamp_g.random = 0;
    }
    if(data_array[0] == 2){
        lamp_g.status = 0;
        lamp_g.random = 2;
    }
    if(data_array[0] == 3){
        lamp_g.status = 1;
        lamp_g.random = 2;
    }
    lamp_g.brightness = data_array[1];
    lamp_g.saturation = data_array[2];
    lamp_g.color = data_array[3];
    lamp_g.speed = data_array[4];

    /*if(previous_state == 0 || lamp_g.status == 0){
        rgbw_dim_light();
    }*/

    /*if(previous_state == status){
      rgbw_calc_pwm(data_array);
      if(random == 2){
      xSemaphoreGive(random_rgb);
      printf("give1\r\n");
      }
      else{
      xSemaphoreTake(random_rgb, 1);
      printf("take1\r\n");
      }
      }
      else{
      rgbw_dim_light(data_array);
      }*/
    if(lamp_g.random == 2){
        xSemaphoreGive(random_rgb);
        printf("give1\r\n");
    }
    else{
        xSemaphoreTake(random_rgb, 1);
        rgbw_calc_pwm();
        printf("take1\r\n");
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

