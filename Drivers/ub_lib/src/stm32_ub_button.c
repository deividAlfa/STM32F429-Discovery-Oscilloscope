//--------------------------------------------------------------
// File     : stm32_ub_button.c
// Datum    : 24.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, (TIM, MISC)
// Funktion : Button Funktionen
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_button.h"

extern TIM_HandleTypeDef UB_BUTTON_TIM;

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
#if BUTTON_USE_TIMER==1
  void P_Button_InitTIM(void);
  void P_Button_InitNVIC(void);
#endif




//--------------------------------------------------------------
// Definition aller Buttons
// Reihenfolge wie bei BUTTON_NAME_t
//
// Widerstand : [GPIO_PuPd_NOPULL,GPIO_PuPd_UP,GPIO_PuPd_DOWN]
//--------------------------------------------------------------
BUTTON_t BUTTON[] = {
  // Name    ,PORT , PIN
  { BLUE_BTN  ,BLUE_BTN_GPIO_Port, BLUE_BTN_Pin, RESET },  // PA0=User-Button auf dem Discovery-Board
};


//--------------------------------------------------------------
// Init aller Buttons
//--------------------------------------------------------------
void UB_Button_Init(void){
  
#if BUTTON_USE_TIMER==1
  // Init Timer
  P_Button_InitTIM();
  // Init NVIC
  P_Button_InitNVIC();
#endif
}


//--------------------------------------------------------------
// Status von einem Button auslesen (nicht entprellt)
// Return data :
//  -> wenn Button losgelassen = BTN_RELEASED
//  -> wenn Button gedrueckt   = BTN_PRESSED
//--------------------------------------------------------------
BUTTON_STATUS_t UB_Button_Read(BUTTON_NAME_t btn_name){

  if( HAL_GPIO_ReadPin(BUTTON[btn_name].BUTTON_PORT, BUTTON[btn_name].BUTTON_PIN) == GPIO_PIN_RESET){
    return(BTN_RELEASED);
  }
  else{ return(BTN_PRESSED); }
} 


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnPressed Ausdataung (entprellt)
// return_val, ist solange true wie der Button bet�tigt ist
//--------------------------------------------------------------
bool UB_Button_OnPressed(BUTTON_NAME_t btn_name){
  if(BUTTON[btn_name].BUTTON_AKT == RESET) {
    return(false);
  }
  else { return(true); }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnClick Ausdataung (entprellt)
// return_val, ist nur einmal true wenn der Button bet�tigt wurde
//--------------------------------------------------------------
bool UB_Button_OnClick(BUTTON_NAME_t btn_name)
{
  uint8_t old;
  static uint8_t old_state[BUTTON_ANZ];

  old = old_state[btn_name];
  old_state[btn_name] = BUTTON[btn_name].BUTTON_AKT;

  if(BUTTON[btn_name].BUTTON_AKT==RESET) {
    return(false);
  }
  else if(old!=RESET) {
    return(false);
  }
  else {
    return(true);
  }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnRelease Ausdataung (entprellt)
// return_val, ist nur einmal true wenn der Button losgelassen wurde
//--------------------------------------------------------------
bool UB_Button_OnRelease(BUTTON_NAME_t btn_name)
{
  uint8_t old;
  static uint8_t old_state[BUTTON_ANZ];

  old=old_state[btn_name];
  old_state[btn_name]=BUTTON[btn_name].BUTTON_AKT;

  if(BUTTON[btn_name].BUTTON_AKT!=RESET) {
    return(false);
  }
  else if(old==RESET) {
    return(false);
  }
  else {
    return(true);
  }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_Button_InitTIM(void){
	HAL_TIM_Base_Start_IT(&UB_BUTTON_TIM);
	__HAL_TIM_CLEAR_FLAG(&UB_BUTTON_TIM,TIM_FLAG_UPDATE);	// Clear flag
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_Button_InitNVIC(void)
{

}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// ISR vom Timer
//--------------------------------------------------------------
void UB_Button_Handler(void){
  BUTTON_NAME_t btn_name;
  uint8_t data;

  // Clear Interrupt Flag
	__HAL_TIM_CLEAR_FLAG(&UB_BUTTON_TIM,TIM_FLAG_UPDATE);	// Clear flag

  // Read all the buttons and store their values
  for(btn_name=0;btn_name<BUTTON_ANZ;btn_name++) {
    data=HAL_GPIO_ReadPin(BUTTON[btn_name].BUTTON_PORT, BUTTON[btn_name].BUTTON_PIN);
    BUTTON[btn_name].BUTTON_AKT=data;
  }
}
#endif
