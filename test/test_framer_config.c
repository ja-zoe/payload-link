#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/framer.h"

#define TEST_SYNC_SIZE 5
#define TEST_LENGTH_SIZE 3
#define TEST_CHECKSUM_SIZE 3
#define TEST_BODY_CAPACITY 400
#define TEST_FRAME_CAPACITY \
    (TEST_SYNC_SIZE + TEST_LENGTH_SIZE + TEST_BODY_CAPACITY + TEST_CHECKSUM_SIZE)

static const uint8_t test_sync[TEST_SYNC_SIZE] = {
    0xA5, 0x5A, 0xA5, 0x11, 0x22,
};

static void test_checksum(void *user_data,
                          const uint8_t *sync,
                          size_t sync_size,
                          const uint8_t *length_field,
                          size_t length_size,
                          const uint8_t *body,
                          size_t body_size,
                          uint8_t *checksum_out,
                          size_t checksum_size) {
    uint32_t value = *(const uint32_t *)user_data;

    assert(checksum_size == TEST_CHECKSUM_SIZE);
    for (size_t i = 0; i < sync_size; i++) {
        value = (value * UINT32_C(33)) ^ sync[i];
    }
    for (size_t i = 0; i < length_size; i++) {
        value = (value * UINT32_C(33)) ^ length_field[i];
    }
    for (size_t i = 0; i < body_size; i++) {
        value = (value * UINT32_C(33)) ^ body[i];
    }

    checksum_out[0] = (uint8_t)(value >> 16);
    checksum_out[1] = (uint8_t)(value >> 8);
    checksum_out[2] = (uint8_t)value;
}

int main(void) {
    static const uint32_t checksum_seed = UINT32_C(0x12345678);
    const plframer_config_t config = {
        .sync = test_sync,
        .sync_size = sizeof(test_sync),
        .length_size = TEST_LENGTH_SIZE,
        .length_byte_order = PLFRAMER_LITTLE_ENDIAN,
        .checksum_size = TEST_CHECKSUM_SIZE,
        .checksum = test_checksum,
        .checksum_user_data = (void *)&checksum_seed,
        .min_body_size = 0,
        .max_body_size = TEST_BODY_CAPACITY,
    };
    uint8_t body[300];
    uint8_t decoded_body[TEST_BODY_CAPACITY];
    uint8_t frame[TEST_FRAME_CAPACITY];
    uint8_t sync_window[sizeof(test_sync)];
    uint8_t expected_checksum[TEST_CHECKSUM_SIZE];
    plframer_decoder_t decoder;
    size_t frame_size;

    for (size_t i = 0; i < sizeof(body); i++) {
        body[i] = (uint8_t)i;
    }

    assert(plframer_config_is_valid(&config));
    frame_size = plframer_encode(&config,
                                 body,
                                 sizeof(body),
                                 frame,
                                 sizeof(frame));
    assert(frame_size == sizeof(test_sync) + TEST_LENGTH_SIZE + sizeof(body) +
                             TEST_CHECKSUM_SIZE);
    assert(memcmp(frame, test_sync, sizeof(test_sync)) == 0);

    /* 300 == 0x00012C, encoded in a three-byte little-endian field. */
    assert(frame[sizeof(test_sync)] == 0x2C);
    assert(frame[sizeof(test_sync) + 1] == 0x01);
    assert(frame[sizeof(test_sync) + 2] == 0x00);

    assert(plframer_decoder_init(&decoder,
                                 &config,
                                 decoded_body,
                                 sizeof(decoded_body),
                                 sync_window,
                                 sizeof(sync_window),
                                 expected_checksum,
                                 sizeof(expected_checksum)));

    for (size_t i = 0; i < frame_size; i++) {
        plframer_decode_result_t result =
            plframer_decoder_feed(&decoder, frame[i]);

        if (i + 1 < frame_size) {
            assert(result == PLFRAMER_DECODE_NEED_MORE);
        } else {
            assert(result == PLFRAMER_DECODE_OK);
        }
    }

    assert(decoder.body_size == sizeof(body));
    assert(memcmp(decoder.body, body, sizeof(body)) == 0);

    printf("test_framer_config: ok\n");
    return 0;
}
