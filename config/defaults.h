#pragma once

#include <stdint.h>
#include <stddef.h>

namespace patient_monitor {
namespace config {

// Firmware versioning
constexpr uint8_t FIRMWARE_VERSION_MAJOR = 0;
constexpr uint8_t FIRMWARE_VERSION_MINOR = 1;
constexpr uint8_t FIRMWARE_VERSION_PATCH = 0;
constexpr uint32_t SCHEMA_VERSION = 1;

// Task Priorities (Higher number = higher FreeRTOS priority)
// BP Worker has highest suitable application priority to respond immediately to exclusive mode.
// ECG task is second highest due to continuous SPI DRDY timing constraints.
constexpr uint32_t PRIORITY_BP_WORKER   = 5;
constexpr uint32_t PRIORITY_ECG_TASK   = 4;
constexpr uint32_t PRIORITY_SPO2_TASK  = 3;
constexpr uint32_t PRIORITY_STATUS_TASK = 2;
constexpr uint32_t PRIORITY_POWER_TASK  = 2;
constexpr uint32_t PRIORITY_BLE_TASK    = 2;
constexpr uint32_t PRIORITY_DISPLAY_TASK= 1;
constexpr uint32_t PRIORITY_SD_TASK     = 1;

// Task Stack Sizes (in 32-bit words for FreeRTOS on ARM Cortex-M4)
constexpr uint16_t STACK_BP_WORKER    = 512;
constexpr uint16_t STACK_ECG_TASK    = 512;
constexpr uint16_t STACK_SPO2_TASK   = 384;
constexpr uint16_t STACK_STATUS_TASK = 384;
constexpr uint16_t STACK_POWER_TASK  = 256;
constexpr uint16_t STACK_BLE_TASK    = 512;
constexpr uint16_t STACK_DISPLAY_TASK= 512;
constexpr uint16_t STACK_SD_TASK     = 768;

// Queue and Buffer Capacities
constexpr size_t ECG_RING_BUFFER_CAPACITY = 256; // Bounded ring buffer for continuous ECG frames
constexpr size_t SPO2_QUEUE_CAPACITY      = 32;  // Bounded queue for SpO2 samples
constexpr size_t EVENT_QUEUE_CAPACITY     = 16;  // Asynchronous device events
constexpr size_t SD_WORK_QUEUE_CAPACITY   = 32;  // Storage requests queue
constexpr size_t STATUS_QUEUE_CAPACITY    = 8;

// Sample & Execution Cadences (in milliseconds)
constexpr uint32_t POWER_POLL_INTERVAL_MS   = 1000;
constexpr uint32_t STATUS_UPDATE_INTERVAL_MS= 500;
constexpr uint32_t DISPLAY_REFRESH_INTERVAL_MS = 200;
constexpr uint32_t SPO2_SAMPLE_INTERVAL_MS  = 40;  // 25 Hz nominal

// Debounce timings (in milliseconds)
constexpr uint32_t BUTTON_DEBOUNCE_TIME_MS  = 50;
constexpr uint32_t BUTTON_LONG_PRESS_MS     = 1000;

// Storage configuration
constexpr uint32_t MAX_SD_FILE_SIZE_BYTES   = 512 * 1024 * 1024; // 512 MiB rotation limit (well below FAT32 4 GiB)
constexpr uint32_t DEFAULT_PATIENT_ID       = 1;
constexpr uint32_t DEFAULT_SESSION_ID       = 1;

} // namespace config
} // namespace patient_monitor
