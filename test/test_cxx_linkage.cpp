// Compile the public headers as C++ and call the C library through them. If a
// header loses its extern "C" guard, the calls below are name-mangled and this
// test fails to link.

#include <cassert>
#include <cstdio>
#include <cstring>

#include "payload_link/crc16.h"
#include "payload_link/frame.h"
#include "payload_link/framer.h"
#include "payload_link/pack.h"

int main() {
    // ICD RevB A1 frame test vector: APID 0x010 telecommand, no secondary
    // header, sequence count 0, section 5.2 Run Experiment (opcode 0x20),
    // command counter 1, argument 1.
    const uint8_t body[] = {0x10, 0x10, 0xC0, 0x00, 0x00, 0x06, 0x20,
                            0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    const uint8_t expected_frame[] = {
        0x1A, 0xCF, 0xFC, 0x1D, 0x00, 0x0D, 0x10, 0x10, 0xC0, 0x00, 0x00,
        0x06, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x9B, 0xBA,
    };
    uint8_t frame[PL_MAX_FRAME_LEN];
    uint8_t packed[2];
    size_t off = 0;
    plframe_decode_ctx_t ctx;
    size_t frame_len = plframe_encode(body, sizeof(body), frame);

    assert(frame_len == sizeof(expected_frame));
    assert(std::memcmp(frame, expected_frame, sizeof(expected_frame)) == 0);

    plframe_decode_init(&ctx);
    for (size_t i = 0; i < frame_len; i++) {
        plframe_decode_result_t result = plframe_decode_feed(&ctx, frame[i]);

        assert(result == (i + 1 < frame_len ? PL_DECODE_NEED_MORE
                                            : PL_DECODE_OK));
    }
    assert(ctx.body_len == sizeof(body));
    assert(std::memcmp(ctx.body, body, sizeof(body)) == 0);

    assert(compute_crc16_ccit(reinterpret_cast<const uint8_t *>("123456789"),
                              9) == 0x29B1);
    assert(plframer_config_is_valid(&PLFRAME_ICD_CONFIG));
    pack_be16(packed, &off, 0x1A2B);
    off = 0;
    assert(unpack_be16(packed, &off) == 0x1A2B);

    std::printf("test_cxx_linkage: ok\n");
    return 0;
}
