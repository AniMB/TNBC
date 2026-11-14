#pragma once

#include <stdint.h>

/**
 * Proposed Pin Mapping for SparkFun Pro nRF52840 Mini
 * 
 * IMPORTANT NOTICE:
 * All pin mappings below are PROPOSED PLACEHOLDERS and marked TODO(HARDWARE).
 * Physical hardware pin routing, voltage level shifting, pull-up/pull-down
 * configurations, and interrupt polarity MUST be confirmed against actual
 * schematics before enabling live hardware.
 * 
 * Do NOT connect unverified actuators or sensors based on this template.
 */

namespace patient_monitor {
namespace pins {

// Hardware confirmation gate: set to 1 only when physical wiring is verified
#ifndef HARDWARE_PINMAP_CONFIRMED
#define HARDWARE_PINMAP_CONFIRMED 0
#endif

#if HARDWARE_PINMAP_CONFIRMED
// Confirmed physical pins would be defined here once schematic is approved
#else
// TODO(HARDWARE): Confirm physical GPIO pin allocations with hardware engineering

// Buttons
constexpr uint8_t PIN_BP_BUTTON        = 0xFF; // TODO(HARDWARE): Assign dedicated interrupt-capable pin
constexpr uint8_t PIN_PROFILE_BUTTON   = 0xFF; // TODO(HARDWARE): Assign debounced input pin

// SPI Bus (Shared or Dedicated)
// SparkFun Mini default SPI: SCK=P0.30, MOSI=P0.03, MISO=P0.31
constexpr uint8_t PIN_SPI_SCK          = 30;   // SparkFun default SCK
constexpr uint8_t PIN_SPI_MOSI         = 3;    // SparkFun default MOSI
constexpr uint8_t PIN_SPI_MISO         = 31;   // SparkFun default MISO

// Chip Selects
constexpr uint8_t PIN_CS_ECG           = 0xFF; // TODO(HARDWARE): Confirm ECG front-end CS pin
constexpr uint8_t PIN_CS_DISPLAY       = 0xFF; // TODO(HARDWARE): Confirm Display CS pin
constexpr uint8_t PIN_CS_SD            = 0xFF; // TODO(HARDWARE): Confirm SD card CS pin

// Interrupt / Data-Ready lines
constexpr uint8_t PIN_ECG_DRDY         = 0xFF; // TODO(HARDWARE): Confirm ECG DRDY interrupt pin

// Blood Pressure Subsystem Control & Fault
constexpr uint8_t PIN_BP_PUMP_ENABLE   = 0xFF; // TODO(HARDWARE): Confirm pump PWM/drive pin
constexpr uint8_t PIN_BP_VALVE_RELEASE = 0xFF; // TODO(HARDWARE): Confirm deflation valve drive pin
constexpr uint8_t PIN_BP_FAULT_SENSE   = 0xFF; // TODO(HARDWARE): Confirm hardware overpressure fault pin

// Power / Battery Monitoring
constexpr uint8_t PIN_BATTERY_ADC      = 0xFF; // TODO(HARDWARE): Confirm battery divider ADC pin

// Status Indicator LEDs (SparkFun Pro nRF52840 Mini has Blue LED on P0.07)
constexpr uint8_t PIN_LED_STATUS       = 7;    // Onboard Blue LED (PIN_LED1)

#endif // HARDWARE_PINMAP_CONFIRMED

} // namespace pins
} // namespace patient_monitor
