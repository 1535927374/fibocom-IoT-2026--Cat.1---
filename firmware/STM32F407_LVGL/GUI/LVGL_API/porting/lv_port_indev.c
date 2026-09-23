/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "lvgl.h"

#include "LCD_Touch.h" 
#include "LCD_Init.h"
#include "LCD_Disp.h"

#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);



/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_touchpad;



/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */
    
		touchpad_init();

    static lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);

    (void)indev_touchpad;

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    TP_Init();
}

/*Will be called by the library to read the touchpad*/
/*
indev_drv �����豸��������ָ��

data
	point
*/
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;
    
    static lv_coord_t last_x = 0, last_y = 0;
    static uint8_t last_state = 0;

    /* ���õײ�����ɨ�� */
    uint8_t sta = TP_Scan(0);

    if(sta & TP_PRES_DOWN) {
        /* �������£���ȡ���겢�߽����� */
        lv_coord_t x = tp_dev.x[0];
        lv_coord_t y = tp_dev.y[0];

        /* �߽��� */
        if(x < 0) x = 0;
        if(y < 0) y = 0;
        if(x >= LCD_W) x = LCD_W - 1;
        if(y >= LCD_H) y = LCD_H - 1;

        /* ���¾�̬���� */
        last_x = x;
        last_y = y;
        last_state = 1;

        data->state   = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;

        #if 0
        static uint32_t debug_cnt = 0;
        if (debug_cnt++ % 50 == 0) {
            printf("Touch Press: x=%d, y=%d, sta=0x%02X\r\n", x, y, sta);
        }
        #endif
    } 
    else {
        /* �����ͷţ�������һ�ε����굫����?�ͷ�״̬ */
        data->state   = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;  // LVGL��Ҫ������������ɵ���¼�
        data->point.y = last_y;
        last_state = 0;
    }
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/

		return (tp_dev.sta & TP_PRES_DOWN) ? true : false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/
	lv_coord_t tp_x = tp_dev.x[0];
	lv_coord_t tp_y = tp_dev.y[0];
	
	if(tp_x < 0) tp_x = 0;
	if(tp_y < 0) tp_y = 0;
	
	if(tp_x > LCD_W) tp_x = LCD_W - 1;
	if(tp_y > LCD_H) tp_y = LCD_H - 1;
	
	*x = tp_x;
	*y = tp_y;
		
}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
