#ifndef __DOCODE_MSG_H__
#define __DOCODE_MSG_H__
#include <inttypes.h>

#define SPM_PAGESIZE 256

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
uint8_t put_msg_in_stk500(msg_body_t* c);
uint8_t put_msg_in_asaprog(msg_body_t* c);
void put_msg_asaprog_OK();

extern msg_body_t MsgGet;
extern msg_body_t MsgRes;

extern uint8_t Sequence; ///< Recording the current sequence of STK500 packet.

#endif // __DOCODE_MSG_H__
