//--------------------------------------------------------------
// File     : adc.c
// Datum    : 05.01.2014
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : ADC, DMA, TIM
// Funktion : ADC
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "adc.h"


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef ADC_TIM;

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_ADC_InitIO(void);
void P_ADC_InitDMA_DoubleBuffer(void);
void P_ADC_InitNVIC(void);
void P_ADC_InitADC(void);
void P_ADC_Start(void);
void P_ADC_InitTimer2(void);
void P_ADC_Clear(void);
void ADC_searchTrigger_A1(void);
void ADC_searchTrigger_B1(void);
void ADC_searchTrigger_A2(void);
void ADC_searchTrigger_B2(void);



//--------------------------------------------------------------
// init vom ADC1 und ADC2 im DMA Mode
// und starten der zyklischen Wandlung
//--------------------------------------------------------------
void ADC_Init_ALL(void)
{
  // init aller Variablen
  ADC_UB.status=ADC_VORLAUF;
  ADC_UB.trigger_pos=0;
  ADC_UB.trigger_quarter=0;
  ADC_UB.dma_status=0;

  P_ADC_Clear();
  P_ADC_InitADC();
  P_ADC_Start();
}


//--------------------------------------------------------------
// �ndern der Frequenz vom Timer2
// (Timebase der Abtastrate)
//
// n : [0...16]
//--------------------------------------------------------------
void ADC_change_Frq(uint8_t n)
{

  uint32_t prescaler, period;

  // Timer anhalten
  __HAL_TIM_DISABLE(&ADC_TIM);
  switch(n) {
    case 0 : // 5s=>5Hz=5s
      prescaler=499;period=35999;
    break;
    case 1 : // 2s=>12,5Hz=80ms
      prescaler=499;period=14399;
    break;
    case 2 : // 1s=>25Hz=40ms
      prescaler=499;period=7199;
    break;
    case 3 : // 500ms=>50Hz=20ms
      prescaler=499;period=3599;
    break;
    case 4 : // 200ms=>125Hz=8ms
      prescaler=499;period=1439;
    break;
    case 5 : // 100ms=>250Hz=4ms
      prescaler=499;period=719;
    break;
    case 6 : // 50ms=>500Hz=2ms
      prescaler=499;period=359;
    break;
    case 7 : // 20ms=>1,25kHz=800us
      prescaler=499;period=143;
    break;
    case 8 : // 10ms=>2,5kHz400us
      prescaler=499;period=71;
    break;
    case 9 : // 5ms=>5kHz=200us
      prescaler=499;period=35;
    break;
    case 10 : // 2ms=>12,5kHz=80us
      prescaler=0;period=7199;
    break;
    case 11 : // 1ms=>25kHz=40us
      prescaler=0;period=3599;
    break;
    case 12 : // 500us=>50kHz=20us
      prescaler=0;period=1799;
    break;
    case 13 : // 200us=>125kHz=8us
      prescaler=0;period=719;
    break;
    case 14 : // 100us=>250kHz=4us
      prescaler=0;period=359;
    break;
    case 15 : // 50us=>500kHz=2us
      prescaler=0;period=179;
    break;
    case 16 : // 25us=>1MHz=1us
      prescaler=0;period=89;
    break;
    default :
      prescaler=OSZI_TIM2_PRESCALE;
      period=OSZI_TIM2_PERIODE;
  }

  // einstellen der neuen datae
  	  ADC_TIM.Instance = TIM2;
  	  ADC_TIM.Init.Prescaler = prescaler;
  	  ADC_TIM.Init.Period = period;
  	  ADC_TIM.Init.CounterMode = TIM_COUNTERMODE_UP;
  	  ADC_TIM.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  	  ADC_TIM.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  	  __HAL_TIM_CLEAR_FLAG(&ADC_TIM,TIM_FLAG_UPDATE);
  	  if (HAL_TIM_Base_Init(&ADC_TIM) != HAL_OK)
  	  {
  	    Error_Handler();
  	  }

  // Timer wieder starten, falls notwendig
  if(ADC_UB.status!=ADC_READY) {
    // Timer2 enable
    __HAL_TIM_ENABLE(&ADC_TIM);
  }
}



