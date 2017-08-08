#include "mqtt.h"
#include "wifi.h"


void mqtt_status(mqtt_message_data_t *md)
{
    int i;
    mqtt_message_t *message = md->message;
    char *mqtt_payload = malloc(sizeof(char) * (int)message->payloadlen);
    printf("mqtt_status: ");
    for( i = 0; i < (int)message->payloadlen; ++i){
        mqtt_payload[i] = ((char *)(message->payload))[i];
        printf("%c", mqtt_payload[i]);
    }
    printf("\n");
    uint8_t status = atoi(mqtt_payload);
    //printf("Status: %d\n", data);

    free(mqtt_payload);

    gpio_write(12, status);
}


const char *mqtt_get_my_id(void)
{
    // Use MAC address for Station as unique ID
    static char my_id[13];
    static bool my_id_done = false;
    int8_t i;
    uint8_t x;
    if (my_id_done)
        return my_id;
    if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)my_id))
        return NULL;
    for (i = 5; i >= 0; --i)
    {
        x = my_id[i] & 0x0F;
        if (x > 9) x += 7;
        my_id[i * 2 + 1] = x + '0';
        x = my_id[i] >> 4;
        if (x > 9) x += 7;
        my_id[i * 2] = x + '0';
    }
    my_id[12] = '\0';
    my_id_done = true;
    return my_id;
}

void mqtt_task(void *pvParameters)
{
    // mqtt
    int ret = 0;
    struct mqtt_network network;
    mqtt_client_t client   = mqtt_client_default;
    char mqtt_client_id[20];
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    mqtt_network_new( &network );
    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, "ESP-");
    strcat(mqtt_client_id, mqtt_get_my_id());

    while(1) {
        xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__,
                MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if( ret ){
            printf("error: %d\n\r", ret);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                mqtt_readbuf, 100);

        data.willFlag       = 0;
        data.MQTTVersion    = 3;
        data.clientID.cstring   = mqtt_client_id;
        data.username.cstring   = MQTT_USER;
        data.password.cstring   = MQTT_PASS;
        data.keepAliveInterval  = 10;
        data.cleansession   = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret){
            printf("error: %d\n\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        printf("done\r\n");

        mqtt_subscribe(&client, "rgbw/1/status", MQTT_QOS1, mqtt_status);

        xQueueReset(publish_queue);

        while(1){

            char msg[PUB_MSG_LEN - 1] = "\0";
            while(xQueueReceive(publish_queue, (void *)msg, 0) == pdTRUE){
                //printf("heartbeat\r\n");
                mqtt_message_t message;
                message.payload = msg;
                message.payloadlen = PUB_MSG_LEN;
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = 0;
                ret = mqtt_publish(&client, "rgbw/1/heartbeat", &message);
                if (ret != MQTT_SUCCESS ){
                    printf("error while publishing message: %d\n", ret );
                    break;
                }
            }

            ret = mqtt_yield(&client, 1000);
            if (ret == MQTT_DISCONNECTED)
                break;
        }
        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        //stack_size_rgbw = uxTaskGetStackHighWaterMark(NULL);
        //printf("mqtt size: %d", stack_size_mqtt);
        taskYIELD();
    }
}

void heartbeat_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char msg[PUB_MSG_LEN];
    uint8_t count = 1;
    // Delay in order to make sure the server misses a heartbeat so the saved data can be sent (server)
    // and received (esp8266) again.
    vTaskDelay(20000 / portTICK_PERIOD_MS);
    xQueueReset(publish_queue);

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, 5000 / portTICK_PERIOD_MS);
        printf("beat: %d\n", count);
        snprintf(msg, PUB_MSG_LEN, "Beat %d\r\n", count++);
        uint8_t toggle = 0;
        if (xQueueSend(publish_queue, (void *)msg, 0) == pdFALSE) {
            printf("Publish queue overflow.\r\n");
            gpio_write(13, toggle);
            toggle ^= 1;
        }
        if(count > 10){
            count = 1;
        }
    }
}
