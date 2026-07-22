# Changelog

All notable changes to this project will be documented in this file.

## [0.3.0-beta.1] - 2026-07-22
### Added
- Added modular HAL to separate API commands from hardware specifics.
- Added local Wi-Fi controller/observer design (SoftAP + WebSocket).
- Added Unit 8Servos 4WD + manipulator layout.
- Added non-blocking actions (wave, dance, celebrate, greet, look).
- Added Sonic safety design and assisted-autonomy.

### Changed
- Reworked `AutonomyManager` into a non-blocking state machine.
- Replaced delays and blocking motion commands with timestamp-based state updates.
- Refactored `Renderer` to decouple from hardware, showing visual state for autonomy, safety, and action transitions.

### Security & Safety
- Safety defaults remain locked (`ALLOW_MOTOR_ARMING = false`). Firmware requires physical bench-testing before enablement.
- Reduced dependencies on dynamic allocation and RTTI.
