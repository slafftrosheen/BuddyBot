# Halo BuddyBot

## Robotic Embodiment Runtime

BuddyBot is the physical robotic embodiment of the Halo platform.

Halo owns perception, cognition, reasoning, memory, planning, and high-level intent generation.
BuddyBot owns physical state, hardware capabilities, deterministic intent validation, command execution, and physical safety.

BuddyBot is a physical embodiment host for Halo.

### Architecture Overview

```text
    Halo Cognitive Runtime
            |
            | Robot Intent
            v
    Halo BuddyBot Runtime
            |
       IntentResolver
            |
       RobotCommand
            |
      CommandExecutor
            |
      ControlRouter
            |
     SafetySupervisor
            |
            v
           HAL
            |
          Robot
```

**Critical Invariant:**
Halo can request behavior.
BuddyBot decides whether that request can be translated into a physical command and SafetySupervisor remains the final authority.

BuddyBot must remain safe and deterministic even if Halo disconnects, crashes, produces invalid requests, or becomes unavailable.

## Role in Halo

BuddyBot is one possible physical embodiment of Halo. Conceptually:

```text
    Halo
      |
      +-- Smart glasses
      +-- Audio wearables
      +-- Phone host
      +-- BuddyBot
      +-- Future embodiments
```

BuddyBot is the embodiment that provides:
- locomotion
- manipulators
- range sensing
- IMU
- physical actions
- local safety enforcement

## Cognitive Boundary

The boundary between Halo and BuddyBot relies on the exchange of state and intent:

```text
Halo → RobotIntent → BuddyBot
```

and:

```text
BuddyBot → RuntimeSnapshot / RuntimeCapabilities → Halo
```

The two directions are:

**OBSERVE:**
```text
    BuddyBot
      |
      +-- RuntimeSnapshot
      +-- RuntimeCapabilities
      |
      v
    Halo
```

**ACT:**
```text
    Halo
      |
      +-- RobotIntent
      |
      v
    BuddyBot
      |
      +-- IntentResolver
      +-- SafetySupervisor
      |
      v
    physical robot
```

Halo does not directly control motors.
Halo does not access BuddyBot HAL internals.
Halo does not bypass SafetySupervisor.

## Documentation

- [Architecture Details](docs/ARCHITECTURE.md): Canonical explanation of boundaries.
- [Halo Embodiment Gateway](docs/HALO_EMBODIMENT_GATEWAY.md): Optional integration overview.
- [Local Control Protocol](docs/WIFI_PROTOCOL.md): Legacy WebSocket low-level API.
- [Configuration Guide](docs/CONFIGURATION.md): Physical config.
- [Safety Test Plan](docs/SAFETY_TEST_PLAN.md): Physical commissioning.
