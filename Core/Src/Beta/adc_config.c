/**
 * @file adc_config.c
 * @author Gian Barta-Dougall
 * @brief System file for adc_config
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */

/* Private Includes */
#include "adc_config.h"

/* Private STM Includes */

/* Private #defines */

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */

/* Public Functions */

void adc_config_init(void) {}

/**
 * @brief Sets up the ADC to read the voltage on the data pin of
 * ambient light sensor 1
 */
void adc_config_als1_init(void) {

    // Initialise the system clock for ADC
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // Enables the ADC clock

    RCC->CCIPR |= 0x03 << 28; // Selects the system clock to be ADC clock

    // Configure ADC clock
    ADC1_COMMON->CCR &= ~(0x03 << 18); // Set no prescaler on ADC clock
    ADC1_COMMON->CCR &= ~(0x03 << 16); // Set the clock mode to system clock

    // Configure ADC settings
    ADC1->CFGR &= ~(0x03 << 3);  // Set the resolution to 12 Bit
    ADC1->CFGR &= ~(0x01 << 5);  // Align the data to the right
    ADC1->CFGR &= ~(0x01 << 16); // Enable discontinuous mode
    ADC1->CFGR &= ~(0x01 << 13); // Set mode to single

    ADC1->ISR |= (0x01 << 2); // Enable EOC flag (End of Conversion Flag)

    // Set the sampling to maximum
    ADC1->SMPR1 = 0x00;        // Set sampling time of channels 0-9 to 2.5 clock cycles
    ADC1->SQR1 &= ~(0xF << 0); // Set channel sequence length to 1
    ADC1->SQR1 &= ~(0xF << 6); // Reset first sequence channel
    ADC1->SQR1 |= (0xA << 6);  // Set the first sequence to channel 10

    /****** START CODE BLOCK ******/
    // Description: Calibrates the ADC. The following lines of code must remain in this order

    ADC1->CR &= ~(ADC_CR_DEEPPWD);            // Take ADC out of deep power down mode
    ADC1->CR |= (ADC_CR_ADVREGEN);            // Enable ADC voltage regulator
    HAL_Delay(1);                             // Max startup time for voltage regulator is 20us
    ADC1->CR &= ~(ADC_CR_ADEN);               // Ensure the ADC is disabled before calibration begins
    ADC1->CR &= ~(ADC_CR_ADCALDIF);           // Select calibration mode to single ended input
    ADC1->CR |= (ADC_CR_ADCAL);               // Start calibration
    while ((ADC1->CR & ADC_CR_ADCAL) != 0) {} // Wait for calibration to finish

    /****** END CODE BLOCK ******/
}

uint16_t adc_config_adc1_convert(void) {

    ADC1->CR |= ADC_CR_ADEN;                    // Enable the ADC
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) {} // Wait until ADC is ready
    ADC1->CR |= ADC_CR_ADSTART;                 // Start conversion
    while ((ADC1->ISR & ADC_ISR_EOC) == 0) {}   // Wait until the ADC has finished converting

    return ADC1->DR; // Return conversion
}

void adc_config_disable_adc1(void) {

    // Wait until there are no on going conversions
    while ((ADC1->CR & (ADC_CR_ADSTART | ADC_CR_JADSTART)) != 0) {}

    // Disable the ADC
    ADC1->CR |= ADC_CR_ADDIS;

    // Wait until the ADC has been disabled
    while ((ADC1->CR & ADC_CR_ADEN) != 0) {}
}

/* Private Functions */
