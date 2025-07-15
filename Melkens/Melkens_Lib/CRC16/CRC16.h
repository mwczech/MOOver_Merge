#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t CRC16(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif // CRC16_H
