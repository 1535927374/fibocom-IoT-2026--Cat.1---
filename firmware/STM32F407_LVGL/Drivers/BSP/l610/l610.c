#include "l610.h"
#include "stdio.h"
#include "string.h"
#include "delay.h"
#include "led.h"
#include "uart1.h"

extern UART_HandleTypeDef g_uart3_handle;
extern uint8_t  g_uart3_rx_buf[];
extern uint16_t g_uart3_rx_sta;

static uint16_t s_last_len = 0;
static uint32_t s_last_tick = 0;

uint8_t l610_wait_receive(void)
{
    if (g_uart3_rx_sta == 0)
        return L610_ERROR;

    if (g_uart3_rx_sta == s_last_len)
    {
        if (HAL_GetTick() - s_last_tick > 50)
        {
            return L610_EOK;
        }
    }
    else
    {
        s_last_len = g_uart3_rx_sta;
        s_last_tick = HAL_GetTick();
    }
    return L610_ERROR;
}

uint16_t l610_copy_rxdata(char *data)
{
    uint16_t len = g_uart3_rx_sta;
    memcpy(data, g_uart3_rx_buf, len);
    return len;
}

void l610_send_data(char *data, uint16_t len)
{
    HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)data, len, 1000);
}

/* 供 mqtt_task 轮询：内部根据 UART 空闲判断一帧收齐，无数据时立即返回 */
void l610_receive_data(void)
{
    (void)l610_wait_receive();
}

void l610_host_rx_flush(void)
{
    uart3_rx_clear();
    s_last_len = 0;
    s_last_tick = 0;
}

static void l610_rx_reset(void)
{
    l610_host_rx_flush();
}

/* Sync AT: substring match after 50 ms RX idle (l610_wait_receive). */
static uint8_t l610_send_cmd(char *cmd, char *ack, uint32_t timeout_ms)
{
    uint32_t max = timeout_ms / 10U;
    l610_rx_reset();

    HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)cmd, strlen(cmd), 100);

    while (max--)
    {
        if (l610_wait_receive() == L610_EOK)
        {
            if (strstr((char *)g_uart3_rx_buf, ack))
            {
                l610_rx_reset();
                return 0;
            }
        }
        delay_ms(10);
    }
    return 1;
}

static int l610_mipcall_urc_ok(const char *s)
{
    const char *p = strstr(s, "+MIPCALL:");
    if (p == NULL)
        return 0;

    if (strstr(s, "\r\nERROR") != NULL || strstr(s, "\nERROR") != NULL)
        return -1;

    p += 9;
    while (*p == ' ' || *p == '\t')
        p++;

    if (p[0] == '0' && (p[1] == '\r' || p[1] == '\n' || p[1] == 0))
        return -1;

    if (p[0] == '1' && p[1] == ',')
        p += 2;

    return (strchr(p, '.') != NULL) ? 1 : 0;
}

static uint8_t l610_mipcall_activate(void)
{
    char cmd[96];
    int attempt;

    for (attempt = 0; attempt < 2; attempt++)
    {
        l610_rx_reset();
        snprintf(cmd, sizeof(cmd), "AT+MIPCALL=1,\"%s\"\r\n", L610_APN);
        HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)cmd, strlen(cmd), 100);

        uint32_t t0 = HAL_GetTick();
        while ((HAL_GetTick() - t0) < 150000U)
        {
            delay_ms(100);
            if (l610_wait_receive() == L610_EOK)
            {
                int st = l610_mipcall_urc_ok((char *)g_uart3_rx_buf);
                if (st < 0)
                {
                    printf("L610: MIPCALL failed (URC/ERROR)\r\n");
                    goto retry;
                }
                if (st > 0)
                {
                    l610_rx_reset();
                    return 0;
                }
            }
        }
        printf("L610: MIPCALL timeout\r\n");
retry:
        l610_rx_reset();
        delay_ms(2000);
    }
    return 1;
}

