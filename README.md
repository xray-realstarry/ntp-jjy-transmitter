# ntp-jjy-transmitter
An NTP-synchronized JJY (40kHz/60kHz) signal generator for radio-controlled clocks.

## Status

Early development. See [docs/requirements.md](docs/requirements.md) and
[docs/bom.md](docs/bom.md).

- `lib/jjy_encoder/` – hardware-independent JJY time code encoder (unit-tested on the host)
- `src/` – firmware (PlatformIO, Arduino-ESP32); currently a placeholder
- `tests/` – host unit tests

## Host tests

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
```

## Firmware

```sh
pio run
```
