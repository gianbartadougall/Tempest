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
// #include "stm32l4xx_hal_msp.h"

/* Private Includes */
#include "adc_config.h"

/* Private STM Includes */
#include "stm32l4xx_hal.h"

/* Private Macros */

// Defines for the different configuration modes for the GPIO pins
#define MODER_INPUT                0x00
#define MODER_OUTPUT               0x01
#define MODER_ALTERNATE_FUNCTION   0x02
#define MODER_ANALOGUE             0x03
#define OTYPER_PUSH_PULL           0x00
#define OTYPER_OPEN_DRAIN          0x01
#define OSPEER_LOW_SPEED           0x00
#define OSPEER_MEDIUM_SPEED        0x01
#define OSPEER_HIGH_SPEED          0x02
#define OSPEER_VERY_HIGH_SPEED     0x03
#define PUPDR_NO_PULL_UP_PULL_DOWN 0x00
#define PUPDR_PULL_UP              0x01
#define PUPDR_PULL_DOWN            0x02

#define EXTI_PORTA 0x00
#define EXTI_PORTB 0x01

#define TIMER_SETTINGS_NOT_VALID(frequency, maxCount) ((SystemCoreClock / frequency) > maxCount)

/* Private Structures and Enumerations */

/* Private Variable Declarations */
UART_HandleTypeDef huart2;

/* Private Function Prototypes */
void hardware_config_gpio_init(void);
void hardware_config_timer_init(void);
void hardware_config_serial_comms_init(void);
void hardware_error_handler(void);
void hardware_config_gpio_reset(void);
void hardware_config_adc_init(void);
void hardware_config_exti_interrupts(void);

/* Public Functions */

void hardware_config_init(void) {

    // Initialise uart communication and debugging
    hardware_config_serial_comms_init();

    // Initialise all GPIO ports and EXTI interrupts
    hardware_config_gpio_init();
    hardware_config_exti_interrupts();

    // Initialise all timers
    hardware_config_timer_init();

    // Initialise all ADCs
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
    HC_BUTTON_1_PORT->MODER &= ~(MODER_ANALOGUE << (HC_BUTTON_1_PIN * 2));
    HC_BUTTON_2_PORT->MODER &= ~(MODER_ANALOGUE << (HC_BUTTON_2_PIN * 2));

#endif

#ifdef MOTOR_MODULE_ENABLED

    // Set pins to inputs so they can be set to outputs afterwards
    HC_MOTOR_PORT_1->MODER &= ~(MODER_ANALOGUE << (HC_MOTOR_PIN_1 * 2));
    HC_MOTOR_PORT_2->MODER &= ~(MODER_ANALOGUE << (HC_MOTOR_PIN_2 * 2));
    HC_MOTOR_PORT_3->MODER &= ~(MODER_ANALOGUE << (HC_MOTOR_PIN_3 * 2));
    HC_MOTOR_PORT_4->MODER &= ~(MODER_ANALOGUE << (HC_MOTOR_PIN_4 * 2));

    // Set motor pins to be outputs
    HC_MOTOR_PORT_1->MODER |= (MODER_OUTPUT << (HC_MOTOR_PIN_1 * 2));
    HC_MOTOR_PORT_2->MODER |= (MODER_OUTPUT << (HC_MOTOR_PIN_2 * 2));
    HC_MOTOR_PORT_3->MODER |= (MODER_OUTPUT << (HC_MOTOR_PIN_3 * 2));
    HC_MOTOR_PORT_4->MODER |= (MODER_OUTPUT << (HC_MOTOR_PIN_4 * 2));

#endif

#ifdef AMBIENT_LIGHT_SENSOR_MODULE_ENABLED

    // Set the pin to analogue mode (High Z)
    HC_ALS_PORT_1->MODER |= (MODER_ANALOGUE << (HC_ALS_PIN_1 * 2));
    HC_ALS_PORT_2->MODER |= (MODER_ANALOGUE << (HC_ALS_PIN_2 * 2));

#endif

#ifdef LED_MODULE_ENABLED

    // Set pins to inputs so they can be set to outputs afterwards
    HC_LED_RED_PORT->MODER &= ~(MODER_ANALOGUE << (HC_LED_RED_PIN * 2));
    HC_LED_GREEN_PORT->MODER &= ~(MODER_ANALOGUE << (HC_LED_GREEN_PIN * 2));
    HC_LED_ORANGE_PORT->MODER &= ~(MODER_ANALOGUE << (HC_LED_ORANGE_PIN * 2));

    // Set pins to outputs
    HC_LED_RED_PORT->MODER |= (MODER_OUTPUT << (HC_LED_RED_PIN * 2));
    HC_LED_GREEN_PORT->MODER |= (MODER_OUTPUT << (HC_LED_GREEN_PIN * 2));
    HC_LED_ORANGE_PORT->MODER |= (MODER_OUTPUT << (HC_LED_ORANGE_PIN * 2));

    // Set all LEDs off
    HC_LED_RED_PORT->BSRR |= (0x10000 << HC_LED_RED_PIN);
    HC_LED_GREEN_PORT->BSRR |= (0x10000 << HC_LED_GREEN_PIN);
    HC_LED_ORANGE_PORT->BSRR |= (0x10000 << HC_LED_ORANGE_PIN);
#endif

#ifdef PIEZO_BUZZER_MODULE_ENABLED

    // Connect GPIO pin to output of pizeo buzzer timer
    HC_PIEZO_BUZZER_PORT->MODER &= ~(MODER_ANALOGUE << (HC_PIEZO_BUZZER_PIN * 2));
    HC_PIEZO_BUZZER_PORT->MODER |= (MODER_ALTERNATE_FUNCTION << (HC_PIEZO_BUZZER_PIN * 2));
    HC_PIEZO_BUZZER_PORT->OSPEEDR |= (OSPEER_HIGH_SPEED << (HC_PIEZO_BUZZER_PIN * 2));

    HC_PIEZO_BUZZER_PORT->AFR[0] &= ~(0x0F << (HC_PIEZO_BUZZER_PIN * 4)); // Reset alternate function
    HC_PIEZO_BUZZER_PORT->AFR[0] |= (0x0E << (HC_PIEZO_BUZZER_PIN * 4));  // Set alternate function to AF14

#endif
}

void hardware_config_exti_interrupts(void) {

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
        #error System clock frequency is too high to generate the required timer frequnecy
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
void hardware_config_serial_comms_init(void) {

    huart2.Instance                    = USART2;
    huart2.Init.BaudRate               = 115200;
    huart2.Init.WordLength             = UART_WORDLENGTH_8B;
    huart2.Init.StopBits               = UART_STOPBITS_1;
    huart2.Init.Parity                 = UART_PARITY_NONE;
    huart2.Init.Mode                   = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        hardware_error_handler();
    }

    // Initialise debugging
    debug_log_init(&huart2);
}

void hardware_error_handler(void) {

    // Initialise onboad LED incase it hasn't been initialised
    board_init();

    // Initialisation error shown by blinking LED (LD3) in pattern
    while (1) {

        for (int i = 0; i < 5; i++) {
            brd_led_toggle();
            HAL_Delay(100);
        }

        HAL_Delay(500);
    }
}