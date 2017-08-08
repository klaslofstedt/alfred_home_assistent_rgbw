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



void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    //gpio_enable(10, GPIO_OUTPUT);
    gpio_enable(12, GPIO_OUTPUT);
    gpio_enable(13, GPIO_OUTPUT);
    //gpio_write(10, 1);
    //gpio_write(12, 1);
    //gpio_write(13, 0);

    vSemaphoreCreateBinary(wifi_alive);

    publish_queue = xQueueCreate(4, PUB_MSG_LEN);

    /*queue_mqtt_status = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_color = xQueueCreate(1, sizeof(uint16_t));
    queue_mqtt_saturation = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_brightness = xQueueCreate(1, sizeof(uint8_t));*/

    //queue_mqtt_rainbow = xQueueCreate(1, sizeof(uint8_t));
    //queue_mqtt_speed = xQueueCreate(1, sizeof(uint8_t));

    xTaskCreate(&wifi_task, (const char *)"wifi_task", 256, NULL, 2, NULL);

    xTaskCreate(&mqtt_task, (const char *)"mqtt_task", 1024, NULL, 4, NULL);

    xTaskCreate(&heartbeat_task, (const char *)"heartbeat_task", 256, NULL, 3, NULL);
}
