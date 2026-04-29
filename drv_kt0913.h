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
#define KT0913_CHIP_ID      0x4B54 // NOTE: ChipIDの0x4B54はASCIIで"KT"

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
    uint8_t amfm_mode; // 0: FM, 1: AM
    // TODO
} kt0913_config_t;

// -----------------------------------------------------------

void drv_kt0913_init(kt0913_config_t *p_config);
void drv_kt0913_set_reg(uint8_t reg_addr, uint8_t reg_val);
void drv_kt0913_mute_onoff(bool is_mute);
// void drv_kt0913_set_freq();

#endif // DRV_KT0913_H