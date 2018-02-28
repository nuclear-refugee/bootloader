#include "decode_msg.h"
#include <inttypes.h>

#define STATUS_START  0
#define STATUS_SEQ    1
#define STATUS_MSG1   2
#define STATUS_MSG2   3
#define STATUS_TOKEN  4
#define STATUS_DATA   5
#define STATUS_CHKSUM 6

#define STK_START 0x1B
#define STK_TOKEN 0x0E

#define ASAPROG_HEADER 0xFC
#define ASAPROG_SEQ    0x01 ///< Sequence
// NOTE this is not implemented in ASAPROG
// both the cmd_ASA_loader and official ASA_M128 programmer

extern uint8_t serial_get();

extern uint8_t serial_put(uint8_t data);

uint8_t get_msg() {
    uint8_t ch;
    uint8_t chksum = 0;
    ch = serial_get();
    // get STK500's START or ASSPROG's header to decode msg type

    // STK500 decoder
    if (ch == STK_START) {
        chksum ^= ch;

        //  Get Sequence
        ch = serial_get();
        chksum ^= ch;
        Sequence = ch;
        // TODO implement Sequence
        // NOTE
        // if (ch != Sequence+1)
        //     return 2;
        // Sequence++;

        // Get Sequence
        ch = serial_get();
        chksum ^= ch;
        MsgGet.bytes = ch;

        // Get Byte high
        ch = serial_get();
        chksum ^= ch;
        MsgGet.bytes = (MsgGet.bytes<<8) + ch;

        // Get Token
        ch = serial_get();
        chksum ^= ch;
        if (ch != STK_TOKEN)
            return RES_ERROR;

        // Get Msgbody (cmd of STK500)
        for (uint16_t i = 0; i < MsgGet.bytes; i++) {
            ch = serial_get();
            chksum ^= ch;
            MsgGet.data[i] = ch;
        }

        // Get chksum
        ch = serial_get();
        if (ch != chksum)
            return RES_ERROR;

        return RES_STK500;

    }
    // ASAPROG decoder
    else if (ch == ASAPROG_HEADER) {

        // Get Header
        for (uint8_t i = 0; i < 3; i++)
            ch = serial_get();
            if (ch != ASAPROG_HEADER)
                return RES_ERROR;

        // Get Sequence
        if (serial_get() != ASAPROG_SEQ)
            return RES_ERROR;

        // Get Byte high
        ch = serial_get();
        chksum += ch;
        MsgGet.bytes = (MsgGet.bytes<<8) + ch;

        // Get Byte low
        ch = serial_get();
        chksum += ch;
        if (ch != STK_TOKEN)
            return RES_ERROR;

        // Get Msgbody (exec binary)
        for (uint16_t i = 0; i < MsgGet.bytes; i++) {
            ch = serial_get();
            chksum += ch;
            MsgGet.data[i] = ch;
        }
        // Fill the page size (SPM_PAGESIZE) if the size is less then SPM_PAGESIZE
        // Otherwise will write trash data into memory
        for (uint16_t i = MsgGet.bytes; i < SPM_PAGESIZE; i++) {
            MsgGet.data[i] = 0;
        }

        // Get chksum
        ch = serial_get();
        if (ch != chksum)
            return RES_ERROR;

        return RES_ASAPROG;

    } else {
        return RES_ERROR;
    }
}
