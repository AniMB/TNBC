# Safety Review & Gate Checklist

## 1. Safety Gates Before Real Hardware Operation

No live hardware actuation (specifically blood pressure cuff inflation or direct electrical electrode connection) is permitted until the following engineering gates are signed off:

- [ ] **Gate 1: Electrical Isolation Review**
  Verification that patient-applied parts (ECG electrodes, SpO₂ optical sensor) are electrically isolated in compliance with IEC 60601-1 standards to prevent microshock or leakage currents.
- [ ] **Gate 2: Independent Overpressure Limiting**
  Verification of a hardware-level overpressure relief valve (pneumatic pop-off valve set to <= 300 mmHg) that operates independently of MCU firmware, software tasks, or battery voltage.
- [ ] **Gate 3: Fail-Safe Deflation on Power Loss or Software Crash**
  Verification that the deflation valve is normally open (NO) so that loss of power or watchdog reset automatically releases cuff pressure.
- [ ] **Gate 4: Thermal & Actuator Limits**
  Maximum pump continuous runtime limited by hardware timer to prevent cuff overinflation under MCU stall conditions.
- [ ] **Gate 5: Non-Clinical Prototype Notice**
  All algorithm outputs (heart rate, SpO₂ percentage, systolic/diastolic blood pressure) are preliminary prototype calculations, not certified clinical diagnostics.

## 2. Demographic Data Safety & Non-Discrimination Policy

- Demographic metadata (gender, race, age range) are collected purely for administrative records and research cohort identification.
- Firmware algorithms MUST NEVER apply unvalidated clinical correction factors or multipliers based on race or gender.
- All demographic queries must offer `UNKNOWN` and `PREFER_NOT_TO_SAY` options.
