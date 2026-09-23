/**
 ****************************************************************************************************
 * @file        uart.c
 * @author      正点原子风格重构
 * @version     V1.0
 * @date        2026-04-09
 * @brief       串口初始化代码(USART1/2/3)，支持printf
 * @license     Copyright (c) 2020-2032
 ****************************************************************************************************
 * @attention
 *
 * STM32F407ZGT6 三串口配置：
 * USART1: TX-PA9,  RX-PA10 (调试串口)
 * USART2: TX-PA2,  RX-PA3 (蓝牙模块，AF7)
 * USART3: TX-PB10, RX-PB11 (广和通 L610 4G)
 *
 ****************************************************************************************************
 */

#include "sys.h"
#include "uart1.h"
#include "string.h"

/* 如果使用os,则包括下面的头文件即可 */
#if SYS_SUPPORT_OS
#include "os.h"                               /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");          /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口1 */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */
    USART1->DR = (uint8_t)ch;                       /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/

/* 串口句柄定义 */
UART_HandleTypeDef g_uart1_handle;                  /* USART1句柄 */
UART_HandleTypeDef g_uart2_handle;                  /* USART2句柄 */
UART_HandleTypeDef g_uart3_handle;                  /* USART3句柄 */

/* 接收缓冲区定义 */
uint8_t g_uart1_rx_buf[UART_RX_BUF_SIZE];          /* USART1接收缓冲 */
uint8_t g_uart2_rx_buf[UART_RX_BUF_SIZE];          /* USART2接收缓冲 (蓝牙) */
uint8_t g_uart3_rx_buf[UART3_RX_BUF_SIZE];         /* USART3接收缓冲 (L610) */

/* 接收状态变量 */
uint16_t g_uart1_rx_sta = 0;                        /* USART1接收状态 */
uint16_t g_uart2_rx_sta = 0;                        /* USART2接收状态 */
uint16_t g_uart3_rx_sta = 0;                        /* USART3接收状态 */

/* HAL库接收缓冲 */
uint8_t g_uart1_rx_byte;                             /* USART1单字节接收缓冲 */
uint8_t g_uart2_rx_byte;                             /* USART2单字节接收缓冲 */
uint8_t g_uart3_rx_byte;                             /* USART3单字节接收缓冲 */

/**
 * @brief USART2 总线空闲回调（弱定义，蓝牙等模块可重写）
 */
__weak void uart2_idle_callback(uint8_t *buf, uint16_t len)
{
    (void)buf;
    (void)len;
}

/**
 * @brief       USART1 初始化函数
 * @param       baudrate: 波特率
 * @retval      无
 */
void uart1_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART1;
    g_uart1_handle.Init.BaudRate = baudrate;
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&g_uart1_handle);
    
    /* 开启接收中断 */
    HAL_UART_Receive_IT(&g_uart1_handle, &g_uart1_rx_byte, 1);
}

/**
 * @brief       USART2 初始化函数 (蓝牙等模块)
 * @param       baudrate: 波特率
 * @retval      无
 */
void uart2_init(uint32_t baudrate)
{
    g_uart2_handle.Instance = USART2;
    g_uart2_handle.Init.BaudRate = baudrate;
    g_uart2_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart2_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart2_handle.Init.Parity = UART_PARITY_NONE;
    g_uart2_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart2_handle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&g_uart2_handle);

    __HAL_UART_ENABLE_IT(&g_uart2_handle, UART_IT_IDLE);

    HAL_UART_Receive_IT(&g_uart2_handle, &g_uart2_rx_byte, 1);
}

/**
 * @brief       USART3 初始化函数 (广和通 L610)
 * @param       baudrate: 波特率
 * @retval      无
 */
void uart3_init(uint32_t baudrate)
{
    g_uart3_handle.Instance = USART3;
    g_uart3_handle.Init.BaudRate = baudrate;
    g_uart3_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart3_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart3_handle.Init.Parity = UART_PARITY_NONE;
    g_uart3_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart3_handle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&g_uart3_handle);
    
    /* 开启接收中断 */
    HAL_UART_Receive_IT(&g_uart3_handle, &g_uart3_rx_byte, 1);
}

