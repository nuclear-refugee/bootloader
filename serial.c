#include "serial.h"
#include "revlib/rev_DEVICE.h"
#include "revlib/rev_serial.h"
#include <stdio.h>

inline uint8_t serial_get() {
    while((UCSR0A&(1<<RXC0))==0)
        ;
	return UDR0;
}

inline uint8_t serial_put(uint8_t data) {
    while((UCSR0A&(1<<UDRE0))==0)
        ;
    UDR0 = data;
    return 0;
}