static uint8_t l610_mipodm_open_host(const char *ip, const char *port)
{
    char cmd[160];
    int attempt;

    for (attempt = 0; attempt < 3; attempt++)
    {
        l610_rx_reset();
        snprintf(cmd, sizeof(cmd), "AT+MIPODM=1,,\"%s\",%s,0\r\n", ip, port);
        HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)cmd, strlen(cmd), 100);

        uint32_t t0 = HAL_GetTick();
        while ((HAL_GetTick() - t0) < 60000U)
        {
            delay_ms(100);
            if (l610_wait_receive() == L610_EOK)
            {
                char *rx = (char *)g_uart3_rx_buf;
                if (strstr(rx, "+MIPODM: 1,1") != NULL)
                {
                    l610_rx_reset();
                    return 0;
                }
                if (strstr(rx, "\r\nERROR") != NULL || strstr(rx, "\nERROR") != NULL)
                {
                    printf("L610: MIPODM ERROR (%s:%s)\r\n", ip, port);
                    break;
                }
            }
        }
        printf("L610: MIPODM timeout (%s:%s) retry %d\r\n", ip, port, attempt + 1);
        l610_rx_reset();
        delay_ms(2000);
    }
    return 1;
}

uint8_t l610_modem_init_link(void)
{
    int i;
    char cmd[128];

    printf("L610: modem link (Fibocom app note V1.0.1)\r\n");

    for (i = 0; i < 10; i++)
    {
        if (l610_send_cmd("AT\r\n", "OK", 1000) == 0)
            break;
        delay_ms(500);
    }
    if (i >= 10)
    {
        printf("L610: no AT response\r\n");
        return L610_ERROR;
    }

    (void)l610_send_cmd("AT+CMEE=2\r\n", "OK", 3000);

    for (i = 0; i < 5; i++)
    {
        if (l610_send_cmd("AT+CPIN?\r\n", "READY", 5000) == 0)
            break;
        delay_ms(1000);
    }
    if (i >= 5)
    {
        printf("L610: SIM not ready\r\n");
        return L610_ERROR;
    }

    (void)l610_send_cmd("AT+CSQ?\r\n", "OK", 3000);

    printf("L610: wait PS attach...\r\n");
    for (i = 0; i < 90; i++)
    {
        if (l610_send_cmd("AT+CGREG?\r\n", "0,1", 3000) == 0 ||
            l610_send_cmd("AT+CGREG?\r\n", "0,5", 3000) == 0)
            break;
        if (l610_send_cmd("AT+CEREG?\r\n", "0,1", 3000) == 0 ||
            l610_send_cmd("AT+CEREG?\r\n", "0,5", 3000) == 0)
            break;
        delay_ms(1000);
    }
    if (i >= 90)
    {
        printf("L610: CS/PS registration fail\r\n");
        return L610_ERROR;
    }

    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", L610_APN);
    if (l610_send_cmd(cmd, "OK", 5000) != 0)
    {
        printf("L610: CGDCONT fail\r\n");
        return L610_ERROR;
    }

    if (l610_mipcall_activate() != 0)
    {
        printf("L610: MIPCALL activate fail\r\n");
        return L610_ERROR;
    }

    if (l610_send_cmd("AT+GTSET=\"IPRFMT\",1\r\n", "OK", 5000) != 0)
    {
        printf("L610: GTSET IPRFMT fail\r\n");
        return L610_ERROR;
    }

    return L610_EOK;
}

/*
 * AT+CCLK? 典型：+CCLK: "26/04/15,14:30:45+32"\r\nOK
 * 仅提取逗号后的 hh:mm:ss（时区 +xx 等一律丢弃）。
 * 按 UTC 理解该时刻，换算为北京时间：+8 小时（86400 内取模，不滚日期）。
 */
