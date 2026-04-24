/*
 * systick_timer.c
 *
 *  Created on: Apr 23, 2026
 *      Author: julius
 */

#include <stdbool.h>
#include <stdint.h>

#include "stm32f091xc.h"
#include "systick_timer.h"
#include "utilities.h"

/*
 * Current project assumption:
 * SYSCLK = 16 MHz
 * SysTick clock source = SYSCLK / 8 = 2 MHz
 * Tick frequency = 20 Hz (50 ms period)
 */

#define SYS_TICK_DEFAULT_CLK   (2000000L)
#define SYS_TICK_HZ            (20L)
#define TIMER_LOAD             ((SYS_TICK_DEFAULT_CLK / SYS_TICK_HZ) - 1)

static volatile ticktime_t global_timer = 0;

/*
 * @brief Initialize SysTick timer for periodic timekeeping
 *
 * Configures SysTick to generate interrupts at a fixed interval using
 * HCLK/8 as the clock source. Each interrupt increments a global timer
 * used for simple time tracking in the system.
 */
void init_systick(void) {
    // CLKSOURCE = 0 so SysTick clock is HCLK/8
    SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;

    // Set LOAD register
    MODIFY_FIELD(SysTick->LOAD, SysTick_LOAD_RELOAD, TIMER_LOAD);

    // Set interrupt priority
    NVIC_SetPriority(SysTick_IRQn, 3);

    // Reset current tick counter value to 0
    MODIFY_FIELD(SysTick->VAL, SysTick_VAL_CURRENT, 0);

    // Enable interrupt and counter
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/*
 * @brief Get the current system time
 *
 * @return current value of the global tick counter
 */
ticktime_t now(void) {
    return global_timer;
}

/*
 * @brief Check if a deadline has been reached
 *
 * Uses signed arithmetic to safely handle timer wraparound.
 *
 * @return true if now_t is at or past deadline_t, false otherwise
 */
bool time_reached(ticktime_t now_t, ticktime_t deadline_t) {
    return (int32_t) (now_t - deadline_t) >= 0;
}

/*
 * @brief SysTick interrupt handler
 *
 * Increments the global timer on each SysTick interrupt. This provides
 * a simple time base for delays and watchdog functionality.
 */
void SysTick_Handler(void) {
    global_timer++;
}
