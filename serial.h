#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <inttypes.h>
// here is interface to set and communicate with serial programming

// atmega series register
// define USART for application programming
#define SERIAL_BAUD 115200
#define BAUD_PRESCALE ((((F_CPU / 16UL) /SERIAL_BAUD )) - 1)

#define SERIAL_UBRRH UBRR0H
#define SERIAL_UBRRL UBRR0L
#define SERIAL_UCSRA UCSR0A
#define SERIAL_UCSRB UCSR0B
#define SERIAL_UCSRC UCSR0C
#define SERIAL_UCSRC UCSR0C
#define SERIAL_UDR   UDR0

/**
  function to init serial programming communication
 */
void serial_init(void);
/**
 * @brief Get one byte data from UART
 */
uint8_t serial_get();

/**
 * @brief Put one byte data to UART
 */
void serial_put(uint8_t data);

#endif // __SERIAL_H__
