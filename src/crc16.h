// crc16.h

#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

uint16_t crc16_ccitt(const uint8_t *buffer, size_t size);

static uint16_t Crc16Table[256];
void crcInit(void);

/// @brief Calculate CRC16 using provided calculation from GCL90 documentation
/// @param block [input] Starting address of message
/// @param length [input] Length of message
/// @return CRC of the block
uint16_t crcCompute(const uint8_t *buffer, size_t size);

#endif // CRC16_H

// EOF