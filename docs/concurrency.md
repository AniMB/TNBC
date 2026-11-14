# Concurrency & FreeRTOS Synchronization

## 1. Task Priorities & Preemption

FreeRTOS task priorities are configured in `config/defaults.h`:

- `PRIORITY_BP_WORKER = 5` (Highest application priority)
- `PRIORITY_ECG_TASK = 4` (High priority for continuous SPI acquisition)
- `PRIORITY_SPO2_TASK = 3` (Medium-high for optical sampling)
- `PRIORITY_STATUS_TASK = 2` (Medium for math/metric snapshot processing)
- `PRIORITY_POWER_TASK = 2` (Medium for battery checks)
- `PRIORITY_BLE_TASK = 2` (Medium for wireless telemetry)
- `PRIORITY_DISPLAY_TASK = 1` (Low for UI rendering)
- `PRIORITY_SD_TASK = 1` (Low for batch SD disk flushing)

## 2. Synchronization Mechanisms

### Bounded Lockless Ring Buffer (`EcgRingBuffer`)
Continuous ECG frames generated at 250-500 Hz cannot tolerate dynamic memory allocation or long lock waits. `EcgRingBuffer` implements a single-producer single-consumer circular queue:
- **Producer (ECG Task):** Writes to `buffer_[head_]`, advances `head_`. If the next position equals `tail_`, the buffer is full: an explicit `overrun_count_` is incremented and the oldest frame is preserved.
- **Consumer (Status / SD Tasks):** Reads from `buffer_[tail_]`, advances `tail_`.

### Bounded Queues
- **SpO₂ Samples:** Bounded FreeRTOS queue (32 samples capacity).
- **Device Events:** Bounded FreeRTOS queue (16 events capacity).

### Mutex Protection (`ISpiBus`)
Shared access to the SPI peripheral is serialized using `ISpiBus::lock(timeout_ms)` and `unlock()`.
- Maximum lock duration is bounded to 20 ms.
- No task may call `vTaskDelay()` or wait for an asynchronous event while holding the SPI mutex.

## 3. ISR Context Rules

1. **BP Button ISR (`handleBpButtonIsr`):** Sets an edge flag or sends a task notification using FreeRTOS `vTaskNotifyGiveFromISR`. Does NOT execute debounce delays, serial logging, or pump control.
2. **Profile Button ISR (`handleProfileButtonIsr`):** Records edge timestamps for external state machine processing.
3. **ECG DRDY ISR (`handleEcgDrdyIsr`):** Signals `EcgTask` that a new conversion is ready. Does NOT initiate SPI bus transactions in interrupt context.
