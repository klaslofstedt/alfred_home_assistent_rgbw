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
    vSemaphoreCreateBinary(sem_mqtt_new);
    vSemaphoreCreateBinary(sem_activate_rainbow);
    //vSemaphoreCreateBinary(random_rgb);
    // Initialize random_rgb as 0 to not kickstart the rainbow_task on boot
    //xSemaphoreTake(random_rgb, 1);

    publish_queue = xQueueCreate(12, PUB_MSG_LEN);

    queue_mqtt_status = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_color = xQueueCreate(1, sizeof(uint16_t));
    queue_mqtt_saturation = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_brightness = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_rainbow = xQueueCreate(1, sizeof(uint8_t));
    queue_mqtt_speed = xQueueCreate(1, sizeof(uint8_t));

    xTaskCreate(&wifi_task, (const char *)"wifi_task", 256, NULL, 2, NULL);

    xTaskCreate(&mqtt_task, (const char *)"mqtt_task", 1024, NULL, 4, NULL);

    xTaskCreate(&rgbw_task, (const char *)"rgbw_task", 1024, NULL, 4, NULL);

    xTaskCreate(&heartbeat_task, (const char *)"heartbeat_task", 256, NULL, 3, NULL);

    xTaskCreate(&rainbow_task, (const char *)"rainbow_task", 256, NULL, 4, NULL); // maybe 3?
}
