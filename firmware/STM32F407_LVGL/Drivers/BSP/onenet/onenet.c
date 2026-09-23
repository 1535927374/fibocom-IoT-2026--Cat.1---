#include "onenet.h"
#include "l610.h"
#include "uart1.h"
#include <stdio.h>

char MQTT_ClientID[100];
char MQTT_UserName[100];
char MQTT_PassWord[200];

uint8_t *mqtt_rxbuf;
uint8_t *mqtt_txbuf;
uint16_t mqtt_rxlen;
uint16_t mqtt_txlen;

/* OneNET「L610/蜂窝链路」：MQTT(CONNACK 成功) 置 1，断开或重连尝试前置 0 */
volatile uint8_t g_mqtt_onenet_connected;
uint8_t _mqtt_txbuf[512];
uint8_t _mqtt_rxbuf[512];

typedef enum
{
    M_RESERVED1 = 0,
    M_CONNECT,
    M_CONNACK,
    M_PUBLISH,
    M_PUBACK,
    M_PUBREC,
    M_PUBREL,
    M_PUBCOMP,
    M_SUBSCRIBE,
    M_SUBACK,
    M_UNSUBSCRIBE,
    M_UNSUBACK,
    M_PINGREQ,
    M_PINGRESP,
    M_DISCONNECT,
    M_RESERVED2
} _typdef_mqtt_message;

const uint8_t parket_connetAck[] = {0x20, 0x02, 0x00, 0x00};
const uint8_t parket_disconnet[] = {0xe0, 0x00};
const uint8_t parket_heart[] = {0xc0, 0x00};
const uint8_t parket_heart_reply[] = {0xc0, 0x00};
const uint8_t parket_subAck[] = {0x90, 0x03};

void mqtt_login_init(char *ProductKey, char *DeviceName, char *DeviceSecret)
{
    (void)snprintf(MQTT_ClientID, sizeof(MQTT_ClientID), "%s", DeviceName);
    (void)snprintf(MQTT_UserName, sizeof(MQTT_UserName), "%s", ProductKey);
    (void)snprintf(MQTT_PassWord, sizeof(MQTT_PassWord),
            "version=2018-10-31&res=products%%2F%s%%2Fdevices%%2F%s&et=2017881776&method=sha1&sign=%s",
            ProductKey, DeviceName, DeviceSecret);
}

/*
 * ???L610 ???? l610_init() ???? TCP ??????????????????????? DISCONNECT????
 * ??????????? mqtt_init ?????? mqtt_disconnect ??? 0xE0 0x00 ???????? OneNet??
 * ????? Broker ???????????????????????? CONNACK??
 * ???????? USART3 ???????????????????
 */
void mqtt_init(void)
{
    mqtt_login_init(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);

    mqtt_rxbuf = _mqtt_rxbuf;
    mqtt_rxlen = sizeof(_mqtt_rxbuf);
    mqtt_txbuf = _mqtt_txbuf;
    mqtt_txlen = sizeof(_mqtt_txbuf);

    memset(mqtt_rxbuf, 0, mqtt_rxlen);
    memset(mqtt_txbuf, 0, mqtt_txlen);

    uart3_rx_clear();
    delay_ms(50);
}

/* MQTT 3.1.1 CONNACK: 0x20, 0x02, ack flags, return code(0=???) */
static int mqtt_find_connack(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (len < 4)
        return -1;

    for (i = 0; i + 4 <= len; i++)
    {
        if (buf[i] == 0x20 && buf[i + 1] == 0x02)
        {
            if (buf[i + 3] != 0)
                return -2;
            return (int)i;
        }
    }
    return -1;
}

static int mqtt_find_suback_prefix(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (len < 2)
        return -1;

    for (i = 0; i + 2 <= len; i++)
    {
        if (buf[i] == parket_subAck[0] && buf[i + 1] == parket_subAck[1])
            return (int)i;
    }
    return -1;
}

