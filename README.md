# Patient Monitor Firmware (TNBC)

> **Important Safety Notice:** This repository contains research and prototyping firmware for an embedded patient monitor. It is **NOT** a certified medical device and is **NOT** authorized or intended for human clinical use or unvalidated cuff actuation.

---

## Maturity & Validation Matrix

| Milestone | Status | Description |
|---|---|---|
| **Host-Tested Logic** | :white_check_mark: **PASSED** | Core state machines, bounded buffers, demographic maps, and session isolation pass host C++ unit tests. |
| **Firmware Compiled** | :white_check_mark: **VERIFIED** | Firmware compiles cleanly for `adafruit:nrf52:sparkfunnrf52840mini` with Adafruit core 1.7.0 and FreeRTOS. |
| **On-Device Tested** | :x: **PENDING** | Requires physical board bench testing and hardware pin confirmation. |
| **Human-Use Validated** | :no_entry: **STRICTLY PROHIBITED** | Unvalidated prototype; no clinical or electrical safety certification. |

---

## Target Hardware Baseline

- **Board:** SparkFun Pro nRF52840 Mini (DEV-15025)
- **Module:** Raytac MDBT50Q-P1M (Nordic nRF52840 SoC)
- **Core / Toolchain:** Adafruit nRF52 Arduino Core (`1.7.0`) with FreeRTOS
- **Board FQBN:** `adafruit:nrf52:sparkfunnrf52840mini`
- **Hardware Interfaces:** See [config/hardware_manifest.yaml](file:///c:/biomed/config/hardware_manifest.yaml) for unconfirmed pin and sensor specifications.

---

## Architectural Highlights

1. **Seven Scheduled Application Tasks + 1 Event-Driven BP Worker:**
   - **ECG Task:** Continuous SPI acquisition via AFE data-ready (DRDY) notification with bounded lockless ring buffer.
   - **SpO₂ Task:** Periodic optical sensor sampling with signal quality confidence metrics.
   - **BLE Task:** Privacy-safe outbound telemetry and authenticated command processing.
   - **Display Task:** Dedicated owner of screen SPI rendering; safely deferred during BP exclusive mode.
   - **Status Calculation Task:** Derives nonclinical prototype metrics with validity indicators.
   - **Power Monitoring Task:** Monitors battery thresholds and alerts critical power states.
   - **SD Card Storage Task:** Sole owner of filesystem offsets; writes versioned binary records with CRC32 checksums.
   - **BP Worker (8th Task):** Highest suitable application priority; woken by GPIO button ISR; operates a non-blocking coordinated mode. Never suspends the scheduler or disables global interrupts.

2. **Demographic Metadata Selection Map:**
   - Second button operates a debounced finite-state UI (`VIEW` -> `EDIT_FIELD` -> `SELECT_OPTION` -> `CONFIRM` -> `VIEW`).
   - Categorizes administrative metadata (Gender, Race, Age Range) with explicit `UNKNOWN` and `PREFER_NOT_TO_SAY` enums.
   - Strictly non-clinical: no unvalidated demographic adjustment multipliers.

3. **Multi-Patient Session Attribution:**
   - Every data batch carries immutable `SampleEnvelope` identifiers (`patient_id`, `session_id`).
   - Storage worker strictly drops and flags misattributed batches if session switching occurs.

---

## Quickstart & Build Instructions

### Prerequisites
- Python 3.8+
- Host C++ Compiler (`g++` or `clang++` with C++17 support)
- GNU Make (`make` or `mingw32-make`)
- Arduino CLI

### Automated Setup

**Windows (PowerShell):**
```powershell
.\scripts\setup.ps1
```

**Linux / macOS:**
```bash
./scripts/setup.sh
```

### Build Commands

```bash
# Verify environment and toolchain
make doctor

# Run static safety audit and ISR checks
make lint

# Compile and run host unit tests
make test

# Compile firmware binary (.hex, .elf)
make firmware

# Run comprehensive check (doctor + lint + test + firmware)
make check
```

---

## Documentation

- [IMPLEMENTATION_SPEC.md](file:///c:/biomed/IMPLEMENTATION_SPEC.md): Full architectural specification and phase requirements.
- [CONTRIBUTING.md](file:///c:/biomed/CONTRIBUTING.md): Developer guide, contribution workflow, and safety guidelines.
- [config/hardware_manifest.yaml](file:///c:/biomed/config/hardware_manifest.yaml): Hardware subsystem status and unknown inventory.
- [config/pins.example.h](file:///c:/biomed/config/pins.example.h): Pin assignment template with `TODO(HARDWARE)` gates.

## License

See [LICENSE](file:///c:/biomed/LICENSE).
Upstream SparkFun board support files licensed under GNU LGPL 2.1. See [hardware/sparkfun_nrf52840_mini/PROVENANCE.md](file:///c:/biomed/hardware/sparkfun_nrf52840_mini/PROVENANCE.md).
