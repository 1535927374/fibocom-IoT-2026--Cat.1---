#ifndef __OLED_H_
#define __OLED_H_

#include "sys.h"

enum message {
    SHOW_INPUT_PWD = 0,
    SHOW_PWD_RIGHT,
    SHOW_PWD_WRONG,
    SHOW_INPUT_OLD_PWD,
    SHOW_INPUT_NEW_PWD,
    SHOW_PWD_CHANGED,
    SHOW_SET_PWD
};

#define OLED_I2C_SCL_CLK()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define OLED_I2C_SCL_PORT       GPIOF
#define OLED_I2C_SCL_PIN        GPIO_PIN_1

#define OLED_I2C_SDA_CLK()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define OLED_I2C_SDA_PORT       GPIOF
#define OLED_I2C_SDA_PIN        GPIO_PIN_0

#define OLED_SCL_RESET()        HAL_GPIO_WritePin(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SCL_SET()          HAL_GPIO_WritePin(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN, GPIO_PIN_SET)

#define OLED_SDA_RESET()        HAL_GPIO_WritePin(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN, GPIO_PIN_RESET)
#define OLED_SDA_SET()          HAL_GPIO_WritePin(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN, GPIO_PIN_SET)

void oled_init(void);
void oled_write_cmd(uint8_t cmd);
void oled_write_data(uint8_t data);
void oled_fill(uint8_t data);
void oled_set_cursor(uint8_t x, uint8_t y);
void oled_show_char(uint8_t x, uint8_t y, uint8_t num, uint8_t size);
void oled_show_string(uint8_t x, uint8_t y, char *p, uint8_t size);
void oled_show_chinese(uint8_t x, uint8_t y, uint8_t N, uint8_t message_type);
void oled_show_image(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t *bmp);
void oled_show_input(void);
void oled_show_right(void);
void oled_show_wrong(void);
void oled_show_old(void);
void oled_show_new(void);
void oled_show_set(void);
void oled_show_changed(void);

#endif
