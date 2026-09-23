#ifndef __MY_GUI_H__
#define __MY_GUI_H__

#include "lvgl.h"
#include <stdint.h>
#include "My_cb.h"
//客厅灯光
__weak void on_livingroom_clicked(uint8_t state);
//卧室灯
__weak void on_bedroom_clicked(uint8_t state);
//窗帘
__weak void on_curtain_clicked(uint8_t state);
//音响
__weak void on_speaker_clicked(uint8_t state);


void LVGL_Project_Main(void);

/* 心知 code_day → 天气面板图标/文字（img_weather / label_weather） */
void my_gui_weather_set_sunny(void);
void my_gui_weather_set_cloudy(void);
void my_gui_weather_set_rain(void);

/* 心知 low/high → 天气区温度「low℃~high℃」；date(YYYY-MM-DD) → 同步日期面板；时间面板仍为 HH:MM:SS */
void my_gui_apply_weather_high_and_date(const char *low_str, const char *high_str, const char *date_str,
                                        const char *code_day_str);

/* 从 date 字符串解析年月日与时、分、秒，写入界面时钟状态；支持 YYYY-MM-DD / YYYY-MM-DD HH:MM:SS / ISO T 分隔 */
void my_gui_sync_clock_from_date_string(const char *date_str);

/* 仅设置时/分/秒（0~23 / 0~59 / 0~59），用于 AT+CCLK 等；label 已创建时立即刷新时间面板 */
void my_gui_set_clock_hms(uint8_t h, uint8_t mi, uint8_t s);

#endif
