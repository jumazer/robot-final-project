# STM32 Mobile Robot Controller

## Overview

This project implements a UART/Bluetooth-controlled mobile robot using an STM32 Nucleo board, PWM motor control, command parsing, and a communication watchdog.

The robot receives commands over Bluetooth, parses them into left/right motor commands, and drives a TB6612FNG motor controller using GPIO direction pins and TIM3 PWM outputs.

The project was originally developed on an STM32F446 board and later ported to the STM32F091RC / Nucleo-F091RC platform.

## Features

- Interrupt-driven USART receive
- Circular buffers for UART input
- Bluetooth command input through USART4
- Debug output through USART2
- TIM3 PWM motor speed control
- GPIO motor direction control
- Fixed movement commands
- Joystick-style mixed drive commands
- Communication watchdog for safe stop behavior
- Unit tests for command parsing and robot logic
- Modular source files for board setup, USART, PWM, parsing, motor control, and timing

## Hardware

Target board:

- STM32 Nucleo-F091RC

External hardware:

- HC-05 Bluetooth module
- TB6612FNG motor driver
- DC motors
- Battery pack / external motor power
- USB connection for debug UART

## Pin Mapping

### USART

| Function | Pin | Peripheral |
|---|---:|---|
| Debug TX | PA2 | USART2_TX |
| Debug RX | PA3 | USART2_RX |
| Bluetooth TX/RX | PA0 / PA1 | USART4 |

### Motor Driver

| Function | Pin |
|---|---:|
| STBY | PA9 |
| AIN1 | PC7 |
| AIN2 | PB6 |
| BIN1 | PB5 |
| BIN2 | PB3 |
| PWMA | PA7 / TIM3_CH2 |
| PWMB | PA6 / TIM3_CH1 |

## Software Architecture

### `main.c`

Initializes the board, UARTs, PWM, motor controller, and SysTick timer. The main loop waits for Bluetooth commands, processes valid commands, and triggers a safe stop if no valid command is received before the watchdog timeout expires.

### `board.c`

Configures GPIO clocks and motor-control GPIO pins. It also places motor-control pins into a safe LOW state during initialization.

### `usart.c`

Configures USART2 for debug communication and USART4 for Bluetooth input. Receive interrupts place incoming bytes into circular buffers so the main loop can process commands without polling the USART hardware directly.

### `cbfifo.c`

Implements a fixed-size circular byte FIFO. Interrupt masking is used during enqueue and dequeue operations to protect shared buffer state between interrupt handlers and foreground code.

### `tim3_pwm.c`

Configures TIM3 PWM outputs on PA6 and PA7. These PWM outputs control motor speed through the TB6612FNG motor driver.

### `motor_control.c`

Converts high-level motor directions and PWM values into GPIO direction settings and TIM3 compare values. This module provides the main motor-control interface used by the command parser.

### `parse_cmd.c`

Parses received command strings into `drive_command_t` values. It supports both fixed commands and joystick-style commands.

### `systick_timer.c`

Provides a simple tick-based timing interface used by the communication watchdog.

## Command Format

### Fixed Commands

| Command | Behavior |
|---|---|
| `w` | Move forward |
| `s` | Move backward |
| `a` / `l` | Turn left |
| `d` / `r` | Turn right |
| `q` | Brake stop |
| `e` | Coast stop |

### Joystick Commands

Joystick commands encode throttle and turn values.

Example:

```text
f50r20
```

This means:

- forward throttle magnitude = 50
- right turn magnitude = 20

The parser converts throttle and turn into left/right motor commands using differential drive mixing:

```text
left  = throttle + turn
right = throttle - turn
```

The resulting values are clamped before being sent to the motor controller.

## Safety Behavior

The main loop includes a communication watchdog. If no valid Bluetooth command is received within the timeout window, the robot calls `brake_stop()` and remains stopped until a new valid command is received.

This prevents the robot from continuing to move if Bluetooth communication drops or the controller stops sending commands.

## Unit Testing

The project includes a host-side unit test for robot command parsing logic.

The test file is:

```text
Test/test_robot_logic.c
```

This test stubs the hardware-facing motor-control function and verifies that command parsing produces the expected drive commands.

To compile and run the test from the repository root:

```bash
gcc -IInc -ICMSIS Test/test_robot_logic.c Src/parse_cmd.c Src/utilities.c -o Test/test_robot_logic && ./Test/test_robot_logic
```

This test does not require the STM32 board because it only checks portable command-parsing logic.

## STM32F446 to STM32F091RC Port

The project was ported from STM32F446 to STM32F091RC by updating board-specific register names and peripheral mappings.

Important changes included:

- `stm32f446xx.h` → `stm32f091xc.h`
- `RCC->AHB1ENR` → `RCC->AHBENR`
- USART status/data registers changed from `SR` / `DR` to `ISR` / `RDR`
- `UART4` naming changed to `USART4`
- Alternate function values were updated for the F091RC
- F446 Cortex-M4 FPU setup was removed because the F091RC uses a Cortex-M0 core without an FPU

## Build Status

The project builds successfully for the STM32F091RC target.

Runtime validation on the final F091RC hardware was limited because the board failed during hardware bring-up, but the code was reviewed and ported to compile cleanly for the F091RC platform.

## Notes

This project uses direct register access through CMSIS device headers rather than HAL. This keeps the code close to the hardware and makes the peripheral configuration explicit.