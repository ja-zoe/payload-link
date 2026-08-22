#ifndef PAYLOAD_LINK_FRAME_H
#define PAYLOAD_LINK_FRAME_H

#include <stddef.h>
#include <stdint.h>

// Wire format (ICD RevB §2.4 + C8):
//
//   | SYNC 0x1ACFFC1D (4B) | LENGTH (2B) | body (L bytes) | CRC-16 (2B) |
//
// This is a synchronous, length-prefixed frame with a trailing CRC -- a
// fixed sync marker so the receiver can find message boundaries in a byte
// stream, an explicit length so it can bound a read before trusting
// anything inside, and a checksum over length+body. No byte-stuffing or
// escaping (unlike SLIP/HDLC/COBS) -- LENGTH already makes escaping
// unnecessary.
//
// SYNC is fixed. LENGTH is big-endian, counts only the body (not the
// frame total). CRC-16/CCITT-FALSE covers everything after LENGTH up to
// (not including) the CRC field itself -- see crc16.h.
//
// This module does NOT know or care what's inside the body. It hands the
// caller L opaque bytes on decode, and expects L opaque bytes to wrap on
// encode. In this project the body always happens to be a CCSDS Space
// Packet (6-byte header + data, per the ICD), but this library has no
// SPP-specific code anywhere -- building/parsing that header is entirely
// the caller's job (CFE_MSG_* on the busOBC side; whatever the Pi side
// ends up using). Keeping the framer ignorant of that is deliberate: it's
// what lets the exact same library be dropped onto any other sync/length/
// CRC-framed link without modification.

#define PL_SYNC_WORD 0x1ACFFC1Du
#define PL_MIN_BODY_LEN 7   // ICD A2: 6-byte SPP header + at least 1 octet of data
#define PL_MAX_BODY_LEN 518 // in this project: 6-byte SPP header + up to 512 bytes of user data
#define PL_MAX_FRAME_LEN (4 + 2 + PL_MAX_BODY_LEN + 2)

// ---- Encode ----

// Wraps body_bytes[0..body_len-1] (an opaque byte string -- in this
// project, a complete SPP header+data region) into a frame written to
// frame_buf. frame_buf must be at least (8 + body_len) bytes: 4 (sync) +
// 2 (length) + body_len + 2 (crc). Returns the number of bytes written,
// or 0 if body_len is out of range (0 or > PL_MAX_BODY_LEN -- see ICD A2:
// an SPP packet cannot have a zero-octet data field, so in this project
// body_len must be at least 7).
//
// TODO: implement.
size_t plframe_encode(const uint8_t *body_bytes, size_t body_len, uint8_t *frame_buf);

// ---- Decode ----

typedef enum {
    PL_DECODE_NEED_MORE, // no complete frame yet, keep feeding bytes
    PL_DECODE_OK,         // a frame completed and passed CRC; see ctx->body/ctx->body_len
    PL_DECODE_BAD_CRC,    // a frame completed but failed CRC; discarded, resyncing
    PL_DECODE_RESYNC,     // lost sync (e.g. corrupted length caused overrun); hunting again
} plframe_decode_result_t;

typedef enum {
    PL_STATE_HUNT_SYNC,
    PL_STATE_READ_LENGTH,
    PL_STATE_READ_BODY,
    PL_STATE_READ_CRC,
} plframe_decode_state_t;

typedef struct {
    plframe_decode_state_t state;

    // Bytes accumulated for whichever field is currently being read
    // (the 2-byte length, or the 2-byte CRC). Body bytes accumulate
    // directly into `body` below instead, since they need to survive
    // into PL_DECODE_OK.
    uint8_t  accum[4];
    size_t   accum_have;   // bytes accumulated so far in the current state
    size_t   accum_need;   // bytes needed to complete the current state

    // Parsed LENGTH field once >= PL_STATE_READ_BODY. Doubles as the
    // final body length on PL_DECODE_OK -- decode only reaches OK once
    // exactly body_len bytes have been read into `body`, so the two
    // meanings ("expected length" mid-parse, "actual length" once done)
    // are always the same number and don't need separate fields.
    uint16_t body_len;

    // Output of the most recently completed frame (valid after
    // PL_DECODE_OK). body_len above gives its length.
    uint8_t  body[PL_MAX_BODY_LEN];
} plframe_decode_ctx_t;

// Resets ctx to PL_STATE_HUNT_SYNC with no accumulated bytes. Call once
// before first use and any time you want to force a resync (e.g. after a
// UART error).
void plframe_decode_init(plframe_decode_ctx_t *ctx);

// Feeds one byte into the decoder. Call in a loop as bytes arrive from the
// UART (real or NOS3-simulated) -- this function never blocks and never
// reads more than the one byte you give it.
//
// TODO: implement the HUNT_SYNC -> READ_LENGTH -> READ_BODY -> READ_CRC
// state machine. On PL_DECODE_OK, ctx->body[0..ctx->body_len-1] holds the
// decoded body, ready for the caller to hand to whatever parses APIDs. On
// CRC mismatch or an implausible LENGTH, drop back to PL_STATE_HUNT_SYNC
// rather than trusting the framing further -- see
// [[ICD RevB — proposed revisions]] C8 for why LENGTH gets a plausibility
// check (bounded by PL_MAX_BODY_LEN) before it's trusted to size a read.
plframe_decode_result_t plframe_decode_feed(plframe_decode_ctx_t *ctx, uint8_t byte);

#endif // PAYLOAD_LINK_FRAME_H
