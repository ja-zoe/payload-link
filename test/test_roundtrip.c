// Check the profile encoder and decoder independently against a literal wire
// image, then exercise back-to-back and maximum-sized frames.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/frame.h"

int main(void) {
    const uint8_t body[] = {0x08, 0x01, 0xC0, 0x00, 0x00,
                            0x02, 0xAA, 0x55, 0x42};
    const uint8_t expected_frame[] = {
        0x1A, 0xCF, 0xFC, 0x1D, /* sync */
        0x00, 0x09,             /* body length, big-endian */
        0x08, 0x01, 0xC0, 0x00, 0x00, 0x02, 0xAA, 0x55, 0x42,
        0xE7, 0xC1, /* CRC-16/CCITT-FALSE over length and body */
    };
    uint8_t frame[PL_MAX_FRAME_LEN];
    uint8_t body_only_crc_frame[sizeof(expected_frame)];
    uint8_t max_body[PL_MAX_BODY_LEN];
    uint8_t max_frame[PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t ctx;
    size_t frame_len = plframe_encode(body, sizeof(body), frame);

    assert(frame_len == sizeof(expected_frame));
    assert(memcmp(frame, expected_frame, sizeof(expected_frame)) == 0);

    plframe_decode_init(&ctx);
    for (size_t pass = 0; pass < 2; pass++) {
        for (size_t i = 0; i < sizeof(expected_frame); i++) {
            plframe_decode_result_t result =
                plframe_decode_feed(&ctx, expected_frame[i]);

            if (i + 1 < sizeof(expected_frame)) {
                assert(result == PL_DECODE_NEED_MORE);
            } else {
                assert(result == PL_DECODE_OK);
                assert(ctx.body_len == sizeof(body));
                assert(memcmp(ctx.body, body, sizeof(body)) == 0);
            }
        }
    }

    // Guard against reverting to the previous body-only CRC coverage.
    memcpy(body_only_crc_frame, expected_frame, sizeof(expected_frame));
    body_only_crc_frame[sizeof(body_only_crc_frame) - 2] = 0x0E;
    body_only_crc_frame[sizeof(body_only_crc_frame) - 1] = 0xB2;
    plframe_decode_init(&ctx);
    for (size_t i = 0; i < sizeof(body_only_crc_frame); i++) {
        plframe_decode_result_t result =
            plframe_decode_feed(&ctx, body_only_crc_frame[i]);

        if (i + 1 < sizeof(body_only_crc_frame)) {
            assert(result == PL_DECODE_NEED_MORE);
        } else {
            assert(result == PL_DECODE_BAD_CRC);
        }
    }

    for (size_t i = 0; i < sizeof(max_body); i++) {
        max_body[i] = (uint8_t)i;
    }
    frame_len = plframe_encode(max_body, sizeof(max_body), max_frame);
    assert(frame_len == PL_MAX_FRAME_LEN);

    plframe_decode_init(&ctx);
    for (size_t i = 0; i < frame_len; i++) {
        plframe_decode_result_t result = plframe_decode_feed(&ctx, max_frame[i]);

        if (i + 1 < frame_len) {
            assert(result == PL_DECODE_NEED_MORE);
        } else {
            assert(result == PL_DECODE_OK);
            assert(ctx.body_len == sizeof(max_body));
            assert(memcmp(ctx.body, max_body, sizeof(max_body)) == 0);
        }
    }

    printf("test_roundtrip: ok\n");
    return 0;
}
