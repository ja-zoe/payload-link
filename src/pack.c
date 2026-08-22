#include "payload_link/pack.h"

// Straightforward big-endian byte packing. Ported from the pattern already
// used in PiDaemon's Network/src/main.c (pack_be16/32/64), extended with the
// matching unpack_* side that main.c didn't need yet but the deframer does.

void pack_u8(uint8_t *buf, size_t *off, uint8_t val) {
    buf[*off] = val;
    *off += 1;
}

void pack_be16(uint8_t *buf, size_t *off, uint16_t val) {
    buf[*off]     = (uint8_t)(val >> 8);
    buf[*off + 1] = (uint8_t)(val & 0xFF);
    *off += 2;
}

void pack_be32(uint8_t *buf, size_t *off, uint32_t val) {
    buf[*off]     = (uint8_t)(val >> 24);
    buf[*off + 1] = (uint8_t)(val >> 16);
    buf[*off + 2] = (uint8_t)(val >> 8);
    buf[*off + 3] = (uint8_t)(val & 0xFF);
    *off += 4;
}

void pack_be64(uint8_t *buf, size_t *off, uint64_t val) {
    for (int i = 0; i < 8; i++) {
        buf[*off + (size_t)i] = (uint8_t)(val >> (56 - 8 * i));
    }
    *off += 8;
}

uint8_t unpack_u8(const uint8_t *buf, size_t *off) {
    uint8_t val = buf[*off];
    *off += 1;
    return val;
}

uint16_t unpack_be16(const uint8_t *buf, size_t *off) {
    uint16_t val = (uint16_t)((buf[*off] << 8) | buf[*off + 1]);
    *off += 2;
    return val;
}

uint32_t unpack_be32(const uint8_t *buf, size_t *off) {
    uint32_t val = ((uint32_t)buf[*off] << 24) | ((uint32_t)buf[*off + 1] << 16) |
                    ((uint32_t)buf[*off + 2] << 8) | (uint32_t)buf[*off + 3];
    *off += 4;
    return val;
}

uint64_t unpack_be64(const uint8_t *buf, size_t *off) {
    uint64_t val = 0;
    for (int i = 0; i < 8; i++) {
        val = (val << 8) | buf[*off + (size_t)i];
    }
    *off += 8;
    return val;
}
