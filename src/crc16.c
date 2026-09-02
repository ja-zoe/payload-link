#include "payload_link/crc16.h" 
#include <stddef.h>

uint16_t compute_crc16_ccit(const uint8_t input_stream[], size_t len) {
  uint16_t crc = 0xFFFF;

  for(size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)input_stream[i] << 8;

    for(uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}
