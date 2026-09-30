// Verify crc16_ccitt against published CRC-16/CCITT-FALSE check values.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/crc16.h"

int main(void) {
    // Standard CRC-16/CCITT-FALSE check value.
    const uint8_t vec[] = "123456789";
    const uint8_t length_field[] = {0x00, 0x09};
    const uint8_t body[] = {0x08, 0x01, 0xC0, 0x00, 0x00,
                            0x02, 0xAA, 0x55, 0x42};
    const uint8_t length_and_body[] = {
        0x00, 0x09, 0x08, 0x01, 0xC0, 0x00,
        0x00, 0x02, 0xAA, 0x55, 0x42,
    };
    uint16_t result = compute_crc16_ccit(vec, strlen((const char *)vec));
    printf("strln = %zu\n", strlen((const char *)vec));
    printf("crc16_ccitt(\"123456789\") = 0x%04X (want 0x29B1)\n", result);
    assert(result == 0x29B1);
    // Zero-length input should return the Init value unchanged -- nothing
    // in the loop body ever runs.
    uint16_t empty = compute_crc16_ccit(vec, 0);
    printf("crc16_ccitt(\"\") = 0x%04X (want 0xFFFF)\n", empty);
    assert(empty == 0xFFFF);

    // Incremental updates must match the same bytes supplied contiguously.
    uint16_t segmented =
        update_crc16_ccit(INIT_CRC_CCIT, length_field, sizeof(length_field));
    segmented = update_crc16_ccit(segmented, body, sizeof(body));
    uint16_t contiguous =
        compute_crc16_ccit(length_and_body, sizeof(length_and_body));
    assert(segmented == contiguous);
    assert(segmented == 0xE7C1);

    // An empty update preserves any caller-supplied intermediate state.
    assert(update_crc16_ccit(UINT16_C(0x1234), vec, 0) == UINT16_C(0x1234));
    printf("test_crc16: PASS\n");
    return 0;
}
