#include "voice.h"

void voice_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    //?????
    __HAL_RCC_GPIOC_CLK_ENABLE();
    //????GPIO?????????
    gpio_initstruct.Mode=GPIO_MODE_INPUT;                   //????
    /* ???��???????�PC7??PC9 ??????? 0~3 ?? ????????????? light_control_task ??????? */
    gpio_initstruct.Pin = GPIO_PIN_7 | GPIO_PIN_9;
    gpio_initstruct.Pull=GPIO_PULLUP;                       //????
    gpio_initstruct.Speed=GPIO_SPEED_FREQ_HIGH;             //????
    HAL_GPIO_Init(GPIOC,&gpio_initstruct);
}
uint8_t voice_value_get(void)
{
    uint8_t b7 = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_SET) ? 1u : 0u;
    uint8_t b9 = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_SET) ? 1u : 0u;
    return (uint8_t)((b7 << 1) | b9);
}

uint8_t voice_onenet_sound_module_status(void)
{
    /*
     * 当前语音为 GPIOC 二线编码（voice_value_get 0~3），无独立总线；
     * 平台「通信失败」多用于 UART/I2C 语音芯片；此处恒报 0=通信正常。
     * 若以后接串口语音模块，可在此根据 ACK/超时返回 1 或 2。
     */
    (void)voice_value_get();
    return 0U;
}

uint8_t voice_living_level_from_raw(uint8_t raw)
{
    /*
     * raw 与灯档对应（按你现象「说中亮却低亮」校正）：
     * 原逻辑 led_status = raw，若硬件「中亮」输出 raw=1（而 1 被当作低亮），
     * 则需交换 raw 1 与 2 的含义：1→中(2)、2→低(1)。
     * 若你模块顺序不同，只改此表四元组即可。
     */
    static const uint8_t map[4] = { 0u, 2u, 1u, 3u };
    return map[(unsigned)raw & 3u];
}
