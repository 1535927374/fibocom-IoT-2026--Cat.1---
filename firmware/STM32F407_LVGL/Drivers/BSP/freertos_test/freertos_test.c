#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "curtain.h"
#include "freertos_test.h"

#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "beep.h"
#include "keyboard.h"
#include "lock.h"
#include "oled.h"
#include "password.h"
#include "w25q128.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "My_GUI.h"

#include "stdio.h"
#include <stdlib.h>
#include "app_config.h"
#include "bluetooth.h"
#include "voice.h"
#include "dht11.h"
#include "bh1750.h"
#include "string.h"
#include "onenet.h"
#include "l610.h"
#include "weather.h"

/* ---------- LVGL ---------- */
#define LVGL_TASK_STACK_WORDS  2048U
#define LVGL_TASK_PRIORITY     12U

/* ---------- 任务句柄 ---------- */
TaskHandle_t keyboard_task_handle;
TaskHandle_t password_task_handle;
TaskHandle_t display_task_handle;
TaskHandle_t monitor_task_handle;
TaskHandle_t beep_task_handle;
TaskHandle_t lock_task_handle;
TaskHandle_t light_control_task_handle;
TaskHandle_t env_upload_task_handle;
TaskHandle_t mqtt_task_handle;  // MQTT任务句柄

/* ---------- 队列 / 同步 ---------- */
QueueHandle_t password_queue;
QueueHandle_t display_queue;
QueueHandle_t beep_queue;
QueueHandle_t lock_queue;
SemaphoreHandle_t w25q128_mutex;
EventGroupHandle_t system_event_group;
TimerHandle_t auto_lock_timer;

/* ---------- 全局变量 ---------- */
uint8_t led_status = 0, voice_status_old = 0;
uint8_t pwd_index = 0;
static uint8_t try_times = 0;
uint8_t door_opened = 0;
static uint8_t s_light_last_bt_seq = 0;
/*
 * 客厅灯最近一次「非语音」指令来源：0=语音在跟手，2=触摸屏，3=OneNET，4=蓝牙(0~3 档)。
 */
static volatile uint8_t s_living_ctrl_last_src;
#define LIVING_SRC_TOUCH   2u
#define LIVING_SRC_ONENET  3u
#define LIVING_SRC_BT      4u

/* 卧室灯最近一次非语音来源：2=触摸，5=蓝牙(字符 4/5) */
static volatile uint8_t s_bedroom_ctrl_last_src;
#define BEDROOM_SRC_TOUCH  2u
#define BEDROOM_SRC_BT     5u

static void living_room_external_set(uint8_t level_0_3, uint8_t src);
static void bedroom_external_set(uint8_t on, uint8_t src);

/* ---------- 消息与状态 ---------- */
typedef struct {
    uint8_t cmd;
    uint8_t data;
} system_msg_t;

enum {
    INPUT_MODE,
    MODIFY_MODE,
    SET_MODE
};

#define EVENT_PWD_CORRECT   (1 << 0)
#define EVENT_PWD_WRONG     (1 << 1)
#define EVENT_ALARM         (1 << 2)

#define CMD_LOCK_OPEN        0x01
#define CMD_LOCK_CLOSE       0x02

#define CMD_BEEP_SHORT       0x03
#define CMD_BEEP_LONG        0x04

#define CMD_OLED_SHOW_INPUT          0x00
#define CMD_OLED_SHOW_RIGHT          0x01
#define CMD_OLED_SHOW_WRONG          0x02
#define CMD_OLED_SHOW_NEW            0x03
#define CMD_OLED_SHOW_OLD            0x04
#define CMD_OLED_SHOW_CHAR           0x05
#define CMD_OLED_SHOW_CHANGED        0x06
#define CMD_OLED_SHOW_SET            0x07
#define CMD_OLED_SHOW_WRONG_DELAY    0x08
#define CMD_OLED_SHOW_CHANGED_DELAY  0x09
#define CMD_OLED_SHOW_ALARM_DELAY    0x0A

#define CMD_KEY_INPUT        0x00

static void send_msg(QueueHandle_t queue, uint8_t cmd, uint8_t data)
{
    system_msg_t msg;
    msg.cmd  = cmd;
    msg.data = data;
    (void)xQueueSend(queue, &msg, portMAX_DELAY);
}

