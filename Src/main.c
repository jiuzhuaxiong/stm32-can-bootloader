//
// CAN Bootloader
//

#include "stm32f3xx_hal.h"
#include "config.h"

#define NUM_OF_PAGES               (NUM_PAGES_TOTAL - MAIN_PROGRAM_PAGE_NUMBER)
#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000000 + (FLASH_PAGE_SIZE*MAIN_PROGRAM_PAGE_NUMBER) // start of page 3


// Public variables
CAN_HandleTypeDef hcan;
pFunction                     JumpAddress;
uint8_t                       PageBuffer[FLASH_PAGE_SIZE];
volatile int                  PageBufferPtr;
uint8_t                       PageIndex;
uint32_t                      PageCRC;
volatile uint8_t              blState;


// Private variables
CanTxMsgTypeDef        canTxMessage;
CanRxMsgTypeDef        canRxMessage;
FLASH_EraseInitTypeDef eraseInitStruct;
CRC_HandleTypeDef hcrc;


// Private methods
static void __clock_init(void);
static void __crc_init(void);
static void __can_init(void);
static void __micro_memcpy(void *dest, void *src, uint32_t n);
static void __jump_to_application();
static void __respond(uint8_t response);


int main(void)
{
	__hal_init();
	__clock_init();
	__can_init();
	__crc_init();

	__GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	// Set receive/transmit buffers and start CAN reception
	hcan.pTxMsg = &canTxMessage;
	hcan.pRxMsg = &canRxMessage;
	HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);


	// Wait for CAN bootload message
	for(uint32_t i=0; i<200; i++)
	{
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
	  HAL_Delay(100);
	}

	// Timed out waiting for host
	if (blState == WAIT_HOST) {
		__jump_to_application();
	}
	while (1)
	{
	}
}


// Initialize system clock
void __clock_init(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}


// Callback triggered when a CAN message is received
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* CanHandle)
{
	if (blState == PAGE_PROG)
	{
		memcpy(&PageBuffer[PageBufferPtr], CanHandle->pRxMsg->Data, CanHandle->pRxMsg->DLC);
		PageBufferPtr += CanHandle->pRxMsg->DLC;

		if (PageBufferPtr == FLASH_PAGE_SIZE)
		{
			HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn);
			HAL_NVIC_DisableIRQ(CAN_RX1_IRQn);

			volatile uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t*) PageBuffer, FLASH_PAGE_SIZE / 4);

			if (/*crc == PageCRC && */PageIndex <= NUM_OF_PAGES)
			{
				HAL_FLASH_Unlock();

				uint32_t PageError = 0;

				eraseInitStruct.TypeErase = TYPEERASE_PAGES;
				eraseInitStruct.PageAddress = MAIN_PROGRAM_START_ADDRESS + PageIndex * FLASH_PAGE_SIZE;
				eraseInitStruct.NbPages = 1;

				HAL_FLASHEx_Erase(&eraseInitStruct, &PageError);

				for (int i = 0; i < FLASH_PAGE_SIZE; i += 4)
				{
					HAL_FLASH_Program(TYPEPROGRAM_WORD, MAIN_PROGRAM_START_ADDRESS + PageIndex * FLASH_PAGE_SIZE + i, *(uint32_t*) &PageBuffer[i]);
				}

				HAL_FLASH_Lock();

				__respond(CAN_RESP_OK);
			}

			else
			{
				__respond(CAN_RESP_ERROR);
			}

			blState = IDLE;

			HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
			HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
		}

		HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);
		return;
	}

	switch (CanHandle->pRxMsg->Data[0])
	{
		case CMD_HOST_INIT:
		{
			blState = IDLE;
			__respond(CAN_RESP_OK);
		} break;

		case CMD_PAGE_PROG:
		{
			if (blState == IDLE)
			{
				// Zero page buffer array
				for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i++)
				{
					PageBuffer[i] = 0;
				}

				memcpy(&PageCRC, &CanHandle->pRxMsg->Data[2], sizeof(uint32_t));
				PageIndex = CanHandle->pRxMsg->Data[1];
				PageCRC = CanHandle->pRxMsg->Data[2]<<24 | CanHandle->pRxMsg->Data[3]<<16 | CanHandle->pRxMsg->Data[4]<<8 | CanHandle->pRxMsg->Data[5];
				blState = PAGE_PROG;
				PageBufferPtr = 0;

			} else {
				// Should never get here
			}
		} break;

		case CMD_BOOT:
		{
			__respond(CAN_RESP_OK);
			__jump_to_application();
		} break;

		default:
			break;
	}

	HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);
}


// Initialize CRC peripheral
static void __crc_init(void)
{
	__CRC_CLK_ENABLE();
	hcrc.Instance = CRC;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
	HAL_CRC_Init(&hcrc);
}


// Small memcpy implementation to minimize code size
void __micro_memcpy(void *dest, void *src, uint32_t n)
{
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
   for (uint32_t i=0; i<n; i++)
       cdest[i] = csrc[i];
}


// Initialize the CAN bus
void __can_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__CAN_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();

	// Initialize CAN rx/tx gpios
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_TIM1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure peripheral
	hcan.Instance = CAN;
	hcan.Init.Prescaler = 12 / 2; // 500kbaud
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SJW = CAN_SJW_1TQ;
	hcan.Init.BS1 = CAN_BS1_4TQ;
	hcan.Init.BS2 = CAN_BS2_3TQ;
	hcan.Init.TTCM = DISABLE;
	hcan.Init.ABOM = ENABLE; // automatic off-bus management
	hcan.Init.AWUM = DISABLE;
	hcan.Init.NART = ENABLE;
	hcan.Init.RFLM = DISABLE;
	hcan.Init.TXFP = DISABLE;
	hcan.pTxMsg = NULL;

	// Initialize peripheral
	HAL_CAN_Init(&hcan);

	// Configure filtering
	CAN_FilterConfTypeDef filter;
    filter.FilterIdHigh = CAN_LISTEN_ADDRESS << 5;
    filter.FilterIdLow =  0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow =  0;
    filter.FilterMode = CAN_FILTERMODE_IDLIST;
    filter.FilterScale = CAN_FILTERSCALE_16BIT;
    filter.FilterNumber = 0;
    filter.FilterFIFOAssignment = CAN_FIFO0;
    filter.BankNumber = 0;
    filter.FilterActivation = ENABLE;
	HAL_CAN_ConfigFilter(&hcan, &filter);

    HAL_NVIC_SetPriority(USB_HP_CAN_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN_TX_IRQn);

    HAL_NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);

    HAL_NVIC_SetPriority(CAN_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN_SCE_IRQn);
}


void __jump_to_application()
{
	JumpAddress = *(__IO pFunction*)(MAIN_PROGRAM_START_ADDRESS + 4);
	__set_MSP(*(__IO uint32_t*) MAIN_PROGRAM_START_ADDRESS);
	HAL_DeInit();
	JumpAddress();
}

void __respond(uint8_t response)
{
	hcan.pTxMsg->StdId = CAN_TRANSMIT_ADDRESS;
	hcan.pTxMsg->DLC = 1;
	hcan.pTxMsg->Data[0] = response;
	HAL_CAN_Transmit_IT(&hcan);
}
