/*
 * motor_control.c
 *
 *  Created on: Apr 15, 2026
 *      Author: julius
 */

#include <stm32f091xc.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "motor_control.h"
#include "utilities.h"

static void strip_newline(char cmd_buffer[]) {
	uint16_t i = 0;
	while(cmd_buffer[i]) {
		if(cmd_buffer[i] == '\r' || cmd_buffer[i] == '\n') {
			cmd_buffer[i] = '\0';
			return;
		}
		i++;
	}
}

static void disable_motors(void) {
    TIM3->CCR2 = 0;
    TIM3->CCR1 = 0;
}

static uint8_t get_pwm_firsthalf(char cmd_buffer[]) {
	char str_pwm[3];
    str_pwm[0] = cmd_buffer[1];
    str_pwm[1] = cmd_buffer[2];
    str_pwm[2] = '\0';

    DBG_PRINTF("str_pwm in first_half is: %s\r\n", str_pwm);

	char *end;
	long val = strtol(str_pwm, &end, 10);

    if (str_pwm == end) {
    	DBG_PRINTF("no digits found\r\n");
        return 0;
    }
    if (*end != '\0') {
    	DBG_PRINTF("unexpected characters found in cmd_buffer\r\n");
    	return 0;

    }
    if (val < 0 || val > 99) {
        DBG_PRINTF("pwm out of range\r\n");
        return 0;
    }

    return (uint8_t) val;
}

static uint8_t get_pwm_lasthalf(char cmd_buffer[]) {
	char str_pwm[3];
    str_pwm[0] = cmd_buffer[4];
    str_pwm[1] = cmd_buffer[5];
    str_pwm[2] = '\0';

    DBG_PRINTF("str_pwm in lasthalf is: %s\r\n", str_pwm);

	char *end;
	long val = strtol(str_pwm, &end, 10);


    if (str_pwm == end) {
    	DBG_PRINTF("no digits found\r\n");
        return 0;
    }
    if (*end != '\0') {
    	DBG_PRINTF("unexpected characters found in cmd_buffer\r\n");
    	return 0;

    }
    if (val < 0 || val > 99) {
        DBG_PRINTF("pwm out of range\r\n");
        return 0;
    }

    return (uint8_t) val;
}

void init_motor_control() {
    // Enable the first motor controller TB6612
    GPIOA->BSRR = GPIO_BSRR_BS_9;	// STBY = 1

    // Enable the second motor controller TB6612
    GPIOC->BSRR = GPIO_BSRR_BS_0;	// STBY = 1
}

void process_command(char cmd_buffer[]) {

	strip_newline(cmd_buffer);
	uint8_t pwm_motor_firsthalf = 40;
	uint8_t pwm_motor_lasthalf = 40;
	char first_char = cmd_buffer[0];
	char command = 'q'; // default to brake stopping

	if(first_char == 'f' || first_char == 'b') {
		pwm_motor_firsthalf = get_pwm_firsthalf(cmd_buffer);
		pwm_motor_lasthalf = get_pwm_lasthalf(cmd_buffer);
	}

	if(pwm_motor_firsthalf == 40 && pwm_motor_lasthalf == 40) {
		command = first_char;
		TIM3->CCR2 = pwm_motor_firsthalf; // set to 99
		TIM3->CCR1 = pwm_motor_lasthalf; // set to 99
	} else if(pwm_motor_firsthalf != 0 && pwm_motor_lasthalf == 0) {
		TIM3->CCR2 = pwm_motor_firsthalf;
		TIM3->CCR1 = pwm_motor_firsthalf;
		command = first_char;
	} else if(pwm_motor_firsthalf == 0 && pwm_motor_lasthalf != 0) {
		pwm_motor_lasthalf = (pwm_motor_lasthalf * 99)/60;

		TIM3->CCR2 = pwm_motor_lasthalf;
		TIM3->CCR1 = pwm_motor_lasthalf;
		command = cmd_buffer[3];
	}

	DBG_PRINTF("pwm_motor_firsthalf is: %u\r\n", pwm_motor_firsthalf);
	DBG_PRINTF("pwm_motor_lasthalf is: %u\r\n", pwm_motor_lasthalf);
	DBG_PRINTF("command is: %c\r\n", command);


	if(command == 'w' || command == 'f') {
		move_forward();
	} else if(command == 'a' || command == 'l') {
		turn_left();
	} else if(command == 'd' || command == 'r') {
		turn_right();
	} else if(command == 's' || command == 'b') {
		DBG_PRINTF("In IF STMT for CMD 's'\r\n");
		move_backward();
	} else if(command == 'q') {
		brake_stop();
	} else if(command == 'e') {
		DBG_PRINTF("In IF STMT for CMD 'e'\r\n");
		stop();
	} else {
		DBG_PRINTF("Bad command sent: %s\r\n", cmd_buffer);
	}
}

/*
 * Move the robot forward using control pins on the TB6612FNG
 * I define forward as having AIN1/BIN1=1, and AIN2/BIN2=0
 */
void move_forward() {
    // motor 1
	GPIOC->BSRR = GPIO_BSRR_BS_7;	// AIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_3;	// BIN2 = 1

    // motor 2
    GPIOC->BSRR = GPIO_BSRR_BS_3; 	// AIN1 = 1
    GPIOC->BSRR = GPIO_BSRR_BR_2; 	// AIN2 = 0
    GPIOC->BSRR = GPIO_BSRR_BR_1; 	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_0; 	// BIN2 = 1
}

/*
 * Move the robot backward using control pins on the TB6612FNG
 * I define backward as having AIN1/BIN1=0, and AIN2/BIN2=1
 */
void move_backward() {
	// motor 1
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_6;	// AIN2 = 1
    GPIOB->BSRR = GPIO_BSRR_BS_5;	// BIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    // motor 2
    GPIOC->BSRR = GPIO_BSRR_BR_3; 	// AIN1 = 0
    GPIOC->BSRR = GPIO_BSRR_BS_2; 	// AIN2 = 1
    GPIOC->BSRR = GPIO_BSRR_BS_1; 	// BIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_0; 	// BIN2 = 0
}

/*
 * Move the robot left using control pins on the TB6612FNG
 * I define channel A as the left motor and channel B as the right motor
 * To turn left turn off the left motor and turn on the right motor
 */
void turn_left() {
	// motor 1
    GPIOC->BSRR = GPIO_BSRR_BS_7;	// AIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0
}

void turn_right() {
	// motor 2
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_3;	// BIN2 = 1
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
}

/*
 * Stop the robot using control pins on the TB6612FNG
 * I define brake stop as having AIN1/BIN1=0, and AIN2/BIN2=0
 * Brake stop means gradually stop the motors
 */
void brake_stop() {
	// motor 1
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    // motor 2
    GPIOC->BSRR = GPIO_BSRR_BR_3; 	// AIN1 = 0
    GPIOC->BSRR = GPIO_BSRR_BR_2; 	// AIN2 = 0
    GPIOC->BSRR = GPIO_BSRR_BR_1; 	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_0; 	// BIN2 = 0

    // Apply full PWM while braking
    TIM3->CCR2 = TIM3->ARR;
    TIM3->CCR1 = TIM3->ARR;
}

/*
 * Stop the robot using control pins on the TB6612FNG
 * I define stop as disabling the motors
 * Stop means to completely shut off the motors and not do a gradual stop
 */
void stop() {
	disable_motors();
}


