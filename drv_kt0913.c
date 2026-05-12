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
// [マクロ]
// KT0913 TUNEレジスタ(0x03)設定値計算マクロ
#define CALC_FM_FREQ_REG_VAL(freq_mhz)    ((uint16_t)(0x8000 | (((uint32_t)((freq_mhz) * 20.0f)) & 0x0FFF)))
// #define CALC_FM_FREQ_REG_VAL(freq_mhz)    ((uint16_t)(0x8000 | (((uint32_t)((freq_mhz) * 20.0f + 0.5f)) & 0x0FFF)))

// -----------------------------------------------------------
// [レジスタテーブル]
const uint8_t g_kt0913_reg_addr_tbl[] = {
    REG_ADDR_CHIP_ID,
    REG_ADDR_SEEK,
    REG_ADDR_TUNE,
    REG_ADDR_VOLUME,
    REG_ADDR_DSPCFGGA,
    REG_ADDR_LOCFGA,
    REG_ADDR_LOCFGC,
    REG_ADDR_RXCFG,
    REG_ADDR_STATUSA,
    REG_ADDR_STATUSB,
    REG_ADDR_STATUSC,
    REG_ADDR_AMSYSCFG,
    REG_ADDR_AMCHAN,
    REG_ADDR_AMCALI,
    REG_ADDR_GPIO,
    REG_ADDR_AMDSP,
    REG_ADDR_AMSTATUSA,
    REG_ADDR_AMSTATUSB,
    REG_ADDR_SOFTMUTE,
    REG_ADDR_USERSTARTCH,
    REG_ADDR_USERGUARD,
    REG_ADDR_USERCHANNUM,
    REG_ADDR_AMCFG,
    REG_ADDR_AMCFG2,
    REG_ADDR_VOLGUARD,
    REG_ADDR_AFC,
};
const uint8_t KT0913_REG_TBL_SIZE = sizeof(g_kt0913_reg_addr_tbl) / sizeof(g_kt0913_reg_addr_tbl[0]);

// [FMラジオ局テーブル]
// NOTE: 周波数は総務省より引用 (https://www.soumu.go.jp/menu_seisaku/ictseisaku/housou_suishin/fm-list.html)
const fm_station_freq_t g_fm_station_freq_tbl[] = {
#ifdef RADIO_AREA_TOKYO
    // [東京エリア]
    {80.0f, CALC_FM_FREQ_REG_VAL(80.0f), "FM東京"},
    {81.3f, CALC_FM_FREQ_REG_VAL(81.3f), "J-WAVE"},
    {82.5f, CALC_FM_FREQ_REG_VAL(82.5f), "NHK FM東京"},
    {89.7f, CALC_FM_FREQ_REG_VAL(89.7f), "InterFM897"},
    {90.5f, CALC_FM_FREQ_REG_VAL(90.5f), "TBSラジオ"},
    {91.6f, CALC_FM_FREQ_REG_VAL(91.6f), "文化放送"},
    {93.0f, CALC_FM_FREQ_REG_VAL(93.0f), "ニッポン放送"},
#else
    // [大阪エリア]
    // NOTE: U8g2のフォントで大阪の'阪'が非対応なので'坂'で対処
    {76.5f, CALC_FM_FREQ_REG_VAL(76.5f), "FM COCOLO"},
    {80.2f, CALC_FM_FREQ_REG_VAL(80.2f), "FM802"},
    {85.1f, CALC_FM_FREQ_REG_VAL(85.1f), "FM 大坂"},
    {88.1f, CALC_FM_FREQ_REG_VAL(88.1f), "NHK FM OSAKA"},
    {89.4f, CALC_FM_FREQ_REG_VAL(89.4f), "a-STATION"},
    {89.9f, CALC_FM_FREQ_REG_VAL(89.9f), "Kiss FM KOBE"},
    {90.6f, CALC_FM_FREQ_REG_VAL(90.6f), "MBSラジオ"},
    {91.9f, CALC_FM_FREQ_REG_VAL(91.9f), "ラジオ大坂OBC"},
    {93.3f, CALC_FM_FREQ_REG_VAL(93.3f), "ABCラジオ"},
    {91.1f, CALC_FM_FREQ_REG_VAL(91.1f), "ラジオ関西"},
#endif
};
const uint8_t FM_STATION_FREQ_TBL_SIZE = sizeof(g_fm_station_freq_tbl) / sizeof(g_fm_station_freq_tbl[0]);

static kt0913_config_t s_drv_cfg;
static kt0913_volume_ctrl_t s_vol_ctrl;

static bool _reg_addr_check(uint8_t reg_addr);
static void _set_reg(uint8_t reg_addr, uint16_t reg_val);
static uint16_t _get_reg(uint8_t reg_addr);
static bool _check_stc_reg(void);
// -----------------------------------------------------------
// [Static]

static bool _reg_addr_check(uint8_t reg_addr)
{
    uint8_t i;

    for(i = 0; i < KT0913_REG_TBL_SIZE; i++)
    {
        if(g_kt0913_reg_addr_tbl[i] == reg_addr) {
            return true;
        }
    }

    return false;
}

