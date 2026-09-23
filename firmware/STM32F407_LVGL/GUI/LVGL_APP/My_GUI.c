#include "My_GUI.h"
#include "dht11.h"
#include "bh1750.h"
#include <stdio.h>
#include "app_config.h"

/* LVGL 字符串为 UTF-8；Keil ARMCC 对源文件内直接写汉字易触发 #870 multibyte 错误，用 \x 保持源码为 ASCII */
#define MY_GUI_U8_QINGTIAN   "\xe6\x99\xb4\xe5\xa4\xa9" /* 晴天 */
#define MY_GUI_U8_DUOYUN     "\xe5\xa4\x9a\xe4\xba\x91" /* 多云 */
#define MY_GUI_U8_YU         "\xe9\x9b\xa8"             /* 雨 */
#define MY_GUI_U8_24C        " 24\xe2\x84\x83 "         /* 24℃ */
#define MY_GUI_U8_DATE_FMT   " %d \xe5\xb9\xb4 %d \xe6\x9c\x88 %d \xe6\x97\xa5 " /* %d年%d月%d日 */
#define MY_GUI_U8_WENDU      "\xe6\xb8\xa9\xe5\xba\xa6" /* 温度 */
#define MY_GUI_U8_SHIDU      "\xe6\xb9\xbf\xe5\xba\xa6" /* 湿度 */
/* 信息面板提示（Font_info_24 字库已含下列字符） */
#define MY_GUI_U8_INFO_RAIN \
	"\xe9\x9b\xa8\xe5\xa4\xa9\xe5\xb8\xa6\xe4\xbc\x9e\xef\xbc\x8c\xe6\xb3\xa8\xe6\x84\x8f\xe8\xb7\xaf\xe6\xbb\x91"
#define MY_GUI_U8_INFO_HOT \
	"\xe5\xa4\xaa\xe7\x83\xad\xe4\xba\x86\xef\xbc\x8c\xe6\x8d\xa2\xe4\xbb\xb6\xe7\x9f\xad\xe8\xa2\x96\xe5\x90\xa7"
#define MY_GUI_U8_INFO_COLD \
	"\xe5\x8f\x98\xe5\x86\xb7\xe4\xba\x86\xef\xbc\x8c\xe8\xae\xb0\xe5\xbe\x97\xe5\x8a\xa0\xe4\xbb\xb6\xe8\xa1\xa3\xe6\x9c\x8d"
#define MY_GUI_U8_INFO_COMFORT \
	"\xe6\xb0\x94\xe6\xb8\xa9\xe8\x88\x92\xe9\x80\x82\xef\xbc\x8c\xe6\xad\xa3\xe5\xb8\xb8\xe7\xa9\xbf\xe8\xa1\xa3"

/*LVGL 对象操作句柄 用于动态更新界面*/
static lv_obj_t* img_weather = NULL;
static lv_obj_t* label_weather = NULL;
static lv_obj_t* label_weather_temp = NULL;
static lv_obj_t* label_time = NULL;
static lv_obj_t* label_date = NULL;

/* 设备控制面板 句柄 */
static lv_obj_t* panel_livingroom;		//客厅灯控制面板
static lv_obj_t* panel_bedroom;			//卧室灯控制面板
static lv_obj_t* panel_curtain;		//窗帘控制面板
static lv_obj_t* panel_speaker;		//音响控制面板

static lv_obj_t* label_dht_temp_val = NULL;
static lv_obj_t* label_dht_humi_val = NULL;
static lv_obj_t* label_bh1750_lux_val = NULL;

static lv_obj_t* label_info_tip = NULL;

/*LVGL 定时器句柄 用于更新时间*/
//static lv_timer_t* time_timer = NULL;

/*声明图像资源*/
LV_IMG_DECLARE(icon_weather_sunny);			// 天气 晴天
LV_IMG_DECLARE(icon_weather_cloudy); 		// 天气 多云
LV_IMG_DECLARE(icon_weather_storm_rain); 	// 天气 雨天
LV_IMG_DECLARE(icon_living_light);  		// 客厅灯
LV_IMG_DECLARE(icon_bedroom_light); 		// 卧室灯
LV_IMG_DECLARE(icon_curtain);	   			// 窗帘
LV_IMG_DECLARE(icon_ac);	   	   			// 空调
LV_IMG_DECLARE(icon_speaker);	   			// 音响
LV_IMG_DECLARE(icon_lock);	   	   			// 门锁

