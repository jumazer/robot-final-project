#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../Inc/parse_cmd.h"
#include "../Inc/motor_control.h"

/*
 * Test strategy:
 * - Call the real process_command() function from parse_cmd.c
 * - Stub motor_apply_command() so we can inspect what parse_cmd.c produced
 * - Link against parse_cmd.c and utilities.c only
 */

/* ---- Captured output from parser ---- */
static drive_command_t g_last_cmd;
static int g_motor_apply_call_count = 0;

static void reset_capture(void) {
    memset(&g_last_cmd, 0, sizeof(g_last_cmd));
    g_motor_apply_call_count = 0;
}

/* ---- Stub for hardware-facing function ---- */
void motor_apply_command(const drive_command_t *cmd) {
    g_last_cmd = *cmd;
    g_motor_apply_call_count++;
}

/* ---- Tests ---- */

static void test_fixed_forward(void) {
    char cmd[] = "w";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.left_pwm == 50);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_fixed_backward_uppercase(void) {
    char cmd[] = "S";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_BACKWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_BACKWARD);
    assert(g_last_cmd.left_pwm == 50);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_fixed_brake(void) {
    char cmd[] = "q";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_BRAKE);
    assert(g_last_cmd.right_dir == MOTOR_DIR_BRAKE);
    assert(g_last_cmd.left_pwm == 50);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_fixed_coast(void) {
    char cmd[] = "e";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_COAST);
    assert(g_last_cmd.right_dir == MOTOR_DIR_COAST);
    assert(g_last_cmd.left_pwm == 0);
    assert(g_last_cmd.right_pwm == 0);
}

static void test_newline_is_stripped(void) {
    char cmd[] = "w\r\n";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.left_pwm == 50);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_joystick_forward_no_turn(void) {
    char cmd[] = "F50L00";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    assert(g_last_cmd.left_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.left_pwm == 50);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_joystick_spin_right_in_place(void) {
    char cmd[] = "F00R60";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    /*
     * From your current mixing logic:
     * throttle = 0
     * turn = -35  (scaled from 60)
     * left_mix  = -35
     * right_mix =  35
     */
    assert(g_last_cmd.left_dir == MOTOR_DIR_BACKWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.left_pwm == 35);
    assert(g_last_cmd.right_pwm == 35);
}

static void test_joystick_forward_right_mix(void) {
    char cmd[] = "F50R60";

    reset_capture();
    assert(process_command(cmd) == true);
    assert(g_motor_apply_call_count == 1);

    /*
     * From your current code:
     * throttle = 50
     * turn = -35
     * left_mix  = 15
     * right_mix = 85 -> clamped to 50
     */
    assert(g_last_cmd.left_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.right_dir == MOTOR_DIR_FORWARD);
    assert(g_last_cmd.left_pwm == 15);
    assert(g_last_cmd.right_pwm == 50);
}

static void test_invalid_command(void) {
    char cmd[] = "xyz";

    reset_capture();
    assert(process_command(cmd) == false);
    assert(g_motor_apply_call_count == 0);
}

static void run_all_tests(void) {
    test_fixed_forward();
    test_fixed_backward_uppercase();
    test_fixed_brake();
    test_fixed_coast();
    test_newline_is_stripped();
    test_joystick_forward_no_turn();
    test_joystick_spin_right_in_place();
    test_joystick_forward_right_mix();
    test_invalid_command();

    printf("All parser logic tests passed.\n");
}

int main(void) {
    run_all_tests();
    return 0;
}
