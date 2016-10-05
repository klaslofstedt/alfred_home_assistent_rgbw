// make
// sudo make flash -j4 -C examples/alfred_rgbw ESPPORT=/dev/ttyUSB0
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include <semphr.h>
#include "pwm.h"
#include "mqtt.h"
#include "wifi.h"


//xSemaphoreHandle wifi_alive;
xSemaphoreHandle pwm_set = 0;
//xQueueHandle publish_queue;

//void pwm_setup(void);

/*
   static void beat_task(void *pvParameters)
   {
   portTickType xLastWakeTime = xTaskGetTickCount();
   char msg[PUB_MSG_LEN];
   int count = 0;

   while (1) {
// just a fancy way of doing vTaskDelay(delay)?
// Probably good to use for 400Hz on quadcopter
vTaskDelayUntil(&xLastWakeTime, 10000 / portTICK_RATE_MS);
printf("beat\r\n");
snprintf(msg, PUB_MSG_LEN, "Beat %d\r\n", count++);
if (xQueueSend(publish_queue, (void *)msg, 0) == pdFALSE) {
printf("Publish queue overflow.\r\n");
}
}
}*/

void pwm_setup(void);

static void pwm_task(void *pvParameters)
{
    pwm_setup();
    
    while(1){
        if(xSemaphoreTake(pwm_set, portMAX_DELAY)){
            // set the pwms
        }
    }
}
void pwm_setup(void)
{
    uint8_t pins[4];
    pins[0] = 12; // red
    pins[1] = 13; // green
    pins[2] = 14; // blue
    pins[3] = 15; // yellow (white)

    pwm_init(4, pins);
    pwm_set_freq(1000);
    pwm_set_duty(UINT16_MAX/2);
    pwm_start();
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    vSemaphoreCreateBinary(wifi_alive);
    //vSemaphoreCreateBinary(wifi_alive);
    vSemaphoreCreateBinary(pwm_set);

    publish_queue = xQueueCreate(3, PUB_MSG_LEN);

    xTaskCreate(
            &wifi_task, 
            (int8_t *)"wifi_task", 
            256, 
            NULL, 
            2, 
            NULL);

    //xTaskCreate(&beat_task, (int8_t *)"beat_task", 256, NULL, 3, NULL);
    xTaskCreate(
            &mqtt_task, 
            (int8_t *)"mqtt_task", 
            1024, 
            NULL, 
            3, 
            NULL);

    /*
    xTaskCreate(
            &parse_task,
            (int8_t *)"parse_task", 
            256, 
            NULL, 
            4, 
            NULL);
            */

    xTaskCreate(
            &pwm_task, 
            (int8_t *)"pwm_task", 
            256, 
            NULL, 
            5, 
            NULL);
}
