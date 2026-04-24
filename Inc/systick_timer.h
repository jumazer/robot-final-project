/*
 * systick_timer.h
 *
 *  Created on: Apr 23, 2026
 *      Author: julius
 */

#ifndef SYSTICK_TIMER_H_
#define SYSTICK_TIMER_H_

#include <stdbool.h>

typedef uint32_t ticktime_t;

void init_systick(void);

ticktime_t now(void);

bool time_reached(ticktime_t now_t, ticktime_t deadline_t);

#endif /* SYSTICK_TIMER_H_ */
