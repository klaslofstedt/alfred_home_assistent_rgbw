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

#define MQTT_HOST ("10.0.0.131")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL
#define PUB_MSG_LEN 16

#define PLUG_PIN 12

QueueHandle_t publish_queue;


void mqtt_status(mqtt_message_data_t *md);
const char *mqtt_get_my_id(void);
void mqtt_message_process(char **data, uint8_t len);
void mqtt_task(void *pvParameters);
void heartbeat_task(void *pvParameters);

#endif
