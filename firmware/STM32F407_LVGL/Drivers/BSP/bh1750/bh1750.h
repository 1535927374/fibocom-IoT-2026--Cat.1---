#ifndef __BH1750_H__
#define __BH1750_H__

#include "sys.h"

/* GY-302(BH1750): I2C1 PB6=SCL PB7=SDA(AF4)；ADDR 接 GND=0x23，接 VCC=0x5C */
#define BH1750_I2C_ADDR_7BIT   0x23u

void bh1750_init(void);
float bh1750_read_lux(void);

/* 最近一次成功读数，供 LVGL 在定时器中刷新界面（勿在 LVGL 中直接调 I2C） */
extern volatile float bh1750_ui_lux;
extern volatile uint8_t bh1750_ui_valid;

#endif
