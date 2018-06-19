#include "revprog.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

void (*main_entry_point)(void) = APP_START_ADDR;

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

void program_page (uint32_t page, uint8_t *buf) {
    uint16_t i;
    uint8_t sreg;
    // Disable interrupts.
    sreg = SREG;
    cli();
    eeprom_busy_wait ();
    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.
    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill (page + i, w);
    }
    boot_page_write(page);      // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    boot_rww_enable();
    SREG = sreg;
}

void erase_all_flash() {
	uint16_t i;
	uint32_t address = 0;
	for(i = 0; i < MAX_PROGSIZE; i++) {
		boot_page_erase(address);
		boot_spm_busy_wait();
		boot_rww_enable();
		address+=SPM_PAGESIZE;
	}
	//boot_rww_enable();
}
