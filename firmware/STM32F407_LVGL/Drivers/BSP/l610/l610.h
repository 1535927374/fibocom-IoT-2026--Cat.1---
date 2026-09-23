#ifndef __L610_H__
#define __L610_H__

/*
 * Fibocom L610 on USART3 (TCP/UDP/透传，见厂家应用指南 V1.0.1).
 * AT: CGDCONT -> MIPCALL -> GTSET="IPRFMT",1 -> MIPODM -> raw 数据
 *
 * 典型启动：l610_modem_init_link → TCP 心知 → l610_weather_http_get_today →
 *          l610_tcp_transparent_close → TCP OneNET → mqtt_connect …
 */

#include "sys.h"
#include <stdint.h>

#define L610_EOK                 0
#define L610_ERROR               1
#define L610_ETIMEOUT            2
#define L610_EINVAL              3

#define L610_APN                 "CMNET"
#define L610_TCP_SERVER_IP       "mqtts.heclouds.com"
#define L610_TCP_SERVER_PORT     "1883"

/* 心知天气（明文 HTTP 80，请填写有效 key） */
#define L610_WEATHER_SERVER_IP   "api.seniverse.com"
#define L610_WEATHER_SERVER_PORT "80"
#define L610_WEATHER_API_KEY     "SUGeW62d9o19Tau7x"

void l610_init(void);

uint8_t l610_modem_init_link(void);
/* AT+CCLK?：解析 hh:mm:ss 视为 UTC，返回北京时间（+8h，86400 内取模） */
uint8_t l610_modem_query_cclk_hms(uint8_t *h, uint8_t *m, uint8_t *s);

uint8_t l610_tcp_transparent_open(const char *ip, const char *port);
void l610_tcp_transparent_close(void);

uint8_t l610_weather_http_get_today(char *out, uint16_t outsz);

void l610_receive_data(void);
void l610_send_data(char *data, uint16_t len);
uint16_t l610_copy_rxdata(char *data);
uint8_t l610_wait_receive(void);
void l610_host_rx_flush(void);

#endif
