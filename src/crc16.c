#include "payload_link/crc16.h"

uint16_t update_crc16_ccit(uint16_t crc,
                           const uint8_t input_stream[],
                           size_t len) {
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)input_stream[i] << 8;

        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & UINT16_C(0x8000)) != 0) {
                crc = (uint16_t)((crc << 1) ^ UINT16_C(0x1021));
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

uint16_t compute_crc16_ccit(const uint8_t input_stream[], size_t len) {
    return update_crc16_ccit(INIT_CRC_CCIT, input_stream, len);
}
