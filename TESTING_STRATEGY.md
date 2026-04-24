# Testing Strategy

## Purpose

This document explains how the STM32 mobile robot controller was tested and how correctness was evaluated. The testing strategy combines automated host-side unit tests with manual hardware validation because the project depends on physical components that cannot be fully reproduced in a desktop test environment.

The main goal of testing was to prove that:

- Valid commands produce the correct left and right motor behavior.
- Invalid or malformed commands are rejected safely.
- Joystick-style commands are parsed and mixed correctly.
- The communication watchdog stops the robot if command input is lost.
- The STM32, HC-05 Bluetooth module, TB6612FNG motor driver, PWM outputs, GPIO direction pins, and motor power wiring work together correctly on the physical robot.

## Testing Scope

The project was tested at three levels:

1. **Automated host-side unit tests** for portable command parsing and drive logic.
2. **Firmware-level validation** using UART debug output from the STM32.
3. **Manual hardware tests** using the fully assembled robot, Bluetooth app, motor driver, and battery power.

This split was necessary because some logic can be tested without hardware, while motor movement, Bluetooth behavior, PWM output, and watchdog behavior must be verified on the real system.

## Automated Unit Tests

The automated unit test is located at:

```text
Test/test_robot_logic.c
```

The test can be built and run from the repository root with:

```bash
gcc -IInc -ICMSIS Test/test_robot_logic.c Src/parse_cmd.c Src/utilities.c -o Test/test_robot_logic && ./Test/test_robot_logic
```

The test stubs the hardware-facing motor control layer and verifies that command parsing produces the expected `drive_command_t` values. This allows the command parser and differential drive logic to be tested on a normal development machine without flashing the STM32.

### Automated Test Coverage

The automated tests cover these areas:

| Test Area | What Is Verified |
|---|---|
| Forward command | `w` produces forward left and right motor commands. |
| Backward command | `s` produces reverse left and right motor commands. |
| Left turn command | `a` / `l` produces the expected left-turn behavior. |
| Right turn command | `d` / `r` produces the expected right-turn behavior. |
| Brake stop | `q` produces the brake stop command. |
| Coast stop | `e` produces the coast stop command. |
| Joystick forward command | Forward joystick strings produce positive throttle. |
| Joystick turn command | Turn values are mixed into left/right motor outputs. |
| Mixed drive command | Combined throttle and turn values produce differential-drive behavior. |
| Clamping | Mixed joystick values are clamped before being applied to motor PWM. |
| Invalid command | Unknown commands are rejected and do not produce unintended movement. |

## Happy-Case Testing

Happy-case testing verifies that the robot behaves correctly when given valid input.

### Manual Command Test Plan

| Step | Input | Expected Result | Result |
|---|---|---|---|
| 1 | Send `w` | Robot moves forward. | Pass |
| 2 | Send `s` | Robot moves backward. | Pass |
| 3 | Send `a` | Robot turns left. | Pass |
| 4 | Send `d` | Robot turns right. | Pass |
| 5 | Send `q` | Robot brake-stops. | Pass |
| 6 | Send `e` | Robot coast-stops. | Pass |

### Joystick Control Test Plan

The robot was also tested using the **BT Car Controller** mobile app. The HC-05 Bluetooth module receives joystick strings over Bluetooth and sends them to the STM32 through USART4.

| Step | Action | Expected Result | Result |
|---|---|---|---|
| 1 | Connect phone to HC-05 | Bluetooth connection succeeds. | Pass |
| 2 | Move joystick forward | Robot moves forward smoothly. | Pass |
| 3 | Move joystick backward | Robot moves backward smoothly. | Pass |
| 4 | Move joystick left | Robot turns left. | Pass |
| 5 | Move joystick right | Robot turns right. | Pass |
| 6 | Release joystick | Robot stops after commands stop arriving. | Pass |
| 7 | Send combined forward/right command | Right-side motor output is reduced relative to left-side output. | Pass |
| 8 | Send combined forward/left command | Left-side motor output is reduced relative to right-side output. | Pass |

## Error-Case and Corner-Case Testing

