#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * 应用级可调参数与调试开关（与密钥无关项集中在此，便于竞赛材料脱敏与裁剪日志）
 */

/* 1=通过 UART 输出 APP_LOG；量产或答辩静音可改为 0 */
#ifndef APP_LOG_ENABLE
#define APP_LOG_ENABLE 1
#endif

#if APP_LOG_ENABLE
#include <stdio.h>
#define APP_LOG(...) printf(__VA_ARGS__)
#else
#define APP_LOG(...) ((void)0)
#endif

/* OneNET 属性上报 JSON 中的业务 id 字段（与平台规则一致时仅改此处） */
#ifndef APP_ONENET_JSON_MSG_ID
#define APP_ONENET_JSON_MSG_ID "18578662932"
#endif

#endif
