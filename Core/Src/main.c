/**
 * @file main.c
 * @author Gian Barta-Dougall
 * @brief Program File
 * @version 0.1
 * @date 2022-04-15
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Public includes */

/* Private includes */
#include "main.h"
#include "board.h"
#include "debug_log.h"
#include "motor_driver.h"
#include "encoder.h"
#include "pushbutton.h"

/* STM32 Includes */

/* Variable Declarations */
UART_HandleTypeDef huart2;

/* Private #defines */
#define MIN_HOLD_TIME 1000
#define MODE_AUTOMATIC 0 
#define MODE_MANUAL 1

#define UP_PUSH_BUTTON PUSH_BUTTON_0
#define DOWN_PUSH_BUTTON PUSH_BUTTON_1

// Motor defines
#define MOTOR MD_MOTOR_0
#define MOTOR_STOP MOTOR_DRIVER_STOP
#define MOTOR_UP MOTOR_DRIVER_DIRECTION_1
#define MOTOR_DOWN MOTOR_DRIVER_DIRECTION_2

/* Variable Declarations */
volatile uint8_t pbUpState = GPIO_PIN_RESET;
volatile uint8_t pbDownState = GPIO_PIN_RESET;

/* Function prototypes */
void hardware_init(void);
void error_handler(void);
void SystemClock_Config(void);
void MX_USART2_UART_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    
	// Reset all peripherals, initialise the flash interface and the systick
    HAL_Init();
	SystemClock_Config();
	MX_USART2_UART_Init();
	
	// Initialise hardware
    hardware_init();

	// Declare local variables
	uint32_t timer = 0;
	uint8_t mode = MODE_AUTOMATIC;
	uint32_t lastLEDUpdateTime = 0;
	char m[40];
	// Main program loop
    while (1) {

		// Check state of buttons
		uint8_t pbUpState = pb_get_state(UP_PUSH_BUTTON);
		uint8_t pbDownState = pb_get_state(DOWN_PUSH_BUTTON);

		// sprintf(m, "%lu \r\n", encoder_distance_travelled() / 1000);
		// debug_prints(m);
		// HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		// HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
		// HAL_Delay(1000);
		// continue;

		// If the state of the buttons is high, record the time
		if ((pbUpState == 1) && (pbDownState == 1)) {

			// Turn off motor
			if (motor_driver_set_motor_state(MOTOR, MOTOR_STOP)) {
				error_handler();
			}
			
			if (timer == 0) {
				
				if ((HAL_MAX_DELAY - MIN_HOLD_TIME) < HAL_GetTick()) {
					// Wait for timer to reset if minimum hold time cannot be acheived
					while ((HAL_MAX_DELAY - MIN_HOLD_TIME) < HAL_GetTick()) {}

					// Skip reset of the code
					continue;
				} else {

					// Set the start time
					timer = HAL_GetTick();
				}
			}

			// Start the timer if the current time is 0
			if ((HAL_GetTick() - timer) > MIN_HOLD_TIME) {
				if (mode == MODE_AUTOMATIC) {
					mode = MODE_MANUAL;
					
					// Reset timer so another toggle won't occur until another minimum hold time
					timer = 0;
				} else {
					mode = MODE_AUTOMATIC;

					// Reset timer so another toggle won't occur until another minimum hold time
					timer = 0;
				}
			}
		} else {
			// Reset the timer
			timer = 0;
		}

		// Check if motor needs to be stopped
		if ((pbUpState == 0) && (pbDownState == 0)) {
			
			uint8_t stopMotor = 
				(encoder_at_maximum_distance() && (motor_driver_get_motor_state(MOTOR) == MOTOR_DOWN)) |
				(encoder_at_minimum_distance() && (motor_driver_get_motor_state(MOTOR) == MOTOR_UP)) |
				(mode == MODE_MANUAL);

			if (stopMotor) {
				if (motor_driver_set_motor_state(MOTOR, MOTOR_STOP)) {
					error_handler();
				}
			}
		}

		if ((pbUpState == 0) && (pbDownState == 1)) {

			// Check that the motor can move down
			if (!encoder_at_maximum_distance()) {
				
				// Move motor down
				if (motor_driver_set_motor_state(MOTOR, MOTOR_DOWN) != HAL_OK) {
					error_handler();
				} else {
					encoder_set_direction_positive();
				}
			}
		}

		if ((pbUpState == 1) && (pbDownState == 0)) {
			
			// Check that the motor can move up
			if (!encoder_at_minimum_distance()) {
				
				// Move motor up
				if (motor_driver_set_motor_state(MOTOR, MOTOR_UP) != HAL_OK) {
					error_handler();
				} else {
					encoder_set_direction_negative();
				}
			}
		}

		/* Stop the motors if the minimum/maximum distance has been reached */	
		uint8_t stopMotor = 
			(encoder_at_maximum_distance() && (motor_driver_get_motor_state(MOTOR) == MOTOR_DOWN)) |
			(encoder_at_minimum_distance() && (motor_driver_get_motor_state(MOTOR) == MOTOR_UP));

		if (stopMotor) {
			if (motor_driver_set_motor_state(MOTOR, MOTOR_STOP) != HAL_OK) {
				error_handler();
			}
		}

		// Blink LED if on AUTOMATIC else keep LED on for MANUAL
		if (mode == MODE_AUTOMATIC) {
			brd_led_off();
		} else if (mode == MODE_MANUAL) {
			brd_led_on();
		}

		// Delay 50ms
		HAL_Delay(10);
	}

    return 0;
}

void hardware_init(void) {

	// Initialise LED
	board_init();

	// Initialise debug log
	debug_log_init(&huart2);

	// Initialise motor driver
	motor_driver_init();

	// Initialise rotary encoder
	encoder_init();

	// Initialise pushbuttons
	pb_init(); 

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    error_handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    error_handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    error_handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    error_handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void) {

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		error_handler();
	}
}

/**
 * @brief Handles initialisation errors
 * 
 */
void error_handler(void) {

	// Initialisation error shown by blinking LED (LD3) in pattern
    while (1) {

		for (int i = 0; i < 5; i++) {
			HAL_GPIO_TogglePin(LD3_PORT, LD3_PIN);
			HAL_Delay(100);
		}

		HAL_Delay(500);
	}
}