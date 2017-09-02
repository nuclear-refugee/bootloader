#include <inttypes.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util\delay.h>
#include "revboot.h"

void (*main_entry_point)(void) = 0x0000;

void prog_init(void);
void prog_putc(uint8_t data);
uint8_t prog_getc();
uint8_t get_data(uint8_t *buf, uint16_t *bytes);
void send_OK();
void send_OK2();
void boot_program_page (uint32_t page, uint8_t *buf);

inline uint8_t bootloader_active() {
    DDRG = 0;
    return !(PING&0x01);
}

int main(void) {
    prog_init();

    if(bootloader_active()) {
        uint8_t buf[SPM_PAGESIZE]={1};
        uint8_t res = 0;
        uint16_t bytes=0;
        uint32_t page = 0;
        get_data(buf,&bytes);
        _delay_ms(30);
        send_OK();
        res = get_data(buf,&bytes);
        while(bytes==SPM_PAGESIZE){
            boot_program_page(page,buf);
            page += SPM_PAGESIZE;
            res = get_data(buf,&bytes);
            _delay_ms(5);
        }
        if (bytes) {
            boot_program_page(page,buf);
        }
        _delay_ms(30);
        send_OK();
    } else {
        main_entry_point();
    }
}

uint8_t get_data(uint8_t *buf, uint16_t *bytes) {
    uint8_t res = 0;
    for (uint8_t i = 0; i < 4; i++)
        if (prog_getc() != 0xfc)
            res = 1;
    if (prog_getc() != 0x01)
        res = 2;
    *bytes = prog_getc();
    *bytes = ((*bytes)<<8) + prog_getc();
    uint8_t chksum    = 0;
    for (uint16_t i = 0; i < *bytes; i++) {
        buf[i] = prog_getc();
        chksum += buf[i];
    }
    for (uint16_t i = *bytes; i < SPM_PAGESIZE; i++) {
        buf[i] = 0;
    }
    uint8_t getchksum = prog_getc();
    if (getchksum != chksum)
        res = 3;

    return res;
}
void send_OK() {
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0xFC);
    prog_putc(0x01);
    prog_putc(0x00);
    prog_putc(0x04);
    prog_putc('O');
    prog_putc('K');
    prog_putc('!');
    prog_putc('!');
    prog_putc(0xC4);
}

void prog_init(void) {
    PROG_UBRRL = BAUD_PRESCALE;
    PROG_UBRRH = (BAUD_PRESCALE >> 8);
    PROG_UCSRB = ((1<<TXEN0)|(1<<RXEN0));
    PROG_UCSRC |= (3<<UCSZ10);
}

void prog_putc(uint8_t data) {
  // Wait until last byte has been transmitted
  while((PROG_UCSRA &(1<<UDRE0)) == 0);
  // Transmit data
  PROG_UDR = data;
}

uint8_t prog_getc() {
  while((PROG_UCSRA &(1<<RXC0)) == 0);
  return PROG_UDR;
}

void boot_program_page (uint32_t page, uint8_t *buf) {
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
    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    boot_rww_enable ();
    SREG = sreg;
}
