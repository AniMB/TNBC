# SparkFun Pro nRF52840 Mini — Board Support Provenance

This directory contains the board definition and variant files required to compile sketches for the **SparkFun Pro nRF52840 Mini** (SparkFun DEV-15025) using the **Adafruit nRF52 Arduino Core** (version `1.7.0`).

## Upstream Provenance

- **Source Repository:** [sparkfun/nRF52840_Breakout_MDBT50Q](https://github.com/sparkfun/nRF52840_Breakout_MDBT50Q)
- **Component Paths:**
  - `Firmware/Arduino/variants/sparkfun_nrf52840_mini/variant.h`
  - `Firmware/Arduino/variants/sparkfun_nrf52840_mini/variant.cpp`
  - `Firmware/Arduino/sparkfun_boards.txt`
- **Module:** Raytac MDBT50Q-P1M (Nordic nRF52840 SoC)
- **Target FQBN:** `adafruit:nrf52:sparkfunnrf52840mini`

## Upstream License and Attribution

- `variant.h` and `variant.cpp`: Copyright (c) 2014-2015 Arduino LLC, Copyright (c) 2016 Sandeep Mistry, Copyright (c) 2018 Adafruit Industries, and SparkFun Electronics. Licensed under the GNU Lesser General Public License (LGPL) v2.1.
- `boards.txt.fragment`: Provided by SparkFun Electronics under the Open Source Hardware license / MIT.

## Packaging and Idempotent Overlay

The Adafruit nRF52 core does not bundle the SparkFun Pro nRF52840 Mini profile by default. The script `scripts/install_board_support.py` overlays this variant and appends the board entry to `boards.txt` in an idempotent manner without destroying other board definitions.
