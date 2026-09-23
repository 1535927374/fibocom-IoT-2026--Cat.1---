# include "led.h"
# include "sys.h"

TIM_HandleTypeDef pwm_handle = {0};
//init函数
void led_living_init(uint16_t arr,uint16_t psc)
{
    pwm_handle.Instance =TIM4;
    pwm_handle.Init.Prescaler=psc;
    pwm_handle.Init.Period=arr;
    pwm_handle.Init.CounterMode=TIM_COUNTERMODE_UP;
    pwm_handle.Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&pwm_handle);
    
    TIM_OC_InitTypeDef pwm_config={0};
    pwm_config.Pulse=0;
    pwm_config.OCMode=TIM_OCMODE_PWM1;
    pwm_config.OCPolarity=TIM_OCPOLARITY_HIGH;
    HAL_TIM_PWM_ConfigChannel(&pwm_handle,&pwm_config,TIM_CHANNEL_1);
    
    HAL_TIM_PWM_Start(&pwm_handle,TIM_CHANNEL_1);
    
}

//msp函数
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM4)
    {
         GPIO_InitTypeDef gpio_struct={0};
         /* TIM4_CH1：使用 PD12，避免与门锁继电器（PB5，见 lock.c）等冲突 */
         __HAL_RCC_GPIOD_CLK_ENABLE();
         __HAL_RCC_TIM4_CLK_ENABLE();

         gpio_struct.Mode = GPIO_MODE_AF_PP;
         gpio_struct.Pin = GPIO_PIN_12;
         gpio_struct.Pull = GPIO_PULLUP;
         gpio_struct.Speed = GPIO_SPEED_FREQ_HIGH;
         gpio_struct.Alternate = GPIO_AF2_TIM4;
         HAL_GPIO_Init(GPIOD, &gpio_struct);
    }
    
    
}
//修改ccr的值
void pwm_compare_set(uint16_t val)
{
    __HAL_TIM_SET_COMPARE(&pwm_handle,TIM_CHANNEL_1,val);
}
void led_living_off(void)
{
    pwm_compare_set(0);
}
void led_living_low(void)
{
    pwm_compare_set(150);
}
void led_living_medium(void)
{
    pwm_compare_set(300);
}
void led_living_high(void)
{
    pwm_compare_set(450);
}

//初始化GPIO函数
void led_bedroom_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    //打开时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    //调用GPIO初始化函数
    gpio_initstruct.Mode=GPIO_MODE_OUTPUT_PP;               //推挽输出
    gpio_initstruct.Pin=GPIO_PIN_11;            //LED1和LED2对应的引脚
    gpio_initstruct.Pull=GPIO_PULLUP;                       //上拉
    gpio_initstruct.Speed=GPIO_SPEED_FREQ_HIGH;             //高速
    HAL_GPIO_Init(GPIOA,&gpio_initstruct);
    //关闭LED
    led_bedroom_off();
    
}
//点亮LED1的函数
void led_bedroom_on(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);     //拉低LED1引脚，点亮LED1
}
//熄灭LED1的函数
void led_bedroom_off(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);       //拉高LED1引脚，熄灭LED1
}
void led_init(void)
{
    led_bedroom_init();
    led_living_init(500-1,72-1);
    led_living_off();
}
