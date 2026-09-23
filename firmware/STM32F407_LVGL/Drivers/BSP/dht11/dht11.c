#include "dht11.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"

/* 单总线异常时避免 while 死等（次数级近似超时，与主频相关） */
#define DHT11_SPIN_MAX  200000u

char dht11_data[5] = {0};

volatile uint8_t dht11_ui_snapshot[4] = {0};
volatile uint8_t dht11_ui_valid = 0;

void dht11_gpio_input(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    DHT11_CLK_ENABLE();                         
    
    gpio_initstruct.Pin = DHT11_PIN;        
    gpio_initstruct.Mode = GPIO_MODE_INPUT;           
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;         
    HAL_GPIO_Init(DHT11_PORT, &gpio_initstruct);
}

void dht11_gpio_output(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    DHT11_CLK_ENABLE();                         
    
    gpio_initstruct.Pin = DHT11_PIN;        
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;           
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;         
    HAL_GPIO_Init(DHT11_PORT, &gpio_initstruct);
}

/* 等待引脚变为指定电平，超时返回 -1 */
static int dht11_wait_level(uint8_t want_high)
{
    uint32_t n = 0;
    if (want_high) {
        while (!DHT11_DQ_IN) {
            if (++n > DHT11_SPIN_MAX)
                return -1;
        }
    } else {
        while (DHT11_DQ_IN) {
            if (++n > DHT11_SPIN_MAX)
                return -1;
        }
    }
    return 0;
}

/* 启动序列；失败返回 -1 */
static int dht11_start(void)
{
    dht11_gpio_output();
    DHT11_DQ_OUT(1);
    DHT11_DQ_OUT(0);
    delay_ms(20);
    DHT11_DQ_OUT(1);

    dht11_gpio_input();
    if (dht11_wait_level(0) < 0)
        return -1;
    if (dht11_wait_level(1) < 0)
        return -1;
    if (dht11_wait_level(0) < 0)
        return -1;
    return 0;
}

uint8_t dht11_read_byte(void)
{
    uint8_t temp = 0;
    uint8_t i = 0;
    uint8_t read_data = 0;

    for (i = 0; i < 8; i++) {
        if (dht11_wait_level(1) < 0)
            return 0xFF;
        delay_us(50);
        if (DHT11_DQ_IN == 1) {
            temp = 1;
            if (dht11_wait_level(0) < 0)
                return 0xFF;
        } else {
            temp = 0;
        }

        read_data = (uint8_t)((read_data << 1) | temp);
    }

    return read_data;
}

uint8_t dht11_read(uint8_t *result)
{
    uint8_t i = 0;

    if (dht11_start() < 0)
        return 0;
    dht11_gpio_input();

    for (i = 0; i < 5; i++)
        dht11_data[i] = dht11_read_byte();
    
    if (dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3] == dht11_data[4]) {
        memcpy(result, dht11_data, 4);
        dht11_ui_snapshot[0] = (uint8_t)dht11_data[0];
        dht11_ui_snapshot[1] = (uint8_t)dht11_data[1];
        dht11_ui_snapshot[2] = (uint8_t)dht11_data[2];
        dht11_ui_snapshot[3] = (uint8_t)dht11_data[3];
        dht11_ui_valid = 1;
        return 1;
    }

    return 0;
}

