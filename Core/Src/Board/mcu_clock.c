/**
 * @file mcu_clock.c
 * @author Gian Barta-Dougall
 * @brief File to initialise clock of microcontroller
 * @version 0.1
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* Public Includes */

/* Private Includes */
#include "mcu_clock.h"

/* STM32 Includes */

/* Private #defines */

// Define system clocks
#define CLK_100KHZ 0x00
#define CLK_200KHZ 0x01
#define CLK_400KHZ 0x03
#define CLK_800KHZ 0x04
#define CLK_1MHZ 0x05
#define CLK_2MHZ 0x06
#define CLK_4MHZ 0x07
#define CLK_8MHZ 0x08
#define CLK_16MHZ 0x09
#define CLK_32MHZ 0x0A
#define CLK_48MHZ 0x0B

#define SYSTEM_CLOCK CLK_32MHZ

uint32_t msiClockRanges[12] = {100000, 200000, 400000, \
        800000, 1000000, 2000000, 4000000, 8000000, \
        16000000, 24000000, 32000000, 48000000};

/* Variable Declarations */
uint32_t systemClock;

/* Function prototypes */


void mcu_clock_init(void) {

    // Turn MSI clock off
    RCC->CR &= ~(0x01);

    RCC->CR |= (0x01 << 3); // Set MSI clock range to operate on RCC_CR register

    // Set MSI clock to operate at 32Mhz
    RCC->CR &= ~(0x0F << 4); // Reset the MSI clock range
    RCC->CR |= (SYSTEM_CLOCK << 4); // Set the MSI clock range to 32Mhz

    // Turn MSI clock on
    RCC->CR |= 0x01;

    // Store the system clock
    systemClock = msiClockRanges[SYSTEM_CLOCK];

    // Wait for MSI clock to stabalise
    while ((RCC->CR & (0x01 << 1)) == 0) {}

    // Set MCO output prescaler to 1
    RCC->CFGR &= ~(0x07 << 28); // Set MCO prescaler to 1
    RCC->CFGR &= ~(0x0F << 24); // Reset MCU clock output
    RCC->CFGR |= (0x02 << 24); // Set MSI clock as MCU clock output
    
    RCC->CFGR &= ~(0x07 << 11); // Set APB2 prescaler to 1
    RCC->CFGR &= ~(0x07 << 8); // Set APB1 prescaler to 1
}