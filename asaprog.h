#ifndef __ASAPROG_H
#define __ASAPROG_H

#include <inttypes.h>

uint8_t get_ASA_prog_cmd(uint8_t *buf, uint16_t *bytes);
uint8_t is_prog_mode(void);
void put_res_of_start();
void put_res_of_last();

#endif // __ASAPROG_H
