# Hardware Wiring & Interface Specification

## 1. SparkFun Pro nRF52840 Mini Pin Allocations

All sensor and actuator pin mappings are currently **unconfirmed** and gated by `TODO(HARDWARE)`. The table below outlines the default onboard layout and candidate allocations:

| Signal Name | SparkFun Pin | Direction | Function / Subsystem | Status |
|---|---|---|---|---|
| **SPI_SCK** | P0.30 (Pin 30) | Output | Shared SPI Clock | Confirmed Onboard SPI |
| **SPI_MOSI**| P0.03 (Pin 3)  | Output | Shared SPI MOSI | Confirmed Onboard SPI |
| **SPI_MISO**| P0.31 (Pin 31) | Input  | Shared SPI MISO | Confirmed Onboard SPI |
| **LED_BLUE**| P0.07 (Pin 7)  | Output | Status Indicator LED | Confirmed Onboard LED |
| **CS_ECG**  | TBD            | Output | ECG Front-End Chip Select | UNCONFIRMED - TODO(HARDWARE) |
| **ECG_DRDY**| TBD            | Input  | ECG Front-End Data Ready Interrupt | UNCONFIRMED - TODO(HARDWARE) |
| **CS_DISP** | TBD            | Output | Display Controller Chip Select | UNCONFIRMED - TODO(HARDWARE) |
| **DISP_DC** | TBD            | Output | Display Data / Command Select | UNCONFIRMED - TODO(HARDWARE) |
| **CS_SD**   | TBD            | Output | SD Card SPI Chip Select | UNCONFIRMED - TODO(HARDWARE) |
| **BP_BTN**  | TBD            | Input  | BP Trigger Button (External Interrupt) | UNCONFIRMED - TODO(HARDWARE) |
| **PROF_BTN**| TBD            | Input  | Demographic Profile UI Button | UNCONFIRMED - TODO(HARDWARE) |
| **BP_PUMP** | TBD            | Output | Cuff Pump Motor Drive (PWM / Digital) | UNCONFIRMED - TODO(HARDWARE) |
| **BP_VALVE**| TBD            | Output | Rapid Deflation Valve Actuation | UNCONFIRMED - TODO(HARDWARE) |
| **BP_FAULT**| TBD            | Input  | Hardware Overpressure Trip Sense | UNCONFIRMED - TODO(HARDWARE) |
| **VBAT_ADC**| TBD            | Input  | Battery Divider Analog Input | UNCONFIRMED - TODO(HARDWARE) |

## 2. Voltage Levels & Bus Considerations

- **SoC I/O Voltage:** 3.3V logic level. Connecting 5V sensors directly to nRF52840 GPIOs will cause permanent damage. Level shifters are mandatory for 5V peripherals.
- **SPI Bus Contention:** Display, ECG, and SD card share the physical SPI bus. Chip select pins must never be pulled low simultaneously.
- **Analog Isolation:** ECG analog front-end signals require appropriate electrical isolation compliant with IEC 60601-1 before any human connection.
