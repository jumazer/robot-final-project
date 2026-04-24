/*
 * motor_control.h
 *
 *  Created on: Apr 15, 2026
 *      Author: julius
 */

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include <stdint.h>

typedef enum {
    MOTOR_DIR_COAST = 0,
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_BACKWARD,
    MOTOR_DIR_BRAKE
} motor_direction_t;

typedef struct {
    motor_direction_t left_dir;
    motor_direction_t right_dir;
    uint8_t left_pwm;
    uint8_t right_pwm;
} drive_command_t;

void init_motor_control(void);

void motor_apply_command(const drive_command_t *cmd);
void motor_drive(motor_direction_t left_dir, uint8_t left_pwm,
                 motor_direction_t right_dir, uint8_t right_pwm);

void move_forward(void);
void move_backward(void);
void turn_left(void);
void turn_right(void);
void brake_stop(void);
void stop(void);

#endif /* MOTOR_CONTROL_H_ */
