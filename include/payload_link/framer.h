#ifndef PAYLOAD_LINK_FRAMER_H
#define PAYLOAD_LINK_FRAMER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Length fields are decoded into uint64_t, so they may contain 1-8 bytes. */
#define PLFRAMER_MAX_LENGTH_SIZE sizeof(uint64_t)

typedef enum {
    PLFRAMER_BIG_ENDIAN,
    PLFRAMER_LITTLE_ENDIAN,
} plframer_byte_order_t;

/*
 * Produces the checksum bytes exactly as they appear on the wire. A profile
 * decides which supplied fields participate in its checksum calculation.
 * checksum_out has config->checksum_size bytes available.
 */
typedef void (*plframer_checksum_fn)(
    void *user_data,
    const uint8_t *sync,
    size_t sync_size,
    const uint8_t *length_field,
    size_t length_size,
    const uint8_t *body,
    size_t body_size,
    uint8_t *checksum_out,
    size_t checksum_size);

typedef struct {
    const uint8_t *sync;
    size_t sync_size;

    size_t length_size;
    plframer_byte_order_t length_byte_order;

    size_t checksum_size;
    plframer_checksum_fn checksum;
    void *checksum_user_data;

    size_t min_body_size;
    size_t max_body_size;
} plframer_config_t;

bool plframer_config_is_valid(const plframer_config_t *config);

/*
 * Encodes one frame into frame_buf. Returns the number of bytes written, or
 * zero for an invalid configuration, invalid body size, or insufficient
 * output capacity.
 */
size_t plframer_encode(const plframer_config_t *config,
                       const uint8_t *body,
                       size_t body_size,
                       uint8_t *frame_buf,
                       size_t frame_capacity);

typedef enum {
    PLFRAMER_DECODE_NEED_MORE,
    PLFRAMER_DECODE_OK,
    PLFRAMER_DECODE_BAD_CHECKSUM,
    PLFRAMER_DECODE_RESYNC,
} plframer_decode_result_t;

typedef enum {
    PLFRAMER_STATE_HUNT_SYNC,
    PLFRAMER_STATE_READ_LENGTH,
    PLFRAMER_STATE_READ_BODY,
    PLFRAMER_STATE_READ_CHECKSUM,
} plframer_decode_state_t;

typedef struct {
    const plframer_config_t *config;
    plframer_decode_state_t state;

    uint8_t *body;
    size_t body_capacity;
    size_t body_size;

    uint8_t *sync_window;
    size_t sync_window_capacity;
    size_t sync_window_count;
    size_t sync_window_next;

    uint8_t length_field[PLFRAMER_MAX_LENGTH_SIZE];
    uint64_t length_value;
    size_t field_have;

    uint8_t *expected_checksum;
    size_t checksum_capacity;
    bool checksum_mismatch;
} plframer_decoder_t;

/*
 * Initializes an allocation-free streaming decoder. The caller owns all
 * supplied storage and must keep it alive for the decoder's lifetime.
 */
bool plframer_decoder_init(plframer_decoder_t *decoder,
                           const plframer_config_t *config,
                           uint8_t *body_storage,
                           size_t body_capacity,
                           uint8_t *sync_window_storage,
                           size_t sync_window_capacity,
                           uint8_t *checksum_storage,
                           size_t checksum_capacity);

plframer_decode_result_t plframer_decoder_feed(plframer_decoder_t *decoder,
                                                uint8_t byte);

#endif /* PAYLOAD_LINK_FRAMER_H */
