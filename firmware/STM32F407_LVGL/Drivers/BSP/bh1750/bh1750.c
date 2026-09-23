#include "bh1750.h"
#include "delay.h"

volatile float bh1750_ui_lux;
volatile uint8_t bh1750_ui_valid;

/* I2C1: PB6 SCL, PB7 SDA (AF4)；门锁已迁至 PB5 */
I2C_HandleTypeDef hi2c1_bh1750;

#define BH1750_I2C_ADDR_8BIT   ((uint16_t)(BH1750_I2C_ADDR_7BIT << 1))

#define BH1750_CMD_POWER_ON   0x01U
#define BH1750_CMD_RESET       0x07U
#define BH1750_CMD_H_MODE      0x10U /* Continuously H-Resolution Mode */

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef g;

    if (hi2c->Instance != I2C1)
        return;

    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    g.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    g.Mode      = GPIO_MODE_AF_OD;
    g.Pull      = GPIO_PULLUP;
    g.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &g);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance != I2C1)
        return;

    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

void bh1750_init(void)
{
    hi2c1_bh1750.Instance             = I2C1;
    hi2c1_bh1750.Init.ClockSpeed      = 100000;
    hi2c1_bh1750.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1_bh1750.Init.OwnAddress1     = 0;
    hi2c1_bh1750.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1_bh1750.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1_bh1750.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1_bh1750.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1_bh1750) != HAL_OK)
        return;

    {
        uint8_t cmd = BH1750_CMD_POWER_ON;
        (void)HAL_I2C_Master_Transmit(&hi2c1_bh1750, BH1750_I2C_ADDR_8BIT, &cmd, 1U, 50U);
    }
    delay_ms(10);

    {
        uint8_t cmd = BH1750_CMD_H_MODE;
        (void)HAL_I2C_Master_Transmit(&hi2c1_bh1750, BH1750_I2C_ADDR_8BIT, &cmd, 1U, 50U);
    }
    delay_ms(180);
}

float bh1750_read_lux(void)
{
    uint8_t raw[2];
    uint16_t v;
    HAL_StatusTypeDef st;

    st = HAL_I2C_Master_Receive(&hi2c1_bh1750, BH1750_I2C_ADDR_8BIT, raw, 2U, 80U);
    if (st != HAL_OK) {
        bh1750_ui_valid = 0U;
        return -1.0f;
    }

    v = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    {
        float lux = (float)v / 1.2f;
        if (lux > 10000.f)
            lux = 10000.f;
        bh1750_ui_lux = lux;
        bh1750_ui_valid = 1U;
        return lux;
    }
}
