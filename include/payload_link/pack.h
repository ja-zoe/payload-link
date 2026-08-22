#ifndef PAYLOAD_LINK_PACK_H
#define PAYLOAD_LINK_PACK_H

#include <stddef.h>
#include <stdint.h>

// Big-endian pack/unpack helpers (ICD §2.1: all multi-byte fields are
// transmitted big-endian regardless of host endianness).
//
// Each pack_* writes at buf + *off, then advances *off by the field size.
// Each unpack_* reads from buf + *off, then advances *off by the field size.
// No bounds checking here — callers are responsible for sizing buffers
// against the frame/packet layout they're building.

void pack_u8(uint8_t *buf, size_t *off, uint8_t val);
void pack_be16(uint8_t *buf, size_t *off, uint16_t val);
void pack_be32(uint8_t *buf, size_t *off, uint32_t val);
void pack_be64(uint8_t *buf, size_t *off, uint64_t val);

uint8_t  unpack_u8(const uint8_t *buf, size_t *off);
uint16_t unpack_be16(const uint8_t *buf, size_t *off);
uint32_t unpack_be32(const uint8_t *buf, size_t *off);
uint64_t unpack_be64(const uint8_t *buf, size_t *off);

#endif // PAYLOAD_LINK_PACK_H
