#ifndef __REVPROG_H__
#define __REVPROG_H__
#include <inttypes.h>
// here is interface to set and communicate with serial programming

// atmega series register
// define USART for application programming

uint8_t test_cmd[265] = {0xFC,0xFC,0xFC,0xFC,0x01,0x01,0x00};
uint16_t getindex = 0;

/**
  function to init serial programming communication
 */
void prog_init(void);
/**
  function to put an uchar in serial programming communication
 */
void prog_putc(uint8_t data);
/**
  function to get an uchar in serial programming communication
 */
uint8_t prog_getc(){
    getindex++;
    return test_cmd[getindex-1];
};

#endif
