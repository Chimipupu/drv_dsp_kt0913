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
// #define DEBUG_TEST_KT0913
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
typedef enum {
    // 東京エリア
    FM_STATION_FM_TOKYO = 0,        // FM東京: 80.0MHz
    FM_STATION_JWAVE,               // J-WAVE: 81.3MHz
    FM_STATION_NHK_FM_TOKYO,        // NHK FM東京: 82.5MHz
    FM_STATION_INTERFM897,          // InterFM897: 89.7MHz
    FM_STATION_TBS_WIDEFM,          // TBSラジオ(ワイドFM): 90.5MHz
    FM_STATION_BUNKA_WIDEFM,        // 文化放送(ワイドFM): 91.6MHz
    FM_STATION_NIPPON_WIDEFM,       // ニッポン放送(ワイドFM): 93.0MHz

    // 大阪エリア
    FM_STATION_FM_COCOLO,           // FM COCOLO: 76.5MHz
    FM_STATION_FM802,               // FM802: 80.2MHz
    FM_STATION_FM_OSAKA,            // FM大阪: 85.1MHz
    FM_STATION_NHK_FM_OSAKA,        // NHK FM大阪: 88.1MHz
    FM_STATION_ALPHA_STATION,       // α-STATION(京都): 89.4MHz
    FM_STATION_KISS_FM_KOBE,        // Kiss FM KOBE（神戸）: 89.9MHz
    FM_STATION_MBS_WIDEFM,          // MBSラジオ(ワイドFM): 90.6MHz
    FM_STATION_OBC_WIDEFM,          // ラジオ大阪OBC(ワイドFM): 91.9MHz
    FM_STATION_ABC_WIDEFM,          // ABCラジオ(ワイドFM): 93.3MHz
    FM_STATION_RADIO_KANSAI_WIDEFM, // ラジオ関西(ワイドFM): 91.1MHz
} E_FM_STATION;

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
bool drv_kt0913_set_fm_freq(E_FM_STATION station);

#endif // DRV_KT0913_H