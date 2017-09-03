#ifndef __GETASACMD_H__
#define __GETASACM_H__
#include <inttypes.h>
/**
  get command from ASA loader(serial programming)
 */
uint8_t get_ASA_prog_cmd(uint8_t *buf, uint16_t *bytes);
#endif
