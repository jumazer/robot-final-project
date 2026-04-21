/*
 * usart2.h
 *
 *  Created on: Apr 17, 2026
 *      Author: julius
 */


#ifndef USART_H_
#define USART_H_

#include <stdbool.h>

void init_usart2(void);

void init_usart4(void);

int usart_getchar(void);

bool bluetooth_try_getcommand(char cmd_buffer[], uint16_t *buffer_index);

#endif /* USART_H_ */