uint8_t mqtt_connect(char *ClientID, char *Username, char *Password)
{
    uint8_t j;
    int ClientIDLen = strlen(ClientID);

    g_mqtt_onenet_connected = 0U;
    int UsernameLen = strlen(Username);
    int PasswordLen = strlen(Password);
    int DataLen;
    int ack;

    mqtt_txlen = 0;
    DataLen = 10 + (ClientIDLen + 2) + (UsernameLen + 2) + (PasswordLen + 2);

    mqtt_txbuf[mqtt_txlen++] = 0x10;

    do
    {
        uint8_t encodedByte = (uint8_t)(DataLen % 128);
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte |= 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (DataLen > 0);

    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = 4;
    mqtt_txbuf[mqtt_txlen++] = 'M';
    mqtt_txbuf[mqtt_txlen++] = 'Q';
    mqtt_txbuf[mqtt_txlen++] = 'T';
    mqtt_txbuf[mqtt_txlen++] = 'T';
    mqtt_txbuf[mqtt_txlen++] = 4;
    mqtt_txbuf[mqtt_txlen++] = 0xc2;
    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = 100;

    mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen);
    mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen);
    memcpy(&mqtt_txbuf[mqtt_txlen], ClientID, (size_t)ClientIDLen);
    mqtt_txlen += (uint16_t)ClientIDLen;

    if (UsernameLen > 0)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen);
        memcpy(&mqtt_txbuf[mqtt_txlen], Username, (size_t)UsernameLen);
        mqtt_txlen += (uint16_t)UsernameLen;
    }

    if (PasswordLen > 0)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen);
        memcpy(&mqtt_txbuf[mqtt_txlen], Password, (size_t)PasswordLen);
        mqtt_txlen += (uint16_t)PasswordLen;
    }

    uart3_rx_clear();
    memset(mqtt_rxbuf, 0, mqtt_rxlen);

    mqtt_send_data(mqtt_txbuf, mqtt_txlen);

    for (j = 0; j < 20; j++)
    {
        delay_ms(400);
        if (l610_wait_receive() == L610_EOK)
        {
            uint16_t rxn = l610_copy_rxdata((char *)mqtt_rxbuf);
            ack = mqtt_find_connack(mqtt_rxbuf, rxn);
            if (ack == -2)
            {
                printf("MQTT CONNACK refused (code!=0), check token/et/sign\r\n");
            }
            if (ack >= 0)
            {
                uart3_rx_clear();
                g_mqtt_onenet_connected = 1U;
                return 0;
            }
        }
    }
    g_mqtt_onenet_connected = 0U;
    return 1;
}

uint8_t mqtt_subscribe_topic(char *topic, uint8_t qos, uint8_t whether)
{
    uint8_t j;
    int topiclen = strlen(topic);
    int DataLen = 2 + (topiclen + 2) + (whether ? 1 : 0);

    mqtt_txlen = 0;

    if (whether)
        mqtt_txbuf[mqtt_txlen++] = 0x82;
    else
        mqtt_txbuf[mqtt_txlen++] = 0xA2;

    do
    {
        uint8_t encodedByte = (uint8_t)(DataLen % 128);
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte |= 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (DataLen > 0);

    mqtt_txbuf[mqtt_txlen++] = 0;
    mqtt_txbuf[mqtt_txlen++] = 0x01;

    mqtt_txbuf[mqtt_txlen++] = BYTE1(topiclen);
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topiclen);
    memcpy(&mqtt_txbuf[mqtt_txlen], topic, (size_t)topiclen);
    mqtt_txlen += (uint16_t)topiclen;

    if (whether)
        mqtt_txbuf[mqtt_txlen++] = qos;

    memset(mqtt_rxbuf, 0, mqtt_rxlen);
    mqtt_send_data(mqtt_txbuf, mqtt_txlen);

    for (j = 0; j < 15; j++)
    {
        delay_ms(80);
        if (l610_wait_receive() == L610_EOK)
        {
            uint16_t rxn = l610_copy_rxdata((char *)mqtt_rxbuf);
            if (mqtt_find_suback_prefix(mqtt_rxbuf, rxn) >= 0)
            {
                uart3_rx_clear();
                return 0;
            }
        }
    }
    return 1;
}

uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{
    int topicLength = strlen(topic);
    int messageLength = strlen(message);
    static uint16_t id = 0;
    int DataLen;

    mqtt_txlen = 0;

    if (qos)
        DataLen = (2 + topicLength) + 2 + messageLength;
    else
        DataLen = (2 + topicLength) + messageLength;

    mqtt_txbuf[mqtt_txlen++] = 0x30;

    do
    {
        uint8_t encodedByte = (uint8_t)(DataLen % 128);
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte |= 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    } while (DataLen > 0);

    mqtt_txbuf[mqtt_txlen++] = BYTE1(topicLength);
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topicLength);
    memcpy(&mqtt_txbuf[mqtt_txlen], topic, (size_t)topicLength);
    mqtt_txlen += (uint16_t)topicLength;

    if (qos)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
        id++;
    }

    memcpy(&mqtt_txbuf[mqtt_txlen], message, (size_t)messageLength);
    mqtt_txlen += (uint16_t)messageLength;

    mqtt_send_data(mqtt_txbuf, mqtt_txlen);
    return mqtt_txlen;
}

uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data)
{
    uint8_t *p;
    uint8_t encodeByte = 0;
    uint32_t multiplier = 1, Remaining_len = 0;
    uint8_t QS_level = 0;

    p = data_received;
    memset(rx_data, 0, sizeof(Mqtt_RxData_Type));

    if ((*p != 0x30) && (*p != 0x32) && (*p != 0x34))
        return 1;

    if (*p != 0x30)
        QS_level = 1;

    p++;
    do
    {
        encodeByte = *p++;
        Remaining_len += (encodeByte & 0x7F) * multiplier;
        multiplier *= 128;

        if (multiplier > 128 * 128 * 128)
            return 2;
    } while ((encodeByte & 0x80) != 0);

    rx_data->topic_len = *p++;
    rx_data->topic_len = rx_data->topic_len * 256 + *p++;
    memcpy(rx_data->topic, p, rx_data->topic_len);
    p += rx_data->topic_len;

    if (QS_level != 0)
        p += 2;

    rx_data->payload_len = Remaining_len - rx_data->topic_len - 2;
    memcpy(rx_data->payload, p, rx_data->payload_len);

    return 0;
}

void mqtt_send_response(const char *request_id, int code, const char *msg)
{
    char body[384];

    if (request_id == NULL)
        request_id = "";
    if (msg == NULL)
        msg = "";

    /* Studio 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 id 一锟铰ｏ拷锟斤拷锟街接匡拷锟斤拷志锟街讹拷锟斤拷为 request_id锟斤拷一锟斤拷锟斤拷锟斤拷 */
    snprintf(body, sizeof(body),
             "{\"id\":\"%s\",\"code\":%d,\"msg\":\"%s\",\"request_id\":\"%s\",\"data\":null}",
             request_id, code, msg, request_id);
    mqtt_publish_data(RELY_PUBLISH_TOPIC, body, 0);
}

void mqtt_send_heart(void)
{
    mqtt_send_data((uint8_t *)parket_heart, sizeof(parket_heart));
}

void mqtt_disconnect(void)
{
    mqtt_send_data((uint8_t *)parket_disconnet, sizeof(parket_disconnet));
    g_mqtt_onenet_connected = 0U;
}

/*
 * 供 OneNET 属性 HumitureModuleStatus 上报（平台侧标识名；语义仍为 L610 上云链路）。
 * 枚举：0 通信正常 1 通信失败 2 设备异常(预留)
 * 以 MQTT CONNACK 成功代表经 L610 透传会话已建立；未自动重连时掉线后仍可能误报 0，见 g_mqtt_onenet_connected 说明。
 */
uint8_t mqtt_onenet_l610_module_status(void)
{
    if (g_mqtt_onenet_connected != 0U)
        return 0U;
    return 1U;
}

void mqtt_send_data(uint8_t *buf, uint16_t len)
{
    l610_send_data((char *)buf, len);
}
