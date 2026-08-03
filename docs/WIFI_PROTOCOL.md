# BuddyBot Wi-Fi Protocol

BuddyBot implements a SoftAP that serves a WebSocket interface for browser control.

## Overview
- **SSID / Password**: Must be provisioned in the ignored `src/arduino_secrets.h`; copy the example file and replace its placeholders before building.
- **IP**: `192.168.4.1` (SoftAP default).
- **HTTP**: Port 80 serves `WebUiPage.h` (HTML/JS/CSS).
- **WebSocket**: `ws://192.168.4.1/ws`

## JSON Command Protocol
The WebSocket accepts bounded JSON objects. Every message includes `"v": 1`; authenticated control commands also require a positive, strictly increasing `id` and the session `token` obtained through pairing. Unknown fields, wrong types, fragmented frames, binary frames, oversized frames, and replayed command IDs are rejected.

### Handshake / Pairing
Every message must include `"v": 1`. `hello`: Ask for initial status
```json
{ "v": 1, "id": 1, "type": "hello" }
```

`pair`: Submit the 8-digit pairing code displayed on the robot screen (spaces are accepted).
```json
{ "v": 1, "id": 1, "type": "pair", "code": "1234 5678" }
```
If successful, the robot replies with `{"type":"paired", "token":"..."}`. This token must be included in all subsequent control messages.

### Control Commands (Require Token)
`arm` / `disarm`: Enable or disable motor output. These and all non-stop control commands require both `id` and `token`. Arming is still denied unless the embedded safety state is `disarmed`, hardware is available, and the firmware arming switch is enabled.
```json
{ "v": 1, "id": 2, "type": "arm", "token": "..." }
```

`move`: Drive in a specific direction for a required bounded duration (50–250ms). Clients must refresh held movement before it expires.
```json
{ "v": 1, "id": 3, "type": "move", "mode": "forward", "durationMs": 250, "token": "..." }
```
Valid modes: `forward`, `reverse`, `turn_left`, `turn_right`.

`stop`: Immediately stops the drive. A paired controller may issue it with or without a token; it is intentionally idempotent and is not blocked by request sequencing.
```json
{ "type": "stop", "token": "..." }
```

`action`: Trigger an animation/manipulator sequence
```json
{ "type": "action", "action": "wave", "token": "..." }
```
Valid actions: `wave`, `look_left`, `look_right`, `greet`, `celebrate`, `dance`, `sleep`.

`mood`: Change the robot's expression
```json
{ "type": "mood", "mood": "happy", "token": "..." }
```
Valid moods: `idle`, `happy`, `curious`, `sleepy`, `excited`, `alert`.

## Telemetry
The robot continuously broadcasts telemetry via WebSocket to all connected clients (no token required).
```json
{
  "type": "telemetry",
  "revision": 1234,
  "motorsArmed": false,
  "rangeMm": 120,
  "obstacleSafetyState": "blocked",
  "safetyState": "fault",
  "safetyFault": "range_sensor_invalid"
}
```

Telemetry also includes `protocolVersion`, `configSchemaVersion`, `hardwareManifestVersion`, and `safetyPolicyVersion` so a companion controller can reject incompatible firmware. When the built-in BMI270 is available, telemetry includes `imuAvailable`, `imuValid`, `imuSampleTimeMs`, `accelG` (`x`, `y`, `z` in g), and `gyroDps` (`x`, `y`, `z` in degrees per second). Autonomous behavior is denied on unhealthy IMU data.

## Session Liveness and Limits

- A token-bearing `ping` with a new request ID renews the controller lease.
- Movement is stopped, faulted, and disarmed when the drive watchdog expires after `WIFI_DRIVE_WATCHDOG_MS`.
- Controller disconnect and lease expiry also fault and disarm the robot.
- Pair failures are globally rate-limited across reconnects. Pairing enters a temporary lockout after the configured failure threshold.
- Unpaired sockets are evicted after `WIFI_UNPAIRED_CLIENT_TIMEOUT_MS`; do not hold idle observer connections indefinitely.

## System Events
The robot broadcasts system events for diagnostics and logging:
```json
{
  "type": "events",
  "events": [
    { "ts": 12345, "sev": "WARN", "code": "drive_watchdog" }
  ]
}
```
