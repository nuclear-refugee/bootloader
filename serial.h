#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <inttypes.h>

/**
 * @brief Get one byte data from UART
 */
uint8_t serial_get();

/**
 * @brief Put one byte data to UART
 */
uint8_t serial_put(uint8_t data);

#endif
