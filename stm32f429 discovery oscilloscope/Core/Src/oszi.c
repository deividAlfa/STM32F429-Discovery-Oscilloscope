//--------------------------------------------------------------
// File     : stm32_ub_oszi.c
// Datum    : 24.03.2014
// Version  : 1.6
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : keine
// Funktion : Oszilloskop
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "oszi.h"

extern TIM_HandleTypeDef htim2;
//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
uint32_t p_oszi_hw_init(void);
void p_oszi_sw_init(void);
void p_oszi_clear_all(void);
void p_oszi_draw_background(void);
void p_oszi_draw_scale(void);
void p_oszi_draw_line_h(uint16_t xp, uint32_t color, uint16_t m);
void p_oszi_draw_line_v(uint16_t yp, uint32_t color, uint16_t m);
void p_oszi_sort_adc(void);
void p_oszi_fill_fft(void);
void p_oszi_draw_adc(void);
int16_t oszi_adc2pixel(uint16_t adc, uint32_t faktor);
void p_oszi_send_data(void);
void p_oszi_send_uart(uint8_t *ptr);
void p_oszi_send_screen(void);


//--------------------------------------------------------------
// Header fuer BMP-Transfer
// (fix als einen kompletten Screen (320x240) im Landscape-Mode)
//--------------------------------------------------------------
uint8_t BMP_HEADER[BMP_HEADER_LEN]={
0x42,0x4D,0x36,0x84,0x03,0x00,0x00,0x00,0x00,0x00, // ID=BM, Filsize=(240x320x3+54)
0x36,0x00,0x00,0x00,0x28,0x00,0x00,0x00,           // Offset=54d, Headerlen=40d
0x40,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x00, // W=320d, H=240d (landscape)
0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x84,0x03,0x00, // 24bpp, unkomprimiert, Data=(240x320x3)
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,           // nc
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};          // nc

volatile uint32_t  GUI_Timer_ms=0;

//--------------------------------------------------------------
// init vom Oszi
//--------------------------------------------------------------
void oszi_init(void)
{
  uint32_t check;

  //---------------------------------------------
  // Hardware init
  //--------------------------------------------- 
  check=p_oszi_hw_init();
  p_oszi_send_uart((uint8_t *)"OSZI 4 STM32F429 [UB]\r\n");
  if(check==1) {
    // Touch init error
    UB_LCD_FillLayer(BACKGROUND_COL);
    UB_Font_DrawString(10,10,"Touch ERR",&Arial_7x10,FONT_COL,BACKGROUND_COL);
    while(1);
  }
  else if(check==2) {
    // Fehler in den Defines
    UB_LCD_FillLayer(BACKGROUND_COL);
    UB_Font_DrawString(10,10,"Wrong ADC Array-LEN",&Arial_7x10,FONT_COL,BACKGROUND_COL);
    while(1);
  }


  //---------------------------------------------
  // FFT init
  //---------------------------------------------
  fft_init();

  //---------------------------------------------
  // Software init
  //---------------------------------------------
  p_oszi_sw_init();
  ADC_change_Frq(Menu.timebase.value);
}


