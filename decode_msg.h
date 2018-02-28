#ifndef __DOCODE_MSG_H__
#define __DOCODE_MSG_H__
#include <inttypes.h>


#define SPM_PAGESIZE 256

/**
 * @brief 4 bytes type of int, can access as uint8_t[4] or uint32_t.
 */
typedef union ADDRESS {
    uint32_t ui32;
    uint8_t ui8[4];
}Addres_t;

/**
 * @brief Command Buffer
 *
 * NOTE The current STK500 firmware can only handle messages with a message body
 * of maximum of 275 bytes.
 * NOTE ASA prog messages with a message body of maximum of 256 bytes.
 */
struct msg_body {
    uint16_t bytes;
    uint8_t data[300];
};

typedef struct msg_body msg_body_t;

#define RES_ERROR   0
#define RES_ASAPROG 1
#define RES_STK500  2

uint8_t get_msg();
msg_body_t MsgGet = {0,{}};

uint8_t Sequence = 0; ///< Recording the current sequence of STK500 packet.

#endif // __DOCODE_MSG_H__
