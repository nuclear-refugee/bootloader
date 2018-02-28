#include "decode_msg.h"
#include <inttypes.h>
#include "revlib/rev_serial.h"
#include "serial.h"

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

// extern uint8_t serial_get();
//
// extern uint8_t serial_put(uint8_t data);

msg_body_t MsgGet;
msg_body_t MsgRes;

uint8_t Sequence = 0; ///< Recording the current sequence of STK500 packet.

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

        // Get Byte high
        ch = serial_get();
        chksum ^= ch;
        MsgGet.bytes = ch;

        // Get Byte Low
        ch = serial_get();
        chksum ^= ch;
        MsgGet.bytes = (MsgGet.bytes<<8) + ch;

        // Get Token
        ch = serial_get();
        chksum ^= ch;
        if (ch != STK_TOKEN)
            return RES_ASAPROG;

        // Get Msgbody (cmd of STK500)
        for (uint16_t i = 0; i < MsgGet.bytes; i++) {
            ch = serial_get();
            chksum ^= ch;
            MsgGet.data[i] = ch;
        }

        // Get chksum
        ch = serial_get();
        if (ch != chksum)
            return RES_ASAPROG;

        return RES_STK500;

    }
    // ASAPROG decoder
    else if (ch == ASAPROG_HEADER) {

        // Get Header
        for (uint8_t i = 0; i < 3; i++)
            ch = serial_get();
            // if (ch != ASAPROG_HEADER)
            //     return RES_ASAPROG;

        // Get Sequence
        ch = serial_get();
        if (ch != ASAPROG_SEQ)
            return RES_ERROR;

        // Get Byte high
        ch = serial_get();
        chksum += ch;
        MsgGet.bytes = ch;

        // Get Byte Low
        ch = serial_get();
        chksum += ch;
        MsgGet.bytes = (MsgGet.bytes<<8) + ch;

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
            return RES_ASAPROG;

        return RES_ASAPROG;

    } else {
        return RES_ERROR;
    }
}

/**
 * @brief Put a command "PutCmd" to UART.
 *
 * 依照 STK500v2 的訊息封包格式將全域變數 "PutCmd" 進行打包，並傳送到UART串列埠。
 */
uint8_t put_msg_in_stk500(msg_body_t* c) {
    uint8_t chksum=0;
    uint8_t ch;

    ch = STK_START;
    chksum ^= ch;
    serial_put(ch);

    ch = Sequence;
    chksum ^= ch;
    serial_put(ch);

    ch = c->bytes>>8;
    chksum ^= ch;
    serial_put(ch);

    ch = c->bytes&0xFF;
    chksum ^= ch;
    serial_put(ch);

    ch = STK_TOKEN;
    chksum ^= ch;
    serial_put(ch);

    for (uint16_t i = 0; i < c->bytes; i++) {
        ch = c->data[i];
        chksum ^= ch;
        serial_put(ch);
    }
    serial_put(chksum);
    return 0;
}

uint8_t put_msg_in_asaprog(msg_body_t* c) {
    return 0;
}

void put_msg_asaprog_OK() {
    serial_put(0xFC);
    serial_put(0xFC);
    serial_put(0xFC);
    serial_put(0xFC);
    serial_put(0x01);
    serial_put(0x00);
    serial_put(0x04);
    serial_put('O');
    serial_put('K');
    serial_put('!');
    serial_put('!');
    serial_put(0xC4);
}
