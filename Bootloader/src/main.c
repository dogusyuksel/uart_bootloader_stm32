#include "main.h"
#include "gpio.h"
#include "usart.h"
#include <stdint.h>

typedef void (*pFunction)(void);

void myjump(uint32_t address) {
    uint32_t jump_address = *(uint32_t *) (address + 4);
    pFunction jump_function = (pFunction) jump_address;

    // De initialize RCC, HAL and disable IRQs
    HAL_RCC_DeInit();
    HAL_DeInit();
    __disable_irq();

    // Reset the SysTick timer
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Set Vector tab offset
    SCB->VTOR = address;
    // Update the Master Stack Pointer
    __set_MSP(*(uint32_t *) address);
    // Activate (privileged) thread mode
    __set_CONTROL(0);
    // Enable IRQs before we jump into the firmware
    __enable_irq();

    jump_function();
}

void SystemClock_Config(void);

int main(void) {
  uint8_t counter = 0;

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();

  while (1) {
    HAL_UART_Transmit(&huart3, (uint8_t *)"hello from bootloader\0", 22, 200);
    HAL_Delay(1000);
    counter++;
    if (counter >= 10) {
        //check the FW exists
        //then jump
        myjump(0x08004000);
    }
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */
