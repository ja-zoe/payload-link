// Encode a fake body, decode it byte-by-byte, and check you get the same
// bytes back. Write this once plframe_encode/plframe_decode_feed have real
// implementations -- it'll just return failure against the stubs.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/frame.h"

int main(void) {
    const uint8_t body[] = {0x08, 0x01, 0xC0, 0x00, 0x00,
                            0x02, 0xAA, 0x55, 0x42};
    uint8_t frame[PL_MAX_FRAME_LEN];
    uint8_t max_body[PL_MAX_BODY_LEN];
    uint8_t max_frame[PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t ctx;
    size_t frame_len = plframe_encode(body, sizeof(body), frame);

    assert(frame_len == PL_SYNC_LEN + PL_LENGTH_LEN + sizeof(body) + PL_CRC_LEN);

    plframe_decode_init(&ctx);
    for (size_t pass = 0; pass < 2; pass++) {
        for (size_t i = 0; i < frame_len; i++) {
            plframe_decode_result_t result = plframe_decode_feed(&ctx, frame[i]);

            if (i + 1 < frame_len) {
                assert(result == PL_DECODE_NEED_MORE);
            } else {
                assert(result == PL_DECODE_OK);
                assert(ctx.body_len == sizeof(body));
                assert(memcmp(ctx.body, body, sizeof(body)) == 0);
            }
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