Error-case testing verifies that bad input or communication failure does not cause unsafe behavior.

| Test Case | Input / Condition | Expected Result | Result |
|---|---|---|---|
| Unknown command | Send an unsupported character such as `x` | Command is ignored or rejected; robot does not move unexpectedly. | Pass |
| Empty command | Send no command | No new movement command is applied. | Pass |
| Repeated command | Send the same valid command repeatedly | Robot continues the requested motion while commands continue arriving. | Pass |
| Command stops arriving | Stop sending commands while robot is moving | Watchdog calls `brake_stop()` after timeout. | Pass |
| Bluetooth disconnect | Disconnect controller while robot is moving | Watchdog stops the robot. | Pass |
| Joystick magnitude too large | Send a mixed command whose computed motor output exceeds allowed range | Output is clamped before being applied to PWM. | Pass |
| Direction reversal | Switch from forward to backward command | Motor direction pins update and robot changes direction. | Pass |
| Stop then move again | Send stop command, wait, then send movement command | Robot stops, then resumes motion on the next valid command. | Pass |

## Watchdog Validation

The communication watchdog was tested by sending a valid movement command and then stopping command input.

### Watchdog Test Procedure

1. Power the STM32 through USB.
2. Power the TB6612FNG motor driver from the external ~7V battery pack.
3. Connect the HC-05 Bluetooth module.
4. Send a valid movement command, such as `w`.
5. Stop sending commands.
6. Observe whether the robot stops after the watchdog timeout.

### Expected Result

The robot should not continue moving indefinitely. If no valid command is received within the timeout window, the firmware should call `brake_stop()` and keep the robot stopped until a new valid command is received.

### Observed Result

The robot stopped after command input stopped, confirming that the watchdog prevents runaway behavior when Bluetooth input is lost or interrupted.

## Hardware Integration Testing

The hardware was validated after confirming the software logic with host-side tests.

| Hardware Area | Validation Method | Expected Result | Result |
|---|---|---|---|
| STM32 power | Power board over USB | Board powers on and debug UART works. | Pass |
| HC-05 Bluetooth | Pair phone and send commands | USART4 receives Bluetooth command bytes. | Pass |
| Debug UART | View output over serial terminal | Debug messages print correctly. | Pass |
| TB6612FNG STBY | Drive PA9 high | Motor driver becomes enabled. | Pass |
| Direction GPIO | Send forward/reverse/turn commands | Direction pins produce expected motor behavior. | Pass |
| PWM outputs | Send speed commands | Motors respond to PWM speed changes. | Pass |
| Common ground | Connect STM32, HC-05, TB6612FNG, and battery grounds together | Stable communication and motor behavior. | Pass |
| Motor power | Power VM from ~7V battery pack | TT gear motors receive motor power. | Pass |

## Evidence Collected

The following evidence is included in the project repository:

- Robot build photo.
- Wiring diagram.
- Embedded system block diagram.
- Driving demo video link.
- Host-side unit test source file.
- README documentation explaining hardware, wiring, command format, software architecture, safety behavior, and testing.

## Limitations

The automated tests focus on command parsing and portable drive logic. They do not directly test STM32 peripheral registers, USART interrupts, TIM3 PWM generation, GPIO output voltage, or real motor movement. Those areas were validated through manual hardware tests and debug output because they require the physical STM32 board, HC-05 Bluetooth module, TB6612FNG motor driver, TT gear motors, and battery power.

A stronger future test setup could include logic-analyzer captures of PWM and GPIO direction pins, oscilloscope measurements of PWM duty cycle, and a scripted Bluetooth/UART command sender to automatically exercise the robot while logging debug output.

## Summary

The project was tested using both automated and manual methods. Automated tests verified command parsing, fixed commands, joystick-style commands, differential-drive mixing, clamping, and invalid command handling. Manual tests verified Bluetooth control, motor driver wiring, PWM motor response, left/right motor grouping, and watchdog-based safe stop behavior. This testing strategy covers happy-case behavior, error cases, and corner cases while accounting for the parts of the project that require physical hardware.
