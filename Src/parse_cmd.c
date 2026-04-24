/*
 * parse_cmd.c
 *
 *  Created on: Apr 23, 2026
 *      Author: julius
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "motor_control.h"
#include "parse_cmd.h"
#include "utilities.h"

typedef enum {
    CMD_INVALID = 0,
    CMD_DRIVE,
    CMD_BRAKE,
    CMD_COAST
} command_mode_t;

typedef struct {
    char key;
    command_mode_t mode;
    motor_direction_t left_dir;
    motor_direction_t right_dir;
    uint8_t left_pwm;
    uint8_t right_pwm;
} command_entry_t;

#define PWM_MAX               50
#define TURN_PWM              35
#define TURN_PWM_MAX          35
#define JOYSTICK_TURN_RAW_MAX 60

static const command_entry_t command_table[] = {
    { 'w', CMD_DRIVE, MOTOR_DIR_FORWARD,  MOTOR_DIR_FORWARD,  PWM_MAX,  PWM_MAX  },
    { 's', CMD_DRIVE, MOTOR_DIR_BACKWARD, MOTOR_DIR_BACKWARD, PWM_MAX,  PWM_MAX  },
    { 'a', CMD_DRIVE, MOTOR_DIR_BACKWARD, MOTOR_DIR_FORWARD,  TURN_PWM, TURN_PWM },
    { 'd', CMD_DRIVE, MOTOR_DIR_FORWARD,  MOTOR_DIR_BACKWARD, TURN_PWM, TURN_PWM },
    { 'l', CMD_DRIVE, MOTOR_DIR_BACKWARD, MOTOR_DIR_FORWARD,  TURN_PWM, TURN_PWM },
    { 'r', CMD_DRIVE, MOTOR_DIR_FORWARD,  MOTOR_DIR_BACKWARD, TURN_PWM, TURN_PWM },
    { 'q', CMD_BRAKE, MOTOR_DIR_BRAKE,    MOTOR_DIR_BRAKE,    PWM_MAX,  PWM_MAX  },
    { 'e', CMD_COAST, MOTOR_DIR_COAST,    MOTOR_DIR_COAST,    0,        0        },
};

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

static uint16_t string_length(const char *s) {
    uint16_t len = 0;

    while(s[len] != '\0') {
        len++;
    }

    return len;
}

static bool is_digit_char(char c) {
    return (c >= '0') && (c <= '9');
}

static uint8_t parse_two_digits(const char *s) {
    return (uint8_t)(((s[0] - '0') * 10) + (s[1] - '0'));
}

static uint8_t clamp_pwm(uint8_t pwm) {
    if(pwm > PWM_MAX) {
        return PWM_MAX;
    }

    return pwm;
}

static void convert_signed_mix_to_drive_command(int left_mix, int right_mix, drive_command_t *out_cmd) {
    if(left_mix > 0) {
        out_cmd->left_dir = MOTOR_DIR_FORWARD;
        out_cmd->left_pwm = (uint8_t)left_mix;
    } else if(left_mix < 0) {
        out_cmd->left_dir = MOTOR_DIR_BACKWARD;
        out_cmd->left_pwm = (uint8_t)(-left_mix);
    } else {
        out_cmd->left_dir = MOTOR_DIR_COAST;
        out_cmd->left_pwm = 0;
    }

    if(right_mix > 0) {
        out_cmd->right_dir = MOTOR_DIR_FORWARD;
        out_cmd->right_pwm = (uint8_t)right_mix;
    } else if(right_mix < 0) {
        out_cmd->right_dir = MOTOR_DIR_BACKWARD;
        out_cmd->right_pwm = (uint8_t)(-right_mix);
    } else {
        out_cmd->right_dir = MOTOR_DIR_COAST;
        out_cmd->right_pwm = 0;
    }

    out_cmd->left_pwm = clamp_pwm(out_cmd->left_pwm);
    out_cmd->right_pwm = clamp_pwm(out_cmd->right_pwm);
}

static bool parse_fixed_command(const char *cmd_buffer, drive_command_t *out_cmd, command_mode_t *out_mode) {
    size_t i;

    if(string_length(cmd_buffer) != 1) {
        return false;
    }

    for(i = 0; i < (sizeof(command_table) / sizeof(command_table[0])); i++) {
        if((char)to_lower(cmd_buffer[0]) == command_table[i].key) {
            *out_mode = command_table[i].mode;
            out_cmd->left_dir = command_table[i].left_dir;
            out_cmd->right_dir = command_table[i].right_dir;
            out_cmd->left_pwm = command_table[i].left_pwm;
            out_cmd->right_pwm = command_table[i].right_pwm;
            return true;
        }
    }

    return false;
}

static bool parse_joystick_command(const char *cmd_buffer,
		drive_command_t *out_cmd, command_mode_t *out_mode) {
    char throttle_dir;
    char turn_dir;
    uint8_t throttle_mag;
    uint8_t turn_mag;
    int throttle;
    int turn;
    int left_mix;
    int right_mix;

    if(string_length(cmd_buffer) != 6) {
        return false;
    }

    throttle_dir = (char)to_lower(cmd_buffer[0]);
    turn_dir = (char)to_lower(cmd_buffer[3]);

    if(!((throttle_dir == 'f') || (throttle_dir == 'b'))) {
        return false;
    }

    if(!((turn_dir == 'l') || (turn_dir == 'r'))) {
        return false;
    }

    if(!is_digit_char(cmd_buffer[1]) || !is_digit_char(cmd_buffer[2]) ||
        !is_digit_char(cmd_buffer[4]) || !is_digit_char(cmd_buffer[5])) {
        return false;
    }

    throttle_mag = parse_two_digits(&cmd_buffer[1]);
    turn_mag = parse_two_digits(&cmd_buffer[4]);

    if(throttle_mag > PWM_MAX) {
        throttle_mag = PWM_MAX;
    }

    if(turn_mag > JOYSTICK_TURN_RAW_MAX) {
        turn_mag = JOYSTICK_TURN_RAW_MAX;
    }

    throttle = (int)throttle_mag;
    if(throttle_dir == 'b') {
        throttle = -throttle;
    }

    turn = ((int) turn_mag * TURN_PWM_MAX) / JOYSTICK_TURN_RAW_MAX;
    if(turn_dir == 'r') {
        turn = -turn;
    }

    left_mix = throttle + turn;
    right_mix = throttle - turn;

    if(left_mix > PWM_MAX) {
        left_mix = PWM_MAX;
    } else if(left_mix < -PWM_MAX) {
        left_mix = -PWM_MAX;
    }

    if(right_mix > PWM_MAX) {
        right_mix = PWM_MAX;
    } else if(right_mix < -PWM_MAX) {
        right_mix = -PWM_MAX;
    }

    convert_signed_mix_to_drive_command(left_mix, right_mix, out_cmd);
    *out_mode = CMD_DRIVE;
    return true;
}

static bool parse_command(char cmd_buffer[], drive_command_t *out_cmd) {
    command_mode_t mode;

    strip_newline(cmd_buffer);

    if(parse_fixed_command(cmd_buffer, out_cmd, &mode)) {
        return true;
    }

    if(parse_joystick_command(cmd_buffer, out_cmd, &mode)) {
        return true;
    }

    return false;
}

bool process_command(char cmd_buffer[]) {
    drive_command_t cmd;

    if(cmd_buffer == NULL) {
        DBG_PRINTF("Bad command sent: NULL\r\n");
        return false;
    }

    if(parse_command(cmd_buffer, &cmd)) {
        DBG_PRINTF("left_dir=%d left_pwm=%u right_dir=%d right_pwm=%u\r\n",
                   cmd.left_dir, cmd.left_pwm, cmd.right_dir, cmd.right_pwm);
        motor_apply_command(&cmd);
        return true;
    } else {
        DBG_PRINTF("Bad command sent: %s\r\n", cmd_buffer);
        return false;
    }
}
