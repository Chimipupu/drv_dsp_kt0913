/**
 * @file dev_dsp_kt0913.ino
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief DSPラジオ(KT0913)のアプリ for Arduino IDE
 * @note マイコン: RP2040
 * @version 0.1
 * @date 2026-05-03
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

// Arduino IDE
#include <U8g2lib.h>
#include <Wire.h>

// KT0913ドライバ
#include "drv_kt0913.h"

// -----------------------------------------------------------
#define I2C_SDA_PIN        4
#define I2C_SCL_PIN        5
#define I2C_ADDR_OLED      0x3C

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C g_lcd(U8G2_R0, U8X8_PIN_NONE);

kt0913_config_t g_kt0913_cfg;

static void _gpio_init(void);
static void _i2c_init(void);
static void _i2c_write(uint8_t reg_addr, uint16_t reg_val);
static uint16_t _i2c_read(uint8_t reg_addr);
static void _ui_draw_fm_freq(float freq_val, char *p_str);
static void _ui_main(void);
// -----------------------------------------------------------
// [Static]

static void _gpio_init(void)
{
    // TODO
}

static void _i2c_init(void)
{
    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
}

static void _i2c_write(uint8_t reg_addr, uint16_t reg_val)
{
#if 1
    Wire.beginTransmission(I2C_ADDR_KT0913);
#else
    Wire.beginTransmission(I2C_ADDR_KT0913 << 1);
#endif
    Wire.write(reg_addr);
    Wire.write((reg_val >> 8) & 0xFF); // 上位バイト
    Wire.write(reg_val & 0xFF);        // 下位バイト
    Wire.endTransmission();
}

static uint16_t _i2c_read(uint8_t reg_addr)
{
    uint16_t reg_val = 0xFFFF;

#if 1
    Wire.beginTransmission(I2C_ADDR_KT0913);
#else
    Wire.beginTransmission(I2C_ADDR_KT0913 << 1);
#endif
    Wire.write(reg_addr);
    Wire.endTransmission(false); // リピートスタート

#if 1
    if(Wire.requestFrom(I2C_ADDR_KT0913, 2) == 2) {
#else
    if(Wire.requestFrom(I2C_ADDR_KT0913 << 1, 2) == 2) {
#endif
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

static void _ui_draw_fm_freq(float freq_val, char *p_str)
{
    int32_t x_title;
    int32_t y_title;
    int32_t x_num;
    int32_t y_num;
    int32_t x_unit;

    x_title = 0;
    y_title = 12; // 12ドットフォント用のベースライン
    x_num   = 24; // 数値を少し右に寄せて中央付近に配置
    y_num   = 31; // 14ドットフォント用のベースライン
    x_unit  = 90; // 単位を右端に配置

    g_lcd.clearBuffer();

    // --- 上段：タイトルの描画 ---
    // 12ドットの日本語フォント（画面上部にぴったり収まる）
    g_lcd.setFont(u8g2_font_b12_t_japanese1);
    g_lcd.setCursor(x_title, y_title);
    g_lcd.print("FM周波数:");
    g_lcd.print(p_str); // ラジオ局名を表示

    // --- 下段：周波数数値の描画 ---
    // 14ドットの太字英数字フォント
    g_lcd.setFont(u8g2_font_helvB14_tr);
    g_lcd.setCursor(x_num, y_num);
    g_lcd.print(freq_val, 1); // 小数点第1位まで表示

    // --- 下段：単位の描画 ---
    // 10ドットの標準英数字フォント
    g_lcd.setFont(u8g2_font_helvR10_tr);
    g_lcd.setCursor(x_unit, y_num);
    g_lcd.print("MHz");

    g_lcd.sendBuffer();
}

static void _ui_main(void)
{
    static uint8_t s_tbl_idx = 0;

    // FM周波数をテーブルから選択
    drv_kt0913_set_fm_freq((E_FM_STATION)s_tbl_idx);

    // UIに周波数とラジオ局名を表示
    _ui_draw_fm_freq(g_fm_station_freq_tbl[s_tbl_idx].fm_rerq_Mhz, g_fm_station_freq_tbl[s_tbl_idx].p_str);

    s_tbl_idx = (s_tbl_idx + 1) % FM_STATION_FREQ_TBL_SIZE;
}

// -----------------------------------------------------------
void setup()
{
    // GPIO初期化
    _gpio_init();

    // I2C初期化
    _i2c_init();

    // UART初期化
    // Serial.begin(115200);

    // LCD初期化
    _lcd_init();
    _ui_draw_fm_freq(0.0, "初期化END");

#if 1
    // KT0913ドライバ初期化
    Serial.printf("KT0913 Driver Init Start\n");
    g_kt0913_cfg.p_i2c_write = _i2c_write;
    g_kt0913_cfg.p_i2c_read = _i2c_read;
    drv_kt0913_init(&g_kt0913_cfg);
    Serial.printf("KT0913 Driver Init Done\n");
#endif
}

void loop()
{
    // UI処理メイン
    _ui_main();

    delay(5000);
}