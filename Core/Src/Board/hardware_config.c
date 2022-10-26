/**
 * @file hardware_configuration.c
 * @author Gian Barta-Dougall
 * @brief System file for hardware_configuration
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
/* Public Includes */
#include "hardware_config.h"
#include "debug_log.h"
#include "board.h"
#include "utilities.h"

/* Private Includes */
#include "adc_config.h"

/* Private STM Includes */
#include "stm32l4xx_hal.h"

/* Private Macros */

#define EXTI_PORTA 0x00
#define EXTI_PORTB 0x01

#define TIMER_SETTINGS_NOT_VALID(frequency, maxCount) ((SystemCoreClock / frequency) > maxCount)

/* Private Structures and Enumerations */

/* Private Variable Declarations */

/* Private Function Prototypes */
void hardware_config_gpio_init(void);
void hardware_config_timer_init(void);
void hardware_config_uart_init(void);
void hardware_error_handler(void);
void hardware_config_gpio_reset(void);
void hardware_config_adc_init(void);
void hardware_config_exti_interrupts(void);

/* Public Functions */

void hardware_config_init(void) {

    // Initialise uart communication and debugging
    hardware_config_uart_init();

    // Initialise all GPIO ports and EXTI interrupts
    hardware_config_gpio_init();
    hardware_config_exti_interrupts();

    // Initialise all timers
    hardware_config_timer_init();

    // // Initialise all ADCs
    hardware_config_adc_init();
}

/* Private Functions */

/**
 * @brief Initialise the GPIO pins used by the system.
 */
