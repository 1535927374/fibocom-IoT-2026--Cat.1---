#ifndef __LOCK_H__
#define __LOCK_H__

#include "stdint.h"

/* 驱动脚：PB5（见 lock.c） */

#define LOCK_STATUS_ON     0
#define LOCK_STATUS_OFF    1

void lock_init(void);
void lock_on(void);
void lock_off(void);
uint8_t lock_status_get(void);

#endif