/* ============================== LVGL 任务 ============================== */
static void lvgl_task(void *pvParameters)
{
    (void)pvParameters;

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    LVGL_Project_Main();
    /* main 里已 get_today_weather，此处界面就绪后刷新图标 */
    show_today_weather();

    for (;;) {
        uint32_t delay_ms = lv_timer_handler();
        if (delay_ms < 1U) delay_ms = 1U;
        if (delay_ms > 30U) delay_ms = 30U;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

/* ============================== 按键任务 ============================== */
void keyboard_task(void *pvParams)
{
    uint8_t key_value;
    for (;;) {
        key_value = keyboard_get_value();
        if (key_value != 0) {
            APP_LOG("按下按键：%c\r\n", key_value);
            send_msg(password_queue, CMD_KEY_INPUT, key_value);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ============================== 密码任务 ============================== */
void password_task(void *pvParams)
{
    system_msg_t rcv_msg;
    uint8_t password_mode = INPUT_MODE;
    uint8_t pwd_temp[10] = {0};

    if (xSemaphoreTake(w25q128_mutex, portMAX_DELAY) == pdTRUE) {
        w25q128_read_data(0x000000, pwd_temp, 10);
        xSemaphoreGive(w25q128_mutex);
        APP_LOG("检查密码：%s\r\n", pwd_temp);
    }

    if (pwd_temp[0] == '\0' || pwd_temp[0] == 0xFF) {
        send_msg(display_queue, CMD_OLED_SHOW_SET, 0);
        password_mode = SET_MODE;
    } else {
        send_msg(display_queue, CMD_OLED_SHOW_INPUT, 0);
    }

    for (;;) {
        if (xQueueReceive(password_queue, &rcv_msg, portMAX_DELAY) == pdPASS) {
            if (rcv_msg.cmd == CMD_KEY_INPUT) {
                if (rcv_msg.data == POUND_KEY) {
                    if (password_mode == SET_MODE) {
                        if (xSemaphoreTake(w25q128_mutex, portMAX_DELAY) == pdTRUE) {
                            password_save();
                            xSemaphoreGive(w25q128_mutex);
                            send_msg(display_queue, CMD_OLED_SHOW_CHANGED_DELAY, 0);
                            send_msg(beep_queue, CMD_BEEP_SHORT, 0);
                            password_mode = INPUT_MODE;
                        }
                    } else {
                        uint8_t result = FALSE;
                        if (xSemaphoreTake(w25q128_mutex, portMAX_DELAY) == pdTRUE) {
                            result = password_compare();
                            xSemaphoreGive(w25q128_mutex);
                        }
                        if (password_mode == INPUT_MODE) {
                            if (result == TRUE) {
                                xEventGroupSetBits(system_event_group, EVENT_PWD_CORRECT);
                                try_times = 0;
                                send_msg(display_queue, CMD_OLED_SHOW_RIGHT, 0);
                                send_msg(beep_queue, CMD_BEEP_SHORT, 0);
                                send_msg(lock_queue, CMD_LOCK_OPEN, 0);
                                door_opened = 1;
                                xTimerStart(auto_lock_timer, 0);
                            } else {
                                xEventGroupSetBits(system_event_group, EVENT_PWD_WRONG);
                                try_times++;
                                if (try_times >= 3) {
                                    xEventGroupSetBits(system_event_group, EVENT_ALARM);
                                    send_msg(beep_queue, CMD_BEEP_LONG, 0);
                                    send_msg(display_queue, CMD_OLED_SHOW_ALARM_DELAY, 0);
                                    try_times = 0;
                                } else {
                                    send_msg(display_queue, CMD_OLED_SHOW_WRONG_DELAY, 0);
                                }
                            }
                        } else if (password_mode == MODIFY_MODE) {
                            if (result == TRUE) {
                                send_msg(display_queue, CMD_OLED_SHOW_NEW, 0);
                                send_msg(beep_queue, CMD_BEEP_SHORT, 0);
                                password_mode = SET_MODE;
                            } else {
                                send_msg(display_queue, CMD_OLED_SHOW_WRONG_DELAY, 0);
                                password_mode = INPUT_MODE;
                            }
                        }
                    }
                    password_input_clear();
                    pwd_index = 0;
                } else if (rcv_msg.data == STAR_KEY) {
                    APP_LOG("按下*键，进入修改密码模式\r\n");
                    password_mode = MODIFY_MODE;
                    send_msg(display_queue, CMD_OLED_SHOW_OLD, 0);
                    password_input_clear();
                    pwd_index = 0;
                } else {
                    if (pwd_index < 10) {
                        password_set_input(rcv_msg.data);
                        pwd_index++;
                        send_msg(display_queue, CMD_OLED_SHOW_CHAR, rcv_msg.data);
                    }
                }
            }
        }
    }
}

/* ============================== OLED 显示任务 ============================== */
void display_task(void *pvParams)
{
    system_msg_t rcv_msg;
    uint8_t local_pwd_index;
    for (;;) {
        if (xQueueReceive(display_queue, &rcv_msg, portMAX_DELAY) == pdPASS) {
            switch (rcv_msg.cmd) {
                case CMD_OLED_SHOW_INPUT:          oled_show_input(); break;
                case CMD_OLED_SHOW_RIGHT:          oled_show_right(); break;
                case CMD_OLED_SHOW_WRONG:          oled_show_wrong(); break;
                case CMD_OLED_SHOW_NEW:            oled_show_new(); break;
                case CMD_OLED_SHOW_OLD:            oled_show_old(); break;
                case CMD_OLED_SHOW_CHANGED:        oled_show_changed(); break;
                case CMD_OLED_SHOW_SET:            oled_show_set(); break;

                case CMD_OLED_SHOW_WRONG_DELAY:
                    oled_show_wrong(); vTaskDelay(pdMS_TO_TICKS(1000)); oled_show_input(); break;
                case CMD_OLED_SHOW_ALARM_DELAY:
                    oled_show_wrong(); vTaskDelay(pdMS_TO_TICKS(3500)); oled_show_input(); break;
                case CMD_OLED_SHOW_CHANGED_DELAY:
                    oled_show_changed(); vTaskDelay(pdMS_TO_TICKS(1000)); oled_show_input(); break;

                case CMD_OLED_SHOW_CHAR:
                    local_pwd_index = pwd_index;
                    oled_show_char(20 + (local_pwd_index - 1)*10, 4, rcv_msg.data, 16);
                    break;
                default: break;
            }
        }
    }
}

/* ============================== 门锁任务 ============================== */
void lock_task(void *pvParams)
{
    system_msg_t rcv_msg;
    for (;;) {
        if (xQueueReceive(lock_queue, &rcv_msg, portMAX_DELAY) == pdPASS) {
            switch (rcv_msg.cmd) {
                case CMD_LOCK_OPEN:  APP_LOG("执行开锁\r\n"); lock_on();  break;
                case CMD_LOCK_CLOSE: APP_LOG("执行关锁\r\n"); lock_off(); break;
                default: break;
            }
        }
    }
}

/* ============================== 蜂鸣器任务 ============================== */
void beep_task(void *pvParams)
{
    system_msg_t rcv_msg;
    for (;;) {
        if (xQueueReceive(beep_queue, &rcv_msg, portMAX_DELAY) == pdPASS) {
            switch (rcv_msg.cmd) {
                case CMD_BEEP_SHORT:
                    beep_on(); vTaskDelay(pdMS_TO_TICKS(300)); beep_off();
                    APP_LOG("执行短鸣\r\n");
                    break;
                case CMD_BEEP_LONG:
                    beep_on(); vTaskDelay(pdMS_TO_TICKS(2000)); beep_off();
                    APP_LOG("执行长鸣\r\n");
                    break;
                default: break;
            }
        }
    }
}

/* ============================== 监控任务 ============================== */
void monitor_task(void *pvParams)
{
    EventBits_t event_bits;
    for (;;) {
        event_bits = xEventGroupWaitBits(
            system_event_group,
            EVENT_PWD_CORRECT | EVENT_PWD_WRONG | EVENT_ALARM,
            pdTRUE, pdFALSE, portMAX_DELAY);

        if (event_bits & EVENT_PWD_CORRECT)  APP_LOG("监控：密码验证成功\r\n");
        if (event_bits & EVENT_PWD_WRONG)    APP_LOG("监控：密码验证失败\r\n");
        if (event_bits & EVENT_ALARM)        APP_LOG("监控：密码验证报警\r\n");
    }
}

/* ============================== 自动关锁 ============================== */
void auto_lock_timer_callback(TimerHandle_t xTimer)
{
    if (door_opened) {
        send_msg(lock_queue, CMD_LOCK_CLOSE, 0);
        send_msg(display_queue, CMD_OLED_SHOW_INPUT, 0);
        door_opened = 0;
    }
}

/* ============================== 灯光控制任务 ============================== */
void light_control_task(void *pvParams)
{
    for (;;) {
        uint8_t vraw = voice_value_get() & 3u;
        uint8_t seq = bt_cmd_seq_get();

        if (seq != s_light_last_bt_seq) {
            uint8_t b;

            s_light_last_bt_seq = seq;
            b = bt_value_get();
            /* 蓝牙 UART 单字节 '0'~'3'：客厅 PWM 档；'4' 卧室开，'5' 卧室关 */
            if (b <= 3u) {
                living_room_external_set(b, LIVING_SRC_BT);
            } else if (b == 4u) {
                bedroom_external_set(1u, BEDROOM_SRC_BT);
            } else if (b == 5u) {
                bedroom_external_set(0u, BEDROOM_SRC_BT);
            }
        } else {
            /* 客厅 0~3：语音口线变化即跟档；不再依赖 s_voice_link_armed/s_voice_idle_snap，
             * 否则触摸/蓝牙等 external_set 后 snap 与 v 对齐会导致「高亮后再低亮」不更新 led_status */
            if (led_status <= 3u) {
                if (vraw != voice_status_old) {
                    led_status = voice_living_level_from_raw(vraw);
                    s_living_ctrl_last_src = 0;
                }
                voice_status_old = vraw;
            } else {
                if (led_status == 4u) {
                    /* 卧室灯亮：口线变化不把 led_status 改成 0~3，避免误关卧室又去调客厅 PWM */
                    voice_status_old = vraw;
                } else if (vraw != voice_status_old) {
                    led_status = voice_living_level_from_raw(vraw);
                    voice_status_old = vraw;
                }
            }
        }

        switch (led_status) {
            case 0: led_living_off();    break;
            case 1: led_living_low();    break;
            case 2: led_living_medium(); break;
            case 3: led_living_high();   break;
            case 4: led_bedroom_on();    break;
            case 5: led_bedroom_off();   break;
            default: break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ---------- OneNET：解析 property/set 的 PUBLISH（应答走 onenet.c -> RELY_PUBLISH_TOPIC） ---------- */
static void mqtt_json_copy_string_field_simple(const char *src, const char *key, char *out,
                                               size_t outn)
{
    char keypat[40];
    const char *p;
    const char *q;

    out[0] = '\0';
    if (outn == 0U)
        return;
    snprintf(keypat, sizeof(keypat), "\"%s\"", key);
    p = strstr(src, keypat);
    if (p == NULL)
        return;
    p += strlen(keypat);
    while (*p == ' ' || *p == '\t' || *p == ':')
        p++;
    if (*p != '"')
        return;
    p++;
    q = strchr(p, '"');
    if (q == NULL)
        return;
    {
        size_t L = (size_t)(q - p);
        if (L >= outn)
            L = outn - 1U;
        memcpy(out, p, L);
        out[L] = '\0';
    }
}

static int mqtt_extract_message_inner_json(const char *outer, char *out, size_t outsz)
{
    const char *key = strstr(outer, "\"message\"");
    const char *p;
    size_t j = 0;

    if (key == NULL || outsz == 0U)
        return -1;
    p = strchr(key, ':');
    if (p == NULL)
        return -1;
    p++;
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p != '"')
        return -1;
    p++;
    while (*p != '\0' && j + 1U < outsz)
    {
        if (*p == '\\' && p[1] == '"')
        {
            out[j++] = '"';
            p += 2;
            continue;
        }
        if (*p == '\\' && p[1] == '\\')
        {
            out[j++] = '\\';
            p += 2;
            continue;
        }
        if (*p == '"')
            break;
        out[j++] = *p++;
    }
    out[j] = '\0';
    return 0;
}

static void mqtt_extract_correlation_id(const char *payload, const char *inner, char *out,
                                        size_t outn)
{
    out[0] = '\0';
    if (outn == 0U || payload == NULL)
        return;

    mqtt_json_copy_string_field_simple(payload, "request_id", out, outn);
    if (out[0] != '\0')
        return;
    mqtt_json_copy_string_field_simple(payload, "id", out, outn);
    if (out[0] != '\0')
        return;

    if (inner != NULL && inner != payload)
    {
        mqtt_json_copy_string_field_simple(inner, "request_id", out, outn);
        if (out[0] != '\0')
            return;
        mqtt_json_copy_string_field_simple(inner, "id", out, outn);
    }
}

static int mqtt_parse_led_brightness(const char *s)
{
    const char *k = strstr(s, "\"led_brightness\"");
    if (k == NULL)
        k = strstr(s, "led_brightness");
    if (k == NULL)
        return -1;
    k = strchr(k, ':');
    if (k == NULL)
        return -1;
    k++;
    while (*k == ' ' || *k == '\t')
        k++;

    if (*k == '{')
    {
        const char *val = strstr(k, "\"value\"");
        if (val == NULL)
            val = strstr(k, "value");
        if (val == NULL)
            return -1;
        val = strchr(val, ':');
        if (val == NULL)
            return -1;
        val++;
        while (*val == ' ' || *val == '\t')
            val++;
        {
            char *end = NULL;
            long n = strtol(val, &end, 10);
            if (end == val)
                return -1;
            return (int)n;
        }
    }

    {
        char *end = NULL;
        long v = strtol(k, &end, 10);
        if (end == k)
            return -1;
        return (int)v;
    }
}

/* 触摸 / OneNET 等：仅写 led_status 与语音基准；GPIO/PWM 由 light_control_task 统一落盘，缩短临界区 */
static void living_room_external_set(uint8_t level_0_3, uint8_t src)
{
    uint8_t vnow = (uint8_t)(voice_value_get() & 3u);

    if (level_0_3 > 3u)
        return;

    taskENTER_CRITICAL();
    led_status = level_0_3;
    voice_status_old = vnow;
    s_living_ctrl_last_src = src;
    taskEXIT_CRITICAL();
}

/* 卧室灯：触摸/蓝牙统一入口；led_status 4=开 5=关（与 light_control_task switch 一致） */
static void bedroom_external_set(uint8_t on, uint8_t src)
{
    uint8_t vnow = (uint8_t)(voice_value_get() & 3u);

    taskENTER_CRITICAL();
    if (on) {
        led_status = 4u;
    } else {
        led_status = 5u;
    }
    voice_status_old = vnow;
    s_bedroom_ctrl_last_src = src;
    taskEXIT_CRITICAL();
}

/* 内层 params：0 关客厅灯；其余为客厅亮度档 */
static void mqtt_apply_led_brightness_onenet(int v)
{
    uint8_t level;

    if (v == 0)
        level = 0;
    else if (v >= 1 && v <= 30)
        level = 1;
    else if (v >= 31 && v <= 60)
        level = 2;
    else if (v >= 61 && v <= 100)
        level = 3;
    else if (v > 100)
        level = 3;
    else
        return;

    living_room_external_set(level, LIVING_SRC_ONENET);
    APP_LOG("客厅灯(OneNET): led_brightness=%d -> level%u\r\n", v, (unsigned)level);
}

/* ============================== 环境数据上传任务 ============================== */
void env_upload_task(void *pvParams)
{
    uint8_t dht11_data[4];
    uint8_t data_send_buf[512];
    int led_brightness_val;
    uint8_t st;
    float luxf;
    int lux_ip;
    int lux_fp1;
    /* OneNET enum LightModuleStatus: 0正常 1通信失败 2设备异常(预留) */
    int light_module_status;
    /* OneNET enum SoundModuleStatus: 0正常 1通信失败 2设备异常 */
    int sound_module_status;
    /*
     * OneNET 标识符 HumitureModuleStatus（与平台物模型一致）；
     * 取值仍表示 L610 蜂窝侧 MQTT 会话：0 正常 1 失败 2 预留（见 mqtt_onenet_l610_module_status）。
     */
    int humiture_module_status;

    for (;;) {
        /* dht11_read 内含 ms 级延时与位时序等待，禁止放在 taskENTER_CRITICAL 内 */
        (void)dht11_read(dht11_data);
        taskENTER_CRITICAL();
        st = led_status;
        taskEXIT_CRITICAL();

        switch (st) {
            case 0: led_brightness_val = 0; break;
            case 1: led_brightness_val = 15; break;
            case 2: led_brightness_val = 45; break;
            case 3: led_brightness_val = 80; break;
            default: led_brightness_val = 0; break;
        }

        luxf = bh1750_read_lux();
        if (luxf < 0.f)
        {
            lux_ip = 0;
            lux_fp1 = 0;
            light_module_status = 1; /* I2C 读失败 -> 通信失败 */
        }
        else
        {
            light_module_status = 0; /* 通信正常 */
            if (luxf > 10000.f)
                luxf = 10000.f;
            {
                int t = (int)(luxf * 10.0f + 0.5f);
                lux_ip = t / 10;
                lux_fp1 = t % 10;
            }
        }

        sound_module_status = (int)voice_onenet_sound_module_status();
        humiture_module_status = (int)mqtt_onenet_l610_module_status();

        APP_LOG("湿度：%d.%d%%RH , 温度：%d.%d C , 光照：%d.%d lx\r\n",
            dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3], lux_ip, lux_fp1);

        memset(data_send_buf, 0, sizeof(data_send_buf));

        snprintf((char *)data_send_buf, sizeof(data_send_buf),
            "{\"id\":\"%s\",\"version\":\"1.0\",\"params\":{\"CurrentTemperature\":{\"value\":%d.%d},\"RelativeHumidity\":{\"value\":%d.%d},\"led_brightness\":{\"value\":%d},\"LightLuxValue\":{\"value\":%d.%d},\"LightModuleStatus\":{\"value\":%d},\"SoundModuleStatus\":{\"value\":%d},\"HumitureModuleStatus\":{\"value\":%d}}}",
            APP_ONENET_JSON_MSG_ID,
            dht11_data[2], dht11_data[3], dht11_data[0], dht11_data[1], led_brightness_val,
            lux_ip, lux_fp1, light_module_status, sound_module_status, humiture_module_status);

        mqtt_publish_data(POST_TOPIC, (char *)data_send_buf, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/*
 * OneNET 属性设置下行示例（外层）：
 * {"title":"属性设置","message":"{\"product_id\":\"...\",\"params\":{\"led_brightness\":30}}",...}
 * "message" 为字符串化 JSON，须二次解析；led_brightness 在内层 params。
 */
/* ============================== MQTT（L610 透传）接收 + 心跳 ============================== */
void mqtt_task(void *pvParams)
{
    static uint32_t heart_tick = 0;
    Mqtt_RxData_Type mqtt_rd;
    uint8_t raw_rx[512];
    char inner_json[480];
    char req_id[48];
    int inner_from_message;

    (void)pvParams;

    for (;;)
    {
        l610_receive_data();

        if (l610_wait_receive() == L610_EOK)
        {
            uint16_t n = l610_copy_rxdata((char *)raw_rx);
            uint16_t off;

            for (off = 0; off < n; off++)
            {
                uint8_t t = raw_rx[off];
                /* MQTT 3.1.1：PUBLISH 报文类型为高 4 位 = 0x3（QoS0/1/2 对应 0x30/0x32/0x34 等） */
                if ((t >> 4) != 3u)
                    continue;
                memset(&mqtt_rd, 0, sizeof(mqtt_rd));
                if (mqtt_receive_handle(raw_rx + off, &mqtt_rd) != 0)
                    continue;

                if (mqtt_rd.topic_len >= sizeof(mqtt_rd.topic))
                    mqtt_rd.topic_len = sizeof(mqtt_rd.topic) - 1U;
                if (mqtt_rd.payload_len >= sizeof(mqtt_rd.payload))
                    mqtt_rd.payload_len = sizeof(mqtt_rd.payload) - 1U;
                mqtt_rd.topic[mqtt_rd.topic_len] = '\0';
                mqtt_rd.payload[mqtt_rd.payload_len] = '\0';

                {
                    const char *tpc = (const char *)mqtt_rd.topic;
                    if (strstr(tpc, SET_TOPIC) == NULL &&
                        strstr(tpc, "thing/property/set") == NULL)
                        continue;
                }

                inner_from_message =
                    (mqtt_extract_message_inner_json((char *)mqtt_rd.payload, inner_json,
                                                     sizeof(inner_json)) == 0)
                        ? 1
                        : 0;
                if (!inner_from_message)
                {
                    strncpy(inner_json, (char *)mqtt_rd.payload, sizeof(inner_json) - 1U);
                    inner_json[sizeof(inner_json) - 1U] = '\0';
                }

                mqtt_extract_correlation_id((char *)mqtt_rd.payload, inner_json, req_id,
                                            sizeof(req_id));

                {
                    /* 标准格式：仅从解包后的内层取 params.led_brightness */
                    int bright = mqtt_parse_led_brightness(inner_json);
                    if (bright < 0 && !inner_from_message)
                        bright = mqtt_parse_led_brightness((char *)mqtt_rd.payload);

                    if (bright >= 0)
                    {
                        mqtt_apply_led_brightness_onenet(bright);
                        mqtt_send_response(req_id, 200, "success");
                    }
                    else
                    {
                        mqtt_send_response(req_id, 10410, "no led_brightness in params");
                    }
                }
                break;
            }
            l610_host_rx_flush();
        }

        if ((xTaskGetTickCount() - heart_tick) > pdMS_TO_TICKS(60000))
        {
            mqtt_send_heart();
            heart_tick = xTaskGetTickCount();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* 覆盖 My_GUI.c 中 __weak：触摸与 OneNET 共用 living_room_external_set */
void on_livingroom_clicked(uint8_t state)
{
    living_room_external_set(state ? 3u : 0u, LIVING_SRC_TOUCH);
    APP_LOG(" 客厅灯(触摸): %s\r\n", state ? " 高档亮 " : " 已关闭 ");
}

void on_bedroom_clicked(uint8_t state)
{
    bedroom_external_set(state ? 1u : 0u, BEDROOM_SRC_TOUCH);
    APP_LOG(" 卧室灯(触摸): %s\r\n", state ? " 开 " : " 关 ");
}

void on_curtain_clicked(uint8_t state)
{
    if (state)
        curtain_on();
    else
        curtain_off();
    APP_LOG(" 窗帘(触摸): %s\r\n", state ? " 开 " : " 关 ");
}

/* ============================== 启动函数 ============================== */
void freertos_test(void)
{
    led_status = 0;
    led_living_off();
    voice_status_old = voice_value_get() & 3u;
    s_light_last_bt_seq = bt_cmd_seq_get();

    password_queue = xQueueCreate(10, sizeof(system_msg_t));
    display_queue  = xQueueCreate(10, sizeof(system_msg_t));
    beep_queue     = xQueueCreate(10, sizeof(system_msg_t));
    lock_queue     = xQueueCreate(10, sizeof(system_msg_t));

    w25q128_mutex     = xSemaphoreCreateMutex();
    system_event_group = xEventGroupCreate();

    auto_lock_timer = xTimerCreate(
        "auto_lock_timer", pdMS_TO_TICKS(1000), pdFALSE, NULL, auto_lock_timer_callback);

    xTaskCreate(lvgl_task, "lvgl", LVGL_TASK_STACK_WORDS, NULL, LVGL_TASK_PRIORITY, NULL);

    xTaskCreate(keyboard_task,   "keyboard_task",   256, NULL, 5, &keyboard_task_handle);
    xTaskCreate(password_task,   "password_task",   512, NULL, 5, &password_task_handle);
    xTaskCreate(display_task,    "display_task",    256, NULL, 5, &display_task_handle);
    xTaskCreate(lock_task,       "lock_task",       256, NULL, 4, &lock_task_handle);
    xTaskCreate(beep_task,       "beep_task",       256, NULL, 4, &beep_task_handle);
    xTaskCreate(monitor_task,    "monitor_task",    256, NULL, 4, &monitor_task_handle);

    xTaskCreate(light_control_task, "light_control_task", 256, NULL, 3, &light_control_task_handle);
    xTaskCreate(env_upload_task,     "env_upload_task",    512, NULL, 2, &env_upload_task_handle);

    xTaskCreate(mqtt_task, "mqtt_task", 2048, NULL, 3, &mqtt_task_handle);
}