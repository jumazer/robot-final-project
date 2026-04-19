/*
 * usart.c
 *
 *  Created on: Apr 17, 2026
 *      Author: julius
 */

#include <stm32f091xc.h>

#include "utilities.h"
#include "cbfifo.h"


#define F_USART_CLOCK (24UL * 1000UL * 1000UL)
#define BAUD_RATE 115200UL

static cbfifo rx;

/*
 * @brief Initializes USART2 for serial communication
 * Configures GPIO, baud rate, parity, stop bits, FIFOs, and interrupts
 *
 * @return void
 */
void init_usart() {

	cb_init(&rx);

	// Enable USART to be used by the system clock
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// Enable GPIOA to be used by the system clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Put PA2 and PA3 in alternate mode
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER2, ESF_GPIO_MODER_ALT_FUNC);
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER3, ESF_GPIO_MODER_ALT_FUNC);
	// Set PA2 and PA3 to be USART2 pins
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL2, 1);
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL3, 1);

	// Set baud rate
	USART2->BRR = F_USART_CLOCK / BAUD_RATE;

	// Turn off parity
	MODIFY_FIELD(USART2->CR1, USART_CR1_PCE, 0);

	// Set the word length: 8 data bits
	MODIFY_FIELD(USART2->CR1, USART_CR1_M, 0);

	// 1 stop bit
	MODIFY_FIELD(USART2->CR2, USART_CR2_STOP, 0);

	// Enable interrupt generation
	MODIFY_FIELD(USART2->CR1, USART_CR1_RXNEIE, 1);

	// Enable receiver and USART
	MODIFY_FIELD(USART2->CR1, USART_CR1_RE, 1);
	MODIFY_FIELD(USART2->CR1, USART_CR1_UE, 1);

	// Enable USART2 interrupts in NVIC
	NVIC_SetPriority(USART2_IRQn, 2);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
}

static inline bool usart_char_available(void) {
	return !cb_empty(&rx);
}

int __io_getchar(void) {
	while(!usart_char_available()) { }
	return (unsigned char) cb_dequeue(&rx);
}

int usart_getchar(void) {
	return __io_getchar();
}

void USART2_IRQHandler(void) {
	unsigned char c;
	if(USART2->ISR & USART_ISR_RXNE) { // Receive buffer not empty
		// Reading from RDR will clear RXNE
		c = USART2->RDR;
		if(!cb_full(&rx)) {
			cb_enqueue(&rx, c);
		}
	}
}
