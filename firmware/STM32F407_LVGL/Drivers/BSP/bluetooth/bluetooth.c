#include "bluetooth.h"
#include "uart1.h"
#include "stdio.h"
#include "stdarg.h"
#include <stddef.h>
#include "app_config.h"

/*
 * 最近一次有效数字命令 '0'~'9'（供 light_control_task）：
 *   '0'~'3' → 客厅灯 PWM 四档；'4' → 卧室开；'5' → 卧室关；其余可扩展。
 */
static uint8_t bt_light_level = 0;
static volatile uint8_t bt_cmd_seq = 0;

void bt_init(uint32_t baudary)
{
    uart2_init(baudary);
}

void uart2_idle_callback(uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }
    APP_LOG("bt:recv: %s\r\n", buf);
    {
        uint8_t c = buf[0];
        if (c >= '0' && c <= '9')
        {
            bt_light_level = (uint8_t)(c - '0');
            bt_cmd_seq++;
        }
    }
    uart2_rx_clear();
}

void bt_send(char *format, ...)
{
    uint8_t send_buf[128] = {0};
    va_list arg;
    int n;
    va_start(arg, format);
    n = vsnprintf((char *)send_buf, sizeof(send_buf), format, arg);
    va_end(arg);
    if (n < 0)
        n = 0;
    if ((size_t)n >= sizeof(send_buf))
        n = (int)sizeof(send_buf) - 1;
    HAL_UART_Transmit(&g_uart2_handle, send_buf, (uint16_t)n, 100);
}

uint8_t bt_value_get(void)
{
    return bt_light_level;
}

uint8_t bt_cmd_seq_get(void)
{
    return bt_cmd_seq;
}