//--------------------------------------------------------------
// start vom Oszi (Endlosloop)
//--------------------------------------------------------------
void oszi_start(void)
{
  MENU_Status_t status;

  p_oszi_draw_background();
  UB_Graphic2D_Copy2DMA(Menu.akt_transparenz);  

  while(1) {
    //---------------------------------------------
    // warten bis GUI-Timer abgelaufen ist
    //---------------------------------------------
    if(GUI_Timer_ms==0) {
      GUI_Timer_ms=GUI_INTERVALL_MS;
      //--------------------------------------
      // User-Button einlesen (fuer RUN/STOP)
      //--------------------------------------
      if(UB_Button_OnClick(BLUE_BTN)==true) {
        status=MENU_NO_CHANGE;
        if(Menu.trigger.mode==2) { // "single"
          if(Menu.trigger.single==4) {
            Menu.trigger.single=5;  // von "Ready" auf "Stop"
            status=MENU_CHANGE_NORMAL;
          }
        }
        else { // "normal" oder "auto"
          if(Menu.trigger.single==0) {
            Menu.trigger.single=1; // von "Run" auf "Stop"
            status=MENU_CHANGE_NORMAL;
          }
          else if(Menu.trigger.single==1) {
            Menu.trigger.single=2; // von "Stop" auf "Weiter"
            status=MENU_CHANGE_NORMAL;
          }
        }
      }
      else {
        //--------------------------------------
        // Test ob Touch betaetigt
        //--------------------------------------
        status=menu_check_touch();
      }
      if(status!=MENU_NO_CHANGE) {
        p_oszi_draw_background();
        if(status==MENU_CHANGE_FRQ) ADC_change_Frq(Menu.timebase.value);
        if(status==MENU_CHANGE_MODE) {
          ADC_change_Mode(Menu.trigger.mode);
          if(Menu.trigger.mode!=2) {
            Menu.trigger.single=0;
          }
          else {
            Menu.trigger.single=3;
          }
          p_oszi_draw_background(); // nochmal zeichnen, zum update
          ADC_UB.status=ADC_VORLAUF;
          __HAL_TIM_ENABLE(&htim2);
        }
        if(status==MENU_SEND_DATA) {
          p_oszi_draw_background(); // nochmal zeichnen, zum update
          p_oszi_draw_adc();
          // gesendet wird am Ende
        }
      }

      if(Menu.trigger.mode==1) {
        //--------------------------------------
        // Trigger-Mode = "AUTO"
        // Screnn immer neu zeichnen
        //--------------------------------------
        if(Menu.trigger.single==0) {
          if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) == 0) {
            ADC_UB.trigger_pos=SCALE_X_MITTE;
            ADC_UB.trigger_quarter=2;
          }
          else {
            ADC_UB.trigger_pos=SCALE_X_MITTE;
            ADC_UB.trigger_quarter=4;
          }
          p_oszi_sort_adc();
          p_oszi_fill_fft();
          if(Menu.fft.mode!=0) fft_calc();
          p_oszi_draw_adc();
          ADC_UB.status=ADC_VORLAUF;
          UB_Led_Toggle(LED_RED);
        }
        else if(Menu.trigger.single==1) {
          // Button "STOP" wurde gedr�ckt
          // Timer analten
          __HAL_TIM_DISABLE(&htim2);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
        else if(Menu.trigger.single==2) {
          // Button "START" wurde gedr�ckt
          Menu.trigger.single=0;
          ADC_UB.status=ADC_VORLAUF;
          __HAL_TIM_ENABLE(&htim2);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }
      else if(Menu.trigger.mode==0) {
        //--------------------------------------
        // Trigger-Mode = "NORMAL"
        // Screnn nur zeichnen, nach Triggerevent
        //--------------------------------------
        if(Menu.trigger.single==0) {
          if(ADC_UB.status==ADC_READY) {
            UB_Led_Toggle(LED_RED);
            p_oszi_sort_adc();
            p_oszi_fill_fft();
            if(Menu.fft.mode!=0) fft_calc();
            p_oszi_draw_adc();
            ADC_UB.status=ADC_VORLAUF;
            __HAL_TIM_ENABLE(&htim2);
          }
          else {
            // oder wenn Menu ver�ndert wurde
            if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
          }
        }
        else if(Menu.trigger.single==1) {
          // Button "STOP" wurde gedr�ckt
          // Timer analten
          __HAL_TIM_DISABLE(&htim2);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
        else if(Menu.trigger.single==2) {
          // Button "START" wurde gedr�ckt
          Menu.trigger.single=0;
          ADC_UB.status=ADC_VORLAUF;
          __HAL_TIM_ENABLE(&htim2);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }
      else {
        //--------------------------------------
        // Trigger-Mode = "SINGLE"
        // Screnn nur einmal zeichnen, nach Triggerevent
        //--------------------------------------
        if(Menu.trigger.single==3) {
          // warten auf Trigger-Event
          if(ADC_UB.status==ADC_READY) {
            Menu.trigger.single=4;
            UB_Led_Toggle(LED_RED);
            p_oszi_sort_adc();
            p_oszi_fill_fft();
            if(Menu.fft.mode!=0) fft_calc();
            p_oszi_draw_adc();
          }
          else {
            // oder wenn Menu ver�ndert wurde
            if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
          }
        }
        else if(Menu.trigger.single==5) {
          // Button "Reset" wurde gedr�ckt
          Menu.trigger.single=3;
          p_oszi_draw_adc();
          ADC_UB.status=ADC_VORLAUF;
          __HAL_TIM_ENABLE(&htim2);
        }
        else {
          // oder wenn Menu ver�ndert wurde
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }

      if(GUI.gui_xpos==GUI_XPOS_OFF) {
        // ohne GUI => ohne Transparenz zeichnen
        UB_Graphic2D_Copy1DMA();
      }
      else {
        // mit GUI => mit Transparenz zeichnen
        UB_Graphic2D_Copy2DMA(Menu.akt_transparenz);
      }

      // Refreh vom LCD
      UB_LCD_Refresh();

      // event. Daten senden
      if(Menu.send.data!=0) {
        p_oszi_send_data();
        Menu.send.data=0;
      }
    }
  }
}


//--------------------------------------------------------------
// init der Hardware
//--------------------------------------------------------------
uint32_t p_oszi_hw_init(void)
{
  uint32_t return_val=0;

  // init Touch
  if(UB_Touch_Init()!=SUCCESS) {
    return_val=1; // Touch error
  }

  // Check Defines
  if(ADC_ARRAY_LEN!=SCALE_W) {
    return_val=2; // define error
  }

  // init der LEDs
  UB_Led_Init();

  // init vom Button
  UB_Button_Init();


  // init vom LCD (und SD-RAM)
  UB_LCD_Init();
  UB_LCD_LayerInit_Fullscreen();
  UB_LCD_SetMode(LANDSCAPE);

  // alle Puffer l�schen
   p_oszi_clear_all();

  // init vom ADC
  ADC_Init_ALL();

  return(return_val);
}


//--------------------------------------------------------------
// init der Software
//--------------------------------------------------------------
void p_oszi_sw_init(void)
{
  //--------------------------------------
  // Default Einstellungen
  //--------------------------------------
  Menu.akt_transparenz=100;
  Menu.akt_setting=SETTING_TRIGGER;

  Menu.ch1.faktor=2;      // 1v/div
  Menu.ch1.visible=0;     // visible=true
  Menu.ch1.position=0;

  Menu.ch2.faktor=2;      // 1v/div
  Menu.ch2.visible=0;     // visible=true
  Menu.ch2.position=-100;

  Menu.timebase.value=12;  // 5ms/div

  Menu.trigger.source=0;  // trigger=CH1
  Menu.trigger.edge=0;    // hi-flanke
  Menu.trigger.mode=1;    // auto
  Menu.trigger.single=0;
  Menu.trigger.value_ch1=2048;
  Menu.trigger.value_ch2=2048;

  Menu.cursor.mode=0;     // cursor Off
  Menu.cursor.p1=2048;
  Menu.cursor.p2=3072;
  Menu.cursor.t1=1700;
  Menu.cursor.t2=2300;
  Menu.cursor.f1=1000;

  Menu.send.mode=0; // nur CH1
  Menu.send.screen=SETTING_TRIGGER;
  Menu.send.data=0;

  Menu.fft.mode=0; // FFT=CH1

  GUI.gui_xpos=GUI_XPOS_OFF;  // GUI ausgeblendet
  GUI.akt_menu=MM_NONE;
  GUI.old_menu=MM_CH1;
  GUI.akt_button=GUI_BTN_NONE;
  GUI.old_button=GUI_BTN_NONE;
}


//--------------------------------------------------------------
// l�scht alle Speicher
//--------------------------------------------------------------
void p_oszi_clear_all(void)
{
  UB_LCD_SetLayer_1();
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_SetLayer_2();
  UB_LCD_SetTransparency(255);
  UB_LCD_FillLayer(BACKGROUND_COL);
  //UB_LCD_Copy_Layer2_to_Layer1();
  UB_LCD_SetLayer_Menu();
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_SetLayer_ADC();
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// zeichnet den Hintergrund vom Oszi
// (Skala, Cursor, Men�s usw)
// Zieladresse im SD-RAM = [MENU]
//--------------------------------------------------------------
void p_oszi_draw_background(void)
{

  UB_LCD_SetLayer_Menu();
  UB_LCD_FillLayer(BACKGROUND_COL);

  // Draw the GUI first
  menu_draw_all();
  // then draw the scale and cursor
  p_oszi_draw_scale();

  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// draws the scale and cursors from the oscillator
//--------------------------------------------------------------
void p_oszi_draw_scale(void)
{
  uint32_t n,m;
  uint16_t xs,ys;
  int16_t signed_int;

  xs=SCALE_START_X;
  ys=SCALE_START_Y;

  //---------------------------------------------
  // grid of individual points
  //---------------------------------------------
  for(m=0;m<=SCALE_H;m+=SCALE_SPACE) {
    for(n=0;n<=SCALE_W;n+=SCALE_SPACE) {
      UB_Graphic2D_DrawPixelNormal(m+xs,n+ys,SCALE_COL);
    }
  }

  //---------------------------------------------
  // X-axis (horizontal center line)
  //---------------------------------------------
  signed_int=SCALE_Y_MITTE+xs;
  p_oszi_draw_line_h(signed_int,SCALE_COL,0);

  //---------------------------------------------
  // Y-axis (vertical center line)
  //---------------------------------------------
  signed_int=SCALE_X_MITTE+ys;
  p_oszi_draw_line_v(signed_int,SCALE_COL,0);
 
  //---------------------------------------------
  // border
  //---------------------------------------------
  // unterste linie

  UB_Graphic2D_DrawStraightDMA(xs-1,ys-1,SCALE_W+2,LCD_DIR_HORIZONTAL,SCALE_COL);
  // oberste linie
  UB_Graphic2D_DrawStraightDMA(xs+SCALE_H+1,ys-1,SCALE_W+2,LCD_DIR_HORIZONTAL,SCALE_COL);
  // linke linie
  UB_Graphic2D_DrawStraightDMA(xs-1,ys-1,SCALE_H+2,LCD_DIR_VERTICAL,SCALE_COL);
  // rechte linie
  UB_Graphic2D_DrawStraightDMA(xs-1,ys+SCALE_W+1,SCALE_H+2,LCD_DIR_VERTICAL,SCALE_COL);


  //---------------------------------------------
  // trigger line (always visible)
  //---------------------------------------------
  if(Menu.trigger.source==0) {
    signed_int=oszi_adc2pixel(Menu.trigger.value_ch1, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,ADC_CH1_COL,1);
    UB_Font_DrawString(signed_int-3,0,"T",&Arial_7x10,ADC_CH1_COL,BACKGROUND_COL);
  }
  else if(Menu.trigger.source==1) {
    signed_int=oszi_adc2pixel(Menu.trigger.value_ch2, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,ADC_CH2_COL,1);
    UB_Font_DrawString(signed_int-3,0,"T",&Arial_7x10,ADC_CH2_COL,BACKGROUND_COL);
  }

  //---------------------------------------------
  // cursor lines (only if activated)
  //---------------------------------------------
  if(Menu.cursor.mode==1) {
    //------------------------------- 
    // Cursor (CH1)
    //-------------------------------
    signed_int=oszi_adc2pixel(Menu.cursor.p1, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=oszi_adc2pixel(Menu.cursor.p2, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==2) {
    //-------------------------------
    // Cursor (CH2)
    //-------------------------------
    signed_int=oszi_adc2pixel(Menu.cursor.p1, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=oszi_adc2pixel(Menu.cursor.p2, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==3) {
    //-------------------------------
    // Cursor (TIME)
    //-------------------------------
    signed_int=Menu.cursor.t1*FAKTOR_T;
    signed_int+=SCALE_START_Y;
    if(signed_int<SCALE_START_Y) signed_int=SCALE_START_Y;
    if(signed_int>SCALE_MY_PIXEL) signed_int=SCALE_MY_PIXEL;

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=Menu.cursor.t2*FAKTOR_T;
    signed_int+=SCALE_START_Y;
    if(signed_int<SCALE_START_Y) signed_int=SCALE_START_Y;
    if(signed_int>SCALE_MY_PIXEL) signed_int=SCALE_MY_PIXEL;

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==4) {
    //-------------------------------
    // Cursor (FFT)
    //-------------------------------
    signed_int=Menu.cursor.f1*FAKTOR_F;
    signed_int+=FFT_START_Y+1;
    if(signed_int<FFT_START_Y) signed_int=FFT_START_Y;
    if(signed_int>(FFT_START_Y+FFT_VISIBLE_LENGTH)) signed_int=(FFT_START_Y+FFT_VISIBLE_LENGTH);

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
}


//--------------------------------------------------------------
// draws a horizontal line on the Oszi grid
// at "xp", with color "c" and mode "m"
//--------------------------------------------------------------
void p_oszi_draw_line_h(uint16_t xp, uint32_t color, uint16_t m)
{
  uint32_t n,t;

  if(m==0) {
    // Linie : "X----X----X----X----X----X"
    for(n=0;n<=SCALE_W;n+=5) {
      UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,color);
    } 
  }
  else if(m==1) {
    // Linie : "X-X-X-X-X-X-X-X-X"
    for(n=0;n<=SCALE_W;n+=2) {
      UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,color);
    }
  }
  else if(m==2) {
    // Linie : "XX---XX---XX---XX---XX"
    t=0;
    for(n=0;n<=SCALE_W;n++) {
      if(t<2) UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,color);
      t++;
      if(t>4) t=0;
    }
  }
}


//--------------------------------------------------------------
// draws a vertical line on the Oszi grid at "yp",
// with color "c" and mode "m"
//--------------------------------------------------------------
void p_oszi_draw_line_v(uint16_t yp, uint32_t color, uint16_t m)
{
  uint32_t n,t;

  if(m==0) {
    // Linie : "X----X----X----X----X----X"
    for(n=0;n<=SCALE_H;n+=5) {
      UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,color);
    } 
  }
  else if(m==1) {
    // Linie : "X-X-X-X-X-X-X-X-X"
    for(n=0;n<=SCALE_H;n+=2) {
      UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,color);
    }
  }
  else if(m==2) {
    // Linie : "XX---XX---XX---XX---XX"
    t=0;
    for(n=0;n<=SCALE_H;n++) {
      if(t<2) UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,color);
      t++;
      if(t>4) t=0;
    }
  }
}


//--------------------------------------------------------------
// sorts the data of the ADC channels from Buffer_A and Buffer_B
// into Buffer_C around the data are sorted in such a way that
// the trigger event is in the middle of the data area
// (this will later be the middle of the screen)
//
// The trigger point can be in one of the 4 quadrants
// from the data buffer
// Quadrant-1 = first half of Buffer-A
// Quadrant-2 = second half of Buffer-A
// Quadrant-3 = first half of Buffer-B
// Quadrant-4 = second half of Buffer-B
//--------------------------------------------------------------
void p_oszi_sort_adc(void)
{
  uint32_t n=0;
  uint32_t start=0,anz1=0,anz2=0;
  uint16_t data;

  if(ADC_UB.trigger_quarter==1) {
    //-------------------------------
    // The trigger point is in Q1
    //-------------------------------
    anz1=(SCALE_X_MITTE-ADC_UB.trigger_pos);
    start=SCALE_W-anz1;

    //-------------------------------
    // copy left part
    //-------------------------------
    for(n=0;n<anz1;n++) {
      data=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=data;
      data=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=data;
    }
    //-------------------------------
    // copy right part
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      data=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=data;
      data=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=data;
    }
  }
  else if(ADC_UB.trigger_quarter==2) {
    //-------------------------------
    // The trigger point is in Q2
    //-------------------------------
    anz1=SCALE_W-((ADC_UB.trigger_pos-SCALE_X_MITTE));
    start=SCALE_W-anz1;

    //-------------------------------
    // copy left part
    //-------------------------------
    for(n=0;n<anz1;n++) {
      data=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=data;
      data=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=data;
    }
    //-------------------------------
    // copy right part
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      data=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=data;
      data=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=data;
    }
  }
  else if(ADC_UB.trigger_quarter==3) {
    //-------------------------------
    // The trigger point is in Q3
    //-------------------------------
    anz1=(SCALE_X_MITTE-ADC_UB.trigger_pos);
    start=SCALE_W-anz1;

    //-------------------------------
    // copy left part
    //-------------------------------
    for(n=0;n<anz1;n++) {
      data=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=data;
      data=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=data;
    }
    //-------------------------------
    // copy right part
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      data=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=data;
      data=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=data;
    }
  }
  else if(ADC_UB.trigger_quarter==4) {
    //-------------------------------
    // The trigger point is in Q4
    //-------------------------------
    anz1=SCALE_W-((ADC_UB.trigger_pos-SCALE_X_MITTE));
    start=SCALE_W-anz1;

    //-------------------------------
    // copy left part
    //-------------------------------
    for(n=0;n<anz1;n++) {
      data=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=data;
      data=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=data;
    }
    //-------------------------------
    // copy right part
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      data=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=data;
      data=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=data;
    }
  }
}


