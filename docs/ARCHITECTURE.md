# Architecture

## 1. System Overview

Halo BuddyBot is the physical robotic embodiment of the Halo platform. Halo handles perception, cognition, memory, reasoning, planning, and high-level intent generation, while BuddyBot manages physical state, hardware capabilities, command execution, and deterministic physical safety.

## 2. Halo Cognitive Runtime

The cognitive side of the architecture runs externally to BuddyBot and provides intelligence. It issues intents rather than low-level commands.

## 3. Halo BuddyBot Embodiment Runtime

The physical embodiment host running on the robot itself. It enforces safe execution of intents provided by the Halo Cognitive Runtime.

## 4. Observation Boundary

BuddyBot provides the Halo Cognitive Runtime with `RuntimeSnapshot` and `RuntimeCapabilities`. This observation is completely read-only.

## 5. Intent Boundary

Halo generates a `RobotIntent` instead of motor commands. The `IntentResolver` on BuddyBot validates the abstract intent and produces physical commands if the action is capable and permitted.

## 6. Command Execution Boundary

The `CommandExecutor` converts resolved commands and passes them to the `ControlRouter` without managing safety rules itself.

## 7. Safety Boundary

The `SafetySupervisor` is the final physical authority. Halo cannot bypass it. Capability descriptions cannot grant permissions; only the SafetySupervisor decides physical allowance.

## 8. Transport Boundary

Transport is not the architectural boundary. Legacy WebSocket protocols (like Local Control Protocol) or future integrations do not define the cognitive contract.

## 9. Failure / Disconnect Behavior

Halo disconnect cannot disable local safety. If the Halo Cognitive Runtime crashes or the connection drops, BuddyBot will halt safely via its own autonomous safety rules.

## 10. Future Integration

Halo integration should eventually use a transport-independent embodiment contract.
