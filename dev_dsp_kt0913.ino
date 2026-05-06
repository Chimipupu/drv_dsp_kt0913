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
#include <Wire.h>

// DSPラジオ
#include "dsp_radio.h"

// -----------------------------------------------------------
#define I2C_SDA_PIN        4
#define I2C_SCL_PIN        5

static void _gpio_init(void);
static void _i2c_init(void);
// -----------------------------------------------------------
// [Static]

static void _gpio_init(void)
{
    // 基板のYD-RP2040のボタン(GPIO24)を割り込みに設定
    // NOTTE: ボタンがONでFMのCH切り替えをコールバック
    pinMode(24, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(24), []() {
        dsp_radio_fm_ch_chg();
    }, FALLING);
}

static void _i2c_init(void)
{
    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
}

// -----------------------------------------------------------
void setup()
{
    // GPIO初期化
    _gpio_init();

    // I2C初期化
    _i2c_init();

    // UART初期化
    Serial.begin(115200);

    // DSPラジオ初期化
    dsp_radio_init();
}

void loop()
{
    // DSPラジオ メイン
    dsp_radio_main();
}