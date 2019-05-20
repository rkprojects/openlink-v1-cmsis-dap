/*

Copyright 2019 Ravikiran Bukkasagara <contact@ravikiranb.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "main.h"
#include "usb_device.h"
#include "DAP.h"
#include "DAP_config.h"
#include "cmsis_compiler.h"
#include "usbd_custom_hid_if.h"



extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t  USB_DAP_Requests [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  
uint8_t  USB_DAP_Responses[DAP_PACKET_COUNT][DAP_PACKET_SIZE];

static uint32_t CommonIndex;  //Current "Request - Response" being processed index linked together.
static uint32_t ResponseIndexSend;
static uint32_t ResponsePendingCount;

volatile uint32_t RequestPendingCount;
extern volatile uint32_t RequestPauseProcessing;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void Send_USB_DAP_Response(void);
static void Timer_Init(void);


int main(void)
{
  uint32_t skip_sending_response;
	
	HAL_Init();

  // Set System clock to 48MHz. 
	SystemClock_Config();

  CommonIndex = 0;
	ResponseIndexSend = 0;
	ResponsePendingCount = 0;
	RequestPendingCount = 0;
	
	MX_GPIO_Init();
	
	MX_USB_DEVICE_Init();
	
	// For DAP time stamp.
	Timer_Init();
	
  DAP_Setup();
  
	while (1) 
	{
		/* 
			Cortex-M0 does not support synchronizing instructions.
			RequestPendingCount variable has a potential racing condition.
			Disabling interrupts while update is a choice.
			Systick interrupt will cause wakeup in case of out of sync between
			USB interrupt and WFI call here.
		*/
		
		if (RequestPendingCount == 0) {
			__WFI();
		}
		
		while ((RequestPendingCount > 0) && (ResponsePendingCount < DAP_PACKET_COUNT)) {
			
			if (USB_DAP_Requests[CommonIndex][0] == ID_DAP_QueueCommands) {
				// Pause request processing until all queue commands are received.
				if (RequestPauseProcessing)
					break;
				
				USB_DAP_Requests[CommonIndex][0] = ID_DAP_ExecuteCommands;
				
				// Avoid sending usb response during processing queued commands.
				skip_sending_response = 1;
			}
			else {
				skip_sending_response = 0;
			}
			
			DAP_ExecuteCommand(USB_DAP_Requests[CommonIndex], USB_DAP_Responses[CommonIndex]);
			
			RequestPendingCount--;
			ResponsePendingCount++;
			CommonIndex++;
			if (CommonIndex >= DAP_PACKET_COUNT) {
				CommonIndex = 0;
			}
			
			if (skip_sending_response == 0)
				Send_USB_DAP_Response();
		}
		
		if (ResponsePendingCount > 0) {
			Send_USB_DAP_Response();
		}
	}
}

static void Send_USB_DAP_Response(void) {
	
	USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef*) hUsbDeviceFS.pClassData;
			
	if (hhid->state == CUSTOM_HID_IDLE) {
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, USB_DAP_Responses[ResponseIndexSend], DAP_PACKET_SIZE);
		ResponseIndexSend++;
		if (ResponseIndexSend >= DAP_PACKET_COUNT) {
			ResponseIndexSend = 0;
		}
		ResponsePendingCount--;
	}
}

static void Timer_Init(void) {
	/* Chain TIM1 (prescaler) -> TIM3 (counter) to create 
		free-running 32bit timer {TIM3, TIM1} with 1us tick.
	*/
	
	// Timer HAL APIs are not used anywhere else.
	TIM_HandleTypeDef    TimerHandle;
	TIM_SlaveConfigTypeDef TIM3_SlaveConfig;
	TIM_MasterConfigTypeDef TIM1_MasterConfig;
	
	// Configure TIM3 as slave.
	memset(&TimerHandle, 0, sizeof(TimerHandle));
	
	TimerHandle.Instance = TIM3;
	TimerHandle.Init.Period            = 0xffff;
  TimerHandle.Init.Prescaler         = 0;
  TimerHandle.Init.ClockDivision     = 0;
  TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimerHandle.Init.RepetitionCounter = 0;
  TimerHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&TimerHandle) != HAL_OK) {
    Error_Handler();
  }
	
	TIM3_SlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	TIM3_SlaveConfig.InputTrigger = TIM_TS_ITR0;
	TIM3_SlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING; // don't care value
	TIM3_SlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1; // don't care value
	TIM3_SlaveConfig.TriggerFilter = 0; // don't care value
	
	if (HAL_TIM_SlaveConfigSynchronization(&TimerHandle, &TIM3_SlaveConfig) != HAL_OK) {
		Error_Handler();
	}

  if (HAL_TIM_Base_Start(&TimerHandle) != HAL_OK) {
    Error_Handler();
  }
	
	// configure TIM1 as master.
	memset(&TimerHandle, 0, sizeof(TimerHandle));
	
	TimerHandle.Instance = TIM1;
	TimerHandle.Init.Period            = 0xffff;
  TimerHandle.Init.Prescaler         = (HAL_RCC_GetHCLKFreq() / TIMESTAMP_CLOCK) - 1;
  TimerHandle.Init.ClockDivision     = 0;
  TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimerHandle.Init.RepetitionCounter = 0;
  TimerHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&TimerHandle) != HAL_OK) {
    Error_Handler();
  }
	
	TIM1_MasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	TIM1_MasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	
	if (HAL_TIMEx_MasterConfigSynchronization(&TimerHandle,  &TIM1_MasterConfig) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_TIM_Base_Start(&TimerHandle) != HAL_OK) {
    Error_Handler();
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInit = {0};
  RCC_ClkInitTypeDef RCC_ClkInit = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInit.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInit.HSEState = RCC_HSE_ON;
  RCC_OscInit.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInit.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInit.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInit.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInit.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInit.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInit.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enables the Clock Security System 
  */
  HAL_RCC_EnableCSS();
}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_Init = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA0 PA1 PA4 PA5 
                           PA6 PA7 */
  GPIO_Init.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_Init.Mode = GPIO_MODE_INPUT;
  GPIO_Init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_Init);

  /*Configure GPIO pin : PB1 */
  GPIO_Init.Pin = GPIO_PIN_1;
  GPIO_Init.Mode = GPIO_MODE_INPUT;
  GPIO_Init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);
}

void Error_Handler(void)
{
  while(1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
	while(1);
}
#endif /* USE_FULL_ASSERT */
