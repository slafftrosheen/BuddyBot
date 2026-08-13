# BuddyBot

A modular robotics platform based on M5Stack ecosystem.

## Quick Start
1. Ensure your hardware matches one of the defined `BuildProfiles` (default is 4WD Servo8).
2. Set up credentials in `src/arduino_secrets.h` (see `docs/CONFIGURATION.md`).
3. Build and flash the `buddybot_safe` environment via PlatformIO.
4. Review `docs/SAFETY_TEST_PLAN.md` and verify the local E-stop before deploying.

## Documentation
- [Release Baseline](docs/RELEASE_BASELINE.md): Project status and definitions.
- [Build Profiles](docs/BUILD_PROFILES.md): Hardware layout configs.
- [Configuration](docs/CONFIGURATION.md): Safe defaults and credentials.
- [Wi-Fi Protocol](docs/WIFI_PROTOCOL.md): WebSocket API commands.
- [Safety Test Plan](docs/SAFETY_TEST_PLAN.md): End-of-day QA checklist.
- [MCP Gateway Contract](docs/MCP_GATEWAY.md): Companion-host integration boundary.
