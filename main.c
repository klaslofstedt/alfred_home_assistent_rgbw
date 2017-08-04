// make
// sudo make flash -j4 -C examples/alfred_rgbw ESPPORT=/dev/ttyUSB0
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include <semphr.h>
#include "mqtt.h"
#include "wifi.h"
#include "rgbw.h"
#include "poor_mans_pwm.h"
#include "pwm.h"


void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    //rgbw_init();

    vSemaphoreCreateBinary(wifi_alive);
    vSemaphoreCreateBinary(mqtt_new);
    //vSemaphoreCreateBinary(random_rgb);
    // Initialize random_rgb as 0 to not kickstart the rainbow_task on boot
    //xSemaphoreTake(random_rgb, 1);

    publish_queue = xQueueCreate(12, PUB_MSG_LEN);

    xTaskCreate(&wifi_task, (int8_t *)"wifi_task", 256, NULL, 2, NULL);

    xTaskCreate(&mqtt_task, (int8_t *)"mqtt_task", 1024, NULL, 5, NULL);

    xTaskCreate(&rgbw_task, (int8_t *)"rgbw_task", 1024, NULL, 4, NULL);

    xTaskCreate(&heartbeat_task, (int8_t *)"heartbeat_task", 256, NULL, 3, NULL);
    /*xTaskCreate(
            &rainbow_task,
            (int8_t *)"rainbow_task",
            256,
            NULL,
            3,
            NULL);
            */
}
