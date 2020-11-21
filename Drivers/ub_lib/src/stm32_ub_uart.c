//--------------------------------------------------------------
// File     : stm32_ub_uart.c
// Datum    : 28.11.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, USART, MISC
// Funktion : UART (RS232) In und OUT
//            Receive wird per Interrupt behandelt
//
// Hinweis  : m�gliche Pinbelegungen
//            UART1 : TX:[PA9,PB6] RX:[PA10,PB7]
//            UART2 : TX:[PA2,PD5] RX:[PA3,PD6]
//            UART3 : TX:[PB10,PC10,PD8] RX:[PB11,PC11,PD9]
//            UART4 : TX:[PA0,PC10] RX:[PA1,PC11]
//            UART5 : TX:[PC12] RX:[PD2]
//            UART6 : TX:[PC6,PG14] RX:[PC7,PG9]
//            UART7 : TX:[PE8,PF7] RX:[PE7,PF6]
//            UART8 : TX:[PE1] RX:[PE0]
//
// Vorsicht : Als Endekennung beim Empfangen, muss der Sender
//            das Zeichen "0x0D" = Carriage-Return
//            an den String anh�ngen !!
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_uart.h"
extern UART_HandleTypeDef huart1;




//--------------------------------------------------------------
// Definition aller UARTs
// Reihenfolge wie bei UART_NAME_t
//--------------------------------------------------------------





//--------------------------------------------------------------
// init aller UARTs
//--------------------------------------------------------------
void UB_Uart_Init(void)
{
	__HAL_UART_ENABLE(&huart1);
    // RX-Puffer vorbereiten
    UART_RX.rx_buffer[0]=RX_END_CHR;
    UART_RX.wr_ptr=0;
    UART_RX.status=RX_EMPTY;
}

//--------------------------------------------------------------
// ein Byte per UART senden
//--------------------------------------------------------------
void UB_Uart_SendByte(uint8_t data)
{
  // warten bis altes Byte gesendet wurde
	while(HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY);
	if( HAL_UART_Transmit(&huart1, &data, 1,3000) !=HAL_OK){ Error_Handler(); }

}

//--------------------------------------------------------------
// einen String per UART senden
//--------------------------------------------------------------
void UB_Uart_SendString(uint8_t *ptr)
{
	uint8_t count=0;
	while(ptr[count++]);	// Count string length

	while(HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY);
	if( HAL_UART_Transmit(&huart1,ptr, count,3000) !=HAL_OK){ Error_Handler(); }

}


//--------------------------------------------------------------
// UART1-Interrupt
//--------------------------------------------------------------
void UB_IRQHandler(void) {

  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) {
	  __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
  }
}

