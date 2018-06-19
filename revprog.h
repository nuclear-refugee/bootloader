#ifndef __REVPROG_H
#define __REVPROG_H

#include <inttypes.h>

#define APP_START_ADDR 0x0000 ///< the start addres of program to load
extern void (*main_entry_point)(void); ///< the start addres of program to load

#define MAX_PROGSIZE 4800 ///< The max app program size, maybe the same with bootloader start addres

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
 * @brief function to init serial programming communication
 */
void prog_init(void);
/**
 * @brief function to put an uchar in serial programming communication
 */
void prog_putc(uint8_t data);
/**
 * @brief function to get an uchar in serial programming communication
 */
uint8_t prog_getc();
/**
 * @brief
 */
void program_page(uint32_t page, uint8_t *buf);
/**
 * @brief
 */
void erase_all_flash(void);

#endif // __REVPROG_H
