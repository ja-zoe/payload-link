// Encode a fake body, decode it byte-by-byte, and check you get the same
// bytes back. Write this once plframe_encode/plframe_decode_feed have real
// implementations -- it'll just return failure against the stubs.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "payload_link/frame.h"

int main(void) {
    // TODO:
    //   1. Build a fake body buffer (doesn't need to be a real CCSDS SPP
    //      header -- any 6+L bytes will do, since this library treats the
    //      body as opaque).
    //   2. plframe_encode() it into a frame buffer.
    //   3. plframe_decode_init() a context.
    //   4. Feed the frame buffer into plframe_decode_feed() one byte at a
    //      time, in a loop, until you get PL_DECODE_OK.
    //   5. assert the decoded ctx->body/ctx->body_len matches what you started with.
    //
    // Once that passes, add a second case: two frames back-to-back in one
    // buffer, decoded in a single feed loop, checking both come out right.
    // That's the realistic UART scenario (bytes don't arrive one frame at
    // a time) and it's a common bug source if the decoder doesn't fully
    // reset between frames.
    

    printf("test_roundtrip: TODO (currently un-implemented)\n");
    return 1;
}
