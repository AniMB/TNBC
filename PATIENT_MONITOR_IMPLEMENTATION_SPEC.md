# Patient Monitor Firmware — GitHub Implementation Specification

> **Status:** Architecture and coding-agent handoff, version 1.0 (2026-11-14).  
> **Target:** SparkFun Pro nRF52840 Mini, Adafruit nRF52 Arduino core, C++ and FreeRTOS.  
> **Audience:** A software engineer or AI coding agent implementing this repository.  
> **Important:** This is a research/prototyping specification, **not** authorization to operate an unvalidated cuff or electrically connect an unvalidated device to a human.

## 0. Instructions to the implementing agent

You are the senior embedded-software engineer responsible for implementing this specification in an existing GitHub repository. **Inspect the repository first** (including `AGENTS.md`, changes, platform setup and tests), preserve existing work, and propose a small staged implementation plan. Do not overwrite files or choose hardware-specific values without evidence. Implement incrementally with runnable host tests, a firmware compile target and GitHub Actions. Report commands executed, passing/failing results and unresolved hardware dependencies.

**Definition of completion:** an organized, documented, testable C++/FreeRTOS firmware skeleton that builds for the actual board once its board definition is installed; meaningful host tests of logic; CI; explicit placeholders for unconfirmed hardware. A simulated build is not evidence that real sensors or patient-facing features work. Never claim medical accuracy, electrical safety or clinical suitability without validation.

### Non-negotiable design constraints

1. **Seven regularly scheduled FreeRTOS application tasks:** SpO₂, ECG, BLE, Display, Status Calculation, Power Monitoring and SD Card Storage.
2. **An additional event-driven BP worker task** has the highest *suitable application* priority. A **BP button GPIO ISR** wakes it. The BP measurement and pump control are **not executed in the ISR**. The worker remains blocked while idle and requests an exclusive *application mode* during BP acquisition; it must never disable interrupts globally or suspend the scheduler for the measurement duration.
3. **A second hardware button** selects demographic metadata (gender, race and age range) using an internal, explicitly defined lookup map and a debounced UI state machine. The associated ISR only reports an edge; it does not edit records or render a screen.
4. **Only one patient is monitored at a time**, but multiple patients and sessions are retained on the SD card. No sample may be written under the wrong patient or session.
5. ECG is acquired **continuously over SPI**, driven by the ECG analog front-end's data-ready indication. Its timing and buffering must be benchmarked under load. The display is also SPI; the SD interface is **to be confirmed**.
6. There is **one owner for each hardware resource** (or one explicit arbitrator): display task owns rendering; storage task owns SD files and offsets; SPI transactions on a shared bus are serialized; control of the pump is restricted to the BP subsystem.
7. Hardware safety cannot rely solely on FreeRTOS priorities, task cooperation or software timeouts. Include physical pressure limiting, safe power-off/deflation design, fault signals and a hardware-level safety review before any cuff actuation.

## 1. Scope and assumptions

| Category | Decision / open item |
|---|---|
| MCU board | **SparkFun Pro nRF52840 Mini** (verify actual revision and bootloader) |
| Firmware environment | Arduino IDE for editing/upload; **Arduino CLI** for reproducible automated builds |
| RTOS | FreeRTOS bundled with the **tested** Adafruit nRF52 Arduino platform; avoid installing a conflicting standalone FreeRTOS library |
| Language | C++ (`.cpp` implementation, `.h` declarations, minimal `.ino` entry) |
| Tested core baseline | User has an Adafruit nRF52 **1.7.0** install; freeze and validate actual compatible board definition revision before treating it as a guaranteed release |
| Board variant | Custom `sparkfun_nrf52840_mini` variant and `boards.txt` entry were manually installed locally; **must be reproducibly packaged** for contributors |
| ECG | SPI ADC/analog front-end; exact IC, sample rate, channels, SPI mode, DRDY polarity **unknown** |
| Display | SPI; exact controller, pins and driver **unknown** |
| SD card | Storage required; whether SPI or another interface **unknown** |
| SpO₂ and BP sensors | Exact ICs, sampling requirements, calibration and communications **unknown** |
| Pump/valves | Driver, pressure sensor, fault behavior and independent relief design **unknown** |
| Power source | Battery circuitry, fuel gauge or ADC scaling **unknown** |
| BLE | Nordic BLE using the compatible Arduino core stack/Bluefruit if verified; protocol, MTU and security TBD |
| Patient profile | Patient ID, session ID, gender, race, age-range metadata; permitted option sets and clinical relevance TBD |

