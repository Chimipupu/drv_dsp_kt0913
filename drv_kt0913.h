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

// -----------------------------------------------------------
// [Define]
#define KT0913_CHIP_ID             0x4B54 // NOTE: ChipIDの0x4B54はASCIIで"KT"

// FM周波数
#define KT0913_FM_FREQ_MHZ_MIN     32.0f
#define KT0913_FM_FREQ_MHZ_MAX     110.0f

// 受信地域
#define RADIO_AREA_TOKYO       0 // 受信地域: 東京
#define RADIO_AREA_OSAKA       1 // 受信地域: 大阪

// -----------------------------------------------------------

typedef void (*i2c_write_func_t)(uint8_t, uint16_t); // I2Cのwrite関数ポインタ
typedef uint16_t (*i2c_read_func_t)(uint8_t);        // I2Cのread関数ポインタ

typedef struct {
    uint8_t radio_area; // 0: 東京, 1: 大阪

    // [呼び元のI2CのRead/Write関数ポインタ]
    i2c_write_func_t p_i2c_write;
    i2c_read_func_t p_i2c_read;
} kt0913_config_t;

// -----------------------------------------------------------
void drv_kt0913_init(kt0913_config_t *p_config);
void drv_kt0913_softmute_onoff(bool is_mute);
bool drv_kt0913_set_fm_freq(float freq_Mhz);

#endif // DRV_KT0913_H