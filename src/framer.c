#include "payload_link/framer.h"

#include <limits.h>
#include <string.h>

static bool plframer_size_add(size_t left, size_t right, size_t *result) {
    if (left > SIZE_MAX - right) {
        return false;
    }
    *result = left + right;
    return true;
}

static bool plframer_length_can_hold(const plframer_config_t *config,
                                     size_t body_size) {
    if (config->length_size == sizeof(uint64_t)) {
        return true;
    }

    return (uint64_t)body_size <
           (UINT64_C(1) << (config->length_size * CHAR_BIT));
}

bool plframer_config_is_valid(const plframer_config_t *config) {
    if (config == NULL || config->sync == NULL || config->sync_size == 0 ||
        config->length_size == 0 ||
        config->length_size > PLFRAMER_MAX_LENGTH_SIZE ||
        config->min_body_size > config->max_body_size ||
        !plframer_length_can_hold(config, config->max_body_size)) {
        return false;
    }

    if (config->length_byte_order != PLFRAMER_BIG_ENDIAN &&
        config->length_byte_order != PLFRAMER_LITTLE_ENDIAN) {
        return false;
    }

    return config->checksum_size == 0 || config->checksum != NULL;
}

static void plframer_write_length(const plframer_config_t *config,
                                  size_t body_size,
                                  uint8_t *output) {
    uint64_t value = body_size;

    for (size_t i = 0; i < config->length_size; i++) {
        size_t output_index = i;
        size_t shift_index = config->length_size - i - 1;

        if (config->length_byte_order == PLFRAMER_LITTLE_ENDIAN) {
            output_index = config->length_size - i - 1;
        }
        output[output_index] = (uint8_t)(value >> (shift_index * CHAR_BIT));
    }
}

size_t plframer_encode(const plframer_config_t *config,
                       const uint8_t *body,
                       size_t body_size,
                       uint8_t *frame_buf,
                       size_t frame_capacity) {
    size_t frame_size;
    size_t body_offset;
    size_t checksum_offset;

    if (!plframer_config_is_valid(config) || frame_buf == NULL ||
        (body == NULL && body_size != 0) ||
        body_size < config->min_body_size ||
        body_size > config->max_body_size) {
        return 0;
    }

    if (!plframer_size_add(config->sync_size, config->length_size,
                           &body_offset) ||
        !plframer_size_add(body_offset, body_size, &checksum_offset) ||
        !plframer_size_add(checksum_offset, config->checksum_size,
                           &frame_size) ||
        frame_size > frame_capacity) {
        return 0;
    }

    memcpy(frame_buf, config->sync, config->sync_size);
    plframer_write_length(config, body_size,
                          &frame_buf[config->sync_size]);
    if (body_size != 0) {
        memmove(&frame_buf[body_offset], body, body_size);
    }

    if (config->checksum_size != 0) {
        config->checksum(config->checksum_user_data,
                         config->sync,
                         config->sync_size,
                         &frame_buf[config->sync_size],
                         config->length_size,
                         &frame_buf[body_offset],
                         body_size,
                         &frame_buf[checksum_offset],
                         config->checksum_size);
    }

    return frame_size;
}

static void plframer_hunt_sync(plframer_decoder_t *decoder) {
    decoder->state = PLFRAMER_STATE_HUNT_SYNC;
    decoder->field_have = 0;
    decoder->length_value = 0;
    decoder->checksum_mismatch = false;
}

static void plframer_sync_window_push(plframer_decoder_t *decoder,
                                      uint8_t byte) {
    size_t sync_size = decoder->config->sync_size;

    decoder->sync_window[decoder->sync_window_next] = byte;
    decoder->sync_window_next = (decoder->sync_window_next + 1) % sync_size;
    if (decoder->sync_window_count < sync_size) {
        decoder->sync_window_count++;
    }
}

static bool plframer_sync_window_matches(const plframer_decoder_t *decoder) {
    const plframer_config_t *config = decoder->config;

    if (decoder->sync_window_count < config->sync_size) {
        return false;
    }

    for (size_t i = 0; i < config->sync_size; i++) {
        size_t window_index = (decoder->sync_window_next + i) % config->sync_size;

        if (decoder->sync_window[window_index] != config->sync[i]) {
            return false;
        }
    }
    return true;
}