**Do not invent** pin numbers, sensor commands, clinical algorithms, BP inflation targets, electrical isolation claims, patient-data encryption design, board FQBN or firmware compatibility. Put these behind configuration interfaces and mark `TODO(HARDWARE)` until the parts and wiring are confirmed.

## 2. Execution model

### 2.1 FreeRTOS ownership table

| Component | Trigger / execution | Responsibility | Data output |
|---|---|---|---|
| ECG task | DRDY notification or device-specific acquisition signal | Bounded SPI frame acquisition, timestamps, gap/overrun counters; never SD writes | ECG ring buffer / batch queue |
| SpO₂ task | Sensor event or periodic schedule | Acquire raw optical measurements and quality flags | SpO₂ sample queue |
| BLE task | BLE events and outbound message queue | Connection lifecycle, authenticated application messages, rate control | Telemetry, safe commands |
| Display task | Screen update notification / periodic schedule | Own display SPI transactions, render immutable status snapshot and settings menu | Display only |
| Status Calculation task | New samples / processing cadence | Transform sample data to *nonclinical prototype* metrics with validity/quality indicators | Versioned status snapshot |
| Power Monitoring task | Periodic timer/event | Battery reading, trend/threshold flags, report fault events | Power status / fault queue |
| SD Card task | Storage work queue | Sole filesystem and offset owner; write ECG batches, SpO₂, BP records, event log; manage sessions | Durable records / write status |
| **BP worker (8th task)** | BP-button ISR notification | Request BP-exclusive mode, validate safety state, execute separately reviewed cuff state machine, emit result | BP record / events |
| Arduino `loop()` task | Core-managed | Minimal diagnostics or sleep (`vTaskDelay`); no duplicate sensor operations | Optional heartbeat |
| GPIO ISR callbacks | Hardware interrupt | Mark event or use permitted `...FromISR` notify, return promptly | Notifications only |
| FreeRTOS software timers | Timer-service task | Short, nonblocking periodic notifications (e.g., power/status refresh) | Notifications only |

**Priority:** BP worker should have the highest appropriate *application-task* priority when ready, followed by time-sensitive ECG acquisition. Do not assign numeric priorities until reviewing `configMAX_PRIORITIES`, BLE internal threads/interrupts and measured timing. No task may busy-spin at the highest priority. A high-priority task that blocks can still allow lower-priority tasks to run; “BP exclusive” means a **coordinated mode**, not globally silencing the system.

### 2.2 BP button / exclusive-mode sequence

1. Button GPIO interrupt fires. ISR debounces only with ISR-safe minimal timestamp/latch policy or defers debounce to worker; uses the correct nRF52 FreeRTOS `FromISR` API **only after verifying interrupt priority compatibility**.
2. BP worker wakes and checks whether a measurement is already underway, battery/power status, sensor health, cuff safety interlocks and patient session state.
3. Central **ModeCoordinator** transitions `MONITORING → BP_PREPARE → BP_ACTIVE` only when allowed. Nonessential workloads are asked to acknowledge a quiescent state (e.g., pause routine display redraws/status processing). Preserve critical power, safety, watchdog, storage integrity and relevant peripheral operations. Decide what gaps in ECG/SpO₂ are allowed and log them explicitly.
4. BP worker runs a **separately designed and hardware-validated** nonblocking state machine. All exits, exceptions and faults demand safe pump-off/pressure-release behavior. Do **not** implement actual patient cuff control until hardware safety requirements and verified thresholds are provided.
5. Submit a timestamped result or failure event, transition through `BP_RECOVER`, restore monitoring, and block awaiting the next trigger.

Never call `delay()`, `Serial.print`, SPI/SD filesystem operations, mutex-taking functions, or lengthy control logic from the BP GPIO ISR. Never call `vTaskSuspendAll()`, `taskENTER_CRITICAL()` or disable interrupts across the BP measurement.

### 2.3 Second button / demographic map

