//--------------------------------------------------------------
// File     : stm32_ub_touch_stmpe811.c
// Datum    : 02.11.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, STM32_UB_&hi2c3
// Funktion : Touch-Controller (Chip = STMPE811)
//
// Hinweis  : Settings :
//            I2C-Slave-ADR = [0x82]
//            FRQ-Max = 100kHz
//            &hi2c3 [SCL=PA8, SDA=PC9]
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_touch_stmpe811.h"
extern I2C_HandleTypeDef hi2c3;


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_Touch_Reset(void);
uint8_t P_Touch_FnctCmd(uint8_t Fct, FunctionalState NewState);
void P_Touch_Config(void);
uint8_t P_Touch_IOAFConfig(uint8_t IO_Pin, FunctionalState NewState);
static uint16_t P_Touch_Read_X(void);
static uint16_t P_Touch_Read_Y(void);
uint16_t P_Touch_Read_16b(uint32_t RegisterAddr);


//--------------------------------------------------------------
// Init vom Touch
// Return_data :
//  -> ERROR   , wenn Touch nicht gefunden wurde
//  -> SUCCESS , wenn Touch OK
//--------------------------------------------------------------
ErrorStatus UB_Touch_Init(void)
{
  uint16_t stmpe_id=0;

  // check for STMPE811
  stmpe_id = P_Touch_Read_16b(IOE_REG_CHIP_ID);		//Read Address 0
  if(stmpe_id!=STMPE811_ID) {
    return(ERROR);
  }

  // Generate SW-Reset
  P_Touch_Reset();

  // init
  P_Touch_FnctCmd(IOE_ADC_FCT, ENABLE);
  P_Touch_Config();

  return(SUCCESS);
}


//--------------------------------------------------------------
// auslesen vom Touch-Status und der Touch-Koordinaten
// Return_data :
//  -> ERROR   , wenn Touch nicht gefunden wurde
//  -> SUCCESS , wenn Touch OK
//
// Touch_Data.status : [TOUCH_PRESSED, TOUCH_RELEASED]
// Touch_Data.xp     : [0...239]
// Touch_Data.yp     : [0...319] 
//--------------------------------------------------------------
ErrorStatus UB_Touch_Read(void)
{
  uint32_t xDiff, yDiff , x , y;
  static uint32_t _x = 0, _y = 0;
  uint8_t i2c_data;
  if(HAL_I2C_Mem_Read(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_TP_CTRL,1,&i2c_data,1,3000)!=HAL_OK){
	  __HAL_I2C_DISABLE(&hi2c3);
	  HAL_Delay(5);
	  __HAL_I2C_ENABLE(&hi2c3);
	  return(ERROR);
	  //Error_Handler();
  }
  if(i2c_data==0xFF) return(ERROR);
  
  if((i2c_data&0x80)==0) {
    Touch_Data.status = TOUCH_RELEASED;
  }
  else {
    Touch_Data.status = TOUCH_PRESSED;
  }

  if(Touch_Data.status==TOUCH_PRESSED) {
    x = P_Touch_Read_X();
    y = P_Touch_Read_Y();
    xDiff = x > _x? (x - _x): (_x - x);
    yDiff = y > _y? (y - _y): (_y - y);
    if (xDiff + yDiff > 5)
    {
      _x = x;
      _y = y;
    }
  }
  
  Touch_Data.xp = _x;
  Touch_Data.yp = _y;
  
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_FIFO_STA,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x00;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_FIFO_STA,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }


  return(SUCCESS);
}


//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
void P_Touch_Reset(void){
	uint8_t data;
	data=2;
	if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_SYS_CTRL1,1,&data,1,3000)!=HAL_OK){ Error_Handler(); }
	HAL_Delay(100);
	data=0;
	if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_SYS_CTRL1,1,&data,1,3000)!=HAL_OK){ Error_Handler(); }
}


//--------------------------------------------------------------
// interne Funktion
// return : 0=ok, >0 = error
//--------------------------------------------------------------
uint8_t P_Touch_FnctCmd(uint8_t Fct, FunctionalState NewState)
{
  uint8_t i2c_data;

  if(HAL_I2C_Mem_Read(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_SYS_CTRL2,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  if(i2c_data==0xFF) return(1);

  if (NewState != DISABLE) {
	  i2c_data &= ~(uint8_t)Fct;
  }
  else {
	  i2c_data |= (uint8_t)Fct;
  }
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_SYS_CTRL2,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  
  return(0);
}


//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
void P_Touch_Config(void){
  uint8_t i2c_data;
  P_Touch_FnctCmd(IOE_TP_FCT, ENABLE);
  i2c_data=0x49;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_ADC_CTRL1,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  HAL_Delay(100);
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_ADC_CTRL2,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  P_Touch_IOAFConfig((uint8_t)TOUCH_IO_ALL, DISABLE);
  i2c_data=0x9A;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_TP_CFG,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_FIFO_TH,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_FIFO_STA,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x00;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_FIFO_STA,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_TP_FRACT_XYZ,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x01;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_TP_I_DRIVE,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0x03;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_TP_CTRL,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  i2c_data=0xFF;
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_INT_STA,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }

  Touch_Data.status = TOUCH_RELEASED;
  Touch_Data.xp = 0;
  Touch_Data.yp = 0;
}

//--------------------------------------------------------------
// interne Funktion
// return : 0=ok, >0 = error
//--------------------------------------------------------------
uint8_t P_Touch_IOAFConfig(uint8_t IO_Pin, FunctionalState NewState)
{
  uint8_t i2c_data;

  if(HAL_I2C_Mem_Read(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_GPIO_AF,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  if(i2c_data==0XFF) return(1);

  if (NewState != DISABLE) {
	  i2c_data |= (uint8_t)IO_Pin;
  }
  else {
	  i2c_data &= ~(uint8_t)IO_Pin;
  }
  if(HAL_I2C_Mem_Write(&hi2c3,STMPE811_I2C_ADDR,IOE_REG_GPIO_AF,1,&i2c_data,1,3000)!=HAL_OK){ Error_Handler(); }
  return(0);
}


//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
static uint16_t P_Touch_Read_X(void)
{
  int32_t x, xr;

  x = P_Touch_Read_16b(IOE_REG_TP_DATA_X);

  if(x <= 3000) {
    x = 3870 - x;
  }
  else {
    x = 3800 - x;
  }

  xr = x / 15;

  if(xr <= 0) {
    xr = 0;
  }
  else if (xr >= 240) {
    xr = 239;
  }

  return (uint16_t)(xr);
}


//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
static uint16_t P_Touch_Read_Y(void)
{
  int32_t y, yr;

  y = P_Touch_Read_16b(IOE_REG_TP_DATA_Y);
  y -= 360;
  yr = y / 11;

  if(yr <= 0) {
    yr = 0;
  }
  else if (yr >= 320) {
    yr = 319;
  }

  return (uint16_t)(yr);
}


//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
uint16_t P_Touch_Read_16b(uint32_t RegisterAddr){
	uint8_t id_H, id_L;
	if(HAL_I2C_Mem_Read(&hi2c3,STMPE811_I2C_ADDR,RegisterAddr,1,&id_H,1,3000)!=HAL_OK){ Error_Handler(); }
	if(HAL_I2C_Mem_Read(&hi2c3,STMPE811_I2C_ADDR,RegisterAddr+1,1,&id_L,1,3000)!=HAL_OK){ Error_Handler(); }
	return ((uint16_t)id_H<<8 | id_L);
}


