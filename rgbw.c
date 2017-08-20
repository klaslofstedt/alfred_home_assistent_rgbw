#include "rgbw.h"
//#include "mqtt.h"
#include "math_lite.h"
#include "poor_mans_pwm.h"
#include <FreeRTOS.h>
#include <semphr.h>
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


void rgbw_status(uint8_t value)
{
  if(value == 0 || value == 1){
    lamp_g.status = value;
  }

  else{ // status = on/off rainbow
    lamp_g.rainbow = value - 2;
    xSemaphoreGive(rainbow_sem);
  }
}

void rgbw_color(uint16_t value)
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


void rgbw_start_lamp()
{
  if(lamp_g.saturation >= 0 && lamp_g.saturation < 200/3){
    lamp_g.color = 0;
    lamp_g.white = 1;
  }
  else if (lamp_g.saturation >= 200/3 && lamp_g.saturation < 2*200/3){
    lamp_g.color = 1;
    lamp_g.white = 1;
  }
  else{
    lamp_g.color = 1;
    lamp_g.white = 0;
  }

  // red
  if(lamp_g.rgb >= 0 && lamp_g.rgb < 127){
    lamp_g.red = 1;
    lamp_g.green = 0;
    lamp_g.blue = 0;
  }// yellow
  else if(lamp_g.rgb >= 127 && lamp_g.rgb < 255 + 127){
    lamp_g.red = 1;
    lamp_g.green = 1;
    lamp_g.blue = 0;
  } // green
  else if(lamp_g.rgb >= 255 + 127 && lamp_g.rgb < 2*255 + 127){
    lamp_g.red = 0;
    lamp_g.green = 1;
    lamp_g.blue = 0;
  } // green blue
  else if(lamp_g.rgb >= 2*255 + 127 && lamp_g.rgb < 3*255 + 127){
    lamp_g.red = 0;
    lamp_g.green = 1;
    lamp_g.blue = 1;
  } // blue
  else if(lamp_g.rgb >= 3*255 + 127 && lamp_g.rgb < 4*255 + 127){
    lamp_g.red = 0;
    lamp_g.green = 0;
    lamp_g.blue = 1;
  } // purple
  else if(lamp_g.rgb >= 4*255 + 127 && lamp_g.rgb < 5*255 + 127){
    lamp_g.red = 1;
    lamp_g.green = 0;
    lamp_g.blue = 1;
  } //red
  else if(lamp_g.rgb >= 5*255 + 127 && lamp_g.rgb <= 6*255){
    lamp_g.red = 1;
    lamp_g.green = 0;
    lamp_g.blue = 0;
  }
  gpio_write(SWITCH, lamp_g.status);
  gpio_write(WHITE, lamp_g.white * lamp_g.status);
  gpio_write(RED, lamp_g.red * lamp_g.color * lamp_g.status);
  gpio_write(GREEN, lamp_g.green * lamp_g.color * lamp_g.status);
  gpio_write(BLUE, lamp_g.blue * lamp_g.color * lamp_g.status);
}

void rainbow_task(void *pvParameters)
{
  while (1) {
    xSemaphoreTake(rainbow_sem, portMAX_DELAY);
    printf("rainbow: %d\n", lamp_g.rainbow);
    lamp_g.saturation = 200;
    lamp_g.rgb += 0xff;
    if(lamp_g.rgb >= 1530){
      lamp_g.rgb = 0;
    }
    lamp_g.brightness = 100;

    rgbw_start_lamp();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    if(lamp_g.rainbow == 1){
      xSemaphoreGive(rainbow_sem);
    }

  }
}



void rgbw_init()
{
  // some inits
  gpio_enable(SWITCH, GPIO_OUTPUT);
  gpio_write(SWITCH, 0);

  gpio_enable(RED, GPIO_OUTPUT);
  gpio_write(RED, 0);

  gpio_enable(GREEN, GPIO_OUTPUT);
  gpio_write(GREEN, 0);

  gpio_enable(BLUE, GPIO_OUTPUT);
  gpio_write(BLUE, 0);

  gpio_enable(WHITE, GPIO_OUTPUT);
  gpio_write(WHITE, 1);
}