//--------------------------------------------------------------
// �ndern vom Mode des DMA
// n != 1 => Double-Buffer-Mode
// n = 1  => Single-Buffer-Mode
//--------------------------------------------------------------
void ADC_change_Mode(uint8_t mode){

	// Merker setzen
	ADC_UB.dma_status=1;

	// Stop Timer2 triggering the ADC
	__HAL_TIM_DISABLE(&ADC_TIM);

	// Disable interrupts
	__HAL_DMA_DISABLE_IT(hadc1.DMA_Handle,(DMA_IT_HT | DMA_IT_TC));

	// Stop DMA and ADC if already running
	if( (HAL_DMA_GetState(hadc1.DMA_Handle) == HAL_DMA_STATE_BUSY)){
		if(HAL_ADCEx_MultiModeStop_DMA(&hadc1) != HAL_OK){ Error_Handler(); }
	}
	if( (HAL_DMA_GetState(hadc1.DMA_Handle) == HAL_DMA_STATE_ERROR)){
		Error_Handler();
	}
	if( (HAL_DMA_GetState(hadc1.DMA_Handle) != HAL_DMA_STATE_READY)){
		Error_Handler();
	}

	// Clear flags
	__HAL_DMA_CLEAR_FLAG(hadc1.DMA_Handle, (DMA_FLAG_TCIF0_4 | DMA_FLAG_HTIF0_4) );

	// Wait for DMA Stream disable
	while((hdma_adc1.Instance->CR & DMA_SxCR_EN) != RESET);

	// Double Buffer Mode
	if(mode != DMA_MODE_SingleBF) {
		//if(HAL_ADCEx_MultiModeStart_DualBF_DMA(&hadc1,(uint32_t*)&ADC_DMA_Buffer_A, (uint32_t*)&ADC_DMA_Buffer_B, ADC_ARRAY_LEN) != HAL_OK){ Error_Handler(); }

		if(HAL_ADCEx_MultiModeStart_DMA(&hadc1,(uint32_t*)&ADC_DMA_Buffer_A, ADC_ARRAY_LEN) != HAL_OK){ Error_Handler(); }

		// We don't have HAL ADC Dual buffer method, so we have to fiddle a bit :)
		// Abort the ADC DMA Stream that HAL_ADCEx_MultiModeStart_DMA has just enabled.
		if(HAL_DMA_Abort(hadc1.DMA_Handle) != HAL_OK){ Error_Handler(); }

		// Copy the first buffer callbacks to the second buffer (The first buffer callbacks were already set by HAL_ADCEx...)
	    hadc1.DMA_Handle->XferM1CpltCallback = hadc1.DMA_Handle->XferCpltCallback;
	    hadc1.DMA_Handle->XferM1HalfCpltCallback = hadc1.DMA_Handle->XferHalfCpltCallback;

	    // Restart the ADC DMA Stream, now in Dual Buffer mode
	    if(HAL_DMAEx_MultiBufferStart_IT(hadc1.DMA_Handle, (uint32_t)&ADC_COMMON_REGISTER(hadc1)->CDR,
				(uint32_t)&ADC_DMA_Buffer_A, (uint32_t)&ADC_DMA_Buffer_B, ADC_ARRAY_LEN) != HAL_OK){ Error_Handler(); }
	}

	// Single Buffer Mode
	else{
		if(HAL_ADCEx_MultiModeStart_DMA(&hadc1,(uint32_t*)&ADC_DMA_Buffer_A, ADC_ARRAY_LEN) != HAL_OK){ Error_Handler(); }
	}

	// Timer wieder starten, falls notwendig
	if((ADC_UB.status!=ADC_READY) || (mode==DMA_MODE_SingleBF)) {
	// Timer2 enable
	__HAL_TIM_ENABLE(&ADC_TIM);
	}

	// Merker zur�cksetzen
	ADC_UB.dma_status=0;
}


