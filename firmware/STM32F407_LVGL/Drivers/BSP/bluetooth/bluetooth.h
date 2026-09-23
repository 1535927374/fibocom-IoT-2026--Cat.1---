#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "sys.h"

void bt_init(uint32_t baudary);
void bt_send(char *format, ...);
uint8_t bt_value_get(void);
uint8_t bt_cmd_seq_get(void);

#endif
