#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
extern void xPortSysTickHandler(void);
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) /* OS开始跑了,才执行正常
的调度处理 */
    {
        xPortSysTickHandler();
    }
}
void delay_init(void)
{
    SysTick_Config(HAL_RCC_GetHCLKFreq()/configTICK_RATE_HZ);
}

/**
  * @brief  微秒级延时
  * @param  nus 延时时长，范围：0~233015
  * @retval 无
  */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t tcnt = 0, told, tnow;
    uint32_t reload = SysTick->LOAD;                //重装载值
    
    ticks = nus * 72;                               //需要计的节拍数
    told = SysTick->VAL;                            //刚进入while循环时计数器的值
    
    while(1)
    {
        tnow = SysTick->VAL;
        if(tnow != told)
        {
            if(tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - (tnow -told);
            
            told = tnow;                            //下次进入while循环时，当前VAL的值作为told
            
            if(tcnt >= ticks)                       //已计的数超过/等于需要计的数时，退出循环
                break;
        }
    }
}

/**
  * @brief  毫秒级延时
  * @param  nms 延时时长，范围：0~4294967295
  * @retval 无
  */
void delay_ms(uint32_t nms)
{
    while(nms--)
        delay_us(1000);
}
 
/**
  * @brief  秒级延时
  * @param  ns 延时时长，范围：0~4294967295
  * @retval 无
  */
void delay_s(uint32_t ns)
{
    while(ns--)
        delay_ms(1000);
}

/**
  * @brief  重写HAL_Delay函数
  * @param  nms 延时时长，范围：0~4294967295
  * @retval 无
  */
void HAL_Delay(uint32_t nms)
{
    delay_ms(nms);
}
