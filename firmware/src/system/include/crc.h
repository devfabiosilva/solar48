#ifndef CRC_H
 #define CRC_H

uint16_t crc16_initial(uint16_t inicial, uint8_t *data, uint16_t length);
#define crc16(data, len) crc16_initial(0xFFFF, data, len)

#endif

