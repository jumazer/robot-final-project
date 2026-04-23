/*
 * motor_control.h
 *
 *  Created on: Apr 15, 2026
 *      Author: julius
 */

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

void init_motor_control();

void process_command(char cmd_buffer[]);

void move_forward(void);

void move_backward(void);

//void turn_left(void);
//
//void turn_right(void);

void brake_stop(void);

void stop(void);

#endif /* MOTOR_CONTROL_H_ */