static void _set_reg(uint8_t reg_addr, uint16_t reg_val)
{
    if(_reg_addr_check(reg_addr)) {
        s_drv_cfg.p_i2c_write(reg_addr, reg_val);
    }
}

static uint16_t _get_reg(uint8_t reg_addr)
{
    uint16_t reg_val = 0xFFFF;

    if(_reg_addr_check(reg_addr)) {
        reg_val = s_drv_cfg.p_i2c_read(reg_addr);
    }

    return reg_val;
}

static bool _check_stc_reg(void)
{
    bool ret = false;
    uint16_t reg_val;

    reg_val = _get_reg(REG_ADDR_STATUSA);

    // Bit14のSTCをチェック(TUNE or SEEKの完了確認)
    if((reg_val & 0x4000) != 0) {
        ret = true;
    }

    return ret;
}

// -----------------------------------------------------------
// [API]

void drv_kt0913_set_reg(uint8_t reg_addr, uint16_t reg_val)
{
    _set_reg(reg_addr, reg_val);
}

uint16_t drv_kt0913_get_reg(uint8_t reg_addr)
{
    return _get_reg(reg_addr);
}

/**
 * @brief FM有効化
 * @note データシートの「3.8.1. Programmable band」でFMを有効化
 */
void drv_kt0913_fm_mode(void)
{
    uint16_t reg_val;

#if 1
    // 0. LOCFGAレジスタ(Addr:0x0A)でAFCを有効化
    reg_val = _get_reg(REG_ADDR_LOCFGA);
    reg_val &= ~0x0100;  // bit8のFMAFCを0に設定(= AFC有効)
    _set_reg(REG_ADDR_LOCFGA, reg_val);
#endif

    // 1. LOCFGCレジスタ(Addr:0x0C)でキャンパスバンドを有効化
    reg_val = _get_reg(REG_ADDR_LOCFGC);
    reg_val |= 0x0008; // bit3のCAMPUSBAND_ENを1に設定(= キャンパスバンド有効)
    _set_reg(REG_ADDR_LOCFGC, reg_val);

#if 0
    // 2. AMSYSCFGレジスタ(Addr:0x16)のAM/FM選択とUSERBAND設定
    reg_val = _get_reg(REG_ADDR_AMSYSCFG);
    reg_val &= ~0x8000; // bit15(AM_FM)を0にクリアしてFMモードに設定
    reg_val |= 0x4000;  // bit14(USERBAND)を1にセットしてカスタム帯域を有効化
    _set_reg(REG_ADDR_AMSYSCFG, reg_val);

    // 3. SEEKレジスタ(Addr:0x02)でFMチャンネルステップを設定
    reg_val = _get_reg(REG_ADDR_SEEK);
    reg_val &= ~0x000C; // bit[3:2]をクリア
    reg_val |= 0x0004;  // bit[3:2]を01 (100KHzステップ)に設定
    _set_reg(REG_ADDR_SEEK, reg_val);

    // 4. USERSTARTCHレジスタ(Addr:0x2F)にFMの開始周波数を設定
    reg_val = _get_reg(REG_ADDR_USERSTARTCH);
    reg_val &= ~0x7FFF; // bit[14:0]をクリア
    reg_val |= 0x05F0;  // 76.0MHz = 1520 * 50kHz (1520 = 0x5F0)
    _set_reg(REG_ADDR_USERSTARTCH, reg_val);

    // 5. USERCHANNUMレジスタ(Addr:0x31)にチャンネル数を設定
    reg_val = _get_reg(REG_ADDR_USERCHANNUM);
    reg_val &= ~0x0FFF; // bit[11:0]をクリア
    // チャンネル数 = 340チャンネル @76.0MHz ~ 110.0MHz @100KHzステップ
    reg_val |= 0x0154;
    _set_reg(REG_ADDR_USERCHANNUM, reg_val);
#endif
}

void drv_kt0913_init(kt0913_config_t *p_config)
{
    uint16_t reg_val;

    // 引数チェック
    if((p_config == NULL) || (p_config->p_i2c_write == NULL || p_config->p_i2c_read == NULL)) {
        return;
    }

    s_drv_cfg = *p_config;

    // [IC内蔵の水晶振動子の安定待ち]
    // STATUSAレジスタ(Addr:0x12)をRead
    reg_val = _get_reg(REG_ADDR_STATUSA);
    // bit15のXTAL_OKが1(Ready)になるまで待つ
    while((reg_val & 0x8000) == 0)
    {
        reg_val = _get_reg(REG_ADDR_STATUSA);
    }

#if 0
    // [外付けの水晶振動子に変更]
    // AMSYSCFGレジスタ(Addr:0x16)のBit12 RCLK_ENをセット
    reg_val = _get_reg(REG_ADDR_AMSYSCFG);
    reg_val |= 0x1000; // Bit12(RCLK_EN)を1にセットして外付けの水晶振動子に変更
    reg_val &= ~0x0F00; // Bit[11:8]を0にして外付けの水晶振動子の32.768kHzを指定
    _set_reg(REG_ADDR_AMSYSCFG, reg_val);
#endif

    // [FMモードの有効化]
    drv_kt0913_fm_mode();

    // [スピーカーの物理ミュートを無効化]
    reg_val = _get_reg(REG_ADDR_VOLUME);
    reg_val |= 0x2000; // bit13(DMUTE)を1にセット
    _set_reg(REG_ADDR_VOLUME, reg_val);

    // [初期音量調整]
    s_vol_ctrl.volume_dB = 15;
    drv_kt0913_volume_ctrl(&s_vol_ctrl);

#if 1
    // [FM SoftMute]
    // NOTE: ノイズ抑制でFMのソフトミュートを有効
    drv_kt0913_softmute_onoff(true);
#endif
}

