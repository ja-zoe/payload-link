#include <stdio.h>
#include <string.h>

#include "payload_link/frame.h"

static void print_bytes(const char *label, const uint8_t *bytes, size_t size) {
    printf("%s (%zu bytes):", label, size);
    for (size_t i = 0; i < size; i++) {
        printf(" %02X", bytes[i]);
    }
    putchar('\n');
}

int main(void) {
    /* The framer treats these bytes as an opaque body. */
    const uint8_t outgoing_body[] = {
        0x08, 0x01, 0xC0, 0x00, 0x00, 0x02, 0x10, 0x20, 0x30,
    };
    uint8_t encoded_frame[PL_MAX_FRAME_LEN];
    plframe_decode_ctx_t decoder;
    size_t frame_size;
    int received_frame = 0;

    frame_size = plframe_encode(outgoing_body,
                                sizeof(outgoing_body),
                                encoded_frame);
    if (frame_size == 0) {
        fprintf(stderr, "Could not encode the outgoing body\n");
        return 1;
    }

    print_bytes("Encoded frame", encoded_frame, frame_size);

    /*
     * A real sender would write encoded_frame to a UART here. A real receiver
     * calls plframe_decode_feed once for each byte read from its UART.
     */
    plframe_decode_init(&decoder);
    for (size_t i = 0; i < frame_size; i++) {
        plframe_decode_result_t result =
            plframe_decode_feed(&decoder, encoded_frame[i]);

        switch (result) {
            case PL_DECODE_NEED_MORE:
                break;

            case PL_DECODE_OK:
                print_bytes("Decoded body", decoder.body, decoder.body_len);
                received_frame = 1;
                break;

            case PL_DECODE_BAD_CRC:
                fprintf(stderr, "Discarded a frame with an invalid CRC\n");
                break;

            case PL_DECODE_RESYNC:
                fprintf(stderr, "Discarded an invalid frame; hunting for sync\n");
                break;
        }
    }

    if (!received_frame || decoder.body_len != sizeof(outgoing_body) ||
        memcmp(decoder.body, outgoing_body, sizeof(outgoing_body)) != 0) {
        fprintf(stderr, "The received body did not match the sent body\n");
        return 1;
    }

    return 0;
}