//--------------------------------------------------------------
// interne Funktion
// l�schen aller ADC-Arrays
//--------------------------------------------------------------
void P_ADC_Clear(void)
{
  uint32_t n;

  for(n=0;n<ADC_ARRAY_LEN;n++) {
    ADC_DMA_Buffer_A[n*2]=0x00;
    ADC_DMA_Buffer_A[(n*2)+1]=0x00;

    ADC_DMA_Buffer_B[n*2]=0x00;
    ADC_DMA_Buffer_B[(n*2)+1]=0x00;

    ADC_DMA_Buffer_C[n*2]=0x00;
    ADC_DMA_Buffer_C[(n*2)+1]=0x00;
  }
}

//		THIS WAS WITH THE PREVIOUS FREQYENCY OF 168MHZ
//		NOW IT RUNS AT 180, THESE TIMING ARE NOT VALID!
//      ADC CLOCK 22.5MHZ
//--------------------------------------------------------------
// interne Funktion
// Init von ADC Nr.1+2 (im Dual regular simultaneous mode)
//
// @ 12bit + ADC_TwoSamplingDelay_5Cycles + 21MHz ADC-Clock :
//
// ADC_SampleTime_3Cycles  => Sample_Time =  3+12+5=20 => 952ns
// ADC_SampleTime_15Cycles => Sample_Time = 15+12+5=32 => 1.52us
// ADC_SampleTime_28Cycles => Sample_Time = 28+12+5=45 => 2.14us
//--------------------------------------------------------------
void P_ADC_InitADC(void){

	// Start ADC2 (Slave)
	if(HAL_ADC_Start(&hadc2) != HAL_OK){ Error_Handler(); }

	// Start ADC1 (Master) in dual Buffer mode
	ADC_change_Mode(0);

}

//--------------------------------------------------------------
// interne Funktion
// Enable und start vom ADC und DMA
//--------------------------------------------------------------
void P_ADC_Start(void){
  // Timer2 enable
  HAL_TIM_Base_Start(&ADC_TIM);
}


//--------------------------------------------------------------
// Interrupt (ISR-Funktion)
// wird bei DMA Interrupt aufgerufen
//   (Bei HalfTransferComplete und TransferCompleteInterrupt)
//
//--------------------------------------------------------------

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
 	if(hadc!=&hadc1){ Error_Handler(); }			// Only the adc1 should call here.

    if(ADC_UB.dma_status==0) {
      if((ADC_UB.status==ADC_RUNNING) || (ADC_UB.status==ADC_PRE_TRIGGER)) {

    	  //If DMA is not currently writing top the first buffer
        if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) == 0) {
          ADC_searchTrigger_A1();				// Search trigger in the first buffer
        }
        else {
          ADC_searchTrigger_B1();				// Search trigger in the second buffer
        }
      }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc!=&hadc1){ Error_Handler(); }			// Only the adc1 should call here. Error if else

    if(ADC_UB.dma_status==0) {
    	__HAL_TIM_DISABLE(&ADC_TIM);

      if(ADC_UB.status!=ADC_VORLAUF) {
        if(ADC_UB.status==ADC_TRIGGER_OK) {
          ADC_UB.status=ADC_READY;
        }
        else {
          __HAL_TIM_ENABLE(&ADC_TIM);
          if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) != 0) {
            ADC_searchTrigger_A2();
          }
          else {
            ADC_searchTrigger_B2();
          }
        }
      }
      else {
      	__HAL_TIM_ENABLE(&ADC_TIM);
        ADC_UB.status=ADC_RUNNING;
      }
    }
    UB_Led_Toggle(LED_GREEN);
 }

void OSZI_TxferError(DMA_HandleTypeDef *_hdma){
	Error_Handler();
}

//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-1
//--------------------------------------------------------------
void ADC_searchTrigger_A1(void)
{
  uint32_t n,ch;
  uint16_t data,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
  else {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-2
//--------------------------------------------------------------
void ADC_searchTrigger_A2(void)
{
  uint32_t n,ch;
  uint16_t data,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
  else {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-3
//--------------------------------------------------------------
void ADC_searchTrigger_B1(void)
{
  uint32_t n,ch;
  uint16_t data,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
  else {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-4
//--------------------------------------------------------------
void ADC_searchTrigger_B2(void)
{
  uint32_t n,ch;
  uint16_t data,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
  else {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      data=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(data>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(data<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
}
