// The test that actually justifies having a sync word at all. Write this
// after test_roundtrip passes.

#include <assert.h>
#include <stdio.h>

#include "payload_link/frame.h"

int main(void) {
    // TODO, three cases, each should end with the decoder recovering on
    // the frame that follows the damage rather than staying desynced:
    //
    //   1. Drop one byte out of the middle of a frame's body, then feed a
    //      second, undamaged frame right after it. Expect PL_DECODE_RESYNC
    //      or PL_DECODE_BAD_CRC for the first frame, then PL_DECODE_OK for
    //      the second.
    //   2. Corrupt one byte inside a frame's body (bit flip). Expect
    //      PL_DECODE_BAD_CRC, then PL_DECODE_OK on the next frame.
    //   3. Feed garbage bytes (not a valid sync word) before a valid frame.
    //      Expect PL_DECODE_NEED_MORE throughout the garbage, then
    //      PL_DECODE_OK once the real frame arrives.

    printf("test_resync: TODO (currently un-implemented)\n");
    return 0;
}
