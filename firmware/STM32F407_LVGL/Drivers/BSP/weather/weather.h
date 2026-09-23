#ifndef WEATHER_H
#define WEATHER_H

#include <stdint.h>

#define WEATHER_JSON_BUF_SIZE  768

typedef struct
{
    char date[28]; /* YYYY-MM-DD 或 YYYY-MM-DD HH:MM:SS */
    char code_day[10];
    char high[10];
    char low[10];
} daily_weather_t;

extern daily_weather_t today_weather;
extern char weather_data[WEATHER_JSON_BUF_SIZE];

void get_today_weather(void);
/* 更新 LVGL 天气图标；须在界面创建后、且在 lv_timer_handler 所在上下文调用 */
void show_today_weather(void);

#endif
