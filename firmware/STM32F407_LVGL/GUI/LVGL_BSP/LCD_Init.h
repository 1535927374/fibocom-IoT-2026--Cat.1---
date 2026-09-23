#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"
#include <stdint.h>
#include "delay.h"

#define USE_HORIZONTAL 2  //扫描方向设置 0为竖屏 2为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 320
#define LCD_H 480

#else
#define LCD_W 480
#define LCD_H 320
#endif


//-----------------LCD引脚定义 (HAL库版)--------------- 

#define LCD_SCLK_Clr() HAL_GPIO_WritePin(LCD_CLK_GPIO_Port, LCD_CLK_Pin, GPIO_PIN_RESET)//SCL=SCLK
#define LCD_SCLK_Set() HAL_GPIO_WritePin(LCD_CLK_GPIO_Port, LCD_CLK_Pin, GPIO_PIN_SET)

#define LCD_MOSI_Clr() HAL_GPIO_WritePin(LCD_MOSI_GPIO_Port, LCD_MOSI_Pin, GPIO_PIN_RESET)//SDA=MOSI
#define LCD_MOSI_Set() HAL_GPIO_WritePin(LCD_MOSI_GPIO_Port, LCD_MOSI_Pin, GPIO_PIN_SET)

#define LCD_RES_Clr()  HAL_GPIO_WritePin(RESER_GPIO_Port, RESER_Pin, GPIO_PIN_RESET)//RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(RESER_GPIO_Port, RESER_Pin, GPIO_PIN_SET)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET)//DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_RESET)//BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)//CS1
#define LCD_CS_Set()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)

// 触摸引脚定义
#define TCLK_Clr()     HAL_GPIO_WritePin(LCD_CLK_GPIO_Port, LCD_CLK_Pin, GPIO_PIN_RESET)
#define TCLK_Set()     HAL_GPIO_WritePin(LCD_CLK_GPIO_Port, LCD_CLK_Pin, GPIO_PIN_SET)

#define TDIN_Clr()     HAL_GPIO_WritePin(LCD_MOSI_GPIO_Port, LCD_MOSI_Pin, GPIO_PIN_RESET)
#define TDIN_Set()     HAL_GPIO_WritePin(LCD_MOSI_GPIO_Port, LCD_MOSI_Pin, GPIO_PIN_SET)

#define DOUT_Read()    HAL_GPIO_ReadPin(TOUCH_MISO_GPIO_Port, TOUCH_MISO_Pin)

#define TCS_Clr()      HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET)
#define TCS_Set()      HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET)

#define PEN_Read()     HAL_GPIO_ReadPin(TOUCH_PEN_GPIO_Port, TOUCH_PEN_Pin)





void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint32_t dat);//写入多个字节
void LCD_WR_REG(uint8_t dat);//写入一个命令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//扫描方向设置
void LCD_Init(void);//LCD初始化
#endif

