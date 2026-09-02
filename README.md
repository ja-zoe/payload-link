# payload-link

An allocation-free C11 library for encoding and incrementally decoding framed
byte streams.

The reusable framing engine handles streams shaped like:

```text
SYNC | LENGTH | BODY | CHECKSUM
```

The body is opaque to the library. Opening serial ports, interpreting body
contents, filtering messages, and implementing retransmission remain the
consumer's responsibility.

## Library layers

- `payload_link/framer.h` is the configurable engine. A
  `plframer_config_t` supplies the sync bytes, a 1-8 byte length field and its
  byte order, body bounds, checksum size, and checksum callback. Decoder
  storage is supplied by the caller, so the engine does not require a heap.
- `payload_link/frame.h` provides a ready-to-use bundled profile and preserves
  the simpler `plframe_*` API for existing consumers.

Changing a field width in a new profile changes configuration and caller-owned
storage sizes, not the generic state-machine logic. See
`test/test_framer_config.c` for a profile with a five-byte sync marker,
three-byte little-endian length, and three-byte checksum.

## Consumer example

`examples/consumer.c` shows the bundled profile from both sides: it encodes an
opaque body, then simulates a streaming receiver by feeding the resulting frame
into the decoder one byte at a time and handling every decoder result.

Build and run it with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/payload_link_example
```

## Bundled profile format

```text
| SYNC 0x1ACFFC1D (4B) | LENGTH (2B) | BODY (L bytes) | CRC-16 (2B) |
```

- `SYNC` is the fixed marker used to find frame boundaries.
- `LENGTH` is a big-endian `uint16_t` containing only the body size.
- `BODY` is passed through without interpretation.
- `CRC-16` is CRC-16/CCITT-FALSE over the body bytes.

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

When this repository is included in a parent CMake build with
`add_subdirectory()`, its tests are registered when the parent configures with
`-DENABLE_UNIT_TESTS=ON`. They are ordinary CTest executables using only the C
standard library.
