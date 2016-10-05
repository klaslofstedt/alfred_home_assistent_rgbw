#include "mqtt.h"
#include "wifi.h"
/*
#include <stdint.h>
#include <inttypes.h>
#include "stdint.h"
#include "inttypes.h"
#include "c_types.h"
*/

rgbw_t lamp_g = {
    .status = 0,
    .red = 0,
    .green= 0,
    .blue = 0,
    .white = 0
};
//xSemaphoreHandle parse_data = 0;

void rgb_calc(uint16_t value, float *r, float *g, float *b)
{
    if((float)value <= 255){
        *r=255;
        *g=(float)value;
        *b=0;
    }
    else if(((float)value > 255) && ((float)value <= 510)){
        *r=255-(510-(float)value);
        *g=255;
        *b=0;
    }
    else if(((float)value > 510) && ((float)value <= 765)){
        *r=0;
        *g=255;
        *b=255-(765-(float)value);
    }
    else if(((float)value > 765) && ((float)value <= 1020)){
        *r=0;
        *g=255-(1020-(float)value);
        *b=255;
    }
    else if(((float)value > 1020) && ((float)value <= 1275)){
        *r=255-(1275-(float)value);
        *g=0;
        *b=255;
    }
    else if(((float)value > 1275) && ((float)value <= 1530)){
        *r=255;
        *g=0;
        *b=1530-(float)value;
    }
    else{
        printf("color error");
    }
}

void saturation_calc(uint16_t value, float *color, float *white)
{
    if((float)value >= 0 && (float)value <= 100){
        *white = 100;
        *color = (float)value;
    }
    else if((float)value > 100 && (float)value <= 200){
        *color = 100;
        *white = 200 - (float)value;
    }
    else{
        printf("saturation error%f\n", (float)value);
    } 
}

void mqtt_message_process(char **data, uint8_t len)
{
    // Totally fucking shit code. Rewrite
    uint8_t i;
    uint8_t array_size = 4;
    char char_num_arr[4] = {0};
    uint16_t data_array[4] = {0};
    uint8_t num_lenght = 0;
    uint8_t arr_ptr = 0;
    uint8_t arr_ptr_copy = 0;
    for(i = 0; i < array_size; i++){
        arr_ptr = arr_ptr_copy;
        uint8_t j = 0;
        for(j = arr_ptr; data[j] != ':' && j < len; j++){
            char_num_arr[j-arr_ptr] = data[j];
            num_lenght++;
            arr_ptr_copy++;
        }
        arr_ptr_copy++;
        uint8_t num_lenght_copy = num_lenght;
        for(j = 0; j < num_lenght_copy; j++)
        {
            uint16_t number = (power(10, num_lenght -1) * (char_num_arr[j] - '0'));
            num_lenght--;
            data_array[i] += number;
        }
    }
    for(i = 0; i < array_size; i++){
        printf("data_array%d\n", data_array[i]);
    }

    // Not too shitty. Make a separate function for this.
    float temp_r = 0, temp_g = 0, temp_b = 0, temp_color = 0, temp_white = 0;
    saturation_calc(data_array[2], &temp_color, &temp_white);
    rgb_calc(data_array[3], &temp_r, &temp_g, &temp_b); 

    lamp_g.status = (uint8_t)(data_array[0]);
    lamp_g.red = (uint8_t)(temp_r * ((float)data_array[1] / 100) * (temp_color / 100) * data_array[0]);
    lamp_g.green = (uint8_t)(temp_g * ((float)data_array[1] / 100) * (temp_color / 100) * data_array[0]);
    lamp_g.blue = (uint8_t)(temp_b * ((float)data_array[1] / 100) * (temp_color / 100)* data_array[0]);
    lamp_g.white = (uint8_t)(0xff * ((float)data_array[1] / 100) * (temp_white / 100) * data_array[0]);
    printf("status%d\n", lamp_g.status);
    printf("red%d\n", lamp_g.red);
    printf("green%d\n", lamp_g.green);
    printf("blue%d\n", lamp_g.blue);
    printf("white%d\n", lamp_g.white);
}

void mqtt_topic_received(mqtt_message_data_t *md)
{
    char **ptr; // data payload
    int i;
    mqtt_message_t *message = md->message;
    ptr = malloc(sizeof(char *) * (int)message->payloadlen);
    printf("Received: ");
    for( i = 0; i < md->topic->lenstring.len; ++i){
        printf("%c", md->topic->lenstring.data[ i ]);
    }
    printf("\nRaw char array: ");
    for( i = 0; i < (int)message->payloadlen; ++i){
        ptr[i] = ((char *)(message->payload))[i];
        printf("%c", ptr[i]);
    }
    printf("\n");
    mqtt_message_process(ptr, message->payloadlen);
    free(ptr);
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
        mqtt_subscribe(&client, "rgbw/1", MQTT_QOS1, mqtt_topic_received);
        //xSemaphoreGive(parse_data); 
        xQueueReset(publish_queue);

        while(1){

            char msg[PUB_MSG_LEN - 1] = "\0";
            while(xQueueReceive(publish_queue, (void *)msg, 0) ==
                    pdTRUE){
                printf("got message to publish\r\n");
                mqtt_message_t message;
                message.payload = msg;
                message.payloadlen = PUB_MSG_LEN;
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = 0;
                ret = mqtt_publish(&client, "mqtt msg: ", &message);
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
        taskYIELD();
    }
}
