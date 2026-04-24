/*
 * motor_control.c
 *
 *  Created on: Apr 15, 2026
 *      Author: julius
 */

#include <stdint.h>

#include "stm32f091xc.h"
#include "debug.h"
#include "motor_control.h"

/*
 * Current controller-1 mapping used by this file
 *
 * PA9 -> STBY
 * PC7 -> AIN1
 * PB6 -> AIN2
 * PB5 -> BIN1
 * PB3 -> BIN2
 * PA7 -> PWMA (TIM3_CH2)
 * PA6 -> PWMB (TIM3_CH1)
 *
 * Assumption for differential drive:
 * - Channel A is left motor
 * - Channel B is right motor
 */

#define PWM_MAX          	40
#define DEFAULT_TURN_PWM 	40

/*
 * @brief Limit a PWM value to the maximum supported duty cycle
 *
 * @return pwm if within range, otherwise PWM_MAX
 */
static uint8_t clamp_pwm(uint8_t pwm) {
    if (pwm > PWM_MAX) {
        return PWM_MAX;
    }

    return pwm;
}

/*
 * @brief Disable PWM output to both motors
 *
 * Sets both TIM3 compare registers to zero so neither motor receives drive power.
 */
static void disable_motors(void) {
    TIM3->CCR2 = 0;
    TIM3->CCR1 = 0;
}

/*
 * @brief Set direction and PWM speed for motor A
 *
 * Motor A uses AIN1/AIN2 for direction and TIM3_CH2 on PA7 for PWM speed.
 */
static void set_motor_a(motor_direction_t dir, uint8_t pwm) {
    pwm = clamp_pwm(pwm);

    if(dir == MOTOR_DIR_FORWARD) {
        GPIOC->BSRR = GPIO_BSRR_BS_7;   // AIN1 = 1
        GPIOB->BSRR = GPIO_BSRR_BR_6;   // AIN2 = 0
        TIM3->CCR2 = pwm;               // PWMA = PA7
    } else if(dir == MOTOR_DIR_BACKWARD) {
        GPIOC->BSRR = GPIO_BSRR_BR_7;   // AIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BS_6;   // AIN2 = 1
        TIM3->CCR2 = pwm;
    } else if(dir == MOTOR_DIR_BRAKE) {
        GPIOC->BSRR = GPIO_BSRR_BR_7;   // AIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BR_6;   // AIN2 = 0
        TIM3->CCR2 = pwm;
    } else {
        GPIOC->BSRR = GPIO_BSRR_BR_7;   // AIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BR_6;   // AIN2 = 0
        TIM3->CCR2 = 0;
    }
}

/*
 * @brief Set direction and PWM speed for motor B
 *
 * Motor B uses BIN1/BIN2 for direction and TIM3_CH1 on PA6 for PWM speed.
 */
static void set_motor_b(motor_direction_t dir, uint8_t pwm) {
    pwm = clamp_pwm(pwm);

    if(dir == MOTOR_DIR_FORWARD) {
        GPIOB->BSRR = GPIO_BSRR_BR_5;   // BIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BS_3;   // BIN2 = 1
        TIM3->CCR1 = pwm;               // PWMB = PA6
    } else if(dir == MOTOR_DIR_BACKWARD) {
        GPIOB->BSRR = GPIO_BSRR_BS_5;   // BIN1 = 1
        GPIOB->BSRR = GPIO_BSRR_BR_3;   // BIN2 = 0
        TIM3->CCR1 = pwm;
    } else if(dir == MOTOR_DIR_BRAKE) {
        GPIOB->BSRR = GPIO_BSRR_BR_5;   // BIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BR_3;   // BIN2 = 0
        TIM3->CCR1 = pwm;
    } else {
        GPIOB->BSRR = GPIO_BSRR_BR_5;   // BIN1 = 0
        GPIOB->BSRR = GPIO_BSRR_BR_3;   // BIN2 = 0
        TIM3->CCR1 = 0;
    }
}

/*
 * @brief Enable the TB6612FNG motor controller
 *
 * Sets STBY high so the motor driver can respond to direction and PWM signals.
 */
void init_motor_control(void) {
    GPIOA->BSRR = GPIO_BSRR_BS_9;   // STBY = 1
}

/*
 * @brief Drive both motors with explicit direction and PWM values
 *
 * Applies the requested left and right motor commands to the motor driver.
 */
void motor_drive(motor_direction_t left_dir, uint8_t left_pwm,
                 motor_direction_t right_dir, uint8_t right_pwm) {
    set_motor_a(left_dir, left_pwm);
    set_motor_b(right_dir, right_pwm);
}

/*
 * @brief Apply a parsed drive command to the motors
 *
 * If the command pointer is invalid, the robot performs a safe brake stop.
 */
void motor_apply_command(const drive_command_t *cmd) {
    if(cmd == NULL) {
        brake_stop();
        return;
    }

    motor_drive(cmd->left_dir, cmd->left_pwm, cmd->right_dir, cmd->right_pwm);
}

/*
 * @brief Drive both motors forward at full speed
 */
void move_forward(void) {
    DBG_PRINTF("forward\r\n");
    motor_drive(MOTOR_DIR_FORWARD, PWM_MAX, MOTOR_DIR_FORWARD, PWM_MAX);
}

/*
 * @brief Drive both motors backward at full speed
 */
void move_backward(void) {
    DBG_PRINTF("backward\r\n");
    motor_drive(MOTOR_DIR_BACKWARD, PWM_MAX, MOTOR_DIR_BACKWARD, PWM_MAX);
}

/*
 * @brief Turn left by driving the two motors in opposite directions
 */
void turn_left(void) {
    DBG_PRINTF("left\r\n");
    motor_drive(MOTOR_DIR_BACKWARD, DEFAULT_TURN_PWM,
                MOTOR_DIR_FORWARD, DEFAULT_TURN_PWM);
}

/*
 * @brief Turn right by driving the two motors in opposite directions
 */
void turn_right(void) {
    DBG_PRINTF("right\r\n");
    motor_drive(MOTOR_DIR_FORWARD, DEFAULT_TURN_PWM,
                MOTOR_DIR_BACKWARD, DEFAULT_TURN_PWM);
}

/*
 * @brief Brake both motors using the motor driver's brake command state
 */
void brake_stop(void) {
    DBG_PRINTF("brake\r\n");
    motor_drive(MOTOR_DIR_BRAKE, PWM_MAX, MOTOR_DIR_BRAKE, PWM_MAX);
}

/*
 * @brief Coast stop both motors
 *
 * Removes PWM drive and places both motors into the coast/disabled state.
 */
void stop(void) {
    disable_motors();
    motor_drive(MOTOR_DIR_COAST, 0, MOTOR_DIR_COAST, 0);
}