Use a small finite-state interface: `VIEW → EDIT_FIELD → SELECT_OPTION → CONFIRM → VIEW`. A single-button UI may use short/long press, but exact gesture mappings must be documented and tested, including debounce and cancellation. Treat gender, race and age-range categories as **configurable administrative metadata** (e.g., coded enums + labels); keep `UNKNOWN` and `PREFER_NOT_TO_SAY` options where appropriate. Do not infer or assign health corrections based on race or gender. Obtain approved schema and categories before locking the map; avoid hard-coded socially reductive defaults. Patient ID and consent/authority to edit metadata must be checked before saving.

## 3. Task communications and synchronization

- Use typed, bounded FreeRTOS **queues** for discrete events and low-rate measurements.
- Use a bounded **single-producer/single-consumer ring buffer** or fixed-size batch pool for continuous ECG; define ownership transfer and memory ordering. Avoid per-sample heap allocation.
- Use direct task notifications for ISR-to-task wakeups where valid; count missed frames independently.
- Use **mutexes** only to protect actual shared resources, e.g., a common SPI bus. Never say a semaphore *stores* a pointer. The SD task owns offsets; other tasks query metadata snapshots.
- Define queue-full policy, backpressure, data-loss counters and visible diagnostics; do not silently overwrite clinical-looking data.
- Avoid holding the SPI mutex through SD card flushes or delays. If the ECG shares a bus, bound maximum transaction length and benchmark contention.
- Time-stamp at acquisition, not merely at SD write time. Establish monotonic sample timing plus wall-clock provenance (RTC or validated clock source) separately.
- Keep ISR-safe APIs, interrupt priority rules and BLE stack interaction documented for the exact board core.

### Recommended C++ interface sketches (design only)

```cpp
struct SampleEnvelope {
    uint32_t patient_id;
    uint32_t session_id;
    uint64_t monotonic_timestamp_us;
    uint32_t sequence;
    uint16_t quality_flags;
};

enum class SystemMode : uint8_t {
    BOOT, IDLE, MONITORING, BP_PREPARE,
    BP_ACTIVE, BP_RECOVER, ERROR, SHUTDOWN
};

struct DeviceEvent {
    enum class Type : uint8_t {
        BP_BUTTON, PROFILE_BUTTON, LOW_BATTERY,
        SENSOR_ERROR, STORAGE_ERROR, SESSION_CHANGE
    } type;
    uint64_t timestamp_us;
};
```

Treat the fields above as an internal contract proposal, not a finalized on-disk layout; explicit serialization, byte order, bounds and schema version are required for persisted data.

## 4. Repository layout

```text
PatientMonitor/
├── PatientMonitor.ino           # Arduino setup(), loop() only
├── README.md
├── IMPLEMENTATION_SPEC.md       # This document (or docs/IMPLEMENTATION_SPEC.md)
├── LICENSE
├── Makefile
├── .gitignore
├── sketch.yaml                  # Only when compatible with custom core packaging
├── config/
│   ├── defaults.h               # Versioned non-secret settings
│   ├── pins.example.h           # Proposed pins; verified board mapping still required
│   └── hardware_manifest.yaml   # Board revision, sensor ICs, wiring, bus allocation
├── include/patient_monitor/
│   ├── app.h
│   ├── system_mode.h
│   ├── events.h
│   ├── patient_session.h
│   ├── status_snapshot.h
│   ├── spi_bus.h
│   ├── bp.h
│   ├── profile_map.h
│   ├── ecg_task.h
│   ├── spo2_task.h
│   ├── ble_task.h
│   ├── display_task.h
│   ├── status_task.h
│   ├── power_task.h
│   └── sd_task.h
├── src/
│   ├── app.cpp
│   ├── control/mode_coordinator.cpp
│   ├── control/bp_button.cpp
│   ├── control/bp_worker.cpp
│   ├── control/profile_button.cpp
│   ├── control/profile_map.cpp
│   ├── tasks/ecg_task.cpp
│   ├── tasks/spo2_task.cpp
│   ├── tasks/ble_task.cpp
│   ├── tasks/display_task.cpp
│   ├── tasks/status_task.cpp
│   ├── tasks/power_task.cpp
│   ├── tasks/sd_task.cpp
│   ├── drivers/               # Board-/device-specific adapters
│   ├── processing/            # Hardware-independent algorithms
│   └── storage/               # Session, record and recovery logic
├── tests/
│   ├── host/                 # Standard C++ tests with mocks/fakes
│   ├── integration/          # On-device test sketches / fixtures
│   └── fixtures/             # Synthetic only; no patient records
├── hardware/
│   └── sparkfun_nrf52840_mini/ # Provenance + pinned boards.txt fragment/variant
├── scripts/
│   ├── setup.sh
│   ├── setup.ps1
│   ├── verify_toolchain.py
│   └── install_board_support.py
├── docs/
│   ├── architecture.md
│   ├── wiring.md
│   ├── concurrency.md
│   ├── storage-format.md
│   ├── safety-review.md
│   └── test-plan.md
└── .github/workflows/ci.yml
```

