#include "serial.h"
#include "revlib/rev_DEVICE.h"
#include <stdio.h>

void serial_init(void) {
    SERIAL_UBRRL = BAUD_PRESCALE;
    SERIAL_UBRRH = (BAUD_PRESCALE >> 8);
    SERIAL_UCSRB = ((1<<TXEN0)|(1<<RXEN0));
    SERIAL_UCSRC |= (3<<UCSZ10);
}

void serial_put(uint8_t data) {
    while((SERIAL_UCSRA &(1<<UDRE0)) == 0);
    SERIAL_UDR = data;
}

uint8_t serial_get() {
    while((SERIAL_UCSRA &(1<<RXC0)) == 0);
    return SERIAL_UDR;
}
