# BuddyBot

A modular robotics platform based on M5Stack ecosystem.

## Hardware Configuration
- Head: Servo (Channel 0)
- Left Arm: Servo (Channel 1)
- Right Arm: Servo (Channel 2)
- Drive System: M5Stack Roller485 (I2C)
- Accessories: Servos (Channels 5-7)
- Range Sensor: Ultrasonic (I2C)

## Roller485 Dual-Wheel Drive Setup
The default drive system for BuddyBot uses two M5Stack Roller485 units communicating via I2C.

### I2C Addresses
- **Left Roller:** `0x64`
- **Right Roller:** `0x65`

*Note: You must manually configure each Roller485 unit with these I2C addresses (e.g. using UIFlow or the Roller config tool) before attaching them to the robot.*

### Important Characteristics
- **Mode:** The library initializes the units in "Speed Mode" (`0x01`).
- **Speed Bounds:** Speeds are given in RPM (multiplied by 100 for the protocol).
- **Arming:** The motor output will only enable if `ALLOW_MOTOR_ARMING` is `true`. By default this is set to `false` for safety.
- **Differential Steering:** Forward drive sets both motors to positive RPM. Turning offsets them appropriately. The library automatically handles Wheel Inversion via `Config.h` if your mechanical mounting has them flipped.

## Actions & Autonomy
- BuddyBot has several predefined actions (Wave, Look, Dance, Celebrate, Sleep).
- An Autonomy manager uses the front-facing ultrasonic sensor to monitor for obstacles, safely back up, turn, and resume its previous drive state.
