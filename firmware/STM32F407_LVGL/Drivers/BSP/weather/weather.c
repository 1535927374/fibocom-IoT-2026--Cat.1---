#include "weather.h"
#include "l610.h"
#include "My_GUI.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "delay.h"

char weather_data[WEATHER_JSON_BUF_SIZE] = {0};
daily_weather_t today_weather = {0};

static void weather_copy_str_field(const char *obj, const char *obj_end, const char *key,
                                   char *out, size_t outn)
{
    char pat[28];
    const char *p;
    size_t i;

    out[0] = '\0';
    if (obj == NULL || obj_end == NULL || key == NULL || outn == 0U)
        return;
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    p = obj;
    while (p < obj_end && (p = strstr(p, pat)) != NULL)
    {
        p += strlen(pat);
        i = 0;
        while (p < obj_end && *p != '"' && i + 1U < outn)
            out[i++] = *p++;
        out[i] = '\0';
        return;
    }
}

/* 取 "daily":[ 后第 idx 个 { ... } */
static int weather_parse_daily_item(const char *json, int idx, const char **pstart, const char **pend)
{
    const char *arr = strstr(json, "\"daily\"");
    const char *s;
    int depth;
    int n;

    if (arr == NULL)
        return -1;
    arr = strchr(arr, '[');
    if (arr == NULL)
        return -1;
    s = arr + 1;
    for (n = 0; n <= idx; n++)
    {
        s = strstr(s, "{");
        if (s == NULL)
            return -1;
        depth = 0;
        const char *e = s;
        for (; *e != '\0'; e++)
        {
            if (*e == '{')
                depth++;
            else if (*e == '}')
            {
                depth--;
                if (depth == 0)
                {
                    if (n == idx)
                    {
                        *pstart = s;
                        *pend = e + 1;
                        return 0;
                    }
                    s = e + 1;
                    break;
                }
            }
        }
        if (depth != 0)
            return -1;
    }
    return -1;
}

void get_today_weather(void)
{
    const char *ps;
    const char *pe;

    memset(weather_data, 0, sizeof(weather_data));
    memset(&today_weather, 0, sizeof(today_weather));

    if (l610_weather_http_get_today(weather_data, sizeof(weather_data)) != L610_EOK)
    {
        printf("weather: HTTP/JSON fetch fail\r\n");
        return;
    }

    if (strstr(weather_data, "results") == NULL)
    {
        printf("weather: no results in payload\r\n");
        return;
    }

    if (weather_parse_daily_item(weather_data, 0, &ps, &pe) != 0)
    {
        printf("weather: parse daily[0] fail\r\n");
        return;
    }

    weather_copy_str_field(ps, pe, "date", today_weather.date, sizeof(today_weather.date));
    weather_copy_str_field(ps, pe, "code_day", today_weather.code_day, sizeof(today_weather.code_day));
    weather_copy_str_field(ps, pe, "high", today_weather.high, sizeof(today_weather.high));
    weather_copy_str_field(ps, pe, "low", today_weather.low, sizeof(today_weather.low));
    printf("today date:%s code:%s high:%s low:%s\r\n", today_weather.date, today_weather.code_day,
           today_weather.high, today_weather.low);

    if (today_weather.date[0] != '\0')
        my_gui_sync_clock_from_date_string(today_weather.date);

    delay_ms(50);
}

void show_today_weather(void)
{
    int code_day = atoi(today_weather.code_day);

    switch (code_day)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            /* 晴天 → img_weather */
            my_gui_weather_set_sunny();
            //文字显示 晴天
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            /* 多云 → img_weather */
            my_gui_weather_set_cloudy();
            //文字显示 多云
            break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            /* 雨天 → img_weather */
            my_gui_weather_set_rain();
            //文字显示 雨
            break;
        default:
            break;
    }

    my_gui_apply_weather_high_and_date(today_weather.low, today_weather.high, today_weather.date,
                                       today_weather.code_day);
}
