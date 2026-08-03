# MCP Gateway Contract

MCP runs on a companion host, never on the ESP32. The companion may translate authenticated MCP requests to BuddyBot's versioned WebSocket protocol, but it cannot issue raw PWM, servo pulse, or bus commands.

## Safety Boundary

- The embedded `SafetySupervisor` is authoritative for every drive request.
- The local button chord E-stop overrides the companion, Wi-Fi controller, serial input, autonomy, and actions.
- A controller disconnect, lease expiry, or drive watchdog timeout faults and disarms the robot.
- A fault remains latched until its condition is healthy and the operator explicitly re-arms.
- Companion tools must report observed telemetry and safety state, not only command acceptance.

## Supported Tool Shape

The gateway should expose only capability-checked, bounded requests:

- `robot.get_status`
- `robot.get_capabilities`
- `robot.observe`
- `robot.move`
- `robot.stop`
- `robot.set_mode`
- `robot.perform_action`
- `robot.set_persona`
- `robot.run_diagnostics`
- `robot.get_event_log`

Every motion request must include a correlation ID, finite duration, and limits that fit the hardware manifest. `robot.stop` must remain idempotent and require no confirmation.

## Transport Requirements

- Authenticate the MCP client before forwarding control requests.
- Preserve the WebSocket protocol version and monotonically increasing request ID.
- Treat a safety `fault`, `estop`, stale telemetry, or lost controller lease as a failed action.
- Do not cache an `armed` result; obtain current telemetry before each movement request.
- Keep long-term memory, user preferences, and any cloud connection on the companion host.
