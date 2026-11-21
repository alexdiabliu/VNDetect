# VNDetect

Capstone venous / optical detection stack. The MSP side lives under `detection/` as an **extremely modular** firmware skeleton: think logic never includes hardware drivers.

User-facing architecture plan (Project store): see Capstone docs `msp-modular-architecture.md`.

## Layout

| Path | Role |
| --- | --- |
| `detection/config/` | Timeouts, thresholds, backend selection placeholders |
| `detection/protocol/` | 2-bit status codes + shared sample types |
| `detection/ports/` | HAL contracts (RGB, Sensor X, status out, UART) + bind registry |
| `detection/drivers/` | Swappable backends (`*_stub` builds without hardware) |
| `detection/app/think/` | Decision / state machine (Heartbeat / ALARM / FAULT) |
| `detection/app/features.*` | Sample → feature extraction |
| `detection/platform/` | Bind selected drivers at boot |
| `detection/main.c` | Poll → features → think → status |
| `clamp/` | XIAO ESP32 side (placeholder) |

## Status protocol (MSP → ESP32)

| Bits | Code | Meaning |
| --- | --- | --- |
| `00` | Heartbeat | Healthy / suspect; periodic keepalive |
| `01` | ALARM | Confirmed disturbance |
| `10` | FAULT | Sensor timeout / invalid |
| `11` | reserved | Do not send |

## Host smoke build (stubs)

```bash
make -C detection
make -C detection run
```

No MSP toolchain or hardware required. Stub drivers keep sensors “alive” and print status lines when `MSP_HOST_BUILD` is set.

## How to add or swap a hardware module

1. **Implement the port** — add `detection/drivers/<area>/your_driver.c` that fills the same vtable as the stub (`rgb_sensor_api_t`, `sensor_x_api_t`, `status_transport_api_t`, and/or `uart_api_t`).
2. **Export a constructor** — e.g. `const rgb_sensor_api_t *rgb_sensor_uart_api(void);` (declare it in `config/drivers_select.h`).
3. **Select it at bind time** — in `platform/board_bind.c`, call `rgb_sensor_bind(rgb_sensor_uart_api());` instead of the stub.
4. **Link it** — replace the stub `.c` with yours in `detection/Makefile` (or your MSP IDE project). Prefer swapping sources over `#ifdef` forests.
5. **Leave think alone** — `app/think/` only sees `ThinkInputs` / `StatusCode`. Change rules there only when product behavior changes, not when UART pins move.

Optional: real UART backends implement `uart_api_t`; framing drivers (RGB, HM-10, ESP32) call `uart_api()` so the physical serial layer swaps once for everyone.

## Think placeholder rules

- **FAULT** — any required sensor unhealthy / timed out  
- **ALARM** — both disturbance flags, or **SUSPECT** longer than `CFG_SUSPECT_TIME_MS`  
- **HEARTBEAT** — NORMAL / SUSPECT (periodic)

Thresholds live in `detection/config/board_config.h` (placeholders only — no pin maps or MSP part numbers invented here).