void drv_kt0913_softmute_onoff(bool is_mute)
{
    uint16_t reg_val;

    // VOLUMEレジスタ(Addr:0x04)のbit15を設定
    reg_val = _get_reg(REG_ADDR_VOLUME);
    if(is_mute) {
        reg_val &= ~0x8000; // bit15(SOFTMUTE_EN)を0にクリアしてFMをソフトミュート
    } else {
        reg_val |= 0x8000; // bit15(SOFTMUTE_EN)を1にセットしてFMをソフトミュートを無効
    }
    _set_reg(REG_ADDR_SOFTMUTE, reg_val);
}

void drv_kt0913_volume_ctrl(kt0913_volume_ctrl_t *p_volume_ctrl)
{
    uint16_t reg_val;

    if(p_volume_ctrl == NULL) {
        return;
    }

    s_vol_ctrl = *p_volume_ctrl;

    // Volumeは5bitの32段階 @RXCFGレジスタ(Addr:0x0F)のbit4:0の5ビットで制御
    reg_val = _get_reg(REG_ADDR_RXCFG);
    reg_val &= ~0x001F; // bit4:0をクリア
    reg_val |= (p_volume_ctrl->volume_dB & 0x1F);
    _set_reg(REG_ADDR_RXCFG, reg_val);

#if 0
    // ゲイン設定 @AMSYSCFGレジスタ(Addr:0x16)のbit[7:6]で制御
    reg_val = _get_reg(REG_ADDR_AMSYSCFG);
    reg_val &= ~0xC000; // bit[7:6]をクリア
    reg_val |= ((p_volume_ctrl->audio_gain & 0x03) << 14); // ゲイン値を設定
    _set_reg(REG_ADDR_AMSYSCFG, reg_val);
#endif

    // Bass Boostの設定 @Volumeレジスタ(Addr:0x04)のbit[9:8]で制御
    reg_val = _get_reg(REG_ADDR_VOLUME);
    if(p_volume_ctrl->is_bass_boost) {
        reg_val |= 0x0300; // bit[9:8]をセットしてBass Boost
    } else {
        reg_val &= ~0x0300; // bit[9:8]を0にクリアしてBass Boost OFF
    }
    _set_reg(REG_ADDR_VOLUME, reg_val);
}

uint8_t drv_kt0913_get_volume_val(void)
{
    uint8_t vol_reg_val;
    uint16_t reg_val;

    // RXCFGレジスタ(Addr:0x0F)にある音量のBit[4:0]の5bitを返す
    reg_val = _get_reg(REG_ADDR_RXCFG);
    vol_reg_val = (uint8_t)(reg_val & 0x1F);

    return vol_reg_val;
}

bool drv_kt0913_set_fm_freq(uint8_t station)
{
    uint16_t reg_val;
    bool is_stc;

    // 引数チェック
    if(station > FM_STATION_RADIO_KANSAI_WIDEFM) {
        return false;
    }

    reg_val = g_fm_station_freq_tbl[station].set_reg_val;

    // [TUNEレジスタ(Addr:0x03)にFM周波数を設定]
    // FMTUNEビット(bit15)は0にして周波数を設定
    _set_reg(REG_ADDR_TUNE, reg_val & ~0x8000);
    // FMTUNEビットを1にして指定の周波数にTUNE開始
    _set_reg(REG_ADDR_TUNE, reg_val | 0x8000);
    // STCビットでTUNEの完了待ち
    is_stc = false;
    while (is_stc = false)
    {
        is_stc = _check_stc_reg();
    }

    drv_kt0913_volume_ctrl(&s_vol_ctrl);

    return true;
}

int8_t drv_kt0913_get_fm_rssi(void)
{
    int8_t rssi;
    uint8_t rssi_reg_val;
    uint16_t reg_val;

    // STATUSAレジスタ(Addr:0x12)のBit[7:3]のFMRSSIビット
    reg_val = _get_reg(REG_ADDR_STATUSA);
    rssi_reg_val = (uint8_t)((reg_val >> 3) & 0x1F);

    // データシートの計算式: RSSI(dBm) = -100 + (RSSI_REG_VAL * 3dB)
    rssi = -100 + (rssi_reg_val * 3);

    return rssi;
}