uint8_t l610_modem_query_cclk_hms(uint8_t *h, uint8_t *m, uint8_t *s)
{
    uint32_t max;
    unsigned hi, mj, sc;
    uint32_t sec;

    if (h == NULL || m == NULL || s == NULL)
        return L610_ERROR;

    l610_rx_reset();
    HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)"AT+CCLK?\r\n", 10, 100);

    max = 300U;
    while (max--)
    {
        delay_ms(10);
        if (l610_wait_receive() == L610_EOK)
        {
            char *rx = (char *)g_uart3_rx_buf;
            const char *comma;
            uint16_t n;

            if (strstr(rx, "+CCLK:") == NULL || strstr(rx, "OK") == NULL)
                continue;
            if (strstr(rx, "\r\nERROR") != NULL)
                break;

            n = g_uart3_rx_sta;
            if (n >= UART3_RX_BUF_SIZE)
                n = UART3_RX_BUF_SIZE - 1U;
            g_uart3_rx_buf[n] = '\0';

            {
                const char *clk = strstr(rx, "+CCLK:");
                if (clk == NULL)
                    break;
                comma = strchr(clk, ',');
            }
            if (comma == NULL)
                break;
            comma++;
            hi = mj = sc = 0U;
            if (sscanf(comma, "%2u:%2u:%2u", &hi, &mj, &sc) != 3)
                break;
            if (hi > 23U || mj > 59U || sc > 59U)
                break;

            sec = (uint32_t)hi * 3600u + (uint32_t)mj * 60u + (uint32_t)sc;
            sec += 8u * 3600u;
            sec %= 86400u;
            *h = (uint8_t)(sec / 3600u);
            *m = (uint8_t)((sec / 60u) % 60u);
            *s = (uint8_t)(sec % 60u);
            l610_rx_reset();
            return L610_EOK;
        }
    }

    l610_rx_reset();
    return L610_ERROR;
}

uint8_t l610_tcp_transparent_open(const char *ip, const char *port)
{
    printf("L610: MIPODM TCP %s:%s\r\n", ip, port);
    if (l610_mipodm_open_host(ip, port) != 0)
        return L610_ERROR;
    l610_rx_reset();
    printf("L610: transparent OK\r\n");
    return L610_EOK;
}

/*
 * 退出透传：+++ 进 AT，再关 MIPODM（以 Fibocom 手册为准，若无效请对照 URC 调整）
 */
void l610_tcp_transparent_close(void)
{
    delay_ms(1200);
    HAL_UART_Transmit(&g_uart3_handle, (uint8_t *)"+++", 3, 100);
    delay_ms(1200);
    l610_host_rx_flush();

    (void)l610_send_cmd("AT\r\n", "OK", 3000);
    (void)l610_send_cmd("AT+MIPODM=0\r\n", "OK", 5000);
    l610_host_rx_flush();
    printf("L610: TCP transparent closed\r\n");
}

/* 明文 HTTP/1.1，端口 80；响应体中的 JSON 拷入 out */
uint8_t l610_weather_http_get_today(char *out, uint16_t outsz)
{
    char req[320];
    const char *body;
    char *dst;
    uint16_t i, ncopy;

    if (out == NULL || outsz == 0U)
        return L610_ERROR;

    snprintf(req, sizeof(req),
             "GET /v3/weather/daily.json?key=%s&location=beijing&language=zh-Hans&unit=c&start=0&days=1 HTTP/1.1\r\n"
             "Host: api.seniverse.com\r\n"
             "Connection: close\r\n"
             "\r\n",
             L610_WEATHER_API_KEY);

    l610_host_rx_flush();
    l610_send_data(req, (uint16_t)strlen(req));

    for (uint32_t w = 0; w < 800U; w++)
    {
        if (l610_wait_receive() == L610_EOK)
            break;
        delay_ms(10);
    }

    if (g_uart3_rx_sta >= UART3_RX_BUF_SIZE)
        g_uart3_rx_sta = UART3_RX_BUF_SIZE - 1U;
    g_uart3_rx_buf[g_uart3_rx_sta] = '\0';

    body = strstr((char *)g_uart3_rx_buf, "\r\n\r\n");
    if (body == NULL)
    {
        l610_host_rx_flush();
        return L610_ERROR;
    }
    body += 4;
    while (*body != '\0' && *body != '{')
        body++;
    if (*body != '{')
    {
        l610_host_rx_flush();
        return L610_ERROR;
    }

    dst = out;
    ncopy = 0;
    for (i = 0; body[i] != '\0' && ncopy + 1U < outsz; i++)
        dst[ncopy++] = body[i];
    dst[ncopy] = '\0';

    l610_host_rx_flush();
    return (strstr(out, "results") != NULL) ? L610_EOK : L610_ERROR;
}

void l610_init(void)
{
    if (l610_modem_init_link() != L610_EOK)
        return;
    if (l610_tcp_transparent_open(L610_TCP_SERVER_IP, L610_TCP_SERVER_PORT) != L610_EOK)
        return;
    printf("L610: transparent data mode OK (OneNET MQTT)\r\n");
    led_living_off();
}
