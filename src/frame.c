#include "payload_link/frame.h"

#include "payload_link/crc16.h"

#include <assert.h>

static const uint8_t payload_link_sync[PL_SYNC_LEN] = {
    (uint8_t)(PL_SYNC_WORD >> 24),
    (uint8_t)(PL_SYNC_WORD >> 16),
    (uint8_t)(PL_SYNC_WORD >> 8),
    (uint8_t)PL_SYNC_WORD,
};

_Static_assert(PL_SYNC_LEN == sizeof(uint32_t),
               "the ICD sync word must occupy four bytes");
_Static_assert(PL_LENGTH_LEN == sizeof(uint16_t),
               "the ICD length field must occupy two bytes");
_Static_assert(PL_CRC_LEN == sizeof(uint16_t),
               "CRC-16 must occupy two bytes");

static void payload_link_crc(void *user_data,
                             const uint8_t *sync,
                             size_t sync_size,
                             const uint8_t *length_field,
                             size_t length_size,
                             const uint8_t *body,
                             size_t body_size,
                             uint8_t *checksum_out,
                             size_t checksum_size) {
    uint16_t crc;

    (void)user_data;
    (void)sync;
    (void)sync_size;
    (void)length_field;
    (void)length_size;

    assert(checksum_size == PL_CRC_LEN);
    crc = compute_crc16_ccit(body, body_size);
    checksum_out[0] = (uint8_t)(crc >> 8);
    checksum_out[1] = (uint8_t)crc;
}

const plframer_config_t PLFRAME_ICD_CONFIG = {
    .sync = payload_link_sync,
    .sync_size = PL_SYNC_LEN,
    .length_size = PL_LENGTH_LEN,
    .length_byte_order = PLFRAMER_BIG_ENDIAN,
    .checksum_size = PL_CRC_LEN,
    .checksum = payload_link_crc,
    .checksum_user_data = NULL,
    .min_body_size = PL_MIN_BODY_LEN,
    .max_body_size = PL_MAX_BODY_LEN,
};

size_t plframe_encode(const uint8_t *body_bytes,
                      size_t body_len,
                      uint8_t *frame_buf) {
    size_t frame_capacity;

    if (body_len < PL_MIN_BODY_LEN || body_len > PL_MAX_BODY_LEN) {
        return 0;
    }
    frame_capacity = PL_SYNC_LEN + PL_LENGTH_LEN + body_len + PL_CRC_LEN;
    return plframer_encode(&PLFRAME_ICD_CONFIG,
                           body_bytes,
                           body_len,
                           frame_buf,
                           frame_capacity);
}

void plframe_decode_init(plframe_decode_ctx_t *ctx) {
    bool initialized = plframer_decoder_init(&ctx->decoder,
                                              &PLFRAME_ICD_CONFIG,
                                              ctx->body,
                                              sizeof(ctx->body),
                                              ctx->sync_window,
                                              sizeof(ctx->sync_window),
                                              ctx->expected_crc,
                                              sizeof(ctx->expected_crc));

    assert(initialized);
    (void)initialized;
    ctx->state = PL_STATE_HUNT_SYNC;
    ctx->body_len = 0;
}

plframe_decode_result_t plframe_decode_feed(plframe_decode_ctx_t *ctx,
                                             uint8_t byte) {
    plframer_decode_result_t result =
        plframer_decoder_feed(&ctx->decoder, byte);

    ctx->state = (plframe_decode_state_t)ctx->decoder.state;
    ctx->body_len = (uint16_t)ctx->decoder.body_size;
    return (plframe_decode_result_t)result;
}
