#ifndef __REVPROG_H__
#define __REVPROG_H__
#include <inttypes.h>
// here is interface to set and communicate with serial programming

// atmega series register
// define USART for application programming
#define PROG_BAUD 115200
#define BAUD_PRESCALE ((((F_CPU / 16UL) /PROG_BAUD )) - 1)

#define PROG_UBRRH UBRR0H
#define PROG_UBRRL UBRR0L
#define PROG_UCSRA UCSR0A
#define PROG_UCSRB UCSR0B
#define PROG_UCSRC UCSR0C
#define PROG_UCSRC UCSR0C
#define PROG_UDR   UDR0

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
uint8_t prog_getc();

#endif
