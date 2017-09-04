#include <inttypes.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util\delay.h>
#include "revprog.h"
#include "getASAcmd.h"

void (*main_entry_point)(void) = 0x0000;

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
        get_ASA_prog_cmd(buf,&bytes);
        _delay_ms(30);
        send_OK();
        res = get_ASA_prog_cmd(buf,&bytes);
        while(bytes==SPM_PAGESIZE){
            boot_program_page(page,buf);
            page += SPM_PAGESIZE;
            res = get_ASA_prog_cmd(buf,&bytes);
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
