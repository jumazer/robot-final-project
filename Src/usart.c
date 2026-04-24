/*
 * usart.c
 *
 *  Created on: Apr 17, 2026
 *      Author: julius
 */

#include <stm32f091xc.h>

#include "utilities.h"
#include "cbfifo.h"
#include "debug.h"


#define F_USART_CLOCK 			(24UL * 1000UL * 1000UL)
#define DEBUG_BAUD_RATE 		115200UL
#define BLUETOOTH_BAUD_RATE 	9600UL

static cbfifo debug_rx;
static cbfifo bluetooth_rx;

/*
 * @brief Initializes USART2 for serial communication
 * Configures GPIO, baud rate, parity, stop bits, FIFOs, and interrupts
 *
 * @return void
 */
void init_usart2() {
	cb_init(&debug_rx);

	// Enable USART2 to be used by the system clock
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
	USART2->BRR = F_USART_CLOCK / DEBUG_BAUD_RATE;

	// Turn off parity
	MODIFY_FIELD(USART2->CR1, USART_CR1_PCE, 0);

	// Set the word length: 8 data bits
	MODIFY_FIELD(USART2->CR1, USART_CR1_M, 0);

	// 1 stop bit
	MODIFY_FIELD(USART2->CR2, USART_CR2_STOP, 0);

	// Enable interrupt generation
	MODIFY_FIELD(USART2->CR1, USART_CR1_RXNEIE, 1);

	// Enable receiver and USART
	MODIFY_FIELD(USART2->CR1, USART_CR1_TE, 1);
	MODIFY_FIELD(USART2->CR1, USART_CR1_RE, 1);
	MODIFY_FIELD(USART2->CR1, USART_CR1_UE, 1);

	// Enable USART2 interrupts in NVIC
	NVIC_SetPriority(USART2_IRQn, 2);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
}

/*
 * @brief Initialize USART4 for Bluetooth receive
 *
 * Configures PA0/PA1 for USART4 alternate function mode, sets the Bluetooth
 * baud rate, enables receive interrupts, and enables the shared USART3_8 IRQ.
 * Received Bluetooth bytes are stored in the bluetooth_rx circular buffer.
 */
void init_usart4(void) {
	cb_init(&bluetooth_rx);

	// Enable USART4 to be used by the system clock
	RCC->APB1ENR |= RCC_APB1ENR_USART4EN;
	// Enable GPIOA to be used by the system clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Put PA0 and PA1 in alternate mode
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER0, ESF_GPIO_MODER_ALT_FUNC);
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER1, ESF_GPIO_MODER_ALT_FUNC);
	// Set PA0 and PA1 to be USART4 pins
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL0, 4);
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL1, 4);

	// Set baud rate
	USART4->BRR = F_USART_CLOCK / BLUETOOTH_BAUD_RATE;

	// Turn off parity
	MODIFY_FIELD(USART4->CR1, USART_CR1_PCE, 0);

	// Set the word length: 8 data bits
	MODIFY_FIELD(USART4->CR1, USART_CR1_M, 0);

	// 1 stop bit
	MODIFY_FIELD(USART4->CR2, USART_CR2_STOP, 0);

	// Enable interrupt generation
	MODIFY_FIELD(USART4->CR1, USART_CR1_RXNEIE, 1);

	// Enable receiver and USART4
	MODIFY_FIELD(USART4->CR1, USART_CR1_RE, 1);
	MODIFY_FIELD(USART4->CR1, USART_CR1_UE, 1);

	// Enable USART4 interrupts in NVIC
	NVIC_SetPriority(USART3_8_IRQn, 2);
	NVIC_ClearPendingIRQ(USART3_8_IRQn);
	NVIC_EnableIRQ(USART3_8_IRQn);
}

/*
 * @brief Check whether debug UART data is available
 *
 * @return true if at least one byte is available in the debug RX buffer
 */
static inline bool usart_char_available(void) {
	return !cb_empty(&debug_rx);
}

/*
 * @brief Blocking getchar implementation for debug USART input
 *
 * Waits until a byte is available from USART2, then removes and returns it.
 */
int __io_getchar(void) {
	while(!usart_char_available()) { }
	return (unsigned char) cb_dequeue(&debug_rx);
}

/*
 * @brief Read one character from the debug USART
 *
 * Wrapper around __io_getchar().
 */
int usart_getchar(void) {
	return __io_getchar();
}

/*
 * @brief Try to read a complete Bluetooth command
 *
 * Reads one byte from the Bluetooth RX buffer, stores it in cmd_buffer,
 * and returns true once a newline-terminated command has been received.
 *
 * @return true if a complete command is ready, false otherwise
 */
bool bluetooth_try_getcommand(char cmd_buffer[] , uint16_t *buffer_index) {
    if(cb_empty(&bluetooth_rx)) {
        return false;
    }

    char c = (char) cb_dequeue(&bluetooth_rx);
    cmd_buffer[*buffer_index] = to_lower(c);
    (*buffer_index)++;

    if(c == '\n') {
    	return true;
    }

    return false;
}

/*
 * @brief USART2 interrupt handler for debug UART receive
 *
 * Reads received bytes from USART2 and stores them in the debug RX buffer.
 */
void USART2_IRQHandler(void) {
	unsigned char c;
	if(USART2->ISR & USART_ISR_RXNE) { // Receive buffer not empty
		// Reading from RDR will clear RXNE
		c = USART2->RDR;
		if(!cb_full(&debug_rx)) {
			cb_enqueue(&debug_rx, c);
		}
	}
}

/*
 * @brief Shared USART3-through-USART8 interrupt handler
 *
 * Handles USART4 receive interrupts for the Bluetooth module. Received bytes
 * are read from USART4 and stored in the Bluetooth RX buffer.
 */
void USART3_4_5_6_7_8_IRQHandler(void) {
	unsigned char c;
	if(USART4->ISR & USART_ISR_RXNE) { // Receive buffer not empty
		// Reading from RDR will clear RXNE
		c = USART4->RDR;
		if(!cb_full(&bluetooth_rx)) {
			cb_enqueue(&bluetooth_rx, c);
		}
	}
}
