//--------------------------------------------------------------
// File     : stm32_ub_sdram.c
// Datum    : 24.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, FMC
// Funktion : Externes SDRAM [8MByte]
//            Typ = IS42S16400J [64Mbit, 1M x 16bit x 4Bank, 7ns]
//            Der Zugriff erfolgt ueber den FMC-Controller
//
// Hinweis  : Das SDRAM benutzt die CPU-Pins :
//             PB5  = SDCKE1 (CKE)      PF0  = A0
//             PB6  = SDNE1  (/CS)      PF1  = A1
//             PC0  = SDNWE  (/WE)      PF2  = A2
//             PD0  = D2                PF3  = A3
//             PD1  = D3                PF4  = A4
//             PD8  = D13               PF5  = A5
//             PD9  = D14               PF11 = SDNRAS (/RAS)
//             PD10 = D15               PF12 = A6
//             PD14 = D0                PF13 = A7
//             PD15 = D1                PF14 = A8
//             PE0  = NBL0   (LDQM)     PF15 = A9
//             PE1  = NBL1   (UDQM)     PG0  = A10
//             PE7  = D4                PG1  = A11
//             PE8  = D5                PG4  = BA0    (BANK A0)
//             PE9  = D6                PG5  = BA1    (BANK A1)
//             PE10 = D7                PG8  = SDCLK  (CLK)
//             PE11 = D8                PG15 = SDNCAS (/CAS)
//             PE12 = D9
//             PE13 = D10
//             PE14 = D11
//             PE15 = D12
//
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_sdram.h"
extern SDRAM_HandleTypeDef hsdram1;

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void SDRAM_InitSequence(void);
void P_SDRAM_delay(uint32_t nCount);



//--------------------------------------------------------------
// Init vom externen SDRAM
// Return_wert :
//  -> ERROR   , wenn SDRAM nicht gefunden wurde
//  -> SUCCESS , wenn SDRAM gefunden wurde
//--------------------------------------------------------------
ErrorStatus UB_SDRAM_Init(void){
  ErrorStatus ret_value=ERROR;
  uint16_t old_Val,new_Val,tmp;
  static uint8_t init_ok=0;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    if(init_ok==1) {
      return(SUCCESS);
    }
    else {
      return(ERROR);
    }
  }

  //-----------------------------------------
  // check ob SDRAM vorhanden ist
  // schreib-/lese-Test auf Adr 0x00
  //-----------------------------------------


  if( HAL_SDRAM_Read_16b(&hsdram1, SDRAM_START_ADR, &old_Val, 1) != HAL_OK){ Error_Handler(); }	// Save old value
  tmp=0x5A3C;
  if( HAL_SDRAM_Write_16b(&hsdram1, SDRAM_START_ADR, &tmp, 1) != HAL_OK){ Error_Handler(); }		// Write new value
  if( HAL_SDRAM_Read_16b(&hsdram1, SDRAM_START_ADR, &new_Val, 1) != HAL_OK){ Error_Handler(); }	// Read new value
  if( HAL_SDRAM_Write_16b(&hsdram1, SDRAM_START_ADR, &old_Val, 1) != HAL_OK){ Error_Handler(); } // Write original value
  if(new_Val==0x5A3C) ret_value=SUCCESS; // Check the new value read is the same as the written value

  // Store init result
  if(ret_value==SUCCESS) {
    init_ok=1;
  }
  else {
    init_ok=2;
  }

  return(ret_value);
}



/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* Set the refresh rate counter */

/* (15.62 us x Freq) - 20 */

//#define REFRESH_COUNT      ((uint32_t)1386)  /* SDRAM refresh counter from forum*/

#define REFRESH_COUNT      ((uint32_t)683)  /* SDRAM refresh counter old setting */
#define SDRAM_TIMEOUT      ((uint32_t)0xFFFF)

/**
 * @brief FMC SDRAM Mode definition register defines
 */

#define SDRAM_MODEREG_BURST_LENGTH_1       ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2       ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4       ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8       ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED   ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2       ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3       ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD  ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE   ((uint16_t)0x0200)

void SDRAM_InitSequence(void) {

	FMC_SDRAM_CommandTypeDef FMC_SDRAMCommandStructure;
	uint32_t tmpr = 0;

	/* Step 1 --------------------------------------------------------------------*/
	/* Configure a clock configuration enable command */
	FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before sending command
	HAL_SDRAM_SendCommand(&hsdram1, &FMC_SDRAMCommandStructure, SDRAM_TIMEOUT);

	/* Step 2 --------------------------------------------------------------------*/
	/* Configure a PALL (precharge all) command */
	FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_PALL;
	FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before sending command
	HAL_SDRAM_SendCommand(&hsdram1, &FMC_SDRAMCommandStructure, SDRAM_TIMEOUT);

	/* Step 3 --------------------------------------------------------------------*/
	/* Configure a Auto-Refresh command */
	FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	FMC_SDRAMCommandStructure.AutoRefreshNumber = 4;
	FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before sending command
	HAL_SDRAM_SendCommand(&hsdram1, &FMC_SDRAMCommandStructure, SDRAM_TIMEOUT);

	/* Step 4 --------------------------------------------------------------------*/
	/* Program the external memory mode register */
	tmpr = (uint32_t)	SDRAM_MODEREG_BURST_LENGTH_1     |
						SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL  |
						SDRAM_MODEREG_CAS_LATENCY_3      |
						SDRAM_MODEREG_OPERATING_MODE_STANDARD |
						SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	/* Step 5 --------------------------------------------------------------------*/
	/* Configure a load Mode register command*/
	FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.ModeRegisterDefinition = tmpr;
	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before sending command
	HAL_SDRAM_SendCommand(&hsdram1, &FMC_SDRAMCommandStructure, SDRAM_TIMEOUT);



	/* Step 6 --------------------------------------------------------------------*/
	/* Set the refresh rate counter */
	/* (7.81 us x Freq) - 20 */
	/* Set the device refresh counter */
	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before sending command
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, REFRESH_COUNT); // local

	while( (HAL_SDRAM_GetState(&hsdram1) & HAL_SDRAM_STATE_BUSY) );					// Check Busy Flag before returning
}



