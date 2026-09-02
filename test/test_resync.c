// The test that actually justifies having a sync word at all. Write this
// after test_roundtrip passes.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/frame.h"

static const uint8_t body[] = {
    0x08, 0x01, 0xC0, 0x00, 0x00, 0x04, 0x10, 0x20, 0x30, 0x40, 0x50,
};

static size_t make_frame(uint8_t *frame) {
    size_t frame_len = plframe_encode(body, sizeof(body), frame);

    assert(frame_len > 0);
    return frame_len;
}

static void assert_decoded_body(const plframe_decode_ctx_t *ctx) {
    assert(ctx->body_len == sizeof(body));
    assert(memcmp(ctx->body, body, sizeof(body)) == 0);
}

static void test_invalid_length(void) {
    const uint8_t bytes[] = {0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x00};
    plframe_decode_ctx_t ctx;

    plframe_decode_init(&ctx);
    for (size_t i = 0; i + 1 < sizeof(bytes); i++) {
        assert(plframe_decode_feed(&ctx, bytes[i]) == PL_DECODE_NEED_MORE);
    }
    assert(plframe_decode_feed(&ctx, bytes[sizeof(bytes) - 1]) == PL_DECODE_RESYNC);
    assert(ctx.state == PL_STATE_HUNT_SYNC);
}

static void test_dropped_body_byte(void) {
    uint8_t frame[PL_MAX_FRAME_LEN];
    uint8_t stream[2 * PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t ctx;
    size_t frame_len = make_frame(frame);
    size_t drop_at = PL_SYNC_LEN + PL_LENGTH_LEN + 2;
    size_t stream_len;
    int saw_bad_crc = 0;
    int saw_ok = 0;

    memcpy(stream, frame, drop_at);
    memcpy(&stream[drop_at], &frame[drop_at + 1], frame_len - drop_at - 1);
    memcpy(&stream[frame_len - 1], frame, frame_len);
    stream_len = (frame_len - 1) + frame_len;

    plframe_decode_init(&ctx);
    for (size_t i = 0; i < stream_len; i++) {
        plframe_decode_result_t result = plframe_decode_feed(&ctx, stream[i]);

        if (result == PL_DECODE_BAD_CRC) {
            saw_bad_crc = 1;
        } else if (result == PL_DECODE_OK) {
            saw_ok = 1;
            assert_decoded_body(&ctx);
        } else {
            assert(result == PL_DECODE_NEED_MORE);
        }
    }

    assert(saw_bad_crc);
    assert(saw_ok);
}

static void test_corrupted_body_byte(void) {
    uint8_t frame[PL_MAX_FRAME_LEN];
    uint8_t stream[2 * PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t ctx;
    size_t frame_len = make_frame(frame);
    size_t corrupt_at = PL_SYNC_LEN + PL_LENGTH_LEN + 2;
    int saw_bad_crc = 0;
    int saw_ok = 0;

    memcpy(stream, frame, frame_len);
    stream[corrupt_at] ^= 0x01u;
    memcpy(&stream[frame_len], frame, frame_len);

    plframe_decode_init(&ctx);
    for (size_t i = 0; i < 2 * frame_len; i++) {
        plframe_decode_result_t result = plframe_decode_feed(&ctx, stream[i]);

        if (result == PL_DECODE_BAD_CRC) {
            saw_bad_crc = 1;
        } else if (result == PL_DECODE_OK) {
            saw_ok = 1;
            assert_decoded_body(&ctx);
        } else {
            assert(result == PL_DECODE_NEED_MORE);
        }
    }

    assert(saw_bad_crc);
    assert(saw_ok);
}

static void test_garbage_prefix(void) {
    const uint8_t garbage[] = {0x00, 0x1A, 0xCF, 0x00, 0xFC, 0x1D, 0x7E};
    uint8_t frame[PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t ctx;
    size_t frame_len = make_frame(frame);

    plframe_decode_init(&ctx);
    for (size_t i = 0; i < sizeof(garbage); i++) {
        assert(plframe_decode_feed(&ctx, garbage[i]) == PL_DECODE_NEED_MORE);
    }
    for (size_t i = 0; i < frame_len; i++) {
        plframe_decode_result_t result = plframe_decode_feed(&ctx, frame[i]);

        if (i + 1 < frame_len) {
            assert(result == PL_DECODE_NEED_MORE);
        } else {
            assert(result == PL_DECODE_OK);
            assert_decoded_body(&ctx);
        }
    }
}

int main(void) {
    test_invalid_length();
    test_dropped_body_byte();
    test_corrupted_body_byte();
    test_garbage_prefix();

    printf("test_resync: ok\n");
    return 0;
}