void hardware_config_gpio_init(void) {

    // Enable clocks for GPIO port A and B
    RCC->AHB2ENR |= 0x03;

    // Configure GPIO pins for the pushbuttons if the module is enabled for the current version
    // being compiled
#ifdef BUTTON_MODULE_ENABLED

    // Set pins to input so they can be set to outputs afterwards
    SET_PIN_MODE_INPUT(HC_BUTTON_1_PORT, HC_BUTTON_1_PIN);
    SET_PIN_MODE_INPUT(HC_BUTTON_2_PORT, HC_BUTTON_2_PIN);

#endif

#ifdef MOTOR_MODULE_ENABLED

    // Set pins to inputs so they can be set to outputs afterwards
    SET_PIN_MODE_INPUT(HC_MOTOR_PORT_1, HC_MOTOR_PIN_1);
    SET_PIN_MODE_INPUT(HC_MOTOR_PORT_2, HC_MOTOR_PIN_2);
    SET_PIN_MODE_INPUT(HC_MOTOR_PORT_3, HC_MOTOR_PIN_3);
    SET_PIN_MODE_INPUT(HC_MOTOR_PORT_4, HC_MOTOR_PIN_4);

    // Set motor pins to be outputs
    SET_PIN_MODE_OUTPUT(HC_MOTOR_PORT_1, HC_MOTOR_PIN_1);
    SET_PIN_MODE_OUTPUT(HC_MOTOR_PORT_2, HC_MOTOR_PIN_2);
    SET_PIN_MODE_OUTPUT(HC_MOTOR_PORT_3, HC_MOTOR_PIN_3);
    SET_PIN_MODE_OUTPUT(HC_MOTOR_PORT_4, HC_MOTOR_PIN_4);

#endif

#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED

    // Set the pin to analogue mode (High Z)
    SET_PIN_MODE_ANALOGUE(HC_ALS_PORT_1, HC_ALS_PIN_1);
    SET_PIN_MODE_ANALOGUE(HC_ALS_PORT_2, HC_ALS_PIN_2);

#endif

#ifdef LED_MODULE_ENABLED

    // Set pins to inputs so they can be set to outputs afterwards
    SET_PIN_MODE_INPUT(HC_LED_RED_PORT, HC_LED_RED_PIN);
    SET_PIN_MODE_INPUT(HC_LED_GREEN_PORT, HC_LED_GREEN_PIN);
    SET_PIN_MODE_INPUT(HC_LED_ORANGE_PORT, HC_LED_ORANGE_PIN);

    // Set pins to outputs
    SET_PIN_MODE_OUTPUT(HC_LED_RED_PORT, HC_LED_RED_PIN);
    SET_PIN_MODE_OUTPUT(HC_LED_GREEN_PORT, HC_LED_GREEN_PIN);
    SET_PIN_MODE_OUTPUT(HC_LED_ORANGE_PORT, HC_LED_ORANGE_PIN);

    // Set all LEDs off
    SET_PIN_LOW(HC_LED_RED_PORT, HC_LED_RED_PIN);
    SET_PIN_LOW(HC_LED_GREEN_PORT, HC_LED_GREEN_PIN);
    SET_PIN_LOW(HC_LED_ORANGE_PORT, HC_LED_ORANGE_PIN);
#endif

#ifdef PIEZO_BUZZER_MODULE_ENABLED

    // Set piezo buzzer pin to output
    SET_PIN_MODE_INPUT(HC_PIEZO_BUZZER_PORT, HC_PIEZO_BUZZER_PIN);
    SET_PIN_MODE_ALTERNATE_FUNCTION(HC_PIEZO_BUZZER_PORT, HC_PIEZO_BUZZER_PIN);
    SET_PIN_SPEED_MEDIUM(HC_PIEZO_BUZZER_PORT, HC_PIEZO_BUZZER_PIN);

    HC_PIEZO_BUZZER_PORT->AFR[0] &= ~(0x0F << (HC_PIEZO_BUZZER_PIN * 4)); // Reset alternate function
    HC_PIEZO_BUZZER_PORT->AFR[0] |= (0x0E << (HC_PIEZO_BUZZER_PIN * 4));  // Set alternate function to AF14

#endif

#ifdef ENCODER_MODULE_ENABLED

    // Set encoder pin to alternate function connected to the input of TIM1
    SET_PIN_MODE_INPUT(HC_ENCODER_1_PORT, HC_ENCODER_1_PIN);
    SET_PIN_MODE_INPUT(HC_ENCODER_2_PORT, HC_ENCODER_2_PIN);

    SET_PIN_MODE_ALTERNATE_FUNCTION(HC_ENCODER_1_PORT, HC_ENCODER_1_PIN);
    SET_PIN_MODE_ALTERNATE_FUNCTION(HC_ENCODER_2_PORT, HC_ENCODER_2_PIN);

    SET_PIN_SPEED_LOW(HC_ENCODER_1_PORT, HC_ENCODER_1_PIN);
    SET_PIN_SPEED_LOW(HC_ENCODER_2_PORT, HC_ENCODER_2_PIN);

    SET_PIN_TYPE_PUSH_PULL(HC_ENCODER_1_PORT, HC_ENCODER_1_PIN);
    SET_PIN_TYPE_PUSH_PULL(HC_ENCODER_2_PORT, HC_ENCODER_2_PIN);

    SET_PIN_PULL_AS_NONE(HC_ENCODER_1_PORT, HC_ENCODER_1_PIN);
    SET_PIN_PULL_AS_NONE(HC_ENCODER_2_PORT, HC_ENCODER_2_PIN);

    HC_ENCODER_1_PORT->AFR[1] &= ~(0x0F); // Reset AF register AF8
    HC_ENCODER_2_PORT->AFR[0] &= ~(0x0F); // Reset AF register AF0

    HC_ENCODER_1_PORT->AFR[1] |= (0x01); // Set AF register AF8 to TIM1 CH1
    HC_ENCODER_2_PORT->AFR[0] |= (0x01); // Set AF register AF0 to TIM2 CH1

#endif

#ifdef DEBUG_LOG_MODULE_ENABLED
    SET_PIN_MODE_INPUT(HC_DEBUG_LOG_RX_PORT, HC_DEBUG_LOG_RX_PIN);
    SET_PIN_MODE_INPUT(HC_DEBUG_LOG_TX_PORT, HC_DEBUG_LOG_TX_PIN);

    SET_PIN_MODE_ALTERNATE_FUNCTION(HC_DEBUG_LOG_RX_PORT, HC_DEBUG_LOG_RX_PIN);
    SET_PIN_MODE_ALTERNATE_FUNCTION(HC_DEBUG_LOG_TX_PORT, HC_DEBUG_LOG_TX_PIN);

    SET_PIN_TYPE_PUSH_PULL(HC_DEBUG_LOG_RX_PORT, HC_DEBUG_LOG_RX_PIN);
    SET_PIN_TYPE_PUSH_PULL(HC_DEBUG_LOG_TX_PORT, HC_DEBUG_LOG_TX_PIN);

    SET_PIN_SPEED_HIGH(HC_DEBUG_LOG_RX_PORT, HC_DEBUG_LOG_RX_PIN);
    SET_PIN_SPEED_HIGH(HC_DEBUG_LOG_TX_PORT, HC_DEBUG_LOG_TX_PIN);

    HC_DEBUG_LOG_RX_PORT->AFR[0] &= ~(0x0F << ((8 % HC_DEBUG_LOG_RX_PIN) * 4));
    HC_DEBUG_LOG_TX_PORT->AFR[0] &= ~(0x0F << ((8 % HC_DEBUG_LOG_TX_PIN) * 4));

    HC_DEBUG_LOG_RX_PORT->AFR[0] |= (7 << (4 * HC_DEBUG_LOG_RX_PIN));
    HC_DEBUG_LOG_TX_PORT->AFR[0] |= (7 << (4 * HC_DEBUG_LOG_TX_PIN));
#endif
}

