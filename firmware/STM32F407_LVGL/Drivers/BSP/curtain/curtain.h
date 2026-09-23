#ifndef __CURTAIN_H__
#define __CURTAIN_H__
#include "sys.h"

/* 窗帘继电器 PC8；on/off 电平与 lock.c（PB5）相同 */

#define CURTAIN_STATUE_ON      0
#define CURTAIN_STATUE_OFF     1

void curtain_init(void);
void curtain_off(void);
void curtain_on(void);
uint8_t curtain_get_status(void);
#endif
