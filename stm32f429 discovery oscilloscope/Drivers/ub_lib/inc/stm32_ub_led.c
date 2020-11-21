//--------------------------------------------------------------
// File     : stm32_ub_led.c
// Datum    : 24.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO
// Funktion : LED Funktionen
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_led.h"


//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
LED_t LED[] = {
  // Name    ,PORT , PIN       , CLOCK              , Init
  {LED_GREEN ,LED_GREEN_GPIO_Port ,LED_GREEN_Pin },   // PG13=Gruene LED auf dem Discovery-Board
  {LED_RED ,LED_RED_GPIO_Port ,LED_RED_Pin }   // PG14=Rote LED auf dem Discovery-Board
};



//--------------------------------------------------------------
// Init aller LEDs
//--------------------------------------------------------------
void UB_Led_Init(void)
{
}


//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void UB_Led_Off(LED_NAME_t led_name)
{
	HAL_GPIO_WritePin(LED[led_name].LED_PORT,LED[led_name].LED_PIN,RESET);
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void UB_Led_On(LED_NAME_t led_name)
{
	HAL_GPIO_WritePin(LED[led_name].LED_PORT,LED[led_name].LED_PIN,SET);
} 

//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void UB_Led_Toggle(LED_NAME_t led_name)
{
	HAL_GPIO_TogglePin(LED[led_name].LED_PORT,LED[led_name].LED_PIN);
}

//--------------------------------------------------------------
// LED ein- oder ausschalten
//--------------------------------------------------------------
void UB_Led_Switch(LED_NAME_t led_name, LED_STATUS_t data)
{
  if(data==LED_OFF) {
    UB_Led_Off(led_name);
  }
  else {
    UB_Led_On(led_name);
  }
}