void hardware_config_exti_interrupts(void) {

    // Enabling the SYSCFG clock is required for EXTI interrupts to work
    __HAL_RCC_SYSCFG_CLK_ENABLE();

#ifdef BUTTON_MODULE_ENABLED

    // Connect GPIO pins to the EXTI lines
    SYSCFG->EXTICR[0] |= (EXTI_PORTA << (4 * (HC_BUTTON_1_PIN % 4)));
    SYSCFG->EXTICR[1] |= (EXTI_PORTB << (4 * (HC_BUTTON_2_PIN % 4)));

    // Configure interrupts to occur on rising and falling edges
    EXTI->RTSR1 |= (0x01 << HC_BUTTON_1_PIN);
    EXTI->RTSR1 |= (0x01 << HC_BUTTON_2_PIN);

    EXTI->FTSR1 |= (0x01 << HC_BUTTON_1_PIN);
    EXTI->FTSR1 |= (0x01 << HC_BUTTON_2_PIN);

    // Ensure interrupts are masked until buttons are enabled.
    // If an interrupt is masked it will not run
    EXTI->IMR1 |= (0x01 << HC_BUTTON_1_PIN);
    EXTI->IMR1 |= (0x01 << HC_BUTTON_2_PIN);

    // Configure priorities for each EXTI line
    HAL_NVIC_SetPriority(HC_BUTTON_1_IQRn, HC_BUTTON_1_ISR_PRIORITY, HC_BUTTON_1_ISR_SUBPRIORITY);
    HAL_NVIC_SetPriority(HC_BUTTON_2_IQRn, HC_BUTTON_2_ISR_PRIORITY, HC_BUTTON_2_ISR_SUBPRIORITY);

    // Enable interrupts for the given EXTI lines
    HAL_NVIC_EnableIRQ(HC_BUTTON_1_IQRn);
    HAL_NVIC_EnableIRQ(HC_BUTTON_2_IQRn);
#endif
}

