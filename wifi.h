#ifndef WIFI_H
#define WIFI_H

#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

//#include <paho_mqtt_c/MQTTESP8266.h>
//#include <paho_mqtt_c/MQTTClient.h>

#include <semphr.h>

SemaphoreHandle_t wifi_alive;

void wifi_task(void *pvParameters);

#endif
