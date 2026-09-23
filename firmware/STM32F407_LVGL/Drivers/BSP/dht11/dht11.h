#ifndef __DHT11_H__
#define __DHT11_H__

#include "sys.h"
#define DHT11_PORT               GPIOD
#define DHT11_PIN                GPIO_PIN_2
#define DHT11_CLK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()

#define DHT11_DQ_OUT(x)          do{ x ? \
                                    HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_SET) : \
                                    HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_RESET);\
                                    }while(0)

#define DHT11_DQ_IN              HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)

/* 最近一次校验成功的数据，供 LVGL 等界面读取（仅写于 dht11_read 成功分支） */
extern volatile uint8_t dht11_ui_snapshot[4];
extern volatile uint8_t dht11_ui_valid;

uint8_t dht11_read(uint8_t *result);

#endif
