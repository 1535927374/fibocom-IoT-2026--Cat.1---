#ifndef __LCD_FONT_H
#define __LCD_FONT_H

#include <stdint.h>

// 字体结构体定义
typedef struct 
{
	uint8_t Index[2];	// 汉字内码索引
	uint8_t Msk[32];    // 点阵码数据(16x16)
}typFNT_GB16;

typedef struct 
{
	uint8_t Index[2];	// 汉字内码索引
	uint8_t Msk[72];    // 点阵码数据(24x24)
}typFNT_GB24;

typedef struct 
{
	uint8_t Index[2];	// 汉字内码索引
	uint8_t Msk[128];   // 点阵码数据(32x32)
}typFNT_GB32;

// ASCII字体 8x16
extern const uint8_t ascii_1608[95][16];

// ASCII字体 16x32
extern const uint8_t ascii_3216[95][64];

// 汉字字库大小定义
#define TFONT16_SIZE 1
#define TFONT24_SIZE 1
#define TFONT32_SIZE 1

// 汉字字库
extern const typFNT_GB16 tfont16[TFONT16_SIZE];
extern const typFNT_GB24 tfont24[TFONT24_SIZE];
extern const typFNT_GB32 tfont32[TFONT32_SIZE];

#endif
