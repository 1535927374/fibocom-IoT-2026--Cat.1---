#include "password.h"
#include "w25q128.h"
#include "oled.h"
#include "keyboard.h"
#include "string.h"
#include "stdio.h"
#include "lock.h"
#include "beep.h"
#include "delay.h"

#define PASSWORD_SIZE   10
uint8_t pwd_input[PASSWORD_SIZE] = {0};
uint8_t pwd_read[PASSWORD_SIZE] = {0};
uint8_t i = 0, key_value = 0, try_times = 0;

//初始化函数
void password_init(void)
{
    w25q128_init();
}

//清空输入缓存
void password_input_clear(void)
{
    memset(pwd_input, 0, PASSWORD_SIZE);
    i = 0;
}

//保存密码
void password_save(void)
{
    w25q128_erase_sector(0x000000);
    w25q128_write_page(0x000000, pwd_input, PASSWORD_SIZE);
    oled_show_changed();
}

//获取键盘输入
uint8_t password_get_input(void)
{
    password_input_clear();
    while(1)
    {
        key_value = keyboard_get_value();
        delay_ms(2);
        if(key_value == POUND_KEY)
        {
            printf("按下了#号键，input: %s\r\n", pwd_input);
            return POUND_KEY;
        }
        else if(key_value == STAR_KEY)
        {
            printf("按下了*号键\r\n");
            return STAR_KEY;
        }
        else if(key_value != 0)
        {
            printf("按下了：%c\r\n", key_value);
            oled_show_char(20 + i * 10, 4, key_value, 16);
            pwd_input[i++] = key_value;
        }
    }
}

//用于保存外部设置密码数据
void password_set_input(uint8_t key)
{
    if(i < PASSWORD_SIZE && key != 0)
        pwd_input[i++] = key;
}

//密码比对
uint8_t password_compare(void)
{
    uint8_t i = 0;
    w25q128_read_data(0x000000, pwd_read, PASSWORD_SIZE);
    
    if(strlen((char *)pwd_input) != strlen((char *)pwd_read))
        return FALSE;
    
    for(i = 0; i < strlen((char *)pwd_read); i++)
    {
        if(pwd_input[i] != pwd_read[i])
            return FALSE;
    }
    
    return TRUE;
}

//密码输入正确的操作
void password_input_right_action(void)
{
    oled_show_right();
    lock_on();
    beep_on();
    delay_ms(300);
    beep_off();
    delay_ms(1000);
    lock_off();
    try_times = 0;
}

//密码输入错误的操作
void password_input_wrong_action(void)
{
    oled_show_wrong();
    try_times++;
    if(try_times >= 3)
    {
        beep_on();
        delay_ms(1000);
        beep_off();
        try_times = 0;
    }
    delay_ms(1000);
}

//旧密码输入正确的操作
void password_old_right_action(void)
{
    oled_show_new();
    password_get_input();
    password_save();
    
    beep_on();
    delay_ms(300);
    beep_off();
    delay_ms(500);
}

//旧密码输入错误的操作
void password_old_wrong_action(void)
{
    oled_show_wrong();
    delay_ms(1000);
}

//检查密码是否存在
void password_check(void)
{
    w25q128_read_data(0x000000, pwd_read, PASSWORD_SIZE);
    printf("请出密码：%s\r\n", pwd_read);
    if(pwd_read[0] == '\0' || pwd_read[0] == 0xFF)
    {
        oled_show_set();
        password_get_input();
        password_save();
    }
}