/**
 * @brief       UART底层初始化函数
 * @param       huart: UART句柄类型指针
 * @note        此函数会被HAL_UART_Init()调用
 * @retval      无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9-TX, PA10-RX */
        gpio_init_struct.Pin = GPIO_PIN_9;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);

        gpio_init_struct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA2-TX, PA3-RX */
        gpio_init_struct.Pin = GPIO_PIN_2;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);

        gpio_init_struct.Pin = GPIO_PIN_3;
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART2_IRQn, 3, 2);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if (huart->Instance == USART3)
    {
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* PB10-TX, PB11-RX (L610) */
        gpio_init_struct.Pin = GPIO_PIN_10;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(GPIOB, &gpio_init_struct);

        gpio_init_struct.Pin = GPIO_PIN_11;
        HAL_GPIO_Init(GPIOB, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
}

/**
 * @brief       Rx传输回调函数
 * @param       huart: UART句柄类型指针
 * @retval      无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if ((g_uart1_rx_sta & 0x8000) == 0)
        {
            if (g_uart1_rx_byte == 0x0d)
            {
                g_uart1_rx_sta |= 0x4000;
            }
            else if (g_uart1_rx_byte == 0x0a)
            {
                if (g_uart1_rx_sta & 0x4000)
                {
                    g_uart1_rx_sta |= 0x8000;
                }
                else
                {
                    g_uart1_rx_sta = 0;
                }
            }
            else
            {
                g_uart1_rx_buf[g_uart1_rx_sta & 0x3FFF] = g_uart1_rx_byte;
                g_uart1_rx_sta++;
                if (g_uart1_rx_sta > (UART_RX_BUF_SIZE - 1))
                {
                    g_uart1_rx_sta = 0;
                }
            }
        }
        HAL_UART_Receive_IT(&g_uart1_handle, &g_uart1_rx_byte, 1);
    }
    else if (huart->Instance == USART2)
    {
        if (g_uart2_rx_sta >= sizeof(g_uart2_rx_buf))
        {
            g_uart2_rx_sta = 0;
        }
        g_uart2_rx_buf[g_uart2_rx_sta++] = g_uart2_rx_byte;
        HAL_UART_Receive_IT(&g_uart2_handle, &g_uart2_rx_byte, 1);
    }
    else if (huart->Instance == USART3)
    {
        /* L610 / USART3 接收 */
        if (g_uart3_rx_sta >= sizeof(g_uart3_rx_buf))
        {
            g_uart3_rx_sta = 0;
        }
        g_uart3_rx_buf[g_uart3_rx_sta++] = g_uart3_rx_byte;
        HAL_UART_Receive_IT(&g_uart3_handle, &g_uart3_rx_byte, 1);
    }
}

/**
 * @brief       USART1 中断服务函数
 * @retval      无
 */
void USART1_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    HAL_UART_IRQHandler(&g_uart1_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

/**
 * @brief       USART2 中断服务函数 (蓝牙)
 * @retval      无
 */
void USART2_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    if (__HAL_UART_GET_FLAG(&g_uart2_handle, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&g_uart2_handle);
        uart2_idle_callback(g_uart2_rx_buf, g_uart2_rx_sta);
    }
    HAL_UART_IRQHandler(&g_uart2_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

/**
 * @brief       USART3 中断服务函数 (L610 蜂窝透传)
 * @note        必须实现，否则 HAL_UART_Receive_IT 收不到数据
 * @retval      无
 */
void USART3_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    HAL_UART_IRQHandler(&g_uart3_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

/**
 * @brief       USART3 接收缓冲区清除 (L610)
 * @retval      无
 */
void uart2_rx_clear(void)
{
    memset(g_uart2_rx_buf, 0, sizeof(g_uart2_rx_buf));
    g_uart2_rx_sta = 0;
}

void uart3_rx_clear(void)
{
    memset(g_uart3_rx_buf, 0, sizeof(g_uart3_rx_buf));
    g_uart3_rx_sta = 0;
}