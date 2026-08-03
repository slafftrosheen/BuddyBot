# BuddyBot Wi-Fi Protocol

BuddyBot implements a SoftAP that serves a WebSocket interface for browser control.

## Overview
- **SSID / Password**: Must be provisioned in the ignored `src/arduino_secrets.h`; copy the example file and replace its placeholders before building.
- **IP**: `192.168.4.1` (SoftAP default).
- **HTTP**: Port 80 serves `WebUiPage.h` (HTML/JS/CSS).
- **WebSocket**: `ws://192.168.4.1/ws`

## JSON Command Protocol
The WebSocket accepts JSON payloads. All messages include an optional `id` for tracking and a `v` protocol version.
For control commands, a `token` must be present, obtained via pairing.

### Handshake / Pairing
Every message must include `"v": 1`. `hello`: Ask for initial status
```json
{ "type": "hello" }
```

`pair`: Submit the 8-digit pairing code displayed on the robot screen (spaces are accepted).
```json
{ "v": 1, "id": 1, "type": "pair", "code": "1234 5678" }
```
If successful, the robot replies with `{"type":"paired", "token":"..."}`. This token must be included in all subsequent control messages.

### Control Commands (Require Token)
`arm` / `disarm`: Enable or disable motor output. These and all non-stop control commands require both `id` and `token`.
```json
{ "v": 1, "id": 2, "type": "arm", "token": "..." }
```

`move`: Drive in a specific direction for a set duration (max 250ms per message; keepalives expected)
```json
{ "v": 1, "id": 3, "type": "move", "mode": "forward", "durationMs": 250, "token": "..." }
```
Valid modes: `forward`, `reverse`, `turn_left`, `turn_right`.

`stop`: Immediately stop drive motors. A paired controller may issue this with or without a token.
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