static plframer_decode_result_t
plframer_prepare_checksum(plframer_decoder_t *decoder) {
    const plframer_config_t *config = decoder->config;

    if (config->checksum_size == 0) {
        plframer_hunt_sync(decoder);
        return PLFRAMER_DECODE_OK;
    }

    config->checksum(config->checksum_user_data,
                     config->sync,
                     config->sync_size,
                     decoder->length_field,
                     config->length_size,
                     decoder->body,
                     decoder->body_size,
                     decoder->expected_checksum,
                     config->checksum_size);
    decoder->state = PLFRAMER_STATE_READ_CHECKSUM;
    decoder->field_have = 0;
    decoder->checksum_mismatch = false;
    return PLFRAMER_DECODE_NEED_MORE;
}

bool plframer_decoder_init(plframer_decoder_t *decoder,
                           const plframer_config_t *config,
                           uint8_t *body_storage,
                           size_t body_capacity,
                           uint8_t *sync_window_storage,
                           size_t sync_window_capacity,
                           uint8_t *checksum_storage,
                           size_t checksum_capacity) {
    if (decoder == NULL || !plframer_config_is_valid(config) ||
        body_capacity < config->max_body_size ||
        sync_window_storage == NULL ||
        sync_window_capacity < config->sync_size ||
        (body_storage == NULL && config->max_body_size != 0) ||
        checksum_capacity < config->checksum_size ||
        (checksum_storage == NULL && config->checksum_size != 0)) {
        return false;
    }

    decoder->config = config;
    decoder->body = body_storage;
    decoder->body_capacity = body_capacity;
    decoder->body_size = 0;
    decoder->sync_window = sync_window_storage;
    decoder->sync_window_capacity = sync_window_capacity;
    decoder->sync_window_count = 0;
    decoder->sync_window_next = 0;
    decoder->expected_checksum = checksum_storage;
    decoder->checksum_capacity = checksum_capacity;
    plframer_hunt_sync(decoder);
    return true;
}

plframer_decode_result_t plframer_decoder_feed(plframer_decoder_t *decoder,
                                                uint8_t byte) {
    const plframer_config_t *config = decoder->config;

    plframer_sync_window_push(decoder, byte);

    switch (decoder->state) {
        case PLFRAMER_STATE_HUNT_SYNC:
            if (plframer_sync_window_matches(decoder)) {
                decoder->state = PLFRAMER_STATE_READ_LENGTH;
                decoder->field_have = 0;
                decoder->length_value = 0;
            }
            return PLFRAMER_DECODE_NEED_MORE;

        case PLFRAMER_STATE_READ_LENGTH:
            decoder->length_field[decoder->field_have] = byte;
            if (config->length_byte_order == PLFRAMER_BIG_ENDIAN) {
                decoder->length_value =
                    (decoder->length_value << CHAR_BIT) | byte;
            } else {
                decoder->length_value |=
                    (uint64_t)byte << (decoder->field_have * CHAR_BIT);
            }
            decoder->field_have++;

            if (decoder->field_have < config->length_size) {
                return PLFRAMER_DECODE_NEED_MORE;
            }
            if (decoder->length_value > SIZE_MAX ||
                (size_t)decoder->length_value < config->min_body_size ||
                (size_t)decoder->length_value > config->max_body_size ||
                (size_t)decoder->length_value > decoder->body_capacity) {
                plframer_hunt_sync(decoder);
                return PLFRAMER_DECODE_RESYNC;
            }

            decoder->body_size = (size_t)decoder->length_value;
            decoder->field_have = 0;
            if (decoder->body_size == 0) {
                return plframer_prepare_checksum(decoder);
            }
            decoder->state = PLFRAMER_STATE_READ_BODY;
            return PLFRAMER_DECODE_NEED_MORE;

        case PLFRAMER_STATE_READ_BODY:
            decoder->body[decoder->field_have++] = byte;
            if (decoder->field_have < decoder->body_size) {
                return PLFRAMER_DECODE_NEED_MORE;
            }
            return plframer_prepare_checksum(decoder);

        case PLFRAMER_STATE_READ_CHECKSUM:
            if (byte != decoder->expected_checksum[decoder->field_have]) {
                decoder->checksum_mismatch = true;
            }
            decoder->field_have++;
            if (decoder->field_have < config->checksum_size) {
                return PLFRAMER_DECODE_NEED_MORE;
            }

            if (decoder->checksum_mismatch) {
                plframer_hunt_sync(decoder);
                return PLFRAMER_DECODE_BAD_CHECKSUM;
            }
            plframer_hunt_sync(decoder);
            return PLFRAMER_DECODE_OK;
    }

    plframer_hunt_sync(decoder);
    return PLFRAMER_DECODE_RESYNC;
}
