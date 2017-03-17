#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}


// This function handles CAN TX and USB high priority interrupts.
void USB_HP_CAN_TX_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan);
}


// This function handles CAN RX0 and USB low priority interrupts.
void USB_LP_CAN_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan);
}


// This function handles CAN RX1 interrupt.
void CAN_RX1_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan);
}


// This function handles CAN SCE interrupt.
void CAN_SCE_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan);
}
