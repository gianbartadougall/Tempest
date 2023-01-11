/**
 * @file serial_comms.c
 * @author Gian Barta-Dougall
 * @brief Library for communicating with mcu over UART
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

/* Public Includes */
#include <stdint.h>
#include <stdio.h>

/* STM32 Includes */

/* Private Includes */
#include "log.h"
#include "chars.h"

/* Includes that are used for processing commmands */
#include "blind.h"

#define MOVE_BLIND_X_UP         "move x up           \t"
#define MOVE_BLIND_X_DOWN       "move x down         \t"
#define INFO_BLIND_X            "info blind x        \t"
#define INFO_ALS_X              "info als x          \t"
#define INFO_BOARD              "info board          \t"
#define SET_BLIND_X_MIN_HEIGHT  "set min height      \t"
#define SET_BLIND_X_MODE_MANUAL "set x mode manual   \t"

#define MOVE_BLIND_1_UP         "move 1 up"
#define MOVE_BLIND_2_UP         "move 2 up"
#define INFO_BLIND_1            "info blind 1"
#define INFO_BLIND_2            "info blind 2"
#define INFO_ALS_1              "info als 1"
#define INFO_ALS_2              "info als 2"
#define INFO_BOARD              "info board"
#define SET_BLIND_1_MODE_MANUAL "set 1 mode manual"
#define SET_BLIND_2_MODE_MANUAL "set 2 mode manual"

const char* TEMPEST_MANUAL = MOVE_BLIND_X_DOWN
    "Moves blind x down\r\n" MOVE_BLIND_X_UP "Moves blind x up\r\n" INFO_BLIND_X
    "Prints the current information on blind x\r\n" INFO_ALS_X "Prints the current information on als x\r\n" INFO_BOARD
    "Prints the current board information\r\n" SET_BLIND_X_MIN_HEIGHT
    "Sets the current height of blind x as the minimum height";

/* Private Macros */
#define ASCII_KEY_ENTER 0x0D
#define ASCII_KEY_ESC   0x1C

/* Private Variables */
char buffer[1024];
uint16_t bufferIndex = 0;

void serial_comms_process_command(char* string);
void serial_comms_process_action(char c);

void serial_comms_init(void) {}

void serial_comms_clear_buffer(void) {
    bufferIndex = 0;
}

void serial_comms_add_to_buffer(char c) {

    // Check whether the character is a letter/number or something else
    if ((c < '!' || c > '~') && (c != ASCII_KEY_ENTER)) {
        serial_comms_process_action(c);
        return;
    }

    buffer[bufferIndex] = c;
    bufferIndex++;

    if (c == ASCII_KEY_ENTER) {
        buffer[bufferIndex++] = '\r';
        buffer[bufferIndex++] = '\n';
        buffer[bufferIndex++] = '\0';
        log_message(buffer);

        // Remove \r\n for processing
        buffer[bufferIndex - 3] = '\0';
        bufferIndex             = 0;

        serial_comms_process_command(buffer);
    }
}

void serial_comms_process_action(char c) {

    if (c == ASCII_KEY_ESC) {

        return;
    }
}

void serial_comms_process_command(char* string) {

    if (chars_same(string, "help") == TRUE) {
        log_print_const(TEMPEST_MANUAL);
        return;
    }

    if (chars_same(string, SET_BLIND_1_MODE_MANUAL) == TRUE) {
        blind_set_bif_mode(BLIND_MODE_MANUAL);
        log_message("Blind 1 mode set to manual\r\n");
    }

    // if (chars_same(string, MOVE_BLIND_1_UP)) {
    //     log_prints("Moving blind 1 upwards\r\n");
    //     return;
    // }

    // if (chars_same(string, MOVE_BLIND_2_UP)) {
    //     log_prints("Moving blind 2 upwards\r\n");
    //     return;
    // }

    // if (chars_same(string, INFO_BLIND_1)) {
    //     log_prints("Printing info on blind 1\r\n");
    //     return;
    // }

    // if (chars_same(string, INFO_BLIND_2)) {
    //     log_prints("Printing info on blind 2\r\n");
    //     return;
    // }

    // if (chars_same(string, INFO_ALS_1)) {
    //     log_prints("Printing info on ALS 1\r\n");
    //     return;
    // }

    // if (chars_same(string, INFO_ALS_2)) {
    //     log_prints("Printing info on ALS 2\r\n");
    //     return;
    // }

    // if (chars_same(string, INFO_BOARD)) {
    //     log_prints("Printing info on board\r\n");
    //     return;
    // }
}

#endif