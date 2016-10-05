#ifndef MQTT_H
#define MGTT_H

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
#include "math_lite.h"

#define MQTT_HOST ("10.0.0.131")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL
#define PUB_MSG_LEN 16

xQueueHandle publish_queue;

typedef struct
{
    uint8_t status;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
} rgbw_t;


void mqtt_topic_received(mqtt_message_data_t *md);
const char *mqtt_get_my_id(void);
void mqtt_task(void *pvParameters);
void mqtt_message_process(char **data, uint8_t len);
void rgb_calc(uint16_t value, float *r, float *g, float *b);
void saturation_calc(uint16_t value, float *color, float *white);

#endif
