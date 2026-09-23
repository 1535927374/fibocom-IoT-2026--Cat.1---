#include "curtain.h"
#include "sys.h"

/* PC8: same drive logic as lock relay (PB5). */
#define CURTAIN_PORT   GPIOC
#define CURTAIN_PIN    GPIO_PIN_8

void curtain_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio_initstruct.Pin       = CURTAIN_PIN;
    gpio_initstruct.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_initstruct.Pull      = GPIO_PULLUP;
    gpio_initstruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(CURTAIN_PORT, &gpio_initstruct);

    curtain_off();
}

void curtain_on(void)
{
    HAL_GPIO_WritePin(CURTAIN_PORT, CURTAIN_PIN, GPIO_PIN_RESET);
}

void curtain_off(void)
{
    HAL_GPIO_WritePin(CURTAIN_PORT, CURTAIN_PIN, GPIO_PIN_SET);
}

uint8_t curtain_get_status(void)
{
    if (HAL_GPIO_ReadPin(CURTAIN_PORT, CURTAIN_PIN) == GPIO_PIN_RESET)
        return CURTAIN_STATUE_ON;
    else
        return CURTAIN_STATUE_OFF;
}
