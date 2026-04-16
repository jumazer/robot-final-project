/*
 * debug.h
 *
 *  Created on: Apr 8, 2026
 *      Author: julius
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

#ifdef DEBUG
  #define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
  #define DBG_PRINTF(...) do{}while(0)
#endif

#ifdef DEBUG
static inline void delay_cycles(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}
#endif

#endif /* DEBUG_H_ */
