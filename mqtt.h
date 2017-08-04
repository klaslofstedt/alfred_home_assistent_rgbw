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

typedef struct
{
    uint8_t status;
    uint16_t color;
    uint8_t white;
    // raw data before conversion
    uint8_t saturation;
    uint8_t brightness;
    uint8_t random;
    uint8_t speed;
} rgbw_lamp_t;

xQueueHandle publish_queue;


void mqtt_status(mqtt_message_data_t *md);
void mqtt_color(mqtt_message_data_t *md);
void mqtt_brightness(mqtt_message_data_t *md);
void mqtt_saturation(mqtt_message_data_t *md);
const char *mqtt_get_my_id(void);
void mqtt_message_process(char **data, uint8_t len);
void mqtt_task(void *pvParameters);
void heartbeat_task(void *pvParameters);

#endif
