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

/*
	Implements CMSIS USART Driver interface with STM32 HAL UART for DAP SWO.
*/

#include "Driver_USART.h"
#include "stm32f0xx_hal.h"

#define DEFAULT_BAUDRATE 38400U

UART_HandleTypeDef huart2;
static ARM_USART_SignalEvent_t app_cb_event;

static HAL_StatusTypeDef SWO_UART_Init(uint32_t baudrate);

static int32_t Initialize (ARM_USART_SignalEvent_t cb_event) {
	app_cb_event = cb_event;
	
	if (SWO_UART_Init(DEFAULT_BAUDRATE) != HAL_OK)
		return ARM_DRIVER_ERROR;
	
	return ARM_DRIVER_OK;
}

static int32_t Uninitialize (void) {
	HAL_StatusTypeDef ret = HAL_UART_DeInit(&huart2);
	if (ret != HAL_OK)
		return ARM_DRIVER_ERROR;
	
	return ARM_DRIVER_OK;
}

static int32_t PowerControl (ARM_POWER_STATE state) {
	UNUSED(state);
	
	return ARM_DRIVER_OK;
}

static int32_t Receive (void *data, uint32_t num) {
	HAL_StatusTypeDef ret;
	ret = HAL_UART_Receive_IT (&huart2, data, (uint16_t) num);
	
	if (ret != HAL_OK)
		return ARM_DRIVER_ERROR;
	
	return ARM_DRIVER_OK;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	UNUSED(huart);
	
	// Called from IRQ Handler
	if (app_cb_event != NULL)
			app_cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	uint32_t event = 0;
	
	if (huart->ErrorCode & HAL_UART_ERROR_PE) {
		event |= ARM_USART_EVENT_RX_PARITY_ERROR;
	}
	
	if (huart->ErrorCode & HAL_UART_ERROR_FE) {
		event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
	}
			
	if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
		event |= ARM_USART_EVENT_RX_OVERFLOW;
	}
		
	if (huart->ErrorCode & (HAL_UART_ERROR_BUSY |
													HAL_UART_ERROR_NE)) {
		// not a proper error mapping.
		event |= ARM_USART_EVENT_RX_BREAK;
	}
		
	// Called from IRQ Handler
	if (app_cb_event != NULL)
			app_cb_event(event);
}
  
static uint32_t GetRxCount (void) {
	// There is no equivalent API in HAL, so hack into it.
	if (huart2.RxState == HAL_UART_STATE_BUSY_RX) {
		// Internally huart->RxXferCount represents: bytes pending.
		// Whats required is: bytes received.
		return (huart2.RxXferSize - huart2.RxXferCount);
	}
	
	return 0;
}

static int32_t Control (uint32_t control, uint32_t arg) {
	HAL_StatusTypeDef ret;
	
	switch (control & ARM_USART_CONTROL_Msk) {
		case ARM_USART_CONTROL_RX:
			if (arg == 0) {
				__HAL_UART_DISABLE(&huart2);
			} else {
				__HAL_UART_ENABLE(&huart2);
			}
			ret = HAL_OK;
			break;
		
		case ARM_USART_ABORT_RECEIVE:
			ret = HAL_UART_AbortReceive(&huart2);
			break;
			
		case ARM_USART_MODE_ASYNCHRONOUS:
			ret = HAL_UART_DeInit(&huart2);
			if (ret != HAL_OK)
				break;
			// arg contains baudrate.
			ret = SWO_UART_Init(arg);
			// Keep uart disabled??
			break;
			
		default:
			ret = HAL_ERROR;
			break;
	}
	
	if (ret != HAL_OK)
		return ARM_DRIVER_ERROR;
	
	return ARM_DRIVER_OK;
	
}

static ARM_USART_STATUS  GetStatus (void) {
	ARM_USART_STATUS status = {0};
	
	if (HAL_UART_GetState (&huart2) == HAL_UART_STATE_BUSY_RX) {
		status.rx_busy = 1;
	}
	
	return status;
}
  


ARM_DRIVER_USART Driver_USART2 = {
	NULL, // GetVersion
  NULL, // GetCapabilities
  Initialize, 
  Uninitialize, 
  PowerControl, 
  NULL, // Send
  Receive, 
  NULL, // Transfer
  NULL, // GetTxCount
  GetRxCount, 
  Control, 
  GetStatus, 
  NULL, // SetModemControl
  NULL, // GetModemStatus
};

static HAL_StatusTypeDef SWO_UART_Init(uint32_t baudrate)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = baudrate;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_8;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  
	return HAL_UART_Init(&huart2);
  
}





