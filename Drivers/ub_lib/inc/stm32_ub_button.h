//--------------------------------------------------------------
// File     : stm32_ub_button.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_BUTTON_H
#define __STM32F4_UB_BUTTON_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "main.h"


//--------------------------------------------------------------
// Aktivieren, Deaktivieren der entprellten Funktionen mit Timer
// 1=aktiv, 0=inaktiv
//--------------------------------------------------------------
#define  BUTTON_USE_TIMER      1  // Funktionen per Timer


#if BUTTON_USE_TIMER==1
  #include <stdbool.h>
#endif


//--------------------------------------------------------------
// Liste aller Buttons
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum 
{
  BLUE_BTN = 0    // BTN1 auf dem STM32F429-Discovery
}BUTTON_NAME_t;

#define  BUTTON_ANZ   1 // Anzahl von Button_NAME_t


//--------------------------------------------------------------
// Status eines Buttons
//--------------------------------------------------------------
typedef enum {
  BTN_RELEASED = 0,  // Button losgelassen
  BTN_PRESSED        // Button gedrueckt
}BUTTON_STATUS_t;



//--------------------------------------------------------------
// Timer f�r Button => TIM7
// Grundfrequenz = 2*APB1 (APB1=45MHz) => TIM_CLK=90MHz
// TIM_Frq = TIM_CLK/(periode+1)/(vorteiler+1)
// TIM_Frq = 20Hz => 50ms (nicht kleiner als 1ms einstellen)
//--------------------------------------------------------------
#if BUTTON_USE_TIMER==1
  #define   UB_BUTTON_TIM              htim5
#endif



//--------------------------------------------------------------
// Struktur eines Buttons
//--------------------------------------------------------------
typedef struct {
  BUTTON_NAME_t BUTTON_NAME;  // Name
  GPIO_TypeDef* BUTTON_PORT;  // Port
  const uint16_t BUTTON_PIN;  // Pin
  uint8_t BUTTON_AKT;         // Istdata
}BUTTON_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Button_Init(void);
void UB_Button_Handler(void);
BUTTON_STATUS_t UB_Button_Read(BUTTON_NAME_t btn_name);
#if BUTTON_USE_TIMER==1
  bool UB_Button_OnPressed(BUTTON_NAME_t btn_name);
  bool UB_Button_OnClick(BUTTON_NAME_t btn_name);
  bool UB_Button_OnRelease(BUTTON_NAME_t btn_name);
#endif

//--------------------------------------------------------------
#endif // __STM32F4_UB_BUTTON_H
