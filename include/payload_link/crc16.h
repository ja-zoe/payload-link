#ifndef PAYLOAD_LINK_CRC16_H
#define PAYLOAD_LINK_CRC16_H

#include <stddef.h>
#include <stdint.h>

// CRC-16/CCITT-FALSE over data[0..length-1].
//
// Parameters (per ICD RevB A1): poly 0x1021, init 0xFFFF, no input
// reflection, no output reflection, xorout 0x0000.
//
// Known-answer test vector: crc16_ccitt((const uint8_t *)"123456789", 9)
// must return 0x29B1. Implement this and get that assertion passing in
// test/test_crc16.c before touching frame.c — everything else depends on
// this being right.
uint16_t crc16_ccitt(const uint8_t *data, size_t length);

#endif // PAYLOAD_LINK_CRC16_H
