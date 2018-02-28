/**
 * @file stk500.c
 * @author LiYu
 * @date 2017.12.27
 * @brief STK500.h function implement.
 */

#include "stk500_command.h"
#include "revlib/ASA/ASA_spi.h"
#include "revlib/ASA/ASA_general.h"
#include "stk500.h"

void RST_init() {
    ASA_REGPUT(RST_DDR, 1, RST_MASK, RST_SHIFT);
}

void RST_en(uint8_t isEnable) {
    if(isEnable>1)
    return;
    ASA_REGPUT(RST_PORT, isEnable, RST_MASK, RST_SHIFT);
}

void isp_enter_progmode() {
    ASA_ID_init();
    RST_init();
    ASA_ID_set(1);
    RST_en(RST_EN);
    _delay_ms(10);
    spi_master_init();
    spi_set_frequency(devider_64);
    spi_en(1);
}

void isp_leave_progmode() {
    ASA_SPIM_cs(0);
    spi_en(!RST_EN);
}

/**
 * @brief 存放可寫入之 STK500 硬體參數。
 *
 * 不可寫入部分使用define預先定義
 */
struct param {
    char vtarget;
    char vadjust;
    char pscale;
    char cmatch;
    char duration;
    char rst_polarity;
    char init;
};

struct param PARAM = {
    STK500_VTARGET,
    STK500_VADJUST,
    STK500_OSC_PSCALE,
    STK500_OSC_CMATCH,
    PARAM_SCK_DURATION,
    STK500_RESET_POLARITY,
    STK500_CONTROLLER_INIT
};

uint8_t isp_set_param(uint8_t id, uint8_t data) {
    switch (id) {
        // only writable parameter
        case PARAM_VTARGET: { PARAM.vtarget = data; break; }
        case PARAM_VADJUST: { PARAM.vadjust = data; break; }
        case PARAM_OSC_PSCALE: { PARAM.pscale = data; break; }
        case PARAM_OSC_CMATCH: { PARAM.cmatch = data; break; }
        case PARAM_SCK_DURATION: { PARAM.cmatch = data; break; }
        case PARAM_RESET_POLARITY: { PARAM.rst_polarity = data; break; }
        case PARAM_CONTROLLER_INIT: { PARAM.init = data; break; }
        default: { return 1; }
    }
    return 0;
}

uint8_t isp_get_param(uint8_t id, uint8_t* data) {
    switch (id) {
        // only readable parameter
        case PARAM_BUILD_NUMBER_LOW:   *data = STK500_BUILD_NUMBER_HIGH; break;
        case PARAM_BUILD_NUMBER_HIGH:  *data = STK500_BUILD_NUMBER_HIGH; break;
        case PARAM_HW_VER:             *data = STK500_HW_VER; break;
        case PARAM_SW_MAJOR:           *data = STK500_SW_MAJOR; break;
        case PARAM_SW_MINOR:           *data = STK500_SW_MINOR; break;
        case PARAM_VTARGET:            *data = PARAM.vtarget; break;
        case PARAM_VADJUST:            *data = PARAM.vadjust; break;
        case PARAM_OSC_PSCALE:         *data = PARAM.pscale; break;
        case PARAM_OSC_CMATCH:         *data = PARAM.cmatch; break;
        case PARAM_SCK_DURATION:       *data = STK500_SCK_DURATION; break;
        case PARAM_TOPCARD_DETECT:     *data = STK500_TOPCARD_DETECT; break;
        case PARAM_DATA:               *data = STK500_DATA; break;
        case PARAM_CONTROLLER_INIT:    *data = PARAM.init; break;
        default: return 1;
    }
    return 0;

}

uint8_t isp_load_flash(uint8_t isHigh, uint8_t addr_a, uint8_t addr_b, uint8_t data) {
    if (isHigh>=2)
        return 1;
    spi_swap(0x40 | (isHigh<<3) );
    spi_swap(addr_a);
    spi_swap(addr_b);
    spi_swap(data);
    return 0;
}

uint8_t isp_write_flash(uint8_t addr_a, uint8_t addr_b) {
    spi_swap(0x4C);
    spi_swap(addr_a);
    spi_swap(addr_b);
    spi_swap(0x00);
    return 0;
}

uint8_t isp_write_eeprom(uint8_t addr_a, uint8_t addr_b, uint8_t data) {
    spi_swap(0xC0);
    spi_swap(addr_a);
    spi_swap(addr_b);
    spi_swap(data);
    return 0;
}
