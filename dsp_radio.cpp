/**
 * @file dsp_radio.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief DSPラジオアプリ
 * @version 0.1
 * @date 2026-05-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "dsp_radio.h"

// Arduino IDE
#include <U8g2lib.h>
#include <Wire.h>

// KT0913ドライバ
#include "drv_kt0913.h"

// -----------------------------------------------------------
#define I2C_ADDR_OLED      0x3C

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C g_lcd(U8G2_R0, U8X8_PIN_NONE);
kt0913_config_t g_kt0913_cfg;

static kt0913_volume_ctrl_t s_vol_ctrl;
static uint8_t s_fm_freq_tbl_idx = 0;
static float s_fm_freq = 76.5f; // 初期周波数
static int8_t s_fm_rssi = 0; // RSSI値

static void _i2c_write(uint8_t reg_addr, uint16_t reg_val);
static uint16_t _i2c_read(uint8_t reg_addr);
static void _ui_draw_fm_freq(float freq_val, char *p_str);

#ifdef DEBUG_DSP_RADIO
static void _dbg_get_all_reg(void);
#endif // DEBUG_DSP_RADIO
// -----------------------------------------------------------
// [Static]

#ifdef DEBUG_DSP_RADIO
static void _dbg_get_all_reg(void)
{
    uint8_t i;
    uint16_t reg_val;

    Serial.println("[DEBUG] DSP(KT0913) All Register Read Dump:");
    for(i = 0; i < KT0913_REG_TBL_SIZE; i++)
    {
        reg_val = drv_kt0913_get_reg(g_kt0913_reg_addr_tbl[i]);
        Serial.printf("[DEBUG] Reg[0x%02X]: 0x%04X\r\n", g_kt0913_reg_addr_tbl[i], reg_val);
    }
}
#endif // DEBUG_DSP_RADIO

static void _i2c_write(uint8_t reg_addr, uint16_t reg_val)
{
    Wire.beginTransmission(I2C_ADDR_KT0913);
    Wire.write(reg_addr);
    Wire.write((reg_val >> 8) & 0xFF); // 上位バイト
    Wire.write(reg_val & 0xFF);        // 下位バイト
    Wire.endTransmission();
}

static uint16_t _i2c_read(uint8_t reg_addr)
{
    uint16_t reg_val = 0xFFFF;

    Wire.beginTransmission(I2C_ADDR_KT0913);
    Wire.write(reg_addr);
    Wire.endTransmission(false); // リピートスタート

    if(Wire.requestFrom(I2C_ADDR_KT0913, 2) == 2) {
        reg_val = (Wire.read() << 8); // 上位バイト
        reg_val |= Wire.read();       // 下位バイト
    }

    return reg_val;
}

static void _lcd_init(void)
{
    g_lcd.setI2CAddress(I2C_ADDR_OLED << 1);
    g_lcd.begin();
    g_lcd.enableUTF8Print();
}

/**
 * @brief UIにFM関連情報を表示
 * @note LCDサイズ: 0.91インチの128x32
 * @param freq_val FM周波数(MHz)
 * @param p_str FMラジオ局の日本語文字列ポインタ
 */
static void _ui_draw_fm_freq(float freq_val, char *p_str)
{
    char buf[128];

    /**
     * @brief LCDの表示例
     * ラジオ局: FM大阪
     * 85.1MHz -65dBm
     */

    g_lcd.clearBuffer();

    // 1行目: FMラジオ局名（日本語）
    g_lcd.setFont(u8g2_font_b12_t_japanese1);
    g_lcd.setCursor(0, 12);
    snprintf(buf, sizeof(buf), "ラジオ局: %s", p_str);
    g_lcd.print(buf);

    // 2行目: FM周波数とRSSI
    g_lcd.setFont(u8g2_font_helvB12_tr);
    g_lcd.setCursor(0, 31);
    snprintf(buf, sizeof(buf), "%.1fMHz %ddBm", freq_val, s_fm_rssi);
    g_lcd.print(buf);

    g_lcd.sendBuffer();
}

// -----------------------------------------------------------
// [API]

