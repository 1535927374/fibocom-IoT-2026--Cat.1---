#ifndef _ONENET_H_
#define _ONENET_H_

#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "delay.h"

#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

extern char MQTT_ClientID[100];
extern char MQTT_UserName[100];
extern char MQTT_PassWord[200];
extern volatile uint8_t g_mqtt_onenet_connected;

typedef struct
{
    uint8_t topic[512];
    uint16_t topic_len;
    uint8_t payload[512];
    uint16_t payload_len;
} Mqtt_RxData_Type;

/* ���豸֤�飨�� OneNET Studio һ�£� */
#define PRODUCT_KEY       "3zyhv1yNRL"
#define DEVICE_NAME       "myhome"
#define DEVICE_SECRET     "FZDWhpvLGp8FR0AWtzHShGn4a7o%3D"

#define RELY_PUBLISH_TOPIC  "$sys/3zyhv1yNRL/myhome/thing/property/set_reply"
#define SET_TOPIC           "$sys/3zyhv1yNRL/myhome/thing/property/set"
#define POST_TOPIC          "$sys/3zyhv1yNRL/myhome/thing/property/post"
#define EVENT_PUBLISH_TOPIC "$sys/3zyhv1yNRL/myhome/thing/event/post"

void mqtt_login_init(char *ProductKey, char *DeviceName, char *DeviceSecret);
uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos);
uint8_t mqtt_subscribe_topic(char *topic, uint8_t qos, uint8_t whether);
void mqtt_init(void);
uint8_t mqtt_connect(char *ClientID, char *Username, char *Password);
void mqtt_send_heart(void);
void mqtt_disconnect(void);
void mqtt_send_data(uint8_t *buf, uint16_t len);
void mqtt_send_response(const char *request_id, int code, const char *msg);
uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data);
/* L610/MQTT 链路 0/1/2，供 OneNET 属性 HumitureModuleStatus 上报 */
uint8_t mqtt_onenet_l610_module_status(void);

#endif
