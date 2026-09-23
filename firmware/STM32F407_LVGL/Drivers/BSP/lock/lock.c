#include "lock.h"
#include "sys.h"

/* Lock relay on PB5; PB6/PB7 for BH1750 I2C1 (SCL/SDA). */
#define LOCK_GPIO_PORT   GPIOB
#define LOCK_GPIO_PIN    GPIO_PIN_5

void lock_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_initstruct.Pin       = LOCK_GPIO_PIN;
    gpio_initstruct.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_initstruct.Pull      = GPIO_PULLUP;
    gpio_initstruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LOCK_GPIO_PORT, &gpio_initstruct);

    lock_off();
}

void lock_on(void)
{
    HAL_GPIO_WritePin(LOCK_GPIO_PORT, LOCK_GPIO_PIN, GPIO_PIN_RESET);
}

void lock_off(void)
{
    HAL_GPIO_WritePin(LOCK_GPIO_PORT, LOCK_GPIO_PIN, GPIO_PIN_SET);
}

uint8_t lock_status_get(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(LOCK_GPIO_PORT, LOCK_GPIO_PIN);
}