/*声明字体资源*/
LV_FONT_DECLARE(Font_weather_30);//字:晴天
LV_FONT_DECLARE(Font_Num_24);//字:24℃
LV_FONT_DECLARE(Font_Num_16);
LV_FONT_DECLARE(Font_Sensor_20);//温湿度光照
LV_FONT_DECLARE(Font_info_24);//信息提醒

/*声明当前时间*/

/* 时分秒默认 0；启动时由 L610 AT+CCLK? 写入，或由心知/同步接口更新 */
static uint8_t cur_hour;
static uint8_t cur_min;
static uint8_t cur_sec;

/* 日期默认占位；同步后来自 today_weather.date */
static uint16_t cur_year = 2026;
static uint8_t cur_month = 1;
static uint8_t cur_day = 1;

/* 设备 ID 枚举 */
typedef enum
{
    DEVICE_LIVINGROOM = 0, 	// 客厅灯
    DEVICE_BEDROOM,       	// 空调
    DEVICE_CURTAIN,  	    // 窗帘
    DEVICE_SPEAKER,  	// 音响
    DEVICE_MAX       	// 设备总数 (用于数组边界检查)
} device_id_t;

// 设备状态数组
// 每个设备对应一个状态位 0 = 关闭 1 等于开启
static uint8_t device_state[DEVICE_MAX] = {0};

/**
 * @brief 创建天气信息面板
 * @param parent：父对象
 * @retval 面板对象指针
 * @note  图标、天气文字、low℃~high℃ 同一行水平排列
 */
