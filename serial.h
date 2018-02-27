#ifndef __SERIAL_H__
#define __SERIAL_H__

/**
 * @brief Get one byte data from UART
 */
inline uint8_t serial_get();

/**
 * @brief Put one byte data to UART
 */
inline uint8_t serial_put(uint8_t data);

#endif
