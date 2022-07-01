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
#include "debug_log.h"
#include "tempest.h"

/* STM32 Includes */

/* Variable Declarations */
UART_HandleTypeDef huart2;

/* Private #defines */

/* Variable Declarations */

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
	char m[40];

	// Main program loop
	while (1) {

		// Update the system state
		// tempest_update_system_state();

		// // Update the motor state
		// tempest_update_motor_state();

		// // Update the mode indicator
		// tempest_update_mode_indicator();

		// Delay for 50ms
		HAL_Delay(50);
    // brd_led_toggle();

		int compValue = ((COMP1->CSR & (0x01 << 30)) == (0x01 << 30)) ? 1 : 0;
		sprintf(m, "Counter: %lu\tCCR1: %lu\tComp: %i\r\n", TIM1->CNT, TIM1->CCR1, compValue);

    	debug_prints(m);
	}
    return 0;
}

void hardware_init(void) {

	// Initialise debug log
	debug_log_init(&huart2);

	// Initialise tempest hardware
	tempest_hardware_init();
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