# BuddyBot Servo Calibration Procedure

BuddyBot supports a 4-wheel continuous-rotation drive system and 4 positional manipulators (head, left arm, right arm, accessory). Before running autonomous behavior or enabling WiFi, you must verify the center point of the continuous rotation servos and the safe travel limits of the positional servos.

Calibration is only allowed via USB Serial connection for safety. Ensure the robot is elevated (wheels not touching the ground) during this procedure.

## 1. Preparation
1. Elevate the robot.
2. Connect to the robot via USB Serial at 115200 baud.
3. Make sure the battery is sufficiently charged.
4. If you have WiFi enabled, do not connect a controller client.

## 2. Unlocking Diagnostics
The diagnostic mode is locked by default to prevent accidental overriding of the hardware limits.
Send this command over Serial:
```
SERVO TEST UNLOCK
```
The robot should reply with `SERVO TEST UNLOCKED`. The safety routines will now allow direct pulse writing.

## 3. Calibrating Continuous Drive Servos (Roles 6-9)
For each wheel, you must find the pulse width where the motor stops spinning. Most continuous servos stop at 1500us.

Channels:
- 6: Front-Left
- 7: Front-Right
- 8: Rear-Left
- 9: Rear-Right

Test the stop point for the Front-Left wheel:
```
SERVO TEST 6 1500
```
If the wheel slowly spins, adjust the pulse width up or down (e.g. `1490` or `1510`) until the wheel is completely stationary.
Repeat for channels 7, 8, and 9. Update the values in `Servo8Bus` configuration if they deviate significantly from 1500us.

## 4. Calibrating Positional Manipulators (Roles 0-3)
For each joint, test the safe limits.

Channels:
- 0: Head (Pan)
- 1: Left Arm
- 2: Right Arm
- 3: Accessory

Test the left arm center point:
```
JOINT MOVE 1 90
```
Test the left arm up point:
```
JOINT MOVE 1 150
```
Test the left arm rest point:
```
JOINT REST 1
```
Check that the arm does not bind or stall at the extents. Update the default resting angles and limits in `src/Config.h` or `RobotHal.cpp` if they hit physical constraints.

## 5. Stopping the Test
When finished, lock the diagnostic mode to restore safety router control:
```
SERVO TEST STOP
```
The robot will reply `SERVO TEST STOPPED`. All servos will be rested.

## Troubleshooting
- **TEST REJECTED**: Ensure you ran `SERVO TEST UNLOCK` first.
- **WIFI Client Busy**: You cannot unlock the servos if a WiFi client is actively controlling the robot. Ensure the browser is disconnected.
- **Pulsing/Jitter**: Make sure the M5StickS3 is properly connected to the Unit 8Servos, and the I2C bus is not experiencing errors (check the Event Log in the WebUI).