static lv_obj_t *create_weather_pannel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);

	/* 加宽以容纳一行：图标 + 两字天气 + 温度段 */
	lv_obj_set_size(panel, 303, 80);
	lv_obj_set_pos(panel, 20, 20);
	lv_obj_set_style_bg_color(panel, lv_color_hex(0x5CB3E8), 0);
	lv_obj_set_style_radius(panel, 11, 0);
	lv_obj_set_style_border_width(panel, 0, 0);
	lv_obj_clear_flag(panel,LV_OBJ_FLAG_SCROLLABLE);

	img_weather = lv_img_create(panel);
	lv_img_set_src(img_weather,&icon_weather_sunny);
	lv_obj_align(img_weather, LV_ALIGN_LEFT_MID, 5, 0);

	label_weather = lv_label_create(panel);
	lv_label_set_text(label_weather, MY_GUI_U8_QINGTIAN);
	lv_obj_set_style_text_font(label_weather,&Font_weather_30,0);
	lv_obj_align_to(label_weather, img_weather, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

	label_weather_temp = lv_label_create(panel);
	lv_label_set_text(label_weather_temp, MY_GUI_U8_24C);
	lv_obj_set_style_text_font(label_weather_temp,&Font_Num_24,0);
	lv_obj_set_style_text_align(label_weather_temp, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align_to(label_weather_temp, label_weather, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	return panel;
}

void my_gui_weather_set_sunny(void)
{
	if (img_weather == NULL || label_weather == NULL)
		return;
	lv_img_set_src(img_weather, &icon_weather_sunny);
	lv_label_set_text(label_weather, MY_GUI_U8_QINGTIAN);
}

void my_gui_weather_set_cloudy(void)
{
	if (img_weather == NULL || label_weather == NULL)
		return;
	lv_img_set_src(img_weather, &icon_weather_cloudy);
	lv_label_set_text(label_weather, MY_GUI_U8_DUOYUN);
}

void my_gui_weather_set_rain(void)
{
	if (img_weather == NULL || label_weather == NULL)
		return;
	lv_img_set_src(img_weather, &icon_weather_storm_rain);
	lv_label_set_text(label_weather, MY_GUI_U8_YU);
}

void my_gui_sync_clock_from_date_string(const char *date_str)
{
	unsigned y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
	int n;

	if (date_str == NULL || date_str[0] == '\0')
		return;

	n = sscanf(date_str, "%u-%u-%uT%u:%u:%u", &y, &mo, &d, &h, &mi, &s);
	if (n != 6) {
		n = sscanf(date_str, "%u-%u-%u %u:%u:%u", &y, &mo, &d, &h, &mi, &s);
	}
	if (n != 6) {
		/* 仅 YYYY-MM-DD：不改时分秒（保留 AT+CCLK 等已写入的时:分:秒） */
		n = sscanf(date_str, "%u-%u-%u", &y, &mo, &d);
		if (n != 3)
			return;
		h = cur_hour;
		mi = cur_min;
		s = cur_sec;
	}

	if (mo < 1U || mo > 12U || d < 1U || d > 31U)
		return;
	if (h > 23U || mi > 59U || s > 59U)
		return;

	cur_year = (uint16_t)y;
	cur_month = (uint8_t)mo;
	cur_day = (uint8_t)d;
	cur_hour = (uint8_t)h;
	cur_min = (uint8_t)mi;
	cur_sec = (uint8_t)s;

	if (label_date != NULL) {
		lv_label_set_text_fmt(label_date, MY_GUI_U8_DATE_FMT,
			(unsigned)cur_year, (unsigned)cur_month, (unsigned)cur_day);
	}
	if (label_time != NULL) {
		lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", cur_hour, cur_min, cur_sec);
	}
}

void my_gui_set_clock_hms(uint8_t h, uint8_t mi, uint8_t s)
{
	if (h > 23U || mi > 59U || s > 59U)
		return;
	cur_hour = h;
	cur_min = mi;
	cur_sec = s;
	if (label_time != NULL)
		lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", cur_hour, cur_min, cur_sec);
}

/* code_day 与 high 更新信息面板；雨天 code 10~17 优先于气温文案 */
static void my_gui_refresh_info_tip(const char *code_day_str, int ok_hi, int hi)
{
	int code = 0;

	if (label_info_tip == NULL)
		return;

	if (code_day_str != NULL && code_day_str[0] != '\0' && sscanf(code_day_str, "%d", &code) == 1) {
		if (code >= 10 && code <= 17) {
			lv_label_set_text(label_info_tip, MY_GUI_U8_INFO_RAIN);
			return;
		}
	}

	if (ok_hi) {
		if (hi > 25) {
			lv_label_set_text(label_info_tip, MY_GUI_U8_INFO_HOT);
			return;
		}
		if (hi < 10) {
			lv_label_set_text(label_info_tip, MY_GUI_U8_INFO_COLD);
			return;
		}
		if (hi >= 20 && hi <= 25) {
			lv_label_set_text(label_info_tip, MY_GUI_U8_INFO_COMFORT);
			return;
		}
	}

	lv_label_set_text(label_info_tip, "");
}

void my_gui_apply_weather_high_and_date(const char *low_str, const char *high_str, const char *date_str,
                                        const char *code_day_str)
{
	int lo = 0;
	int hi = 0;
	static char tbuf[40];
	int ok_lo = (low_str != NULL && low_str[0] != '\0' && sscanf(low_str, "%d", &lo) == 1);
	int ok_hi = (high_str != NULL && high_str[0] != '\0' && sscanf(high_str, "%d", &hi) == 1);

	if (label_weather_temp != NULL) {
		if (ok_lo && ok_hi)
			lv_snprintf(tbuf, sizeof(tbuf), "%d\xe2\x84\x83~%d\xe2\x84\x83", lo, hi);
		else if (ok_hi)
			lv_snprintf(tbuf, sizeof(tbuf), "--\xe2\x84\x83~%d\xe2\x84\x83", hi);
		else if (ok_lo)
			lv_snprintf(tbuf, sizeof(tbuf), "%d\xe2\x84\x83~--\xe2\x84\x83", lo);
		else
			lv_snprintf(tbuf, sizeof(tbuf), "--\xe2\x84\x83~--\xe2\x84\x83");
		lv_label_set_text(label_weather_temp, tbuf);
	}

	my_gui_sync_clock_from_date_string(date_str);
	my_gui_refresh_info_tip(code_day_str, ok_hi, hi);
}

/* 时钟与标签刷新：必须在 lv_timer_handler() 上下文中执行，禁止在 TIM2 等硬件中断里调用 lv_* */
static void time_timer_cb(lv_timer_t * t)
{
  (void)t;
  uint8_t month_days;
  uint8_t is_leap;

  cur_sec++;

  if (cur_sec >= 60) {
    cur_sec = 0;
    cur_min++;

    if (cur_min >= 60) {
      cur_min = 0;
      cur_hour++;

      if (cur_hour >= 24) {
        cur_hour = 0;
        cur_day++;

        is_leap = ((cur_year % 4 == 0 && cur_year % 100 != 0) || (cur_year % 400 == 0)) ? 1 : 0;

        switch (cur_month) {
          case 1: month_days = 31; break;
          case 2: month_days = is_leap ? 29 : 28; break;
          case 3: month_days = 31; break;
          case 4: month_days = 30; break;
          case 5: month_days = 31; break;
          case 6: month_days = 30; break;
          case 7: month_days = 31; break;
          case 8: month_days = 31; break;
          case 9: month_days = 30; break;
          case 10: month_days = 31; break;
          case 11: month_days = 30; break;
          case 12: month_days = 31; break;
          default: month_days = 31; break;
        }

        if (cur_day > month_days) {
          cur_day = 1;
          cur_month++;
          if (cur_month > 12) {
            cur_month = 1;
            cur_year++;
          }
        }
      }
    }
  }

  if (label_time != NULL) {
    lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", cur_hour, cur_min, cur_sec);
  }
  if (label_date != NULL) {
    lv_label_set_text_fmt(label_date, MY_GUI_U8_DATE_FMT, cur_year, cur_month, cur_day);
  }
}

/* UTF-8 摄氏度符号（Font_Sensor_20 含 ℃、% 与数字） */
static const char s_utf8_celsius[] = "\xE2\x84\x83";

/* 温湿度与光照数值刷新：须在 lv_timer / lvgl 任务上下文中调用 */
static void dht11_ui_timer_cb(lv_timer_t *t)
{
  static char buf_t[24];
  static char buf_h[24];
  static char buf_l[24];

  (void)t;
  if (label_dht_temp_val == NULL || label_dht_humi_val == NULL) {
    return;
  }
  if (!dht11_ui_valid) {
    lv_snprintf(buf_t, sizeof(buf_t), "--%s", s_utf8_celsius);
    lv_snprintf(buf_h, sizeof(buf_h), "--%%");
    lv_label_set_text(label_dht_temp_val, buf_t);
    lv_label_set_text(label_dht_humi_val, buf_h);
  } else {
    /* 仅整数；[0] 湿度整数 %[RH]，[2] 温度整数 ℃ */
    lv_snprintf(buf_t, sizeof(buf_t), "%d%s", (int)dht11_ui_snapshot[2], s_utf8_celsius);
    lv_snprintf(buf_h, sizeof(buf_h), "%d%%", (int)dht11_ui_snapshot[0]);
    lv_label_set_text(label_dht_temp_val, buf_t);
    lv_label_set_text(label_dht_humi_val, buf_h);
  }

  if (label_bh1750_lux_val != NULL) {
    if (!bh1750_ui_valid) {
      lv_label_set_text(label_bh1750_lux_val, "--");
    } else {
      int t10 = (int)(bh1750_ui_lux * 10.0f + 0.5f);
      int ip = t10 / 10;
      int fp = t10 % 10;
      if (ip < 0)
        ip = 0;
      lv_snprintf(buf_l, sizeof(buf_l), "%d.%d", ip, fp);
      lv_label_set_text(label_bh1750_lux_val, buf_l);
    }
  }
}
/**
 * @brief 创建时间和日期面板
 * @param parent：父对象
 * @retval 时间面板对象指针
 * @note  包含两个字面吧：
 * 		  1.时间面板 - 显示 HH:MM:SS 格式的当前时间
 * 		  2.日期面板 - 显示 YYYY/MM/DD格式的当前日期
 * 		  时间面板位置：（325，15）尺寸：140x45
 * 		  日期面板位置：（365，70）尺寸：100x40
 */
static lv_obj_t *create_time_panel(lv_obj_t *parent)
{
	/*---------时间面板----------*/
	lv_obj_t* panel_time = lv_obj_create(parent);
	lv_obj_set_size(panel_time,150,45);								// 设置尺寸
	lv_obj_set_pos(panel_time,325,15);								// 设置位置
	lv_obj_set_style_bg_color(panel_time, lv_color_hex(0x5CB3E8), 0);//设置背景颜色
	lv_obj_set_style_radius(panel_time,10,0);						// 设置圆角
	lv_obj_set_style_border_width(panel_time,0,0);					// 设置边框宽度
	lv_obj_clear_flag(panel_time,LV_OBJ_FLAG_SCROLLABLE);			// 关闭滚动属性
	/*---------日期面板----------*/
	lv_obj_t* panel_date = lv_obj_create(parent);
	lv_obj_set_size(panel_date,150,40);								// 设置尺寸
	lv_obj_set_pos(panel_date,325,70);								// 设置位置
	lv_obj_set_style_bg_color(panel_date, lv_color_hex(0x5CB3E8), 0);//设置背景颜色
	lv_obj_set_style_radius(panel_date,10,0);						// 设置圆角
	lv_obj_set_style_border_width(panel_date,0,0);					// 设置边框宽度
	lv_obj_clear_flag(panel_date,LV_OBJ_FLAG_SCROLLABLE);			// 关闭滚动属性

	//时间标签
	label_time = lv_label_create(panel_time);
	lv_label_set_text_fmt(label_time,"%02d:%02d:%02d",cur_hour,cur_min,cur_sec);
	lv_obj_set_style_text_font(label_time,&Font_Num_24,0);
	lv_obj_center(label_time);

	//日期标签
	label_date = lv_label_create(panel_date);
	lv_label_set_text_fmt(label_date, MY_GUI_U8_DATE_FMT, cur_year, cur_month, cur_day);
	lv_obj_set_style_text_font(label_date,&Font_Num_16,0);
	lv_obj_center(label_date);

	lv_timer_create(time_timer_cb, 1000, NULL);

	return panel_time;
}
void device_btn_event_cb(lv_event_t * e)
{
	/* 用 SHORT_CLICKED：松手且未判为滚动时即触发，比 CLICKED 更稳；图标子对象可点，避免部分机型点图标无事件 */
	if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED)
		return;

	lv_obj_t *target = lv_event_get_target(e);
	lv_obj_t *panel = target;
	if (lv_obj_check_type(target, &lv_img_class))
		panel = lv_obj_get_parent(target);

	device_id_t dev_id = (device_id_t)(intptr_t)lv_event_get_user_data(e);

	device_state[dev_id] = !device_state[dev_id];

	if (device_state[dev_id])
		lv_obj_set_style_bg_color(panel, lv_color_hex(0x2196F3), 0);
	else
		lv_obj_set_style_bg_color(panel, lv_color_hex(0x5CB3E8), 0);

	switch (dev_id)
	{
		case DEVICE_LIVINGROOM:		//客厅灯
			on_livingroom_clicked(device_state[dev_id]);
			break;
		case DEVICE_BEDROOM:		//卧室灯
			on_bedroom_clicked(device_state[dev_id]);
			break;
		case DEVICE_CURTAIN:		//窗帘
			on_curtain_clicked(device_state[dev_id]);
			break;
		case DEVICE_SPEAKER:		//音响
			on_speaker_clicked(device_state[dev_id]);
			break;
	}
}
/*
创建设备控制面板
		parent			父对象
		icon_src		图标
		x				
		y
		dev_id			设备编号
*/
static lv_obj_t *create_device_panel_icon(lv_obj_t *parent, const void *icon_src,
                                          lv_coord_t x, lv_coord_t y, device_id_t dev_id)
{
    // 创建面板容器
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 70, 80);                              // 设置尺寸
    lv_obj_set_pos(panel, x, y);                                 // 设置坐标
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x5CB3E8), 0); // 设置背景颜色
    lv_obj_set_style_radius(panel, 10, 0);                       // 设置圆角
    lv_obj_set_style_border_width(panel, 0, 0);                  // 无边框
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);            // 禁止滚动

    lv_obj_add_event_cb(panel, device_btn_event_cb, LV_EVENT_SHORT_CLICKED, (void *)(intptr_t)dev_id);

    lv_obj_t *img = lv_img_create(panel);
    lv_img_set_src(img, icon_src);
    lv_obj_center(img);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, device_btn_event_cb, LV_EVENT_SHORT_CLICKED, (void *)(intptr_t)dev_id);

    return panel;
}
//客厅灯控面板
static lv_obj_t *create_living_panel(lv_obj_t *parent)
{
	panel_livingroom = create_device_panel_icon( parent , &icon_living_light , 15 , 130 , DEVICE_LIVINGROOM);
	return panel_livingroom ; 
}
//卧室灯面板
static lv_obj_t *create_bedroom_panel(lv_obj_t *parent)
{
	panel_bedroom = create_device_panel_icon( parent , &icon_bedroom_light , 95 , 130 , DEVICE_BEDROOM);
	return panel_bedroom ; 
}
//窗帘控制面板
static lv_obj_t *create_curtain_panel(lv_obj_t *parent)
{
	panel_curtain = create_device_panel_icon( parent , &icon_curtain , 175 , 130 , DEVICE_CURTAIN);
	return panel_curtain ;
}
//音响控制面板
static lv_obj_t *create_speaker_panel(lv_obj_t *parent)
{
	panel_speaker = create_device_panel_icon( parent , &icon_speaker , 255 , 130 , DEVICE_SPEAKER);
	return panel_speaker ;
}
//温湿度控制面板
static lv_obj_t *create_temp_humi_panel(lv_obj_t *parent)
{
	//创建温度湿度面板
	lv_obj_t* panel = lv_obj_create(parent);
	lv_obj_set_size(panel,130,80);								// 设置尺寸
	lv_obj_set_pos(panel , 335 , 130);							// 设置坐标
	lv_obj_set_style_bg_color(panel,lv_color_hex(0x5CB3E8),0);	// 设置背景颜色
	lv_obj_set_style_radius(panel , 10, 0);						// 设置圆角
	lv_obj_set_style_border_width(panel , 0, 0);				// 无边框
	lv_obj_clear_flag(panel,LV_OBJ_FLAG_SCROLLABLE);			// 禁止滚动

	//温度显示   上半部分
	lv_obj_t* label_temp_title = lv_label_create(panel);
	lv_label_set_text(label_temp_title, MY_GUI_U8_WENDU);
	lv_obj_set_style_text_font(label_temp_title,&Font_Sensor_20,0);//绑定标签和字体
	lv_obj_align(label_temp_title,LV_ALIGN_TOP_LEFT,5,0);

	label_dht_temp_val = lv_label_create(panel);
	lv_label_set_text(label_dht_temp_val, "--\xE2\x84\x83");
	lv_obj_set_style_text_font(label_dht_temp_val, &Font_Sensor_20, 0); /* 与「温度」标题同字库同风格 */
	lv_obj_set_style_text_color(label_dht_temp_val, lv_color_hex(0x000000), 0);
	lv_obj_align(label_dht_temp_val, LV_ALIGN_TOP_RIGHT, -5, 0);

	//湿度显示   下半部分
	lv_obj_t* label_humidity_title = lv_label_create(panel);
	lv_label_set_text(label_humidity_title, MY_GUI_U8_SHIDU);
	lv_obj_set_style_text_font(label_humidity_title,&Font_Sensor_20,0);
	lv_obj_align(label_humidity_title,LV_ALIGN_BOTTOM_LEFT,5,0);

	label_dht_humi_val = lv_label_create(panel);
	lv_label_set_text(label_dht_humi_val, "--%%");
	lv_obj_set_style_text_font(label_dht_humi_val, &Font_Sensor_20, 0); /* 与「湿度」标题同字库同风格 */
	lv_obj_set_style_text_color(label_dht_humi_val, lv_color_hex(0x000000), 0);
	lv_obj_align(label_dht_humi_val, LV_ALIGN_BOTTOM_RIGHT, -5, -4);

	lv_timer_create(dht11_ui_timer_cb, 1000, NULL);

	return panel;
}
/**
 * @brief 创建提醒信息面板
 * @param parent：父对象
 * @retval 面板对象指针
 * @note  提醒信息
 */
