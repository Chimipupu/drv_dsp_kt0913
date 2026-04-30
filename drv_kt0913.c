/**
 * @file drv_kt0913.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief DSPラジオIC KT0913 ドライバ
 * @version 0.1
 * @date 2026-04-29
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_kt0913.h"

// -----------------------------------------------------------
// KT0913レジスタ
typedef enum {
    REG_ADDR_CHIP_ID = 0x01,
    REG_ADDR_SEEK = 0x02,
    REG_ADDR_TUNE = 0x03,
    REG_ADDR_VOLUME = 0x04,
    REG_ADDR_DSPCFGGA = 0x05,
    REG_ADDR_LOCFGA = 0x0A,
    REG_ADDR_LOCFGC = 0x0C,
    REG_ADDR_RXCFG  = 0x0F,
    REG_ADDR_STATUSA = 0x12,
    REG_ADDR_STATUSB = 0x13,
    REG_ADDR_STATUSC = 0x14,
    REG_ADDR_AMSYSCFG = 0x16,
    REG_ADDR_AMCHAN  = 0x17,
    REG_ADDR_AMCALI  = 0x18,
    REG_ADDR_GPIO  = 0x1D,
    REG_ADDR_AMDSP = 0x22,
    REG_ADDR_AMSTATUSA = 0x24,
    REG_ADDR_AMSTATUSB = 0x25,
    REG_ADDR_SOFTMUTE = 0x2E,
    REG_ADDR_USERSTARTCH = 0x2F,
    REG_ADDR_USERSTARTNUM = 0x30,
    REG_ADDR_USERCHANNUM = 0x31,
    REG_ADDR_AMCFG = 0x33,
    REG_ADDR_AMCFG2 = 0x34,
    REG_ADDR_VOLGUARD = 0x3A,
    REG_ADDR_AFC = 0x3C,
} KT0913_REG_ADDR;

// FMラジオ局構造体
typedef struct {
    float fm_rerq_Mhz;       // FM周波数(MHz)
    uint16_t set_reg_val;    // KT0913のTUNEレジスタ(Addr:0x03)に設定値
} fm_station_freq_t;

// KT0913 TUNEレジスタ(0x03)設定値計算マクロ
#define CALC_FM_FREQ_REG_VAL(freq_mhz)    ((uint16_t)(0x8000 | (((uint32_t)((freq_mhz) * 20.0f)) & 0x0FFF)))

// [FMラジオ局テーブル]
// NOTE: 周波数は総務省より引用 (https://www.soumu.go.jp/menu_seisaku/ictseisaku/housou_suishin/fm-list.html)
const fm_station_freq_t g_fm_station_freq_tbl[] = {
    // 東京エリア
    {80.0f, CALC_FM_FREQ_REG_VAL(80.0f)}, // FM東京:             80.0MHz
    {81.3f, CALC_FM_FREQ_REG_VAL(81.3f)}, // J-WAVE:             81.3MHz
    {82.5f, CALC_FM_FREQ_REG_VAL(82.5f)}, // NHK FM東京:         82.5MHz
    {89.7f, CALC_FM_FREQ_REG_VAL(89.7f)}, // InterFM897:         89.7MHz
    {90.5f, CALC_FM_FREQ_REG_VAL(90.5f)}, // TBSラジオ(ワイドFM): 90.5MHz
    {91.6f, CALC_FM_FREQ_REG_VAL(91.6f)}, // 文化放送(ワイドFM):  91.6MHz
    {93.0f, CALC_FM_FREQ_REG_VAL(93.0f)}, // ニッポン放送(ワイドFM): 93.0MHz

    // 大阪エリア
    {76.5f, CALC_FM_FREQ_REG_VAL(76.5f)}, // FM COCOLO:            76.5MHz
    {80.2f, CALC_FM_FREQ_REG_VAL(80.2f)}, // FM802:                80.2MHz
    {85.1f, CALC_FM_FREQ_REG_VAL(85.1f)}, // FM大阪:                85.1MHz
    {88.1f, CALC_FM_FREQ_REG_VAL(88.1f)}, // NHK FM大阪:            88.1MHz
    {89.4f, CALC_FM_FREQ_REG_VAL(89.4f)}, // α-STATION(京都):       89.4MHz
    {89.9f, CALC_FM_FREQ_REG_VAL(89.9f)}, // Kiss FM KOBE（神戸）:  89.9MHz
    {90.6f, CALC_FM_FREQ_REG_VAL(90.6f)}, // MBSラジオ(ワイドFM):    90.6MHz
    {91.9f, CALC_FM_FREQ_REG_VAL(91.9f)}, // ラジオ大阪OBC(ワイドFM): 91.9MHz
    {93.3f, CALC_FM_FREQ_REG_VAL(93.3f)}, // ABCラジオ(ワイドFM):     93.3MHz
    {91.1f, CALC_FM_FREQ_REG_VAL(91.1f)}, // ラジオ関西(ワイドFM):    91.1MHz
};
const uint8_t FM_STATION_FREQ_TBL_SIZE = sizeof(g_fm_station_freq_tbl) / sizeof(g_fm_station_freq_tbl[0]);

static kt0913_config_t *s_p_config;
static void _set_reg(uint8_t reg_addr, uint16_t reg_val);
static uint16_t _get_reg(uint8_t reg_addr);
// -----------------------------------------------------------
// [Static]

static void _set_reg(uint8_t reg_addr, uint16_t reg_val)
{
    if(reg_addr <= 0x3C && reg_addr >= 0x01) {
        s_p_config->p_i2c_write(reg_addr, reg_val);
    }
}

static uint16_t _get_reg(uint8_t reg_addr)
{
    uint16_t reg_val = 0xFFFF;

    if(reg_addr <= 0x3C && reg_addr >= 0x01) {
        reg_val = s_p_config->p_i2c_read(reg_addr);
    }

    return reg_val;
}
// -----------------------------------------------------------
// [API]

void drv_kt0913_init(kt0913_config_t *p_config)
{
    uint16_t reg_val;

    // 引数チェック
    if((p_config == NULL) || (p_config->p_i2c_write == NULL || p_config->p_i2c_read == NULL))
    {
        return;
    }

    s_p_config->radio_area = p_config->radio_area;
    s_p_config->p_i2c_write = p_config->p_i2c_write;
    s_p_config->p_i2c_read = p_config->p_i2c_read;

    // [水晶振動子の安定待ち]
    // STATUSAレジスタ(Addr:0x12)をRead
    reg_val = _get_reg(REG_ADDR_STATUSA);
    // bit15のXTAL_OKが1(Ready)になるまで待つ
    while((reg_val & 0x8000) == 0)
    {
        reg_val = _get_reg(REG_ADDR_STATUSA);
    }

    // [スピーカーの物理ミュートを無効化]
    reg_val = _get_reg(REG_ADDR_VOLUME);
    reg_val |= 0x2000; // bit13(DMUTE)を1にセット
    _set_reg(REG_ADDR_VOLUME, reg_val);

    // [ゲインを最大の6dBに設定]
    reg_val = _get_reg(REG_ADDR_AMSYSCFG);
    reg_val &= ~0xC000; // bit[7:6]をクリア
    reg_val |= (AUDIO_GAIN_6DB << 14); // ゲイン値を設定
    _set_reg(REG_ADDR_AMSYSCFG, reg_val);

    // [FMの初期値]
    if(p_config->radio_area == RADIO_AREA_TOKYO) {
        drv_kt0913_set_fm_freq(80.0f); // FM東京: 80.0MHz
    } else {
        drv_kt0913_set_fm_freq(85.1f); // FM大阪: 85.1MHz
    }
}

void drv_kt0913_softmute_onoff(bool is_mute)
{
    uint16_t reg_val;

    // SOFTMUTEレジスタ(Addr:0x2E)のbit15のSOFTMUTE_ENを設定
    reg_val = _get_reg(REG_ADDR_SOFTMUTE);
    if(is_mute) {
        reg_val |= 0x8000; // bit15(SOFTMUTE_EN)を1にセット
    } else {
        reg_val &= ~0x8000; // bit15(SOFTMUTE_EN)を0にクリア
    }
    _set_reg(REG_ADDR_SOFTMUTE, reg_val);
}
void drv_kt0913_volume_ctrl(kt0913_volume_ctrl_t *p_volume_ctrl)
{
    uint16_t reg_val;

    if(p_volume_ctrl == NULL) {
        return;
    }

    // Volumeは5bitの32段階 @RXCFGレジスタ(Addr:0x0F)のbit4:0の5ビットで制御
    reg_val = (p_volume_ctrl->volume_dB & 0x1F) / 100; // 0 ~ 0x1Fにスケーリング
    _set_reg(REG_ADDR_RXCFG, reg_val);

    // ゲイン設定 @AMSYSCFGレジスタ(Addr:0x16)のbit[7:6]で制御
    reg_val = _get_reg(REG_ADDR_AMSYSCFG);
    reg_val &= ~0xC000; // bit[7:6]をクリア
    reg_val |= ((p_volume_ctrl->audio_gain & 0x03) << 14); // ゲイン値を設定
    _set_reg(REG_ADDR_AMSYSCFG, reg_val);

    // ベースブーストの設定 @Volumeレジスタ(Addr:0x04)のbit[9:8]で制御
    reg_val = _get_reg(REG_ADDR_VOLUME);
    if(p_volume_ctrl->is_bass_boost) {
        reg_val |= 0x0300; // bit[9:8]をセットしてベースブーストON
    } else {
        reg_val &= ~0x0300; // bit[9:8]を0にクリアしてベースブーストOFF
    }
    _set_reg(REG_ADDR_VOLUME, reg_val);
}

bool drv_kt0913_set_fm_freq(E_FM_STATION station)
{
    // 引数チェック
    if(station > FM_STATION_RADIO_KANSAI_WIDEFM) {
        return false;
    }

    // TUNEレジスタ(Addr:0x03)にFM周波数を設定
    _set_reg(REG_ADDR_TUNE, g_fm_station_freq_tbl[station].set_reg_val);

    return true;
}

#ifdef DEBUG_TEST_KT0913
#include <stdio.h>
int main(void)
{
    uint16_t i;

    // FMラジオ局テーブルの周波数とレジスタ設定値を表示
    printf("FMラジオ局テーブルサイズ: %d Byte\n", sizeof(g_fm_station_freq_tbl));
    for(i = 0; i < FM_STATION_FREQ_TBL_SIZE; i++)
    {
        printf("FM周波数: %.1fMHz, レジスタ設定値: 0x%04X\n", g_fm_station_freq_tbl[i].fm_rerq_Mhz, g_fm_station_freq_tbl[i].set_reg_val);
    }

    return 0;
}
#endif // DEBUG_TEST_KT0913