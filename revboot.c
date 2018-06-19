#include <inttypes.h>
#include <util\delay.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include "revprog.h"
#include "asaprog.h"

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
                        erase_all_flash();
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
