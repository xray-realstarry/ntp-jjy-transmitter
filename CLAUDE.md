# CLAUDE.md

NTP-synchronized JJY (40kHz/60kHz) transmitter on ESP32 for radio-controlled
clocks. Read `docs/requirements.md` and `docs/bom.md` first; they hold the
agreed requirements, staged hardware plan and open items.

## Conventions

- Write all code comments in English. Docs under `docs/` are in Japanese.
- Keep `lib/jjy_encoder/` free of Arduino/ESP-IDF dependencies so it stays
  unit-testable on the host.
- Firmware: PlatformIO + Arduino-ESP32 (target ESP32-C3). Generate the
  40kHz/60kHz carrier with LEDC directly (no harmonic trick).

## Build and test

- Host tests: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure`
- Firmware: `pio run` (not yet verified to build; first check is on CI/Mac)

## JJY signal facts (do not regress)

- Each second starts at full carrier amplitude for 0.8 s (bit 0), 0.5 s
  (bit 1) or 0.2 s (marker), then drops to ~10% for the rest of the second.
- Transmit one frequency at a time. HT-034RC (test clock) supports both 40
  and 60kHz and scans both when reception is forced.
- Frame layout is implemented from known JJY specs and checked with a
  hand-computed vector. It has NOT been checked against the official NICT
  spec yet (esp. LS1/LS2 and reserved bits).

## Legal

Japanese weak-radio limit: 500uV/m or less at 3 m. Keep output as low as
works, with hardware and firmware clamps. Re-verify against current law.

## Git

- Do not commit to `main` directly; use feature branches.
- Do not open a PR unless asked.
