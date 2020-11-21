//--------------------------------------------------------------
// File     : stm32_ub_uart.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_UART_H
#define __STM32F4_UB_UART_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "main.h"



//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM1 = 0   // COM1 (TX=PA9, RX=PA10)
}UART_NAME_t;

#define  UART_ANZ   1 // Anzahl von UART_NAME_t


//--------------------------------------------------------------
// Endekennung beim Senden
//--------------------------------------------------------------
typedef enum {
  NONE = 0,  // keine Endekennung
  LFCR,      // LineFeed + CarriageReturn (0x0A,0x0D)
  CRLF,      // CarriageReturn + LineFeed (0x0D,0x0A)
  LF,        // nur LineFeed (0x0A)
  CR         // nur CarriageReturn (0x0D)
}UART_LASTBYTE_t;


//--------------------------------------------------------------
// Status beim Empfangen
//--------------------------------------------------------------
typedef enum {
  RX_EMPTY = 0,  // nichts empfangen
  RX_READY,      // es steht was im Empfangspuffer
  RX_FULL        // RX-Puffer ist voll
}UART_RXSTATUS_t;



//--------------------------------------------------------------
// Defines fuer das Empfangen
//--------------------------------------------------------------
#define  RX_BUF_SIZE   50    // Gr�sse vom RX-Puffer in Bytes
#define  RX_FIRST_CHR  0x20  // erstes erlaubte Zeichen (Ascii-data)
#define  RX_LAST_CHR   0x7E  // letztes erlaubt Zeichen (Ascii-data)
#define  RX_END_CHR    0x0D  // Endekennung (Ascii-data)


//--------------------------------------------------------------
// Struktur f�r UART_RX
//--------------------------------------------------------------
typedef struct {
  char rx_buffer[RX_BUF_SIZE]; // RX-Puffer
  uint8_t wr_ptr;              // Schreib Pointer
  UART_RXSTATUS_t status;      // RX-Status
}UART_RX_t;
UART_RX_t UART_RX;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Uart_Init(void);
void UB_Uart_SendByte(uint8_t data);
void UB_Uart_SendString(uint8_t *ptr);
void UB_IRQHandler(void);



//--------------------------------------------------------------
#endif // __STM32F4_UB_UART_H