void hardware_config_timer_init(void) {

#ifdef TASK_SCHEDULER_MODULE_ENABLED

    /* If the system clock is too high, this timer will count too quickly and the timer
        will reach its maximum count and reset before the timer count reaches the number
        of ticks required to operate at the specified frequency. Lower system clock or
        increase the timer frequency if you get this error. E.g: System clock = 1Mhz and
        timer frequnecy = 1Hz => timer should reset after 1 million ticks to get a frequnecy
        of 1Hz but the max count < 1 million thus 1Hz can never be reached */
    #if ((SYSTEM_CLOCK_CORE / HC_TS_TIMER_FREQUENCY) > HC_TS_TIMER_MAX_COUNT)
        #error System clock frequency is too high to generate the required timer frequnecy
    #endif

    /* Configure timer for task scheduler*/
    HC_TS_TIMER_CLK_ENABLE();                                         // Enable the clock
    HC_TS_TIMER->CR1 &= ~(TIM_CR1_CEN);                               // Disable counter
    HC_TS_TIMER->PSC = (SystemCoreClock / HC_TS_TIMER_FREQUENCY) - 1; // Set timer frequency
    HC_TS_TIMER->ARR = HC_TS_TIMER_MAX_COUNT;                         // Set maximum count for timer
    HC_TS_TIMER->CNT = 0;                                             // Reset count to 0
    HC_TS_TIMER->DIER &= 0x00;                                        // Disable all interrupts by default
    HC_TS_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);         // Set CH1 capture compare to mode to frozen

    /* Enable interrupt handler */
    HAL_NVIC_SetPriority(HC_TS_TIMER_IRQn, HC_TS_TIMER_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HC_TS_TIMER_IRQn);

#endif

#ifdef SYNCHRONOUS_TIMER_MODULE_ENABLED

    #if ((SYSTEM_CLOCK_CORE / HC_SYNCH_TIMER_FREQUENCY) > HC_SYNCH_TIMER_MAX_COUNT)
        #error System clock frequency is too high to generate the required timer frequnecy
    #endif

    /* Configure timer for task scheduler*/
    HC_SYNCH_TIMER_CLK_ENABLE();                                            // Enable the clock
    HC_SYNCH_TIMER->CR1 &= ~(TIM_CR1_CEN);                                  // Disable counter
    HC_SYNCH_TIMER->PSC = (SystemCoreClock / HC_SYNCH_TIMER_FREQUENCY) - 1; // Set timer frequency
    HC_SYNCH_TIMER->ARR = HC_SYNCH_TIMER_MAX_COUNT;                         // Set maximum count for timer
    HC_SYNCH_TIMER->CNT = 0;                                                // Reset count to 0
    HC_SYNCH_TIMER->DIER &= 0x00;                                           // Disable all interrupts by default
    HC_SYNCH_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);            // Set CH1 capture compare to mode to frozen

    /* Enable interrupt handler */
    HAL_NVIC_SetPriority(HC_SYNCH_TIMER_IRQn, HC_SYNCH_TIMER_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HC_SYNCH_TIMER_IRQn);

#endif

#ifdef PIEZO_BUZZER_MODULE_ENABLED

    #if ((SYSTEM_CLOCK_CORE / HC_PIEZO_BUZZER_TIMER_FREQUENCY) > HC_PIEZO_BUZZER_TIMER_MAX_COUNT)
        #error System clock frequency is too high to generate the required timer frequnecy for the piezo buzzer
    #endif

    /* Configure timer for piezo buzzer */
    HC_PIEZO_BUZZER_TIMER_CLK_ENABLE();                                                   // Enable the clock
    HC_PIEZO_BUZZER_TIMER->CR1 &= ~(TIM_CR1_CEN);                                         // Disable counter
    HC_PIEZO_BUZZER_TIMER->PSC = (SystemCoreClock / HC_PIEZO_BUZZER_TIMER_FREQUENCY) - 1; // Set timer frequency
    HC_PIEZO_BUZZER_TIMER->ARR = HC_PIEZO_BUZZER_TIMER_MAX_COUNT;                         // Set maximum count for timer
    HC_PIEZO_BUZZER_TIMER->CNT = 0;                                                       // Reset count to 0
    HC_PIEZO_BUZZER_TIMER->DIER &= 0x00; // Disable all interrupts by default

    /**/
    HC_PIEZO_BUZZER_TIMER->CCER |= TIM_CCER_CC1E;                          // Enable capture compare output
    HC_PIEZO_BUZZER_TIMER->CCMR1 &= ~(TIM_CCMR1_CC1S);                     // Configure channel 1 to be output
    HC_PIEZO_BUZZER_TIMER->CCMR1 &= ~(TIM_CCMR1_OC1M);                     // Reset output compare mode
    HC_PIEZO_BUZZER_TIMER->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // Set output compare mode to PWM mode 1

    /**/
    HC_PIEZO_BUZZER_TIMER->BDTR |= TIM_BDTR_MOE; // Enable OC and OCN output

    /* Enable interrupt handler */
    HAL_NVIC_SetPriority(HC_PIEZO_BUZZER_TIMER_IRQn, HC_PIEZO_BUZZER_TIMER_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HC_PIEZO_BUZZER_TIMER_IRQn);

