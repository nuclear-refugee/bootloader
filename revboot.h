#ifndef __REVBOOT_H__
#define __REVBOOT_H__

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

#endif
