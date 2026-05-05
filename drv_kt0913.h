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

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------
// [コンパイルスイッチ]
// #define DBG_TEST_KT0913 // 単体テスト

// -----------------------------------------------------------
// [マクロ]
// KT0913 TUNEレジスタ(0x03)設定値計算マクロ
#define CALC_FM_FREQ_REG_VAL(freq_mhz)    ((uint16_t)(0x8000 | (((uint32_t)((freq_mhz) * 20.0f)) & 0x0FFF)))

// -----------------------------------------------------------
// [Define]
#define I2C_ADDR_KT0913            0x35

#define KT0913_CHIP_ID             0x4B54 // NOTE: ChipIDの0x4B54はASCIIで"KT"

// FM周波数
#define KT0913_FM_FREQ_MHZ_MIN     32.0f
#define KT0913_FM_FREQ_MHZ_MAX     110.0f

// 受信地域
// #define RADIO_AREA_TOKYO           0 // 受信地域: 東京
#define RADIO_AREA_OSAKA           1 // 受信地域: 大阪

// -----------------------------------------------------------
// FMラジオ局構造体
typedef struct {
    float fm_rerq_Mhz;    // FM周波数(MHz)
    uint16_t set_reg_val; // KT0913のTUNEレジスタ(Addr:0x03)に設定値
    char *p_str;          // FMラジオ局名
} fm_station_freq_t;
extern const fm_station_freq_t g_fm_station_freq_tbl[];
extern const uint8_t FM_STATION_FREQ_TBL_SIZE;

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

// KT0913ドライバ初期化構造体
typedef struct {
    // uint8_t radio_area; // 0: 東京, 1: 大阪
    // bool is_stereo;    // true: ステレオ, false: モノラル

    // [呼び元のI2CのRead/Write関数ポインタ]
    i2c_write_func_t p_i2c_write;
    i2c_read_func_t p_i2c_read;
} kt0913_config_t;

typedef enum {
    AUDIO_GAIN_3DB       = 0, // 3dB  (AMSYSCFGレジスタのAU_GAIN bit[7:6]=0b00 ※デフォルト値)
    AUDIO_GAIN_6DB       = 1, // 6dB  (AMSYSCFGレジスタのAU_GAIN bit[7:6]=0b01)
    AUDIO_GAIN_MINUS_3DB = 2, // -3dB (AMSYSCFGレジスタのAU_GAIN bit[7:6]=0b10)
    AUDIO_GAIN_0DB       = 3, // 0dB  (AMSYSCFGレジスタのAU_GAIN bit[7:6]=0b11)
} E_AUDIO_GAIN;

// KT0913ボリューム制御構造体
typedef struct {
    bool is_bass_boost; // ベースブーストの有無
    uint8_t volume_dB; // 32段階のボリュームレベル
    E_AUDIO_GAIN audio_gain; // オーディオゲイン (-3dB,0dB,3dB,6dBの4段階)
} kt0913_volume_ctrl_t;

// -----------------------------------------------------------
void drv_kt0913_init(kt0913_config_t *p_config);
void drv_kt0913_softmute_onoff(bool is_mute);
void drv_kt0913_volume_ctrl(kt0913_volume_ctrl_t *p_volume_ctrl);
bool drv_kt0913_set_fm_freq(uint8_t station);

#ifdef __cplusplus
}
#endif

#endif // DRV_KT0913_H