#endif

#ifdef ENCODER_MODULE_ENABLED

    #if ((SYSTEM_CLOCK_CORE / HC_ENCODER_1_TIMER_FREQUENCY) > HC_ENCODER_1_TIMER_MAX_COUNT)
        #error System clock frequency is too high to generate the required timer frequency for the encoder
    #endif

    // Enable the clock for the timer
    HC_ENCODER_1_TIMER_CLK_ENABLE();

    // Set the sampling rate. (I'm not 100% sure if this is required. I
    // tested changing it and it didn't seem to affect the output)
    HC_ENCODER_1_TIMER->PSC = ((SystemCoreClock / HC_ENCODER_1_TIMER_FREQUENCY) - 1);

    // Set the maximum count for the timer
    HC_ENCODER_1_TIMER->ARR = HC_ENCODER_1_TIMER_MAX_COUNT; // Set the maximum count
    HC_ENCODER_1_TIMER->CR1 &= ~(0x01 << 4);                // Set the timer to count upwards
    HC_ENCODER_1_TIMER->CR2 &= ~(0x01 << 7);                // Set CH1 to timer input 1

    // Set TIM1 to input and map TIM_CH1 GPIO pin to trigger input 1 (TI1)
    HC_ENCODER_1_TIMER->CCMR1 &= ~(0x03 << 0); // Reset capture compare
    HC_ENCODER_1_TIMER->CCMR1 |= (0x01 << 0);  // Set capture compare to input (IC1 mapped to TI1)

    // Configure slave mode control
    HC_ENCODER_1_TIMER->SMCR &= ~(0x07 << 4);           // Reset trigger selection
    HC_ENCODER_1_TIMER->SMCR |= (0x05 << 4);            // Set trigger to Filtered Timer Input 1 (TI1FP1)
    HC_ENCODER_1_TIMER->SMCR &= ~((0x01 << 16) | 0x07); // Reset slave mode selection
    HC_ENCODER_1_TIMER->SMCR |= 0x07;                   // Set rising edge of selected trigger to clock the counter

    /* Configure channel 2 and 3 to trigger interrupts on capture compare values */
    HC_ENCODER_1_TIMER->DIER = 0x00; // Clear all interrupts

    // Enable capture compare on CH2, CH3 and UIE
    HC_ENCODER_1_TIMER->DIER |= ((0x01 << 0) | (0x01 << 2) | (0x01 << 3));

    HC_ENCODER_1_TIMER->CCMR2 &= ~(0x03 << 0);                  // Reset capture compare 3 to output
    HC_ENCODER_1_TIMER->CCMR2 &= ~((0x01 << 16) | (0x07 << 4)); // Reset output compare mode 3 to frozen

    HC_ENCODER_1_TIMER->CCMR1 &= ~(0x03 << 8);                   // Reset capture compare 2 to output
    HC_ENCODER_1_TIMER->CCMR1 &= ~((0x01 << 24) | (0x07 << 12)); // Reset output compare mode 2 to frozen

    // Disables UEV generation. This ensures that on counter underflow/overflow, the counter continues
    // count correctly as the shadow registers retain all their values
    HC_ENCODER_1_TIMER->CR1 |= TIM_CR1_UDIS;

    // Enable the interrupts
    HAL_NVIC_SetPriority(HC_ENCODER_1_TIMER_IRQn, HC_ENCODER_1_TIMER_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HC_ENCODER_1_TIMER_IRQn);

    /****** START CODE BLOCK ******/
    // Description: Configuration for the second encoder

    #if ((SYSTEM_CLOCK_CORE / HC_ENCODER_2_TIMER_FREQUENCY) > HC_ENCODER_2_TIMER_MAX_COUNT)
        #error System clock frequency is too high to generate the required timer frequency for the encoder
    #endif

    // Enable the clock for the timer
    HC_ENCODER_2_TIMER_CLK_ENABLE();

    // Set the sampling rate. (I'm not 100% sure if this is required. I
    // tested changing it and it didn't seem to affect the output)
    HC_ENCODER_2_TIMER->PSC = ((SystemCoreClock / HC_ENCODER_1_TIMER_FREQUENCY) - 1);

    // Set the maximum count for the timer
    HC_ENCODER_2_TIMER->ARR = HC_ENCODER_1_TIMER_MAX_COUNT; // Set the maximum count
    HC_ENCODER_2_TIMER->CR1 &= ~(0x01 << 4);                // Set the timer to count upwards
    HC_ENCODER_2_TIMER->CR2 &= ~(0x01 << 7);                // Set CH1 to timer input 1

    // Set TIM2 to input and map TIM_CH1 GPIO pin to trigger input 1 (TI1)
    HC_ENCODER_2_TIMER->CCMR1 &= ~(0x03 << 0); // Reset capture compare
    HC_ENCODER_2_TIMER->CCMR1 |= (0x01 << 0);  // Set capture compare to input (IC1 mapped to TI1)

    // Configure slave mode control
    HC_ENCODER_2_TIMER->SMCR &= ~(0x07 << 4);           // Reset trigger selection
    HC_ENCODER_2_TIMER->SMCR |= (0x05 << 4);            // Set trigger to Filtered Timer Input 1 (TI1FP1)
    HC_ENCODER_2_TIMER->SMCR &= ~((0x01 << 16) | 0x07); // Reset slave mode selection
    HC_ENCODER_2_TIMER->SMCR |= 0x07;                   // Set rising edge of selected trigger to clock the counter

    /* Configure channel 2 and 3 to trigger interrupts on capture compare values */
    HC_ENCODER_2_TIMER->DIER = 0x00; // Clear all interrupts

    // Enable capture compare on CH2, CH3 and UIE
    HC_ENCODER_2_TIMER->DIER |= ((0x01 << 0) | (0x01 << 2) | (0x01 << 3));

    HC_ENCODER_2_TIMER->CCMR2 &= ~(0x03 << 0);                  // Reset capture compare 3 to output
    HC_ENCODER_2_TIMER->CCMR2 &= ~((0x01 << 16) | (0x07 << 4)); // Reset output compare mode 3 to frozen

    HC_ENCODER_2_TIMER->CCMR1 &= ~(0x03 << 8);                   // Reset capture compare 2 to output
    HC_ENCODER_2_TIMER->CCMR1 &= ~((0x01 << 24) | (0x07 << 12)); // Reset output compare mode 2 to frozen

    // Disables UEV generation. This ensures that on counter underflow/overflow, the counter continues
    // count correctly as the shadow registers retain all their values
    HC_ENCODER_2_TIMER->CR1 |= TIM_CR1_UDIS;

    // Enable the interrupts
    HAL_NVIC_SetPriority(HC_ENCODER_2_TIMER_IRQn, HC_ENCODER_2_TIMER_ISR_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HC_ENCODER_2_TIMER_IRQn);

    /****** END CODE BLOCK ******/

#endif
}

void hardware_config_adc_init(void) {
    // Setup ADC for ALS1
    adc_config_als1_init();
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
void hardware_config_uart_init(void) {

#ifdef DEBUG_LOG_MODULE_ENABLED

    /**
     * Note that in the STM32 uart setup, the PWREN bit and the SYSCGEN bit are set
     * in the APB1ENR register. I'm pretty sure the SYSCGEN is only setup for exti
     * interrupts so I have that bit set in the exti interrupt function however if
     * you are setting up uart and there are no interrupts and it doesn't work, it
     * is most likely because either PWREN is not set or SYSCGEN is not set
     */

    // Enable UART clock
    HC_DEBUG_LOG_CLK_ENABLE();

    __HAL_RCC_PWR_CLK_ENABLE();

    // Set baud rate
    USART2->BRR = SystemCoreClock / HC_DEBUG_LOG_BUAD_RATE;

    // Enable the USART to let comms occur
    USART2->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);
#endif
}