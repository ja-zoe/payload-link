#include "payload_link/frame.h"
#include "payload_link/crc16.h"
#include "payload_link/pack.h"

size_t plframe_encode(const uint8_t *body_bytes, size_t body_len, uint8_t *frame_buf) {
    (void)body_bytes;
    (void)body_len;
    (void)frame_buf;

    if (body_len == 0 || body_len > PL_MAX_BODY_LEN || body_len < PL_MIN_BODY_LEN) {
        return 0
    }

    // TODO:
    //   1. Validate body_len (7 <= body_len <= PL_MAX_BODY_LEN -- reject 0,
    //      see ICD A2: a zero-octet SPP data field is not representable).
    //   2. pack_be32 the sync word (or write 4 raw bytes -- pack_be32 works
    //      since PL_SYNC_WORD fits in uint32_t).
    //   3. pack_be16 the length (== body_len).
    //   4. memcpy body_bytes in.
    //   5. crc16_ccitt() over bytes from the length field's end through the
    //      end of body_bytes (NOT including sync, NOT including the CRC
    //      field itself -- this is the byte-range detail flagged in the
    //      vault note as the easiest thing to get wrong).
    //   6. pack_be16 the CRC.
    //   7. return total bytes written (8 + body_len).

    return 0;
}

void plframe_decode_init(plframe_decode_ctx_t *ctx) {
    ctx->state = PL_STATE_HUNT_SYNC;
    ctx->accum_have = 0;
    ctx->accum_need = 4; // hunting for the 4-byte sync word
    ctx->body_len = 0;
}

plframe_decode_result_t plframe_decode_feed(plframe_decode_ctx_t *ctx, uint8_t byte) {
    (void)ctx;
    (void)byte;

    // TODO: state machine. Rough shape per state:
    //
    // PL_STATE_HUNT_SYNC:
    //   Shift byte into a 4-byte window (a plain uint32_t shift register
    //   is simplest -- see the vault note on why). When the window ==
    //   PL_SYNC_WORD, move to PL_STATE_READ_LENGTH and reset accum_have.
    //   Otherwise: PL_DECODE_NEED_MORE.
    //
    // PL_STATE_READ_LENGTH:
    //   Accumulate 2 bytes into ctx->accum. Once accum_have == 2,
    //   unpack_be16 it into ctx->body_len. Sanity check:
    //   7 <= body_len <= PL_MAX_BODY_LEN (matches encode's validation --
    //   this is the "plausibility check before trusting LENGTH to size a
    //   read" mentioned in frame.h). If it fails, drop to HUNT_SYNC and
    //   return PL_DECODE_RESYNC. Otherwise move to PL_STATE_READ_BODY,
    //   reset accum_have.
    //   Return PL_DECODE_NEED_MORE either way (unless resyncing).
    //
    // PL_STATE_READ_BODY:
    //   Accumulate body_len bytes directly into ctx->body (not
    //   ctx->accum -- accum is sized for the 4-byte/2-byte fields only).
    //   Once you've read body_len bytes, move to PL_STATE_READ_CRC, reset
    //   accum_have. Return PL_DECODE_NEED_MORE.
    //
    // PL_STATE_READ_CRC:
    //   Accumulate 2 bytes into ctx->accum. Once you have both,
    //   unpack_be16 the received CRC and compare against
    //   crc16_ccitt(ctx->body, ctx->body_len). On match: drop to
    //   HUNT_SYNC, return PL_DECODE_OK (ctx->body/ctx->body_len are
    //   already in place, nothing left to copy). On mismatch: drop to
    //   HUNT_SYNC, return PL_DECODE_BAD_CRC.
    //
    // Test this against test/test_resync.c: a single dropped byte
    // mid-body must not desync the receiver permanently -- it should
    // recover on the next real sync word in the stream.

    return PL_DECODE_NEED_MORE;
}
