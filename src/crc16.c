#include "payload_link/crc16.h"

uint16_t crc16_ccitt(const uint8_t *data, size_t length) {
    (void)data;
    (void)length;

    // TODO: implement CRC-16/CCITT-FALSE.
    //
    // Register-based algorithm sketch (see the walkthrough in the vault note
    // for why each piece is there):
    //
    //   crc = 0xFFFF                                  // Init
    //   for each byte b in data:
    //       crc ^= (uint16_t)b << 8                    // mix byte into top of register
    //       repeat 8 times:
    //           if crc & 0x8000: crc = (crc << 1) ^ 0x1021
    //           else:            crc = crc << 1
    //   return crc                                     // no XorOut, so nothing more to do
    //
    // Get test/test_crc16.c passing (both the "123456789" vector AND the
    // zero-length case, which should return 0xFFFF unchanged) before moving on.

    return 0;
}
