#include <inttypes.h>
#include "revprog.h"
#include "getASAcmd.h"

#define SPM_PAGESIZE 256

uint8_t get_ASA_prog_cmd(uint8_t *buf, uint16_t *bytes) {
    uint8_t res = 0;
    for (uint8_t i = 0; i < 4; i++)
        if (prog_getc() != 0xfc)
            res = 1;
    if (prog_getc() != 0x01)
        res = 2;
    *bytes = prog_getc();
    *bytes = ((*bytes)<<8) + prog_getc();
    uint8_t chksum    = 0;
    for (uint16_t i = 0; i < *bytes; i++) {
        buf[i] = prog_getc();
        chksum += buf[i];
    }
    for (uint16_t i = *bytes; i < SPM_PAGESIZE; i++) {
        buf[i] = 0;
    }
    uint8_t getchksum = prog_getc();
    if (getchksum != chksum)
        res = 3;

    return res;
}
