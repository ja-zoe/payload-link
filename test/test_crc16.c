// The oracle. Get this passing before writing anything else -- crc16_ccitt
// is the one piece with a published right answer, so it's the cheapest way
// to catch a mistake before it propagates into frame.c.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/crc16.h"

int main(void) {
    // Standard CRC-16/CCITT-FALSE check value.
    const uint8_t vec[] = "123456789";
    uint16_t result = crc16_ccitt(vec, strlen((const char *)vec));
    printf("crc16_ccitt(\"123456789\") = 0x%04X (want 0x29B1)\n", result);
    assert(result == 0x29B1);

    // Zero-length input should return the Init value unchanged -- nothing
    // in the loop body ever runs.
    uint16_t empty = crc16_ccitt(vec, 0);
    printf("crc16_ccitt(\"\") = 0x%04X (want 0xFFFF)\n", empty);
    assert(empty == 0xFFFF);

    printf("test_crc16: PASS\n");
    return 0;
}
