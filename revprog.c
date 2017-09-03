#include "revprog.h"
#include <avr/io.h>

void prog_init(void) {
    PROG_UBRRL = BAUD_PRESCALE;
    PROG_UBRRH = (BAUD_PRESCALE >> 8);
    PROG_UCSRB = ((1<<TXEN0)|(1<<RXEN0));
    PROG_UCSRC |= (3<<UCSZ10);
}

void prog_putc(uint8_t data) {
    while((PROG_UCSRA &(1<<UDRE0)) == 0);
    PROG_UDR = data;
}

uint8_t prog_getc() {
    while((PROG_UCSRA &(1<<RXC0)) == 0);
    return PROG_UDR;
}
