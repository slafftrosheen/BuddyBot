# BuddyBot Wi-Fi Protocol

BuddyBot implements a SoftAP with a captive portal that serves a WebSocket interface for browser control.

## Overview
- **SSID / Password**: Configured in `src/arduino_secrets.h` or `src/Config.h`.
- **IP**: `192.168.4.1` (SoftAP default).
- **HTTP**: Port 80 serves `WebUiPage.h` (HTML/JS/CSS).
- **WebSocket**: `ws://192.168.4.1/ws`

## JSON Command Protocol
The WebSocket accepts JSON payloads. All messages include an optional `id` for tracking and a `v` protocol version.
For control commands, a `token` must be present, obtained via pairing.

### Handshake / Pairing
`hello`: Ask for initial status
```json
{ "type": "hello" }
```

`pair`: Submit the 4-digit PIN displayed on the robot screen
```json
{ "type": "pair", "code": "1234" }
```
If successful, the robot replies with `{"type":"paired", "token":"..."}`. This token must be included in all subsequent control messages.

### Control Commands (Require Token)
`arm` / `disarm`: Enable or disable motor output
```json
{ "type": "arm", "token": "..." }
```

`move`: Drive in a specific direction for a set duration (max 250ms per message; keepalives expected)
```json
{ "type": "move", "mode": "forward", "durationMs": 250, "token": "..." }
```
Valid modes: `forward`, `reverse`, `turn_left`, `turn_right`.

`stop`: Immediately stop drive motors
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
  "obstacleSafetyState": "blocked"
}
```

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
