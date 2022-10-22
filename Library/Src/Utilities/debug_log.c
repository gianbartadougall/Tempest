/**
 * @file debug_log.c
 * @author Gian Barta-Dougall
 * @brief Library for debugging
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Private Includes */
#include "debug_log.h"

// Handle for uart
UART_HandleTypeDef* debugLogHandle;

void debug_log_init(UART_HandleTypeDef* huart) {
    debugLogHandle = huart;
}

void debug_prints(char* msg) {
    uint16_t i = 0;

    // Transmit until end of message reached
    while (msg[i] != '\0') {
        while ((USART2->ISR & USART_ISR_TXE) == 0) {};

        USART2->TDR = msg[i];
        i++;
    }
}

char debug_getc(void) {

    while (!(USART2->ISR & USART_ISR_RXNE)) {};
    return USART2->RDR;
}

void debug_clear(void) {
    // Prints ANSI escape codes that will clear the terminal screen
    debug_prints("\033[2J\033[H");
}

/**
 * @brief This is a test function. Code can be used for an actual uart communication
 * peripheral file where something like a desktop computer can talk to the STM32.
 *
 * This function reads serial input from a program like putty
 * and puts it into a string and then echos the message back to the
 * terminal after the return key is pressed (0x0D)
 *
 */
void serial_communicate(void) {

    debug_clear();
    char msg[100];
    uint8_t i = 0;
    while (1) {

        char c = debug_getc();
        // sprintf(msg, "%x\r\n", c);
        // debug_prints(msg);
        if (c == 0x0D) {
            msg[i++] = '\r';
            msg[i++] = '\n';
            msg[i++] = '\0';
            debug_prints(msg);
            i = 0;
        } else {
            msg[i] = c;
            i++;
        }
    }
}