/**
 * @file drv_kt0913.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief DSPラジオIC KT0913 ドライバ
 * @version 0.1
 * @date 2026-04-29
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_KT0913_H
#define DRV_KT0913_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// -----------------------------------------------------------
// [コンパイルスイッチ]
// #define RADIO_AREA_JAPAN_TOKYO
#define RADIO_AREA_JAPAN_OSAKA
// -----------------------------------------------------------
// [Define]
#define KT0913_CHIP_ID      0x4B54 // NOTE: ChipIDの0x4B54はASCIIで"KT"

// -----------------------------------------------------------

typedef struct {
    uint8_t amfm_mode; // 0: FM, 1: AM
    // TODO
} kt0913_config_t;

// -----------------------------------------------------------

void drv_kt0913_init(kt0913_config_t *p_config);
void drv_kt0913_mute_onoff(bool is_mute);
bool drv_kt0913_set_fm_freq(float freq_Mhz);

#endif // DRV_KT0913_H