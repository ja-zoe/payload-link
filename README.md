# payload-link

Encodes and decodes the RS-422 frame between the busOBC and payOBC, per ICD RevB §2.4 / C8.

**Scope, on purpose:**

- This library does **not** open serial ports, does not know about NOS3 or hwlib, does not know
  about specific APIDs, and does not implement retransmission. It turns a CCSDS Space Packet
  (header + data) into a framed byte sequence, and turns a byte stream back into CCSDS Space
  Packets. That's it.
- I/O lives in each consumer: `hwlib` on the busOBC side, POSIX termios on the Pi side.
- APID whitelisting and retransmission (ICD §5.8/§5.9) are application-layer concerns and live in
  the consumers too.

Consumed by the `payload_if` cFS app, the busOBC standalone binary, and the Pi daemon — all three
link against the same compiled library so the byte format cannot drift between them.

## Wire format (ICD RevB §2.4 + C8)

```
| SYNC 0x1ACFFC1D (4B) | LENGTH (2B) | SPP header + data (6 + L bytes) | CRC-16 (2B) |
```

- `SYNC`: fixed 4-byte marker. The receiver scans for this to (re)synchronize.
- `LENGTH`: big-endian uint16, number of bytes in the SPP header + data (i.e. `6 + L`, not
  including sync/length/CRC). Lets the deframer bound a read before parsing the SPP header.
- `body`: opaque to this library — passed through unchanged. In this project it's always a
  complete CCSDS Space Packet (6-byte header + data), per the ICD, but nothing in this repo knows
  that. Building/parsing the SPP header is the caller's job, not this library's.
- `CRC-16`: CRC-16/CCITT-FALSE (poly `0x1021`, init `0xFFFF`, no reflection, xorout `0x0000`)
  computed over everything **after** SYNC and LENGTH, up to but not including the CRC field itself.
  Test vector: `crc16_ccitt("123456789", 9) == 0x29B1`.

See [[ICD RevB — proposed revisions]] (A1/A2/A3/C8) for how these numbers were derived.

## Build

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
ctest --output-on-failure
```
