/**
 * @file uart_interrupts.c
 * @author Gian Barta-Dougall
 * @brief File to store interrupt handlers for uart for STM32L432KC mcu
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright (c) 2022
 *
 */
/* Public Includes */
#include "stm32l432xx.h"

/* Private Includes */
#include "serial_comms.h"
#include "log.h"

void USART2_IRQHandler(void) {

    if ((USART2->ISR & USART_ISR_RXNE) != 0) {

        // Skip accidental noise

        char c = USART2->RDR;
        if (c == 0xFF) {
            return;
        }

        // Print the character to the console
        USART2->TDR = c;

        // Copy bit into buffer. Reading RDR automatically clears flag
        serial_comms_add_to_buffer(c);
    }

    if ((USART2->ISR & USART_ISR_PE) != 0) {
        log_prints("Parity error\r\n");
    }
}