/*
 * cbfifo.c
 *
 *  Created on: Apr 17, 2026
 *      Author: julius
 */

#include <stddef.h>
#include <stm32f091xc.h>

#include "cbfifo.h"
#include "usart.h"


#define CBFIFO_SIZE 256

bool cb_empty(cbfifo *cb) {
	if(cb->length == 0) {
		return true;
	} else {
		return false;
	}
}

bool cb_full(cbfifo *cb) {
	if(cb->length == CBFIFO_SIZE) {
		return true;
	} else {
		return false;
	}
}

int cb_length(cbfifo *cb) {
	return cb->length;
}

void cb_init(cbfifo *cb) {
	for(int i = 0; i < CBFIFO_SIZE; i++) {
		cb->buffer[i] = 0;
	}

	cb->head = 0;
	cb->tail = 0;
	cb->length = 0;
}

int cb_enqueue(cbfifo *cb, unsigned char c) {
    if(cb == NULL) return -1;

    // save current masking state
    uint32_t masking_state = __get_PRIMASK();
	// disable interrupts
	__disable_irq();
    if(!cb_full(cb)) {
    	cb->buffer[cb->tail] = c;
    	// update variables
    	cb->tail = (cb->tail + 1) & (CBFIFO_SIZE - 1);
    	cb->length++;
    	__set_PRIMASK(masking_state);
    	return 1; // success
    } else {
    	__set_PRIMASK(masking_state);
    	return 0; // failure
    }
}

unsigned char cb_dequeue(cbfifo *cb) {
	unsigned char c = 0;

	// save current masking state
	uint32_t masking_state = __get_PRIMASK();
	// disable interrupts
	__disable_irq();
	if(!cb_empty(cb)) {
		c = cb->buffer[cb->head];
		cb->buffer[cb->head] = '_'; // Used for debugging
		// update variables
		cb->head = (cb->head + 1) & (CBFIFO_SIZE - 1);
		cb->length--;
		// restore interrupts
	}
	__set_PRIMASK(masking_state);
	return c;
}
