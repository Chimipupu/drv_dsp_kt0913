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
    REG_ADDR_SYSCFG  = 0x16,
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

typedef struct {
    float fm_rerq_Mhz;
    uint16_t set_reg_val;
    char *p_statio_str;
} fm_station_freq_t;

const fm_station_freq_t g_fm_station_freq_tbl[] = {
#if RADIO_AREA_JAPAN_TOKYO
    {80.0f, 0x8640, "TOKYO FM"},                           // TOKYO FM: 80.0MHz
    {81.3f, 0x865A, "J-WAVE"},                             // J-WAVE: 81.3MHz
    {82.5f, 0x8672, "NHK-FM-TOKYO"},                       // NHK FM東京: 82.5MHz
    {89.7f, 0x8702, "InterFM897"},                         // InterFM897: 89.7MHz
    {90.5f, 0x8712, "TBS Radio (Wide FM)"},                // TBSラジオ(ワイドFM): 90.5MHz
    {91.6f, 0x8728, "Nippon Cultural Broadcasting (Wide FM)"}, // 文化放送(ワイドFM): 91.6MHz
    {93.0f, 0x8744, "Nippon Broadcasting (Wide FM)"},      // ニッポン放送(ワイドFM): 93.0MHz
#else
    {77.0f, 0x8604, "e-radio"},                            // e-radio: 77.0MHz
    {80.2f, 0x8644, "FM802"},                              // FM802: 80.2MHz
    {85.1f, 0x86A6, "FM OSAKA"},                           // FM OSAKA: 85.1MHz
    {87.4f, 0x86D4, "NHK FM NARA"},                        // NHK FM奈良: 87.4MHz
    {88.1f, 0x86E2, "NHK FM OSAKA"},                       // NHK FM大阪: 88.1MHz
    {89.4f, 0x86FC, "alpha-station"},                      // α-STATION: 89.4MHz
    {89.9f, 0x8706, "Kiss FM KOBE"},                       // Kiss FM KOBE: 89.9MHz
    {90.6f, 0x8714, "MBS Radio (Wide FM)"},                // MBSラジオ(ワイドFM): 90.6MHz
    {91.9f, 0x872E, "OBC Radio Osaka (Wide FM)"},          // OBCラジオ大阪(ワイドFM): 91.9MHz
    {93.3f, 0x874A, "ABC Radio (Wide FM)"}                 // ABCラジオ(ワイドFM): 93.3MHz
#endif
};
const uint8_t FM_STATION_FREQ_TBL_SIZE = sizeof(g_fm_station_freq_tbl) / sizeof(g_fm_station_freq_tbl[0]);

static void _set_reg(uint8_t reg_addr, uint16_t reg_val);
static uint16_t _get_reg(uint8_t reg_addr);
// -----------------------------------------------------------
// [Static]

static void _set_reg(uint8_t reg_addr, uint16_t reg_val)
{
    // TODO: 実装
}

static uint16_t _get_reg(uint8_t reg_addr)
{
    // TODO: 実装
}
// -----------------------------------------------------------
// [API]

void drv_kt0913_init(kt0913_config_t *p_config)
{
    uint16_t reg_val;

    // 1) STATUSAレジスタ(Addr:0x12)をRead
    reg_val = _get_reg(REG_ADDR_STATUSA);

    // 2) bit15のXTAL_OKが1(Ready)になるまで待つ
    // TODO: 実装

    // VOLUMEレジスタ(Addr:0x04)のbit13のDMUTEを1(Mute disable) に設定
    // TODO: 実装
}

void drv_kt0913_mute_onoff(bool is_mute)
{
    // TODO: 実装
}

bool drv_kt0913_set_fm_freq(float freq_Mhz)
{
    bool is_success;
    uint8_t i;
    uint8_t tbl_size;

    is_success = false;

    // 指定のFM周波数をテーブルで検索
    for(i = 0; i < FM_STATION_FREQ_TBL_SIZE; i++)
    {
        if (g_fm_station_freq_tbl[i].fm_rerq_Mhz == freq_Mhz)
        {
            // TUNEレジスタ(Addr:0x03)にFM周波数を設定
            _set_reg(REG_ADDR_TUNE, g_fm_station_freq_tbl[i].set_reg_val);
            is_success = true;
            break;
        }
    }

    return is_success;
}