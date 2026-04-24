/*
 * board.c
 *
 *  Created on: Apr 12, 2026
 *      Author: julius
 */

#include "stm32f091xc.h"
#include "utilities.h"

/*
 * @brief Initialize GPIOs for motor controller interface
 *
 * Enables GPIO clocks and configures all motor-control pins as outputs.
 * These pins drive the TB6612FNG motor driver:
 *   - STBY: standby control
 *   - AIN1/AIN2: motor A direction
 *   - BIN1/BIN2: motor B direction
 *
 * All pins are set LOW after initialization to place the motor controller
 * in a safe, inactive state (motors disabled) at startup.
 */
void init_board(void) {
    // Enable clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Configure outputs for motor controller
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER9, ESF_GPIO_MODER_OUTPUT);   // STBY for motor 1
    MODIFY_FIELD(GPIOC->MODER, GPIO_MODER_MODER7, ESF_GPIO_MODER_OUTPUT);   // AIN1 for motor 1
    MODIFY_FIELD(GPIOB->MODER, GPIO_MODER_MODER6, ESF_GPIO_MODER_OUTPUT);	// AIN2 for motor1
    MODIFY_FIELD(GPIOB->MODER, GPIO_MODER_MODER5, ESF_GPIO_MODER_OUTPUT);   // BIN1 for motor 1
    MODIFY_FIELD(GPIOB->MODER, GPIO_MODER_MODER3, ESF_GPIO_MODER_OUTPUT);   // BIN2 for motor 1


    // Set safe initial states for motor motor controller
    GPIOA->BSRR = GPIO_BSRR_BR_9;   // STBY low
    GPIOC->BSRR = GPIO_BSRR_BR_7;   // AIN1 low
    GPIOB->BSRR = GPIO_BSRR_BR_6;   // AIN2 low
    GPIOB->BSRR = GPIO_BSRR_BR_5;   // BIN1 low
    GPIOB->BSRR = GPIO_BSRR_BR_3;   // BIN2 low
}