static lv_obj_t *create_info_pannel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);

	lv_obj_set_size(panel, 303, 80);
	lv_obj_set_pos(panel, 15, 220);
	lv_obj_set_style_bg_color(panel, lv_color_hex(0x5CB3E8), 0);
	lv_obj_set_style_radius(panel, 11, 0);
	lv_obj_set_style_border_width(panel, 0, 0);
	lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

	label_info_tip = lv_label_create(panel);
	lv_obj_set_width(label_info_tip, 283);
	lv_label_set_long_mode(label_info_tip, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_font(label_info_tip, &Font_info_24, 0);
	lv_obj_set_style_text_color(label_info_tip, lv_color_hex(0x000000), 0);
	lv_label_set_text(label_info_tip, "");
	lv_obj_align(label_info_tip, LV_ALIGN_TOP_LEFT, 10, 8);

	return panel;
}
//光照检测
static lv_obj_t *create_light_panel(lv_obj_t *parent)
{
	//创建 光照 面板
	lv_obj_t* panel = lv_obj_create(parent);
	lv_obj_set_size(panel,130,80);								// 设置尺寸
	lv_obj_set_pos(panel , 335 , 225);							// 设置坐标
	lv_obj_set_style_bg_color(panel,lv_color_hex(0x5CB3E8),0);	// 设置背景颜色
	lv_obj_set_style_radius(panel , 10, 0);						// 设置圆角
	lv_obj_set_style_border_width(panel , 0, 0);				// 无边框
	lv_obj_clear_flag(panel,LV_OBJ_FLAG_SCROLLABLE);			// 禁止滚动	

	//光照显示
	lv_obj_t* label_light_sensor_title = lv_label_create(panel);
	lv_label_set_text(label_light_sensor_title,"光照");
	lv_obj_set_style_text_font(label_light_sensor_title,&Font_Sensor_20,0);
	lv_obj_align(label_light_sensor_title,LV_ALIGN_LEFT_MID,5,0);

	label_bh1750_lux_val = lv_label_create(panel);
	lv_label_set_text(label_bh1750_lux_val, "--");
	lv_obj_set_style_text_font(label_bh1750_lux_val, &Font_Sensor_20, 0);
	lv_obj_set_style_text_color(label_bh1750_lux_val, lv_color_hex(0x000000), 0);
	lv_obj_align_to(label_bh1750_lux_val, label_light_sensor_title, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	return panel;
}
void LVGL_Project_Main(void)
{
	// 获取源屏幕对象
	lv_obj_t *scr = lv_scr_act();
	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	// 背景颜色
	lv_obj_set_style_bg_color(scr, lv_color_hex(0xADD2EE), 0);

    /*-----------------------顶部信息面板---------------------------*/
	// 天气信息面板
	create_weather_pannel(scr);
	//时间信息面板
	create_time_panel(scr);

	/*-----------------------设备控制面板（第一行）---------------------------*/
	create_living_panel(scr);			//客厅灯
	create_bedroom_panel(scr);			//空调
	create_curtain_panel(scr);		    //窗帘
	create_speaker_panel(scr);		    //音响
	create_temp_humi_panel(scr);		//温湿度检测

	/*-----------------------提醒信息面板（第二行）---------------------------*/
	create_info_pannel(scr);
  create_light_panel(scr);		    //光照检测
}

// 客厅灯控制回调函数
//使用_weak修饰，用户可在其他文件中重写实现
__weak void on_livingroom_clicked(uint8_t state)
{
	if(state)
	{
		APP_LOG("客厅灯光已经启动\n");
	}
	else
	{
		APP_LOG("客厅灯光已经关闭\n");
	}
}
// 卧室灯控制回调函数
//使用_weak修饰，用户可在其他文件中重写实现
__weak void on_bedroom_clicked(uint8_t state)
{
	if(state)
	{
		APP_LOG("空调已经启动\n");
	}
	else
	{
		APP_LOG("空调已经关闭\n");
	}
}
// 窗帘控制回调函数
//使用_weak修饰，用户可在其他文件中重写实现
__weak void on_curtain_clicked(uint8_t state)
{
	if(state)
	{
		APP_LOG("窗帘已经启动\n");
	}
	else
	{
		APP_LOG("窗帘已经关闭\n");
	}
}
// 音响控制回调函数
//使用_weak修饰，用户可在其他文件中重写实现
__weak void on_speaker_clicked(uint8_t state)
{
	if(state)
	{
		APP_LOG("音响已经启动\n");
	}
	else
	{
		APP_LOG("音响已经关闭\n");
	}
}


