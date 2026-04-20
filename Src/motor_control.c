/*
 * motor_control.c
 *
 *  Created on: Apr 15, 2026
 *      Author: julius
 */

#include <stm32f091xc.h>

#include "debug.h"
#include "motor_control.h"

/*
 * @brief Converts an uppercase letter to lowercase
 * Leaves all non-uppercase characters unchanged
 *
 * @param[c] Character to convert
 *
 * @return Lowercase version of the character if uppercase, otherwise original character
 */
static int to_lower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (int)(c + ('a' - 'A'));
    }
    return c;
}

void init_motor_control() {
    // Enable the motor controller TB6612
    GPIOA->BSRR = GPIO_BSRR_BS_9;	// STBY = 1
}

void process_command(char c) {
	char lower_c = to_lower(c);

	switch(lower_c) {
		case 'w':
			move_forward();
			break;
		case 'a':
			turn_left();
			break;
		case 's':
			move_backward();
			break;
		case 'd':
			turn_right();
			break;
		case 'b':
			brake_stop();
		case 'q':
			stop();
	}
}


static void enable_motors(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_7;   // PWMA = 1
    GPIOA->BSRR = GPIO_BSRR_BS_10;  // PWMB = 1
}

static void disable_motors(void) {
    GPIOA->BSRR = GPIO_BSRR_BR_7;   // PWMA = 0
    GPIOA->BSRR = GPIO_BSRR_BR_10;  // PWMB = 0
}

/*
 * Move the robot forward using control pins on the TB6612FNG
 * I define forward as having AIN1/BIN1=1, and AIN2/BIN2=0
 */
void move_forward(void) {
    GPIOC->BSRR = GPIO_BSRR_BS_7;	// AIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_5;	// BIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    enable_motors();
}

/*
 * Move the robot backward using control pins on the TB6612FNG
 * I define backward as having AIN1/BIN1=0, and AIN2/BIN2=1
 */
void move_backward(void) {
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_6;	// AIN2 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BS_3;	// BIN2 = 1

    enable_motors();
}

/*
 * Move the robot left using control pins on the TB6612FNG
 * I define channel A as the left motor and channel B as the right motor
 * To turn left turn off the left motor and turn on the right motor
 */
void turn_left(void) {
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0

    GPIOB->BSRR = GPIO_BSRR_BS_5;	// BIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    enable_motors();

    delay_cycles(5000000);

    brake_stop();

}

void turn_right(void) {
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    GPIOC->BSRR = GPIO_BSRR_BS_7;	// AIN1 = 1
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0

    enable_motors();


    delay_cycles(5000000);

    brake_stop();
}

/*
 * Stop the robot using control pins on the TB6612FNG
 * I define brake stop as having AIN1/BIN1=0, and AIN2/BIN2=0
 */
void brake_stop(void) {
    GPIOC->BSRR = GPIO_BSRR_BR_7;	// AIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_6;	// AIN2 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_5;	// BIN1 = 0
    GPIOB->BSRR = GPIO_BSRR_BR_3;	// BIN2 = 0

    enable_motors();
}

/*
 * Stop the robot using control pins on the TB6612FNG
 * I define stop as disabling the motors
 */
void stop(void) {
	disable_motors();
}