//--------------------------------------------------------------
// fills the FFT input buffer with the
// sample data from CH1 or CH2 (fill the rest with 0)
//--------------------------------------------------------------
void p_oszi_fill_fft(void)
{
  uint32_t n,m;

  if(Menu.fft.mode==1) {
    m=0;
    for(n=0;n<FFT_LENGTH;n++) {
      if(m<SCALE_W) {
        FFT_DATA_IN[n]=(float)((ADC_DMA_Buffer_C[(m*2)]-2048.0)/1000.0);
      }
      else {
        FFT_DATA_IN[n]=0.0;
      }
      m++;
    }
  }
  else if(Menu.fft.mode==2) {
    m=0;
    for(n=0;n<FFT_LENGTH;n++) {
      if(m<SCALE_W) {
        FFT_DATA_IN[n]=(float)((ADC_DMA_Buffer_C[(m*2)+1]-2048.0)/1000.0);
      }
      else {
        FFT_DATA_IN[n]=0.0;
      }
      m++;
    }
  }
}


//--------------------------------------------------------------
// Records the data of the two ADC channels (and the FFT)
//--------------------------------------------------------------
void p_oszi_draw_adc(void)
{
  uint32_t n=0;
  int16_t ch1_data1,ch1_data2;
  int16_t ch2_data1,ch2_data2;
  int16_t fft_data1,fft_data2;

  p_oszi_draw_background();
  UB_LCD_SetLayer_Menu();

  // startdatae
  ch1_data1=oszi_adc2pixel(ADC_DMA_Buffer_C[0], Menu.ch1.faktor);
  ch1_data1+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
  if(ch1_data1<SCALE_START_X) ch1_data1=SCALE_START_X;
  if(ch1_data1>SCALE_MX_PIXEL) ch1_data1=SCALE_MX_PIXEL;

  ch2_data1=oszi_adc2pixel(ADC_DMA_Buffer_C[1], Menu.ch2.faktor);
  ch2_data1+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
  if(ch2_data1<SCALE_START_X) ch2_data1=SCALE_START_X;
  if(ch2_data1>SCALE_MX_PIXEL) ch2_data1=SCALE_MX_PIXEL;

  fft_data1=FFT_UINT_DATA[0];
  fft_data1+=FFT_START_X;
  if(fft_data1<SCALE_START_X) fft_data1=SCALE_START_X;
  if(fft_data1>SCALE_MX_PIXEL) fft_data1=SCALE_MX_PIXEL;

  // komplette Kurve
  for(n=1;n<SCALE_W;n++) {
    if(Menu.ch1.visible==0) {
      ch1_data2=oszi_adc2pixel(ADC_DMA_Buffer_C[n*2], Menu.ch1.faktor);
      ch1_data2+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
      if(ch1_data2<SCALE_START_X) ch1_data2=SCALE_START_X;
      if(ch1_data2>SCALE_MX_PIXEL) ch1_data2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(ch1_data1,SCALE_START_Y+n,ch1_data2,SCALE_START_Y+n+1,ADC_CH1_COL);
      ch1_data1=ch1_data2;
    }

    if(Menu.ch2.visible==0) {
      ch2_data2=oszi_adc2pixel(ADC_DMA_Buffer_C[(n*2)+1], Menu.ch2.faktor);
      ch2_data2+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
      if(ch2_data2<SCALE_START_X) ch2_data2=SCALE_START_X;
      if(ch2_data2>SCALE_MX_PIXEL) ch2_data2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(ch2_data1,SCALE_START_Y+n,ch2_data2,SCALE_START_Y+n+1,ADC_CH2_COL);
      ch2_data1=ch2_data2;
    }
  }

  // nur die linke h�lfte der FFT zeichnen
  // (die rechte ist das Spiegelbild)
  if(Menu.fft.mode!=0) {
    for(n=1;n<FFT_VISIBLE_LENGTH;n++) {
      fft_data2=FFT_UINT_DATA[n];
      fft_data2+=FFT_START_X;
      if(fft_data2<SCALE_START_X) fft_data2=SCALE_START_X;
      if(fft_data2>SCALE_MX_PIXEL) fft_data2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(fft_data1,FFT_START_Y+n,fft_data2,FFT_START_Y+n+1,FFT_COL);
      fft_data1=fft_data2;
    }
  }

  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// Convert adc-data into pixel position
//--------------------------------------------------------------
int16_t oszi_adc2pixel(uint16_t adc, uint32_t faktor)
{
  int16_t return_val=0;

  switch(faktor) {
    case 0 : return_val=adc*FAKTOR_5V;break;
    case 1 : return_val=adc*FAKTOR_2V;break;
    case 2 : return_val=adc*FAKTOR_1V;break;
    case 3 : return_val=adc*FAKTOR_0V5;break;
    case 4 : return_val=adc*FAKTOR_0V2;break;
    case 5 : return_val=adc*FAKTOR_0V1;break;
    case 6 : return_val=adc*FAKTOR_0V05;break;
  }

  return(return_val);
}


//--------------------------------------------------------------
// Daten per UART senden
//--------------------------------------------------------------
void p_oszi_send_data(void)
{
  uint32_t n;
  uint16_t data1,data2;
  char buf[16];
  extern const SM_Item_t UM_01[];
  extern const SM_Item_t UM_02[];

  //--------------------------------
  // send Screen as Bitmap
  //--------------------------------
  if(Menu.send.mode==6) {
    p_oszi_send_screen();
    return;
  }

  //--------------------------------
  // send settings
  //--------------------------------
  p_oszi_send_uart((uint8_t *)"SETTINGS:\r\n");
  if((Menu.send.mode==0) || (Menu.send.mode==1) || (Menu.send.mode==4) || (Menu.send.mode==5)) {
    sprintf(buf,"CH1=%s/div\r\n",UM_01[Menu.ch1.faktor].stxt);
    p_oszi_send_uart((uint8_t *)buf);
  }
  if((Menu.send.mode==2) || (Menu.send.mode==3) || (Menu.send.mode==4) || (Menu.send.mode==5)) {
    sprintf(buf,"CH2=%s/div\r\n",UM_01[Menu.ch2.faktor].stxt);
    p_oszi_send_uart((uint8_t *)buf);
  }
  sprintf(buf,"Time=%s/div\r\n",UM_02[Menu.timebase.value].stxt);
  p_oszi_send_uart((uint8_t *)buf);
  p_oszi_send_uart((uint8_t *)"1div=25\r\n");

  sprintf(buf,"count=%d\r\n",SCALE_W);
  p_oszi_send_uart((uint8_t *)buf);

  //--------------------------------
  // send data
  //--------------------------------
  p_oszi_send_uart((uint8_t *)"DATA:\r\n");
  if((Menu.send.mode==0) || (Menu.send.mode==1)) {
    p_oszi_send_uart((uint8_t *)"CH1\r\n");
    for(n=0;n<SCALE_W;n++) {
      data1=ADC_DMA_Buffer_C[n*2];
      sprintf(buf,"%d\r\n",data1);
      p_oszi_send_uart((uint8_t *)buf);
    }
  }
  else if((Menu.send.mode==2) || (Menu.send.mode==3)) {
    p_oszi_send_uart((uint8_t *)"CH2\r\n");
    for(n=0;n<SCALE_W;n++) {
      data2=ADC_DMA_Buffer_C[(n*2)+1];
      sprintf(buf,"%d\r\n",data2);
      p_oszi_send_uart((uint8_t *)buf);
    }
  }
  else if((Menu.send.mode==4) || (Menu.send.mode==5)) {
    p_oszi_send_uart((uint8_t *)"CH1,CH2\r\n");
    for(n=0;n<SCALE_W;n++) {
      data1=ADC_DMA_Buffer_C[n*2];
      data2=ADC_DMA_Buffer_C[(n*2)+1];
      sprintf(buf,"%d,%d\r\n",data1,data2);
      p_oszi_send_uart((uint8_t *)buf);
    }
  }
  //--------------------------------
  // send fft
  //--------------------------------
  if((Menu.send.mode==1) || (Menu.send.mode==3) || (Menu.send.mode==5)) {
    if(Menu.fft.mode==1) {
      p_oszi_send_uart((uint8_t *)"FFT:");
      p_oszi_send_uart((uint8_t *)"CH1");
      sprintf(buf,"count=%d",FFT_VISIBLE_LENGTH);
      p_oszi_send_uart((uint8_t *)buf);
      for(n=0;n<FFT_VISIBLE_LENGTH;n++) {
        data2=FFT_UINT_DATA[n];
        sprintf(buf,"%d",data2);
        p_oszi_send_uart((uint8_t *)buf);
      }
    }
    else if(Menu.fft.mode==2) {
      p_oszi_send_uart((uint8_t *)"FFT:\r\n");
      p_oszi_send_uart((uint8_t *)"CH2\r\n");
      sprintf(buf,"count=%d\r\n",FFT_VISIBLE_LENGTH);
      p_oszi_send_uart((uint8_t *)buf);
      for(n=0;n<FFT_VISIBLE_LENGTH;n++) {
        data2=FFT_UINT_DATA[n];
        sprintf(buf,"%d\r\n",data2);
        p_oszi_send_uart((uint8_t *)buf);
      }
    }
  }
  p_oszi_send_uart((uint8_t *)"END.\r\n");
}

//--------------------------------------------------------------
// string per UART senden
//--------------------------------------------------------------
void p_oszi_send_uart(uint8_t *ptr)
{
  UB_Uart_SendString(ptr);
}


//--------------------------------------------------------------
// Screen als Bitmap (*.bmp) per UART senden
// dauert bei 115200 Baud ca. 20 sekunden
//--------------------------------------------------------------
void p_oszi_send_screen(void)
{
  uint32_t n,adr;
  uint16_t x,y,color;
  uint8_t r,g,b;

  // BMP-Header senden
  for(n=0;n<BMP_HEADER_LEN;n++) {
    UB_Uart_SendByte(BMP_HEADER[n]);
  }

  // den richigen Buffer zum senden raussuchen
  if(LCD_CurrentLayer==1) {
    adr=LCD_FRAME_BUFFER;
  }
  else {
    adr=LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  }

  // alle Farb-Daten senden
  for(x=0;x<LCD_MAXX;x++) {
    for(y=0;y<LCD_MAXY;y++) {
      n=y*(LCD_MAXX*2)+(x*2);
      color=*(volatile uint16_t*)(adr+n);
      r=((color&0xF800)>>8);  // 5bit rot
      g=((color&0x07E0)>>3);  // 6bit gruen
      b=((color&0x001F)<<3);  // 5bit blau
      UB_Uart_SendByte(b);
      UB_Uart_SendByte(g);
      UB_Uart_SendByte(r);
    }
  }
}