void dsp_radio_fm_ch_chg(void)
{
    // FM周波数をテーブルから選択
    drv_kt0913_set_fm_freq((E_FM_STATION)s_fm_freq_tbl_idx);
    s_fm_freq = g_fm_station_freq_tbl[s_fm_freq_tbl_idx].fm_rerq_Mhz;

    // UIに周波数とラジオ局名を表示
    _ui_draw_fm_freq(s_fm_freq, g_fm_station_freq_tbl[s_fm_freq_tbl_idx].p_str);
    Serial.printf("FM Freq: %.1f MHz (%s)\r\n", s_fm_freq, g_fm_station_freq_tbl[s_fm_freq_tbl_idx].p_str);
    s_fm_rssi = drv_kt0913_get_fm_rssi();
    Serial.printf("RSSI: %d dBm\r\n", s_fm_rssi);

    s_fm_freq_tbl_idx = (s_fm_freq_tbl_idx + 1) % FM_STATION_FREQ_TBL_SIZE;
}

void dsp_radio_vol_ctrl(bool is_vol_up)
{
    uint8_t vol_val = drv_kt0913_get_volume_val();

    if(is_vol_up) {
        if(vol_val < 31) {
            vol_val++;
        }
    } else {
        if(vol_val > 0) {
            vol_val--;
        }
    }

    s_vol_ctrl.volume_dB = vol_val;
    drv_kt0913_volume_ctrl(&s_vol_ctrl);
    Serial.printf("Volume: %d\r\n", vol_val);
}

void dsp_radio_init(void)
{
    // KT0913ドライバにI2CのRead/Write関数を渡して初期化
    g_kt0913_cfg.p_i2c_write = _i2c_write;
    g_kt0913_cfg.p_i2c_read  = _i2c_read;
    drv_kt0913_init(&g_kt0913_cfg);

    // 音量調節
    s_vol_ctrl.is_bass_boost = true;
    s_vol_ctrl.volume_dB = 15; // 0 ~ 31の32段階
    s_vol_ctrl.audio_gain = AUDIO_GAIN_3DB;
    drv_kt0913_volume_ctrl(&s_vol_ctrl);

    // FMの初期値
#ifdef RADIO_AREA_TOKYO
    drv_kt0913_set_fm_freq(FM_STATION_FM_TOKYO); // FM東京: 80.0MHz
#else
    drv_kt0913_set_fm_freq(FM_STATION_FM_OSAKA); // FM大阪: 85.1MHz
#endif

#ifdef DEBUG_DSP_RADIO
    // [DEBUG] DSPの全レジスタを読み出し
    _dbg_get_all_reg();
#endif // DEBUG_DSP_RADIO

    // LCD初期化
    _lcd_init();

    // LCDのUIにFM関連情報を表示
    _ui_draw_fm_freq(g_fm_station_freq_tbl[s_fm_freq_tbl_idx].fm_rerq_Mhz,
                    g_fm_station_freq_tbl[s_fm_freq_tbl_idx].p_str);
}

void dsp_radio_main(void)
{
    // シリアルの受信でDSPラジオを制御
    if (Serial.available() > 0) {
        char c = Serial.read();

        // アルファベットのみ処理
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            // 'n'を受信: FMラジオのCHを切り替え
            if (c == 'n') {
                dsp_radio_fm_ch_chg();
#ifdef DEBUG_DSP_RADIO
                // [DEBUG] DSPの全レジスタを読み出し
                _dbg_get_all_reg();
#endif // DEBUG_DSP_RADIO
            }
            // 'u'を受信: 音量アップ
            else if (c == 'u') {
                dsp_radio_vol_ctrl(true);
#ifdef DEBUG_DSP_RADIO
                // [DEBUG] DSPの全レジスタを読み出し
                _dbg_get_all_reg();
#endif // DEBUG_DSP_RADIO
            }
            // 'd'を受信: 音量ダウン
            else if (c == 'd') {
                dsp_radio_vol_ctrl(false);
#ifdef DEBUG_DSP_RADIO
                // [DEBUG] DSPの全レジスタを読み出し
                _dbg_get_all_reg();
#endif // DEBUG_DSP_RADIO
            }
        }
    }
}