# Contributing to Patient Monitor CVTD( Cardio Vascular Triage Device)

Thank you for contributing to the Patient Monitor project. This document guides developers through setting up the environment, compiling firmware, running tests without hardware, and submitting changes.

## Safety & Prototyping Disclaimer

> [!WARNING]
> This repository is a research and prototyping firmware baseline. It is **NOT** validated or authorized for electrical connection to humans or operation of an unvalidated blood pressure cuff. Never test actuators or live cuff pumps on humans.

---

## 1. Prerequisites & Quickstart

You can contribute to all core logic, FreeRTOS contracts, state machines, and data processing **without physical hardware**.

### Required Tools

- **Git**
- **Python 3.8+**
- **Host C++ Compiler** supporting C++17 (`g++` or `clang++`)
- **GNU Make** (`make` on Linux/macOS, `mingw32-make` or `make` on Windows)
- **Arduino CLI** (for firmware compilation)

### Automated Setup

#### Windows (PowerShell)
```powershell
.\scripts\setup.ps1
```

#### Linux / macOS
```bash
./scripts/setup.sh
```

---

## 2. Make Interface

The repository provides a standardized Make workflow across all platforms:

| Command | Description |
|---|---|
| `make setup` | Install board support definitions into Adafruit nRF52 core |
| `make doctor` | Verify toolchain, compiler, board FQBN, and hardware manifest |
| `make test` | Compile and execute host unit tests (runs entirely in software) |
| `make firmware` | Build the target sketch binary (`.hex`, `.elf`) for the SparkFun board |
| `make lint` | Run static safety analysis and ISR code safety checks |
| `make check` | Run all checks (`doctor` -> `lint` -> `test` -> `firmware`) |
| `make clean` | Clean repository build outputs |
| `make upload PORT=...` | Upload compiled firmware to target hardware via specified serial port |

---

## 3. Architecture Rules for Contributors

1. **Hardware Confirmation Gates:**
   Never hardcode physical GPIO pins, ADC scaling factors, or sensor addresses without confirmation. Place hardware specifics behind interfaces with explicit `TODO(HARDWARE)` markers in `config/pins.example.h` and `config/hardware_manifest.yaml`.

2. **ISR Safety Invariants:**
   Button and DRDY interrupt service routines (ISRs) MUST only record timestamps or signal FreeRTOS tasks. Never call `delay()`, `malloc()`, `Serial.print()`, mutex operations, or filesystem functions from ISR context.

3. **Blood Pressure Exclusive Mode:**
   BP measurement is coordinated via `ModeCoordinator`. It must never call `vTaskSuspendAll()` or disable global interrupts for the duration of the cycle.

4. **Multi-Patient Session Attribution:**
   All sensor data packets carry immutable `SampleEnvelope` identifiers (`patient_id` and `session_id`). Storage logic strictly rejects batches that do not match the active session.

5. **Demographic Metadata:**
   Demographic categories (gender, race, age range) are administrative metadata with `UNKNOWN` and `PREFER_NOT_TO_SAY` options. They must never be used for unvalidated clinical correction rules.

---

## 4. Branching & Pull Requests

- Work on feature branches branched from `main`.
- Ensure `make check` passes before opening a Pull Request.
- Continuous Integration (CI) enforces `make doctor`, `make lint`, `make test`, and `make firmware` on all pull requests.
