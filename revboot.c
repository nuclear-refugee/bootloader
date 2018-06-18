#include <inttypes.h>
#include <util\delay.h>
#include <avr/boot.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "revprog.h"
#include "asaprog.h"

void (*main_entry_point)(void) = 0x0000;

void program_page(uint32_t page, uint8_t *buf);
void erase_all_flash(void);

int main(void) {

    if(is_prog_mode()) {
        prog_init();
        uint8_t buf[SPM_PAGESIZE];
        uint8_t res = 0;
        uint16_t bytes=0;
        uint32_t page = 0;
        uint8_t status = 0;

        while(1) {
            switch (status) {
                case 0: // wait for cmd
                    res = get_ASA_prog_cmd(buf,&bytes);
                    if(res==1) {  // start asaprog cmd
                        status = 1;
                        // erase_all_flash();
                        put_res_of_start();
                    }
                    break;
                case 1: // wait data cmd and prog
                    res = get_ASA_prog_cmd(buf,&bytes);
                    if(res==2) { // data cmd
                        if(bytes==0) {
                            status = 0;
                            put_res_of_last();
                        } else {
                            // NOTE is it need to set buf to 0 if the bytes is not full?
                            program_page(page,buf);
                            page += SPM_PAGESIZE;
                        }
                    } else {
                        status = 0;
                    }
                    break;
            }
        }
    } else {
        main_entry_point();
    }
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
