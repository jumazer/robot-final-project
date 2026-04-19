/*
 * cbfifo.h
 *
 *  Created on: Apr 17, 2026
 *      Author: julius
 */

#ifndef CBFIFO_H_
#define CBFIFO_H_

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_SIZE 256
typedef struct {
	unsigned char buffer[QUEUE_SIZE];
	uint8_t head;
	uint8_t tail;
	int length;
} cbfifo;

bool cb_empty(cbfifo *cb);

bool cb_full(cbfifo *cb);

int cb_length(cbfifo *cb);

void cb_init(cbfifo *cb);

int cb_enqueue(cbfifo *cb, unsigned char c);

unsigned char cb_dequeue(cbfifo *cb);

#endif /* CBFIFO_H_ */
