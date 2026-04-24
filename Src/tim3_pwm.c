/*
 * init_tim3_pwm.c
 *
 *  Created on: Apr 21, 2026
 *      Author: julius
 */

#include "stm32f091xc.h"

#include "utilities.h"

// using the formula prescaler = (timer_clock / (f_pwm * (ARR+1)))
// f_pwm = 5000
#define PWM_ARR				99
#define PWM_PRESCALER		48

/*
 * @brief Initialize TIM3 PWM outputs for motor speed control
 *
 * Configures PA7 as TIM3_CH2 and PA6 as TIM3_CH1 using alternate function 1.
 * TIM3 is configured for PWM mode with both channels starting at 0% duty cycle.
 *
 * These PWM outputs drive the TB6612FNG motor controller:
 *   - TIM3_CH2 / PA7 -> PWMA
 *   - TIM3_CH1 / PA6 -> PWMB
 */
void init_tim3_pwm(void) {
    // Enable the clock to GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // initialize PA7 with alternate function 1: TIM3_CH2
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER7, ESF_GPIO_MODER_ALT_FUNC);
    MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFRL7, 1);

    // initialize PA6 with alternate function 1: TIM3_CH1
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER6, ESF_GPIO_MODER_ALT_FUNC);
    MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFRL6, 1);

    // Configure TIM3 counter and prescaler
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // enable clock for TIM3
    TIM3->PSC = PWM_PRESCALER - 1; // set the prescaler
    TIM3->ARR = PWM_ARR;
    TIM3->CR1 = 0;

    // Configure TIM3 channel 2
    TIM3->CCR2 = 0;
    MODIFY_FIELD(TIM3->CCMR1, TIM_CCMR1_OC2M, 6); // Select PWM mode
    TIM3->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload register
    TIM3->CCER |= TIM_CCER_CC2E; // Enable channel output on OC2

    // Configure TIM3 channel 1
    TIM3->CCR1 = 0;
    MODIFY_FIELD(TIM3->CCMR1, TIM_CCMR1_OC1M, 6); // Select PWM mode
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload register
    TIM3->CCER |= TIM_CCER_CC1E; // Enable channel output on OC1

    // Generate updates
    TIM3->EGR |= TIM_EGR_UG; // Generate Update

    // Enable timer
    TIM3->CR1 |= TIM_CR1_CEN;
}
