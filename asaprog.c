#include <inttypes.h>
#include "revprog.h"
#include <avr/io.h>
#include <util\delay.h>

#define HEADER 0xFC
#define CMD_START 0xFA
#define CMD_DATA  0xFC
#define M128_DEVICE_ID 0x01

/**
 * @brief Check ASA_M128 is in prog mode.
 *
 * ASA_M128 use PG1 to classify.
 */
uint8_t is_prog_mode() {
    DDRG = 0;
    return !(PING&0x01);
}

/**
 * @brief get command from ASA loader(serial programming)
 *
 * res = 0 -> ERROR
 * res = 1 -> start asaprog cmd
 * res = 2 -> data cmd
 */
uint8_t get_ASA_prog_cmd(uint8_t *buf, uint16_t *bytes) {
    uint8_t type = 0;
    uint8_t chksum = 0;

    for(int i = 0; i<3; i++)
        if(prog_getc() != 0xFC)
            return 0;

    switch (prog_getc()) {
        case CMD_START:
            type = 1;
            break;
        case CMD_DATA:
            type = 2;
            break;
        default:
            return 0;
    }

    if(prog_getc() != M128_DEVICE_ID)
        return 0;

    *bytes = prog_getc();
    *bytes = ((*bytes)<<8) + prog_getc();
    for (int i = 0; i < *bytes; i++) {
        buf[i] = prog_getc();
        chksum += buf[i];
    }

    if (prog_getc() != chksum)
        return 0;

    return type;
}

/**
 * @brief response to PC that this device is ASA device
 */
void put_res_of_start() {
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFB);
    prog_putc(0x01);
    prog_putc(0x00);
    prog_putc(0x04);
    prog_putc('O');
    prog_putc('K');
    prog_putc('!');
    prog_putc('!');
    prog_putc(0xDC);
}

/**
 * @brief response to PC after the last packet from PC.
 *
 * the last packet is b'\xFC\xFC\xFC\xFC\x01\x00\x00\x00'
 */
void put_res_of_last() {
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFD);
    prog_putc(0x01);
    prog_putc(0x00);
    prog_putc(0x04);
    prog_putc('O');
    prog_putc('K');
    prog_putc('!');
    prog_putc('!');
    prog_putc(0xDC);
}
