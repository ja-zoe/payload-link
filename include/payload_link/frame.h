#ifndef PAYLOAD_LINK_FRAME_H
#define PAYLOAD_LINK_FRAME_H

#include <stddef.h>
#include <stdint.h>

#include "payload_link/framer.h"

/*
 * Payload-link ICD profile:
 *
 *   | SYNC 0x1ACFFC1D (4B) | LENGTH (2B BE) | body (L bytes) | CRC-16 (2B) |
 *
 * The generic framing behavior lives in framer.h. These constants and the
 * PLFRAME_ICD_CONFIG object specialize it for ICD RevB section 2.4 / C8.
 */
#define PL_SYNC_WORD UINT32_C(0x1ACFFC1D)
#define PL_SYNC_LEN 4
#define PL_LENGTH_LEN 2
#define PL_CRC_LEN 2
#define PL_MIN_BODY_LEN 7
#define PL_MAX_BODY_LEN 518
#define PL_MAX_FRAME_LEN \
    (PL_SYNC_LEN + PL_LENGTH_LEN + PL_MAX_BODY_LEN + PL_CRC_LEN)

extern const plframer_config_t PLFRAME_ICD_CONFIG;

/*
 * Compatibility encoder for existing payload-link consumers. frame_buf must
 * have at least PL_SYNC_LEN + PL_LENGTH_LEN + body_len + PL_CRC_LEN bytes
 * available. Returns bytes written or 0.
 */
size_t plframe_encode(const uint8_t *body_bytes,
                      size_t body_len,
                      uint8_t *frame_buf);

typedef enum {
    PL_DECODE_NEED_MORE = PLFRAMER_DECODE_NEED_MORE,
    PL_DECODE_OK = PLFRAMER_DECODE_OK,
    PL_DECODE_BAD_CRC = PLFRAMER_DECODE_BAD_CHECKSUM,
    PL_DECODE_RESYNC = PLFRAMER_DECODE_RESYNC,
} plframe_decode_result_t;

typedef enum {
    PL_STATE_HUNT_SYNC = PLFRAMER_STATE_HUNT_SYNC,
    PL_STATE_READ_LENGTH = PLFRAMER_STATE_READ_LENGTH,
    PL_STATE_READ_BODY = PLFRAMER_STATE_READ_BODY,
    PL_STATE_READ_CRC = PLFRAMER_STATE_READ_CHECKSUM,
} plframe_decode_state_t;

typedef struct {
    plframer_decoder_t decoder;
    uint8_t sync_window[PL_SYNC_LEN];
    uint8_t expected_crc[PL_CRC_LEN];

    /* Compatibility view of the generic decoder's current state/output. */
    plframe_decode_state_t state;
    uint16_t body_len;
    uint8_t body[PL_MAX_BODY_LEN];
} plframe_decode_ctx_t;

void plframe_decode_init(plframe_decode_ctx_t *ctx);

/* Feeds exactly one byte. body and body_len are valid after PL_DECODE_OK. */
plframe_decode_result_t plframe_decode_feed(plframe_decode_ctx_t *ctx,
                                             uint8_t byte);

#endif /* PAYLOAD_LINK_FRAME_H */
