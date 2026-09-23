#ifndef __VOICE_H__
#define __VOICE_H__
#include "sys.h"
#include <stdint.h>

void voice_init(void);
uint8_t voice_value_get(void);

/*
 * 将语音模块二线编码 raw(0~3) 映射为客厅灯档位 0关/1低/2中/3高。
 * 常见模块物理顺序与软件档位不一致（例如中亮=raw1、低亮=raw2），在此统一。
 */
uint8_t voice_living_level_from_raw(uint8_t raw);

/* OneNET 物模型 SoundModuleStatus 枚举 0/1/2（通信正常/失败/设备异常） */
uint8_t voice_onenet_sound_module_status(void);

#endif