**Arduino compilation caveat:** A project-root `include/` directory is **not automatically** added to Arduino's include path. Firmware `.cpp` files may include it with correctly resolved relative paths, or the build can supply `-I` flags; host Make/CMake tests use their own `-Iinclude`. The `.ino` file can use `#include "include/patient_monitor/app.h"`. Arduino compiles `.cpp` files recursively in sketch `src/`; keep host tests outside `src/`. Use include guards/`#pragma once` and avoid duplicate definitions. If converting into a reusable Arduino library, follow its `library.properties`/`src/` layout and make library discovery explicit.

## 5. Entry-point contract

`PatientMonitor.ino` should be small:

```cpp
#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include "include/patient_monitor/app.h"

void setup() {
    // Logging + safe hardware states first; must return controlled status.
    patient_monitor::initialize();
}

void loop() {
    // Core already runs loop() inside a FreeRTOS task.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

`initialize()` owns the strict order: set pump/valves to verified safe startup state → validate configuration → initialize time and bus controllers → initialize queues/buffers → initialize storage/session manager → initialize sensor/communications drivers → create seven tasks and BP worker, checking every task result and stack allocation → attach buttons/DRDY interrupts only after task handles are valid → enable periodic timer notifications → transition to `IDLE`. Startup failure must be visible, fail safe and not attempt cuff inflation. The Adafruit core starts the RTOS scheduler: **do not call `vTaskStartScheduler()` again**.

## 6. Data model and SD storage

**One active patient/session**; multiple historical records:

```text
SD_CARD/
├── schema.json or binary version manifest
├── patients.idx
├── PAT0001/
│   ├── profile.bin
│   ├── SESSION0001/
│   │   ├── session.meta
│   │   ├── ecg_0001.bin
│   │   ├── spo2_0001.bin
│   │   ├── bp.bin
│   │   └── events.log
│   └── SESSION0002/...
└── PAT0002/...
```

- Use file-relative offsets, **not physical SD sectors or raw memory pointers**. An offset is data maintained by the SD task, not contained in a semaphore.
- Every queued batch carries immutable patient/session identifiers; a session switch must stop/safely quiesce acquisition, drain tagged writes, flush/check/close files, then open a new session.
- Use **versioned binary records** with known endianness, sequence number, timestamp, stream ID, payload length and integrity checksum; supply an offline parser and format documentation.
- Prefer batched, append-only recording; update offsets in RAM after successful writes, periodically checkpoint recoverable metadata. Design power-loss recovery, torn-write detection, file rotation, disk-full response and unexpected card removal.
- FAT32 has a **4 GiB minus one byte** maximum individual file size; rotate files before that limit. Determine actual filesystem capabilities before implementing.
- Bound RAM use. Calculate buffer capacity against measured worst-case SD latency plus other bus and BLE contention. If loss is unavoidable, preserve explicit gap markers and loss counters.
- Consider stored records and demographic data sensitive: no identifiable patient data in Git, test fixtures, debug logs, filenames or BLE advertising. Before real use, review encryption at rest, encryption/authentication in transit, key handling, access control, retention and applicable privacy law.

## 7. Hardware interface and timing design

### ECG

Device driver exposes `begin`, `readFrame`, `selfTest`, `readError` and DRDY-ready signaling with no filesystem knowledge. Verify ADC framing, channels, sign extension, resolution, SPI mode/frequency, per-frame size, interrupt polarity and clock source against the actual datasheet. ISR must only signal. Record sequence and overrun metrics. Benchmark sample acquisition while display, SD, BLE and BP mode events are active.

### SPI and display

Do not assume three independent SPI controllers are exposed by this Arduino board variant. Document actual nRF52840 SPIM instances, pin routing and library support. Use per-device chip select, `SPI.beginTransaction(...)`/`endTransaction()` when using the Arduino SPI interface, and bus ownership/mutex if shared. Verify the SD library's bus and transaction behavior; do not hold the bus across sleeps. Keep display work rate-limited and permit safe deferral during BP-exclusive mode.

### BP

Keep hardware-facing operations behind `IBpHardware` (pressure reading, pump/valve command, independent fault indication). Implement mocked logic and safety-state transitions before the actuator driver. Button event is an interrupt; **measurement is a high-priority task**, never a long ISR. Require approved engineering limits and validated independent fail-safe mechanisms before enabling pump hardware in a build intended for human connection.

### SpO₂, power and BLE

Define device-specific drivers after choosing ICs. SpO₂ signal quality and motion artifacts must be represented explicitly. Power task reports state without monopolizing the ADC/peripheral bus; critical power cutoff must have hardware support. BLE service UUIDs, data rates, pairing/bonding and authorization are unspecified; default to **no identifiable telemetry** until a reviewed security profile is supplied.

### Demographic selection

Provide a `ProfileSelectionMap` interface: allowed fields, allowed code values, display labels, option ordering and default/unknown values. Map must be versioned; input events handled outside ISR. Save changes only on explicit confirmation with patient/session association and an audit event. Do not use demographic categories as clinical adjustment multipliers without separate validated requirements.

## 8. Repeatable developer environment

Do not require all developers to install the same Arduino **IDE**. Require all automated builds to use the **same pinned toolchain, platform, board variant and library set**.

1. Define a canonical Arduino CLI version, Adafruit core version and SHA-256/revision of the SparkFun board definition/variant; pin transitive libraries and any helper test compiler/framework.
2. Provide platform-specific `setup.ps1` (Windows PowerShell) and `setup.sh` (Linux/macOS). Installation must be idempotent, check versions and display an actionable error on mismatch. Do not overwrite an unrelated globally installed Arduino platform silently.
3. Prefer an **isolated Arduino CLI data directory or versioned custom board platform** rather than modifying the user's global `boards.txt`. If the tested SparkFun definition must be overlaid onto `adafruit:nrf52@1.7.0`, create a *local copy*, patch exact anchored lines idempotently and verify its hash. Respect upstream licenses and include attribution.
4. Verify the **actual** board FQBN with `arduino-cli board listall` before writing it into CI; never assume `adafruit:nrf52:sparkfunnrf52840mini` is correct merely because it was used illustratively in chat.
5. For Arduino CLI profiles (`sketch.yaml`), test their interaction with the custom platform before relying on their isolation; stock Adafruit profiles do not magically include manual SparkFun variant edits.
6. `make upload` must accept a per-machine port/serial target as an argument or environment variable; no contributor-specific `COM5`, `/dev/ttyACM0` or local path may be committed.
7. Document bootloader, SoftDevice/BLE core and upload method compatibility; compiling and uploading are distinct validation steps.

Target Make interface:

| Command | Expected behavior |
|---|---|
| `make setup` | Check/install approved dependencies and custom board support |
| `make doctor` | Print tool, core, FQBN, libraries and hardware-manifest validation |
| `make test` | Compile/run host unit tests; no physical board needed |
| `make firmware` | Build the Arduino sketch for the exact pinned board |
| `make upload PORT=...` | Upload only after build; explicit port; no silent autodetection |
| `make lint` | Apply formatting and static analysis where configured |
| `make check` | Run tests, lint, firmware compile and configuration audit |
| `make clean` | Remove only repository-owned build outputs |

Do not fabricate a `Makefile` that pretends host `.cpp` tests have validated hardware ISR timing. Arduino CLI still performs the firmware build internally; GNU Make is the **orchestrator**. Host tests can use a separate compiler, CMake or a lightweight testing framework.

## 9. GitHub collaboration and CI

- Use a documented `main` branch, pull-request workflow, code review and required checks for merge. Do not force-push or overwrite other contributors' changes.
- `.github/workflows/ci.yml`: pin action versions; install the pinned toolchain; run `make doctor`, `make test`, `make lint` and `make firmware` with synthetic data only. Upload `.elf`/`.hex` artifacts when produced (with version information), but never release a device image as medically validated.
- Add `CONTRIBUTING.md` describing Windows/Linux/macOS setup, build commands, branching, test requirements and hardware-free contribution path.
- Add `hardware_manifest.yaml` describing board revision, pin labels, ADC/front-end IC models, voltage levels, SPI modes, CS pins, pin collisions, bootloader and power design. Unknown values must fail a *hardware-ready* check but should not prevent mocked host tests.
- Add `.gitignore` for build outputs, IDE caches, `.env`, credentials and captured/real patient data. Version-control a `.env.example` if needed, never secrets.
- Add CI checks detecting unsupported compiler/core versions, missing board variant, duplicate task definitions and unexpected uninitialized hardware config.
- Include dependency/license notices, a change log, issue templates for hardware defects and test matrix reporting.

## 10. Tests and failure injection

### Host unit tests (must run without board)

1. ECG frame parsing, sign extension, ordered timestamps, buffer wrap, overflow counters and producer-consumer correctness.
2. SpO₂ data-validity handling with synthetic input (not clinical validation).
3. Status snapshot calculations; invalid/out-of-date inputs must not appear as valid readings.
4. BP button debounce, duplicate presses, state transition rejection, timeout/fault paths and simulated pump-off/release-on-error invariants. No physical cuff actuation in CI.
5. Profile button short/long press interpretation, full menu coverage, age-range boundaries, unknown/prefer-not options, map versioning and save confirmation.
6. SD record serialization/deserialization, checksums, power-cut/torn-write recovery, file rotation, disk-full behavior, session misattribution prevention and patient switching with queued batches.
7. BLE authorization logic, malformed command handling, rate limiting and privacy-safe advertising data.
8. Power threshold behavior, invalid ADC values and fault event publication.
9. Mode coordinator exclusivity, acknowledgements, cancellation and recovery with randomized event ordering.

### On-device integration tests (real board required)

1. Confirm all eight task creations and stack high-water marks; FreeRTOS remains responsive after hours of operation.
2. Validate GPIO ISR priorities, debouncing and watchdog/safety behavior; no blocking calls in ISRs.
3. Measure ECG frame-loss rate at target data rate during concurrent SPI display, SD batch writes, BLE use and BP transitions.
4. Measure worst-case SD write stalls, queue depths, dropped batches and automatic recovery after card removal or power loss.
5. Verify patient/session record attribution after rapid but valid switching.
6. Validate pin mapping, SPI buses and voltage compatibility with the **actual hardware**.
7. Confirm independent BP hardware safeguards **before** permitting live pump tests; initially exercise a simulated load and pressure source.

## 11. Safety and privacy gates

**STOP / do not implement or enable a patient-connected release if any is missing:**

- Schematic and isolation review, leakage-current protection, appropriate medical electrical safety evaluation and sensor connection design.
- Independent BP overpressure limiting, hardware-safe deflation on software crash, stuck drive or power loss, and validated pressure-sensing failure detection.
- Defined failure response for low battery, watchdog reset, SPI errors and SD-full conditions; an SD problem must not compromise safe cuff release.
- Reviewed physiological algorithms and calibration. Raw ADC values and preliminary calculations **are not** certified clinical measurements.
- Privacy/security plan for named patient data and demographic attributes, including encryption and authorized access; regulatory review as appropriate to deployment.

Race and gender fields are demographic metadata only. Do not insert race-based adjustments to SpO₂/BP/ECG values by default. Their use in algorithms requires separate evidence, ethics and product requirements.

## 12. Work packages / implementation order

**Phase 0 — Inspect and stabilize:** verify exact board, existing files, `boards.txt` identity, bootloader, compiler and core version; create an inventory of unknown hardware fields. Do not make pin guesses.

**Phase 1 — Reproducible skeleton:** repository layout, `README`, build instructions, isolated pinned board package, `make setup/doctor/firmware/test`, GitHub Actions; `.ino` initialization compiles using safe no-op drivers.

**Phase 2 — RTOS contracts:** typed messages, mode coordinator, seven tasks + event-driven BP worker, queues/ring buffer, task creation checks, tracing and fake drivers; host tests pass.

**Phase 3 — Storage:** multi-patient session manager, immutable batch IDs, versioned record format, SD worker interface, host filesystem fake and crash-recovery tests.

**Phase 4 — Input and UI:** BP-trigger button ISR shell, demographic button ISR shell, debounce/menu/profile map and display abstraction; prove no ISR includes long-running logic.

**Phase 5 — Hardware integration:** implement tested ECG/SpO₂/display/SD/power/BLE adapters incrementally after actual components/pins are provided. Measure scheduling, buffering and bus utilization.

**Phase 6 — BP safety integration:** only after independent pressure hardware protection and reviewed specifications are delivered, integrate the BP worker/control state machine and validate on nonhuman test fixtures first.

**Phase 7 — Release readiness:** run the complete hardware integration matrix, security/privacy review, documentation, regression tests and engineering sign-off; do not call it a medical device without the required validation and approvals.

## 13. Acceptance criteria for first engineering milestone

- [ ] A clean clone on at least one supported host OS can run `make setup`, `make doctor`, `make test` and `make firmware` successfully without manually editing the global Arduino installation.
- [ ] CI reproduces the test and compile results with pinned dependencies.
- [ ] Exactly seven periodic/event-driven application-service tasks plus one sleeping BP worker are modeled, with checked creation, explicit ownership and no tasks started repeatedly from `loop()`.
- [ ] Two button ISR entry points exist and only notify/defer; no pump control, filesystem writes or lengthy work occurs in ISR context.
- [ ] Simulated BP-exclusive mode preserves modeled critical safety services and restores normal mode on faults.
- [ ] ECG buffer supports continuous simulated production with measured overrun counters; SD writer handles concurrent synthetic data and correct session IDs.
- [ ] Patient metadata UI and internal map are versioned, test-covered and do not assume a clinical correction rule.
- [ ] A fresh patient/session switch demonstrably cannot misfile queued samples.
- [ ] `README` differentiates *host-tested*, *firmware compiled*, *on-device tested* and *human-use validated* claims.
- [ ] Unknown physical hardware choices remain clearly marked; simulated drivers never silently masquerade as real drivers.

## 14. Required report format after each implementation phase

The implementing agent should provide:

1. **Files changed** and the architectural intention of each change.
2. **Build and test commands** with actual results, including warnings and failed cases.
3. **Hardware assumptions and blockers**, with the exact data sheet / pin map / safety artifact needed next.
4. **Remaining risks** (especially ECG sample loss, SPI contention, BP control and patient-record mix-up).
5. **Next smallest safe milestone**, without claiming unperformed validation.

---

## Appendix A — Required information before real-device drivers

Request from the hardware owner: exact SparkFun board SKU/revision; installed bootloader; complete wiring/pin map; ECG front-end part number and data rate; display controller; SD module and filesystem; SpO₂ IC; pressure sensor and its ADC/interface; compressor/pump, valve driver, independently limited pressure source, relief mechanism and safety circuitry; battery/fuel-gauge design; intended BLE clients; expected recording duration; patient-ID flow; permitted demographic option list; selected clock/RTC and privacy model.

## Appendix B — Authoritative references to consult when implementing

- Arduino CLI documentation: https://docs.arduino.cc/arduino-cli/
- Arduino platform specification: https://docs.arduino.cc/arduino-cli/platform-specification/
- Arduino sketch build specification: https://docs.arduino.cc/arduino-cli/sketch-build-process/
- SparkFun nRF52840 Arduino setup: https://learn.sparkfun.com/tutorials/nrf52840-development-with-arduino-and-circuitpython/arduino-installation
- Adafruit nRF52 Arduino core source: https://github.com/adafruit/Adafruit_nRF52_Arduino
- FreeRTOS documentation: https://www.freertos.org/
- Nordic nRF52840 product documentation: https://www.nordicsemi.com/Products/nRF52840

Recheck current upstream documentation and the exact installed core before taking API/priority assumptions into production firmware.
