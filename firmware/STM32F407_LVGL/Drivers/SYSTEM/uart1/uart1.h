/**
 ****************************************************************************************************
 * @file        uart.h
 * @author      正点原子风格重构
 * @version     V1.0
 * @date        2026-04-09
 * @brief       串口初始化代码(USART1/2/3)，支持printf
 * @license     Copyright (c) 2020-2032
 ****************************************************************************************************
 * @attention
 *
 * STM32F407ZGT6 三串口默认配置：
 * USART1: TX-PA9,  RX-PA10 (调试串口)
 * USART2: TX-PA2,  RX-PA3 (蓝牙模块，AF7)
 * USART3: TX-PB10, RX-PB11 (广和通 L610 4G)
 *
 * 通过修改下方宏定义,可灵活切换引脚
 *
 ****************************************************************************************************
 */

#ifndef __UART1_H
#define __UART1_H

#include "stdio.h"
#include "sys.h"

/*******************************************************************************************************/
/* USART1 引脚和串口定义 (调试串口) */
#define USART1_TX_GPIO_PORT              GPIOA
#define USART1_TX_GPIO_PIN               GPIO_PIN_9
#define USART1_TX_GPIO_AF                GPIO_AF7_USART1
#define USART1_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART1_RX_GPIO_PORT              GPIOA
#define USART1_RX_GPIO_PIN               GPIO_PIN_10
#define USART1_RX_GPIO_AF                GPIO_AF7_USART1
#define USART1_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART1_UX                        USART1
#define USART1_UX_IRQn                   USART1_IRQn
#define USART1_UX_IRQHandler             USART1_IRQHandler
#define USART1_UX_CLK_ENABLE()           do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)

/*******************************************************************************************************/
/* USART2 引脚和串口定义 (蓝牙) */
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_GPIO_PIN               GPIO_PIN_2
#define USART2_TX_GPIO_AF                GPIO_AF7_USART2
#define USART2_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_GPIO_PIN               GPIO_PIN_3
#define USART2_RX_GPIO_AF                GPIO_AF7_USART2
#define USART2_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART2_UX                        USART2
#define USART2_UX_IRQn                   USART2_IRQn
#define USART2_UX_IRQHandler             USART2_IRQHandler
#define USART2_UX_CLK_ENABLE()           do{ __HAL_RCC_USART2_CLK_ENABLE(); }while(0)

/*******************************************************************************************************/
/* USART3 引脚和串口定义 (L610 4G) */
#define USART3_TX_GPIO_PORT              GPIOB
#define USART3_TX_GPIO_PIN               GPIO_PIN_10
#define USART3_TX_GPIO_AF                GPIO_AF7_USART3
#define USART3_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define USART3_RX_GPIO_PORT              GPIOB
#define USART3_RX_GPIO_PIN               GPIO_PIN_11
#define USART3_RX_GPIO_AF                GPIO_AF7_USART3
#define USART3_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define USART3_UX                        USART3
#define USART3_UX_IRQn                   USART3_IRQn
#define USART3_UX_IRQHandler             USART3_IRQHandler
#define USART3_UX_CLK_ENABLE()           do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)

/*******************************************************************************************************/

#define UART_RX_BUF_SIZE   512                    /* USART1/2 接收缓冲 */
#define UART3_RX_BUF_SIZE  1024                   /* USART3 L610（与 WEATHER_JSON_BUF_SIZE 匹配） */
#define UART_EN_RX         1                      /* 使能（1）/禁止（0）串口接收 */

/* USART1 全局变量声明 */
extern UART_HandleTypeDef g_uart1_handle;       /* UART1句柄 */
extern uint8_t  g_uart1_rx_buf[UART_RX_BUF_SIZE]; /* UART1接收缓冲 */
extern uint16_t g_uart1_rx_sta;                /* UART1接收状态标记 */

/* USART2 全局变量声明 (蓝牙) */
extern UART_HandleTypeDef g_uart2_handle;       /* UART2句柄 */
extern uint8_t  g_uart2_rx_buf[UART_RX_BUF_SIZE]; /* UART2接收缓冲 */
extern uint16_t g_uart2_rx_sta;                /* UART2接收字节计数 */

/* USART3 全局变量声明 (L610) */
extern UART_HandleTypeDef g_uart3_handle;       /* UART3句柄 */
extern uint8_t  g_uart3_rx_buf[UART3_RX_BUF_SIZE]; /* UART3接收缓冲 (L610) */
extern uint16_t g_uart3_rx_sta;                /* UART3接收状态标记 */

/* 函数声明 */
void uart1_init(uint32_t baudrate);             /* USART1初始化函数 */
void uart2_init(uint32_t baudrate);             /* USART2初始化函数 (蓝牙) */
void uart2_rx_clear(void);                      /* USART2接收缓冲区清除 */
void uart3_init(uint32_t baudrate);             /* USART3初始化 (L610) */
void uart3_rx_clear(void);                       /* USART3接收缓冲区清除 */
void uart2_idle_callback(uint8_t *buf, uint16_t len); /* USART2 IDLE 帧结束回调，可重写 */

#endif
