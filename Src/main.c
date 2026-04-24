#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "debug.h"
#include "board.h"
#include "usart.h"
#include "tim3_pwm.h"
#include "motor_control.h"
#include "parse_cmd.h"
#include "cbfifo.h"
#include "systick_timer.h"
#include "stm32f091xc.h"

#define CMD_TIMEOUT_TICKS 40

static char cmd_buffer[QUEUE_SIZE];
static uint16_t buffer_index = 0;

/*
 * @brief Main entry point for the robot controller
 *
 * Initializes all hardware peripherals (GPIO, USART, PWM, motor control,
 * and SysTick timer), then enters the main control loop.
 *
 * The main loop:
 *   - Receives Bluetooth commands via USART4
 *   - Parses commands into motor drive actions
 *   - Applies commands to the motor controller
 *   - Feeds a communication watchdog on valid commands
 *
 * If no valid command is received within a timeout window, the system
 * performs a safe stop to prevent uncontrolled motion.
 *
 * This design separates interrupt-driven input (USART + FIFO) from
 * command processing in the main loop.
 */
int main(void)
{
	ticktime_t last_cmd_time;
	bool timed_out = false;

	init_board();
	init_usart2();
	init_usart4();
	init_tim3_pwm(); // using PA6 and PA7 for TIM3
	init_motor_control();
	init_systick();
	last_cmd_time = now();

	DBG_PRINTF("STARTING PROGRAM!\r\n");

	while(1) {
	    if (bluetooth_try_getcommand(cmd_buffer, &buffer_index)) {
	        if(process_command(cmd_buffer)) {
	            last_cmd_time = now();
	            timed_out = false;
	            DBG_PRINTF("watchdog fed\r\n");
	        }

	        buffer_index = 0;
	        memset(cmd_buffer, 0, QUEUE_SIZE);
	    }

	    if (!timed_out && time_reached(now(), last_cmd_time + CMD_TIMEOUT_TICKS)) {
	        DBG_PRINTF("Command timeout - safe stop\r\n");
	        brake_stop();
	        timed_out = true;
	    }
	}
}
