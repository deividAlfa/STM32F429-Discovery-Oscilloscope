//--------------------------------------------------------------
// File     : menu.c
// Datum    : 24.03.2014
// Version  : 1.6
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : keine
// Funktion : Menu
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "menu.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void p_menu_draw_BOT(uint32_t mm_nr, const SM_Item_t um[], uint32_t um_nr, uint32_t mode);
void p_menu_draw_BOT_TRG(void);
void p_menu_draw_BOT_CH1(void);
void p_menu_draw_BOT_CH2(void);
void p_menu_draw_BOT_CUR(void);
void p_menu_draw_BOT_FFT(void);
void p_menu_draw_BOT_SEND(void);
void p_menu_draw_BOT_VERSION(void);
void p_menu_draw_BOT_HELP(void);
void P_FloatToDezStr(float data);
float P_Volt_to_Float(uint32_t faktor, int16_t pos);
float P_Time_to_Float(uint32_t faktor, uint16_t pos);
float P_FFT_to_Float(uint32_t faktor, uint16_t pos);
uint16_t LINE(uint16_t n);
uint16_t GET_LINE(uint16_t xp);
void p_menu_draw_GUI(void);
void p_gui_draw_TOP(uint32_t mm_nr, const SM_Item_t um[], uint32_t um_nr);
void p_get_GUI_button(uint16_t x, uint16_t y);
MENU_Status_t p_make_GUI_changes(void);
MENU_Status_t p_gui_inc_menu(void);
MENU_Status_t p_gui_dec_menu(void);
uint16_t inc_uintval(uint16_t data, uint16_t startdata);
uint16_t dec_uintval(uint16_t data, uint16_t startdata);
int16_t inc_intval(int16_t data, uint16_t startdata);
int16_t dec_intval(int16_t data, uint16_t startdata);


//--------------------------------------------------------------
// Globale Buffer (f�r printf)
//--------------------------------------------------------------
char buf[30];
char bval[10];


//--------------------------------------------------------------
// Menu-Eintr�ge
// Gleiche Reihenfolge wie "MM_Akt_Item_t"
// TXT,Ypos,Items
//--------------------------------------------------------------
const MM_Item_t MM_ITEM[] = {
  {"",0,0},
  {"CH1=",1*FONT_W  ,7},    // CH1            [UM_01]
  {"CH2=",11*FONT_W ,7},    // CH2            [UM_01]
  {"T=",21*FONT_W   ,17},   // TIME           [UM_02]
  {"Menu=",31*FONT_W,8},    // Menu           [UM_03]
  {"S=",1*FONT_W    ,2},    // TRG Source     [UM_04]
  {"E=",8*FONT_W    ,2},    // TRG Edge       [UM_05]
  {"M=",14*FONT_W   ,3},    // TRG Mode       [UM_06]
  {"V=",24*FONT_W   ,1},    // TRG Value      [UM_07]
  {"",35*FONT_W     ,1},    // TRG Reset      [UM_10]
  {"V=",1*FONT_W    ,2},    // CH Visible     [UM_08]
  {"P=",8*FONT_W    ,1},    // CH Position    [UM_07]
  {"M=",1*FONT_W    ,5},    // CUR Mode       [UM_09]
  {"A=",9*FONT_W    ,1},    // CUR A Position [UM_07]
  {"B=",21*FONT_W   ,1},    // CUR B Position [UM_07]
  {"M=",1*FONT_W    ,7},    // SEND Mode      [UM_11]
  {"S=",16*FONT_W   ,2},    // SEND Screen    [UM_14]
  {"",28*FONT_W     ,2},    // SEND Data      [UM_12]
  {"FFT=",1*FONT_W  ,3},    // FFT Mode       [UM_13]
};




//--------------------------------------------------------------
// Untermenu : CH1/CH2 Vorteiler
//--------------------------------------------------------------
const SM_Item_t UM_01[] = {
  {"5.0V"},
  {"2.0V"},
  {"1.0V"},
  {"0.5V"},
  {"0.2V"},
  {"0.1V"},
  {"0.05V"},
};


//--------------------------------------------------------------
// Untermenu : Timebase
//--------------------------------------------------------------
const SM_Item_t UM_02[] = {
  {"5.0s "},
  {"2.0s "},
  {"1.0s "},
  {"500ms"},
  {"200ms"},
  {"100ms"},
  {"50ms "},
  {"20ms "},
  {"10ms "},
  {"5ms  "},
  {"2ms  "},
  {"1ms  "},
  {"500us"},
  {"200us"},
  {"100us"},
  {"50us "},
  {"25us "},
};


//--------------------------------------------------------------
// Untermenu : Settings
//--------------------------------------------------------------
const SM_Item_t UM_03[] = {
  {"TRIGGER"},
  {"CH1    "},
  {"CH2    "},
  {"CURSOR "},
  {"FFT    "},
  {"SEND   "},
  {"VERSION"},
  {"HELP   "},
};


//--------------------------------------------------------------
// Untermenu-4 : TRG Source
//--------------------------------------------------------------
const SM_Item_t UM_04[] = {
  {"CH1"},
  {"CH2"},
};


//--------------------------------------------------------------
// Untermenu-5 : TRG Edge
//--------------------------------------------------------------
const SM_Item_t UM_05[] = {
  {"Hi"},
  {"Lo"},
};


//--------------------------------------------------------------
// Untermenu-6 : TRG Mode
//--------------------------------------------------------------
const SM_Item_t UM_06[] = {
  {"Normal"},
  {"Auto  "},
  {"Single"},
};


//--------------------------------------------------------------
// Untermenu-7 (dummy)
//--------------------------------------------------------------
const SM_Item_t UM_07[] = {
  {""},
};


//--------------------------------------------------------------
// Untermenu-8 : CH Visible
//--------------------------------------------------------------
const SM_Item_t UM_08[] = {
  {"On "},
  {"Off"},
};


//--------------------------------------------------------------
// Untermenu-9 : CUR Mode
//--------------------------------------------------------------
const SM_Item_t UM_09[] = {
  {"Off "},
  {"CH1 "},
  {"CH2 "},
  {"Time"},
  {"FFT "},
};


//--------------------------------------------------------------
// Untermenu-10 : TRG Reset
//--------------------------------------------------------------
const SM_Item_t UM_10[] = {
  {"RUN  "},
  {"STOP "},
  {"RUN  "},
  {"WAIT "},
  {"READY"},
  {"WAIT "},
};


//--------------------------------------------------------------
// Untermenu-11 : SEND Mode
//--------------------------------------------------------------
const SM_Item_t UM_11[] = {
  {"CH1        "},
  {"CH1+FFT    "},
  {"CH2        "},
  {"CH2+FFT    "},
  {"CH1+CH2    "},
  {"CH1+CH2+FFT"},
  {"Screen-BMP "},
};


//--------------------------------------------------------------
// Untermenu-12 : SEND Data
//--------------------------------------------------------------
const SM_Item_t UM_12[] = {
  {"START"},
  {"WAIT "},
};


//--------------------------------------------------------------
// Untermenu-13 : FFT Mode
//--------------------------------------------------------------
const SM_Item_t UM_13[] = {
  {"Off"},
  {"CH1"},
  {"CH2"},
};


//--------------------------------------------------------------
// Untermenu-14 : SEND Screen
//--------------------------------------------------------------
const SM_Item_t UM_14[] = {
  {"TRIGGER"},
  {"CURSOR "},
};


//--------------------------------------------------------------
// draws the complete menu
// (all TOP and all BOTTOM menu items and the GUI)
//--------------------------------------------------------------
void menu_draw_all(void)
{
  //---------------------------------
  // upper menu bar
  //---------------------------------
  // Background bar
  UB_Graphic2D_DrawFullRectDMA(LCD_MAXX-FONT_H-2,0,LCD_MAXY,FONT_H+2,MENU_BG_COL);
  // TOP-Menu
  p_gui_draw_TOP(MM_CH1, UM_01, Menu.ch1.faktor);
  p_gui_draw_TOP(MM_CH2, UM_01, Menu.ch2.faktor);
  p_gui_draw_TOP(MM_TIME, UM_02, Menu.timebase.value);  
  if(Menu.send.data==0) {
    p_gui_draw_TOP(MM_SETTING, UM_03, Menu.akt_setting);
  }
  else {
    UB_Font_DrawString(LINE(1),MM_ITEM[MM_SETTING].yp,"please wait",&Arial_7x10,MENU_VG_COL,MENU_AK_COL);
  }

  //---------------------------------
  // lower menu bar
  //---------------------------------
  // Background bar
  UB_Graphic2D_DrawFullRectDMA(0,0,LCD_MAXY,FONT_H+2,MENU_BG_COL);
  // Bottom-Menu
  if(Menu.akt_setting==SETTING_TRIGGER) p_menu_draw_BOT_TRG();
  if(Menu.akt_setting==SETTING_CH1) p_menu_draw_BOT_CH1();
  if(Menu.akt_setting==SETTING_CH2) p_menu_draw_BOT_CH2();
  if(Menu.akt_setting==SETTING_CURSOR) p_menu_draw_BOT_CUR();
  if(Menu.akt_setting==SETTING_FFT) p_menu_draw_BOT_FFT();
  if(Menu.akt_setting==SETTING_SEND) p_menu_draw_BOT_SEND();
  if(Menu.akt_setting==SETTING_VERSION) p_menu_draw_BOT_VERSION();
  if(Menu.akt_setting==SETTING_HELP) p_menu_draw_BOT_HELP();

  if(GUI.gui_xpos==GUI_XPOS_OFF) {
    Menu.akt_transparenz=100;
  }
  else {
    Menu.akt_transparenz=200;
    //--------------------------
    // GUI
    //--------------------------
    p_menu_draw_GUI();
  }
}


//--------------------------------------------------------------
// draws the GUI
// (also draws the pressed button)
//--------------------------------------------------------------
void p_menu_draw_GUI(void)
{
  DMA2D_Koord koord;

  //--------------------------
  // leere GUI zeichnen
  //--------------------------
  koord.source_xp=0;
  koord.source_yp=0;
  koord.source_w=GUI1.width;
  koord.source_h=GUI1.height;
  koord.dest_xp=GUI_YPOS;
  koord.dest_yp=GUI.gui_xpos;

  UB_Graphic2D_CopyImgDMA(&GUI1,koord);


  //--------------------------
  // Draw the pressed button
  //--------------------------
  if(GUI.akt_button==GUI_BTN_RIGHT) {
    koord.source_xp=GUI1.width/2;
    koord.source_yp=GUI1.height/2;
    koord.source_w=GUI1.width/2;
    koord.source_h=GUI1.height/2;
    koord.dest_xp=GUI_YPOS+(GUI1.width/2);
    koord.dest_yp=GUI.gui_xpos+(GUI1.height/2);
    UB_Graphic2D_CopyImgDMA(&GUI2,koord);  
  }
  else if(GUI.akt_button==GUI_BTN_LEFT) {
    koord.source_xp=GUI1.width/2;
    koord.source_yp=0;
    koord.source_w=GUI1.width/2;
    koord.source_h=GUI1.height/2;
    koord.dest_xp=GUI_YPOS+(GUI1.width/2);
    koord.dest_yp=GUI.gui_xpos;
    UB_Graphic2D_CopyImgDMA(&GUI2,koord);
  }
  else if(GUI.akt_button==GUI_BTN_UP) {
    koord.source_xp=GUI1.width/4;
    koord.source_yp=0;
    koord.source_w=GUI1.width/4;
    koord.source_h=GUI1.height;
    koord.dest_xp=GUI_YPOS+(GUI1.width/4);
    koord.dest_yp=GUI.gui_xpos;
    UB_Graphic2D_CopyImgDMA(&GUI2,koord);
  }
  else if(GUI.akt_button==GUI_BTN_DOWN) {
    koord.source_xp=0;
    koord.source_yp=0;
    koord.source_w=GUI1.width/4;
    koord.source_h=GUI1.height;
    koord.dest_xp=GUI_YPOS;
    koord.dest_yp=GUI.gui_xpos;
    UB_Graphic2D_CopyImgDMA(&GUI2,koord);
  }
}


//--------------------------------------------------------------
// test which of the four buttons of the GUI is activated
// y, x = touch position
//--------------------------------------------------------------
void p_get_GUI_button(uint16_t x, uint16_t y)
{
  if(x>(GUI_YPOS+(GUI1.width/2))) {
    // left/right
    if(y>(GUI.gui_xpos+(GUI1.height/2))) {
      GUI.akt_button=GUI_BTN_RIGHT;
    }
    else {
      GUI.akt_button=GUI_BTN_LEFT;
    }
  }
  else {
    // up/down
    if(x>(GUI_YPOS+(GUI1.width/4))) {
      GUI.akt_button=GUI_BTN_UP;
    }
    else {
      GUI.akt_button=GUI_BTN_DOWN;
    }
  }
}


//--------------------------------------------------------------
// increments the current menu item
//--------------------------------------------------------------
MENU_Status_t p_gui_inc_menu(void)
{
  MENU_Status_t return_val=MENU_NO_CHANGE;
  uint32_t max;
  uint16_t value;
  int16_t ivalue;

  if(GUI.akt_menu==MM_NONE) return(return_val);

  // alle "normalen" Menupunkte koennen nur einmal betaetigt werden
  if((GUI.akt_menu!=MM_TRG_VAL) && (GUI.akt_menu!=MM_CH_POS) &&
    (GUI.akt_menu!=MM_CUR_P1) && (GUI.akt_menu!=MM_CUR_P2)) {
    // wenn schon mal betaetigt
    if(GUI.old_button==GUI.akt_button) {
      // verlassen ohne was zu aendern
      return(return_val);
    }
  }

  // Maxumum-Value des Menupunktes
  max=MM_ITEM[GUI.akt_menu].um_cnt;

  // default returndata
  return_val=MENU_CHANGE_NORMAL;

  if(GUI.akt_menu==MM_CH1) {
    if(Menu.ch1.faktor<max-1) Menu.ch1.faktor++;
  }
  else if(GUI.akt_menu==MM_CH2) {
    if(Menu.ch2.faktor<max-1) Menu.ch2.faktor++;
  }
  else if(GUI.akt_menu==MM_TIME) {
    if(Menu.timebase.value<max-1) Menu.timebase.value++;
    return_val=MENU_CHANGE_FRQ;
  }
  else if(GUI.akt_menu==MM_SETTING) {
    if(Menu.akt_setting<max-1) Menu.akt_setting++;
  }
  else if(GUI.akt_menu==MM_TRG_SOURCE) {
    if(Menu.trigger.source<max-1) Menu.trigger.source++;
  }
  else if(GUI.akt_menu==MM_TRG_EDGE) {
    if(Menu.trigger.edge<max-1) Menu.trigger.edge++;
  }
  else if(GUI.akt_menu==MM_TRG_MODE) {
    if(Menu.trigger.mode<max-1) Menu.trigger.mode++;
    return_val=MENU_CHANGE_MODE;
  }
  else if(GUI.akt_menu==MM_TRG_VAL) {
    if(Menu.trigger.source==0) { // CH1
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.trigger.value_ch1, 10);
      }
      else {
        value=dec_uintval(Menu.trigger.value_ch1, 0);
      }
      Menu.trigger.value_ch1=value;
    }
    else { // CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.trigger.value_ch2, 10);
      }
      else {
        value=dec_uintval(Menu.trigger.value_ch2, 0);
      }
      Menu.trigger.value_ch2=value;
    }
    return_val=MENU_CHANGE_VALUE;
  }
  else if(GUI.akt_menu==MM_TRG_RESET) {
    if(Menu.trigger.mode==2) { // "single"
      if(Menu.trigger.single==4) Menu.trigger.single=5;  // von "Ready" auf "Stop"
    }
    else { // "normal" oder "auto"
      if(Menu.trigger.single==0) {
        Menu.trigger.single=1; // von "Run" auf "Stop"
      }
      else if(Menu.trigger.single==1) {
        Menu.trigger.single=2; // von "Stop" auf "Weiter"
      }
    }
  }
  else if(GUI.akt_menu==MM_CH_VIS) {
    if(Menu.akt_setting==SETTING_CH1) { // CH1
      if(Menu.ch1.visible<max-1) Menu.ch1.visible++;
    }
    else if(Menu.akt_setting==SETTING_CH2) { // CH2
      if(Menu.ch2.visible<max-1) Menu.ch2.visible++;
    }
  }
  else if(GUI.akt_menu==MM_CH_POS) {
    if(Menu.akt_setting==SETTING_CH1) { // CH1
      if(GUI.old_button!=GUI.akt_button) {
        ivalue=dec_intval(Menu.ch1.position, 1);
      }
      else {
        ivalue=dec_intval(Menu.ch1.position, 0);
      }
      Menu.ch1.position=ivalue;
    }
    else if(Menu.akt_setting==SETTING_CH2) { // CH2
      if(GUI.old_button!=GUI.akt_button) {
        ivalue=dec_intval(Menu.ch2.position, 1);
      }
      else {
        ivalue=dec_intval(Menu.ch2.position, 0);
      }
      Menu.ch2.position=ivalue;
    }
    return_val=MENU_CHANGE_VALUE;
  }
  else if(GUI.akt_menu==MM_CUR_MODE) {
    if(Menu.cursor.mode<max-1) Menu.cursor.mode++;
  }
  else if(GUI.akt_menu==MM_CUR_P1) {
    if(Menu.cursor.mode==3) { // TIME
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.cursor.t1, 1);
      }
      else {
        value=dec_uintval(Menu.cursor.t1, 0);
      }
      Menu.cursor.t1=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if((Menu.cursor.mode==1) || (Menu.cursor.mode==2)) { // CH1+CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.cursor.p1, 10);
      }
      else {
        value=dec_uintval(Menu.cursor.p1, 0);
      }
      Menu.cursor.p1=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if(Menu.cursor.mode==4) { // FFT
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.cursor.f1, 1);
      }
      else {
        value=dec_uintval(Menu.cursor.f1, 0);
      }
      Menu.cursor.f1=value;
      return_val=MENU_CHANGE_VALUE;
    }
  }
  else if(GUI.akt_menu==MM_CUR_P2) {
    if(Menu.cursor.mode==3) { // TIME
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.cursor.t2, 1);
      }
      else {
        value=dec_uintval(Menu.cursor.t2, 0);
      }
      Menu.cursor.t2=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if((Menu.cursor.mode==1) || (Menu.cursor.mode==2)) { // CH1+CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=dec_uintval(Menu.cursor.p2, 10);
      }
      else {
        value=dec_uintval(Menu.cursor.p2, 0);
      }
      Menu.cursor.p2=value;
      return_val=MENU_CHANGE_VALUE;
    }
  }
  else if(GUI.akt_menu==MM_FFT_MODE) {
    if(Menu.fft.mode<max-1) Menu.fft.mode++;
  }
  else if(GUI.akt_menu==MM_SEND_MODE) {
    if(Menu.send.mode<max-1) Menu.send.mode++;
  }
  else if(GUI.akt_menu==MM_SEND_SCREEN) {
    if(Menu.send.screen<max-1) Menu.send.screen++;
  }
  else if(GUI.akt_menu==MM_SEND_DATA) {
    if(Menu.send.data==0) Menu.send.data=1;
    return_val=MENU_SEND_DATA;
  }

  return(return_val);
}


//--------------------------------------------------------------
// Decrements the current menu item
//--------------------------------------------------------------
MENU_Status_t p_gui_dec_menu(void)
{
  MENU_Status_t return_val=MENU_NO_CHANGE;
  uint16_t value;
  int16_t ivalue;

  if(GUI.akt_menu==MM_NONE) return(return_val);

  // alle "normalen" Menupunkte koennen nur einmal betaetigt werden
  if((GUI.akt_menu!=MM_TRG_VAL) && (GUI.akt_menu!=MM_CH_POS) &&
    (GUI.akt_menu!=MM_CUR_P1) && (GUI.akt_menu!=MM_CUR_P2)) {
    // wenn schon mal betaetigt
    if(GUI.old_button==GUI.akt_button) {
      // verlassen ohne was zu aendern
      return(return_val);
    }
  }

  // default returndata
  return_val=MENU_CHANGE_NORMAL;

  if(GUI.akt_menu==MM_CH1) {
    if(Menu.ch1.faktor>0) Menu.ch1.faktor--;
  }
  else if(GUI.akt_menu==MM_CH2) {
    if(Menu.ch2.faktor>0) Menu.ch2.faktor--;
  }
  else if(GUI.akt_menu==MM_TIME) {
    if(Menu.timebase.value>0) Menu.timebase.value--;
    return_val=MENU_CHANGE_FRQ;
  }
  else if(GUI.akt_menu==MM_SETTING) {
    if(Menu.akt_setting>0) Menu.akt_setting--;
  }
  else if(GUI.akt_menu==MM_TRG_SOURCE) {
    if(Menu.trigger.source>0) Menu.trigger.source--;
  }
  else if(GUI.akt_menu==MM_TRG_EDGE) {
    if(Menu.trigger.edge>0) Menu.trigger.edge--;
  }
  else if(GUI.akt_menu==MM_TRG_MODE) {
    if(Menu.trigger.mode>0) Menu.trigger.mode--;
    return_val=MENU_CHANGE_MODE;
  }
  else if(GUI.akt_menu==MM_TRG_VAL) {
    if(Menu.trigger.source==0) { // CH1
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.trigger.value_ch1, 10);
      }
      else {
        value=inc_uintval(Menu.trigger.value_ch1, 0);
      }
      Menu.trigger.value_ch1=value;
    }
    else { // CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.trigger.value_ch2, 10);
      }
      else {
        value=inc_uintval(Menu.trigger.value_ch2, 0);
      }
      Menu.trigger.value_ch2=value;
    }
    return_val=MENU_CHANGE_VALUE;
  }
  else if(GUI.akt_menu==MM_TRG_RESET) {
    if(Menu.trigger.mode==2) { // "single"
      if(Menu.trigger.single==4) Menu.trigger.single=5;  // von "Ready" auf "Stop"
    }
    else { // "normal" oder "auto"
      if(Menu.trigger.single==0) {
        Menu.trigger.single=1; // von "Run" auf "Stop"
      }
      else if(Menu.trigger.single==1) {
        Menu.trigger.single=2; // von "Stop" auf "Weiter"
      }
    }
  }
  else if(GUI.akt_menu==MM_CH_VIS) {
    if(Menu.akt_setting==SETTING_CH1) { // CH1
      if(Menu.ch1.visible>0) Menu.ch1.visible--;
    }
    else if(Menu.akt_setting==SETTING_CH2) { // CH2
      if(Menu.ch2.visible>0) Menu.ch2.visible--;
    }
  }
  else if(GUI.akt_menu==MM_CH_POS) {
    if(Menu.akt_setting==SETTING_CH1) { // CH1
      if(GUI.old_button!=GUI.akt_button) {
        ivalue=inc_intval(Menu.ch1.position, 1);
      }
      else {
        ivalue=inc_intval(Menu.ch1.position, 0);
      }
      Menu.ch1.position=ivalue;
    }
    else if(Menu.akt_setting==SETTING_CH2) { // CH2
      if(GUI.old_button!=GUI.akt_button) {
        ivalue=inc_intval(Menu.ch2.position, 1);
      }
      else {
        ivalue=inc_intval(Menu.ch2.position, 0);
      }
      Menu.ch2.position=ivalue;
    }
    return_val=MENU_CHANGE_VALUE;
  }
  else if(GUI.akt_menu==MM_CUR_MODE) {
    if(Menu.cursor.mode>0) Menu.cursor.mode--;
  }
  else if(GUI.akt_menu==MM_CUR_P1) {
    if(Menu.cursor.mode==3) { // TIME
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.cursor.t1, 1);
      }
      else {
        value=inc_uintval(Menu.cursor.t1, 0);
      }
      Menu.cursor.t1=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if((Menu.cursor.mode==1) || (Menu.cursor.mode==2)) { // CH1+CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.cursor.p1, 10);
      }
      else {
        value=inc_uintval(Menu.cursor.p1, 0);
      }
      Menu.cursor.p1=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if(Menu.cursor.mode==4) { // FFT
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.cursor.f1, 1);
      }
      else {
        value=inc_uintval(Menu.cursor.f1, 0);
      }
      Menu.cursor.f1=value;
      return_val=MENU_CHANGE_VALUE;
    }
  }
  else if(GUI.akt_menu==MM_CUR_P2) {
    if(Menu.cursor.mode==3) { // TIME
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.cursor.t2, 1);
      }
      else {
        value=inc_uintval(Menu.cursor.t2, 0);
      }
      Menu.cursor.t2=value;
      return_val=MENU_CHANGE_VALUE;
    }
    else if((Menu.cursor.mode==1) || (Menu.cursor.mode==2)) { // CH1+CH2
      if(GUI.old_button!=GUI.akt_button) {
        value=inc_uintval(Menu.cursor.p2, 10);
      }
      else {
        value=inc_uintval(Menu.cursor.p2, 0);
      }
      Menu.cursor.p2=value;
      return_val=MENU_CHANGE_VALUE;
    }
  }
  else if(GUI.akt_menu==MM_FFT_MODE) {
    if(Menu.fft.mode>0) Menu.fft.mode--;
  }
  else if(GUI.akt_menu==MM_SEND_MODE) {
    if(Menu.send.mode>0) Menu.send.mode--;
  }
  else if(GUI.akt_menu==MM_SEND_SCREEN) {
    if(Menu.send.screen>0) Menu.send.screen--;
  }
  else if(GUI.akt_menu==MM_SEND_DATA) {
    if(Menu.send.data==0) Menu.send.data=1;
    return_val=MENU_SEND_DATA;
  }

  return(return_val);
}



//--------------------------------------------------------------
// changes the menu, depending on which GUI button is pressed
//--------------------------------------------------------------
MENU_Status_t p_make_GUI_changes(void)
{
  MENU_Status_t return_val=MENU_NO_CHANGE;

  if(GUI.akt_button==GUI_BTN_RIGHT) {
    if(GUI.old_button!=GUI.akt_button) {
      return_val=MENU_CHANGE_GUI;
      // Bottom-Trigger
      if(GUI.akt_menu==MM_TRG_VAL) GUI.akt_menu=MM_TRG_RESET;
      if(GUI.akt_menu==MM_TRG_MODE) GUI.akt_menu=MM_TRG_VAL;
      if(GUI.akt_menu==MM_TRG_EDGE) GUI.akt_menu=MM_TRG_MODE;
      if(GUI.akt_menu==MM_TRG_SOURCE) GUI.akt_menu=MM_TRG_EDGE;

      // Bottom-CH
      if(GUI.akt_menu==MM_CH_VIS) GUI.akt_menu=MM_CH_POS;

      // Bottom-Cursor
      if(GUI.akt_menu==MM_CUR_P1) {
        if(Menu.cursor.mode!=4) GUI.akt_menu=MM_CUR_P2;
      }
      if(GUI.akt_menu==MM_CUR_MODE) {
        if(Menu.cursor.mode>0) GUI.akt_menu=MM_CUR_P1;
      }

      // Bottom-Send
      if(GUI.akt_menu==MM_SEND_SCREEN) GUI.akt_menu=MM_SEND_DATA;
      if(GUI.akt_menu==MM_SEND_MODE) GUI.akt_menu=MM_SEND_SCREEN;

      // TOP (last Entry)
      if(GUI.akt_menu==MM_SETTING) {
        if(Menu.akt_setting==SETTING_TRIGGER) GUI.akt_menu=MM_TRG_SOURCE;
        if(Menu.akt_setting==SETTING_CH1) GUI.akt_menu=MM_CH_VIS;
        if(Menu.akt_setting==SETTING_CH2) GUI.akt_menu=MM_CH_VIS;
        if(Menu.akt_setting==SETTING_CURSOR) GUI.akt_menu=MM_CUR_MODE;
        if(Menu.akt_setting==SETTING_FFT) GUI.akt_menu=MM_FFT_MODE;
        if(Menu.akt_setting==SETTING_SEND) GUI.akt_menu=MM_SEND_MODE;
      }

      // TOP
      if(GUI.akt_menu==MM_TIME) GUI.akt_menu=MM_SETTING;
      if(GUI.akt_menu==MM_CH2) GUI.akt_menu=MM_TIME;
      if(GUI.akt_menu==MM_CH1) GUI.akt_menu=MM_CH2;
    }
  }
  else if(GUI.akt_button==GUI_BTN_LEFT) {
    if(GUI.old_button!=GUI.akt_button) {
      return_val=MENU_CHANGE_GUI;
      // TOP
      if(GUI.akt_menu==MM_CH2) GUI.akt_menu=MM_CH1;
      if(GUI.akt_menu==MM_TIME) GUI.akt_menu=MM_CH2;
      if(GUI.akt_menu==MM_SETTING) GUI.akt_menu=MM_TIME;

      // Bottom-Trigger
      if(GUI.akt_menu==MM_TRG_SOURCE) GUI.akt_menu=MM_SETTING;
      if(GUI.akt_menu==MM_TRG_EDGE) GUI.akt_menu=MM_TRG_SOURCE;
      if(GUI.akt_menu==MM_TRG_MODE) GUI.akt_menu=MM_TRG_EDGE;
      if(GUI.akt_menu==MM_TRG_VAL) GUI.akt_menu=MM_TRG_MODE;
      if(GUI.akt_menu==MM_TRG_RESET) GUI.akt_menu=MM_TRG_VAL;

      // Bottom-CH
      if(GUI.akt_menu==MM_CH_VIS) GUI.akt_menu=MM_SETTING;
      if(GUI.akt_menu==MM_CH_POS) GUI.akt_menu=MM_CH_VIS;

      // Bottom-Cursor
      if(GUI.akt_menu==MM_CUR_MODE) GUI.akt_menu=MM_SETTING;
      if(GUI.akt_menu==MM_CUR_P1) GUI.akt_menu=MM_CUR_MODE;
      if(GUI.akt_menu==MM_CUR_P2) GUI.akt_menu=MM_CUR_P1;

      // Bottom-FFT
      if(GUI.akt_menu==MM_FFT_MODE) GUI.akt_menu=MM_SETTING;
      
      // Bottom-Send
      if(GUI.akt_menu==MM_SEND_MODE) GUI.akt_menu=MM_SETTING;
      if(GUI.akt_menu==MM_SEND_SCREEN) GUI.akt_menu=MM_SEND_MODE;
      if(GUI.akt_menu==MM_SEND_DATA) GUI.akt_menu=MM_SEND_SCREEN;
    }
  }
  else if(GUI.akt_button==GUI_BTN_DOWN) {
    return_val=p_gui_inc_menu();
  }
  else if(GUI.akt_button==GUI_BTN_UP) {
    return_val=p_gui_dec_menu();
  }

  GUI.old_button=GUI.akt_button;

  return(return_val);
}


//--------------------------------------------------------------
// checks and updates the GUI
// Show or hide GUI or change an active menu item
//--------------------------------------------------------------
MENU_Status_t menu_check_touch(void)
{
  MENU_Status_t return_val=MENU_NO_CHANGE;
  uint16_t x,y;
  static uint16_t x_old=999,y_old=999;
  static uint16_t gui_changed=0;

  //------------------------
  // Touch auslesen
  //------------------------
  UB_Touch_Read();
  if(Touch_Data.status==TOUCH_PRESSED) {
    // Touch ist bet�tigt
    x=Touch_Data.xp;
    y=Touch_Data.yp;

    if((x!=x_old) || (y!=y_old)) {
      x_old=x;
      y_old=y;
    }
    else {
      if(GUI.gui_xpos==GUI_XPOS_OFF) {
        // GUI ist im Moment noch AUS
        if(gui_changed==0) {
          // GUI einschalten (an einer von 3 positionen)
          GUI.gui_xpos=GUI_XPOS_RIGHT;
          if(y<GUI_XPOS_RIGHT) GUI.gui_xpos=GUI_XPOS_MID;
          if(y<GUI_XPOS_MID) GUI.gui_xpos=GUI_XPOS_LEFT;
          GUI.akt_menu=GUI.old_menu;
          gui_changed=1;
          return_val=MENU_CHANGE_GUI;
        }
      }
      else if(GUI.gui_xpos==GUI_XPOS_RIGHT) {
        // GUI-rechts ist aktiv
        if(gui_changed==0) {
          if(y<GUI_XPOS_RIGHT) {
            // GUI ausschalten
            GUI.gui_xpos=GUI_XPOS_OFF;
            GUI.old_menu=GUI.akt_menu;
            GUI.akt_menu=MM_NONE;
            gui_changed=1;
            return_val=MENU_CHANGE_GUI;
          }
          else {
            // GUI check
            p_get_GUI_button(x,y);
            return_val=p_make_GUI_changes();
          }
        }
      }
      else if(GUI.gui_xpos==GUI_XPOS_LEFT) {
        // GUI-links ist aktiv
        if(gui_changed==0) {
          if(y>GUI_XPOS_MID) {
            // GUI ausschalten
            GUI.gui_xpos=GUI_XPOS_OFF;
            GUI.old_menu=GUI.akt_menu;
            GUI.akt_menu=MM_NONE;
            gui_changed=1;
            return_val=MENU_CHANGE_GUI;
          }
          else {
            // GUI check
            p_get_GUI_button(x,y);
            return_val=p_make_GUI_changes();
          }
        }
      }
      else {
        // GUI-mitte ist aktiv
        if(gui_changed==0) {
          if((y<GUI_XPOS_MID) || (y>GUI_XPOS_RIGHT)) {
            // GUI ausschalten
            GUI.gui_xpos=GUI_XPOS_OFF;
            GUI.old_menu=GUI.akt_menu;
            GUI.akt_menu=MM_NONE;
            gui_changed=1;
            return_val=MENU_CHANGE_GUI;
          }
          else {
            // GUI check
            p_get_GUI_button(x,y);
            return_val=p_make_GUI_changes();
          }
        }
      }

      // beim senden von Daten die GUI abschalten
      if(return_val==MENU_SEND_DATA) {
        // GUI ausschalten
        GUI.gui_xpos=GUI_XPOS_OFF;
        GUI.old_menu=MM_SETTING;
        GUI.akt_menu=MM_NONE;
        gui_changed=1;
        if(Menu.send.mode==6) {
          // auf gewaehltes Menu umschalten
          if(Menu.send.screen==0) Menu.akt_setting=SETTING_TRIGGER;
          if(Menu.send.screen==1) Menu.akt_setting=SETTING_CURSOR;
        }
        else {
          Menu.akt_setting=SETTING_SEND;
        }
      }
    }
  }
  else {
    // Touch ist nicht bet�tigt
    gui_changed=0;
    if(GUI.old_button!=GUI_BTN_NONE) {
      return_val=MENU_CHANGE_GUI;
    }
    GUI.akt_button=GUI_BTN_NONE;
    GUI.old_button=GUI_BTN_NONE;
  }

  return(return_val);
}


//--------------------------------------------------------------
// draws a TOP menu item
//--------------------------------------------------------------
void p_gui_draw_TOP(uint32_t mm_nr, const SM_Item_t um[], uint32_t um_nr)
{
  sprintf(buf,"%s%s",MM_ITEM[mm_nr].txt,um[um_nr].stxt);
  if(GUI.akt_menu==mm_nr) {
    UB_Font_DrawString(LINE(1),MM_ITEM[mm_nr].yp,buf,&Arial_7x10,MENU_VG_COL,MENU_AK_COL);
  }
  else {
    UB_Font_DrawString(LINE(1),MM_ITEM[mm_nr].yp,buf,&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
  }
}


//--------------------------------------------------------------
// draws a BOTTOM menu item
//--------------------------------------------------------------
void p_menu_draw_BOT(uint32_t mm_nr, const SM_Item_t um[], uint32_t um_nr, uint32_t mode)
{

  if(mode==0) {
    // standard Menupunkt
    sprintf(buf,"%s%s",MM_ITEM[mm_nr].txt,um[um_nr].stxt);
  }
  else if(mode==1) {
    // Sondermenupunkt : "Trigger Value"
    if(Menu.trigger.source==0) {
      P_FloatToDezStr(FAKTOR_ADC*Menu.trigger.value_ch1);
      sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
    }
    if(Menu.trigger.source==1) {
      P_FloatToDezStr(FAKTOR_ADC*Menu.trigger.value_ch2);
      sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
    }
  }
  else if(mode==2) {
    // Sondermenupunkt : "Channel position"
    if(Menu.akt_setting==SETTING_CH1) {
      P_FloatToDezStr(P_Volt_to_Float(Menu.ch1.faktor,Menu.ch1.position));
      sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
    }
    if(Menu.akt_setting==SETTING_CH2) {
      P_FloatToDezStr(P_Volt_to_Float(Menu.ch2.faktor,Menu.ch2.position));
      sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
    }
  }
  else if(mode==3) {
    // Sondermenupunkt : "CH1/CH2 Cursor-A position"
    P_FloatToDezStr(FAKTOR_ADC*Menu.cursor.p1);
    sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
  }
  else if(mode==4) {
    // Sondermenupunkt : "CH1/CH2 Cursor-B position"
    P_FloatToDezStr(FAKTOR_ADC*Menu.cursor.p2);
    sprintf(buf,"%s%sV",MM_ITEM[mm_nr].txt,bval);
  }
  else if(mode==5) {
    // Sondermenupunkt : "TIME Cursor-A position"
    P_FloatToDezStr(P_Time_to_Float(Menu.timebase.value, Menu.cursor.t1));
    sprintf(buf,"%s%s",MM_ITEM[mm_nr].txt,bval);
  }
  else if(mode==6) {
    // Sondermenupunkt : "TIME Cursor-B position"
    P_FloatToDezStr(P_Time_to_Float(Menu.timebase.value, Menu.cursor.t2));
    sprintf(buf,"%s%s",MM_ITEM[mm_nr].txt,bval);
  }
  else if(mode==7) {
    // Sondermenupunkt : "Trigger Reset"
    sprintf(buf,"%s%s",MM_ITEM[mm_nr].txt,um[um_nr].stxt);
  }
  else if(mode==8) {
    // Sondermenupunkt : "FFT Cursor-A position"
    P_FloatToDezStr(P_FFT_to_Float(Menu.timebase.value, Menu.cursor.f1));
    if(Menu.timebase.value<=12) {
      sprintf(buf,"%s%sHz",MM_ITEM[mm_nr].txt,bval);
    }
    else {
      sprintf(buf,"%s%skHz",MM_ITEM[mm_nr].txt,bval);
    }
  }

  if(GUI.akt_menu==mm_nr) {
    UB_Font_DrawString(LINE(24),MM_ITEM[mm_nr].yp,buf,&Arial_7x10,MENU_VG_COL,MENU_AK_COL);
  }
  else {
    UB_Font_DrawString(LINE(24),MM_ITEM[mm_nr].yp,buf,&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
  }
}


//--------------------------------------------------------------
// draws submenu: "Trigger"
//--------------------------------------------------------------
void p_menu_draw_BOT_TRG(void)
{
  p_menu_draw_BOT(MM_TRG_SOURCE,UM_04,Menu.trigger.source,0);
  p_menu_draw_BOT(MM_TRG_EDGE,UM_05,Menu.trigger.edge,0);
  p_menu_draw_BOT(MM_TRG_MODE,UM_06,Menu.trigger.mode,0);
  p_menu_draw_BOT(MM_TRG_VAL,UM_07,0,1);
  p_menu_draw_BOT(MM_TRG_RESET,UM_10,Menu.trigger.single,7);
}


//--------------------------------------------------------------
// draws submenu: "CH1"
//--------------------------------------------------------------
void p_menu_draw_BOT_CH1(void)
{
  p_menu_draw_BOT(MM_CH_VIS,UM_08,Menu.ch1.visible,0);
  p_menu_draw_BOT(MM_CH_POS,UM_07,0,2);
}


//--------------------------------------------------------------
// draws submenu: "CH2"
//--------------------------------------------------------------
void p_menu_draw_BOT_CH2(void)
{
  p_menu_draw_BOT(MM_CH_VIS,UM_08,Menu.ch2.visible,0);
  p_menu_draw_BOT(MM_CH_POS,UM_07,0,2);
}


//--------------------------------------------------------------
// draws submenu: "CURSOR"
//--------------------------------------------------------------
void p_menu_draw_BOT_CUR(void)
{
  uint16_t delta;

  p_menu_draw_BOT(MM_CUR_MODE,UM_09,Menu.cursor.mode,0);
  if((Menu.cursor.mode==1) || (Menu.cursor.mode==2)) {
    // Cursor = CH1/CH2
    p_menu_draw_BOT(MM_CUR_P1,UM_07,0,3);
    p_menu_draw_BOT(MM_CUR_P2,UM_07,0,4);
    if(Menu.cursor.p1>=Menu.cursor.p2) {
      delta=Menu.cursor.p1-Menu.cursor.p2;
    }
    else {
      delta=Menu.cursor.p2-Menu.cursor.p1;
    }
    P_FloatToDezStr(FAKTOR_ADC*delta);
    sprintf(buf,"~=%sV",bval);
    UB_Font_DrawString(LINE(24),33*FONT_W,buf,&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
  }
  else if(Menu.cursor.mode==3) {
    // Cursor = TIME
    p_menu_draw_BOT(MM_CUR_P1,UM_07,0,5);
    p_menu_draw_BOT(MM_CUR_P2,UM_07,0,6);
    if(Menu.cursor.t1>=Menu.cursor.t2) {
      delta=Menu.cursor.t1-Menu.cursor.t2;
    }
    else {
      delta=Menu.cursor.t2-Menu.cursor.t1;
    }
    P_FloatToDezStr(P_Time_to_Float(Menu.timebase.value, (delta+2048)));
    if(Menu.timebase.value<3) {
      sprintf(buf,"~=%ss",bval);
    }
    else if(Menu.timebase.value<12) {
      sprintf(buf,"~=%sms",bval);
    }
    else {
      sprintf(buf,"~=%sus",bval);
    }

    UB_Font_DrawString(LINE(24),33*FONT_W,buf,&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
  }
  else if(Menu.cursor.mode==4) {
    // Cursor = FFT
    p_menu_draw_BOT(MM_CUR_P1,UM_07,0,8);
  }
}


//--------------------------------------------------------------
// draws submenu: "FFT"
//--------------------------------------------------------------
void p_menu_draw_BOT_FFT(void)
{
  p_menu_draw_BOT(MM_FFT_MODE,UM_13,Menu.fft.mode,0);
}



//--------------------------------------------------------------
// draws submenu: "SEND"
//--------------------------------------------------------------
void p_menu_draw_BOT_SEND(void)
{
  p_menu_draw_BOT(MM_SEND_MODE,UM_11,Menu.send.mode,0);
  p_menu_draw_BOT(MM_SEND_SCREEN,UM_14,Menu.send.screen,0);
  p_menu_draw_BOT(MM_SEND_DATA,UM_12,Menu.send.data,0);
}


//--------------------------------------------------------------
// draws submenu: "VERSION"
//--------------------------------------------------------------
void p_menu_draw_BOT_VERSION(void)
{
 // UB_Font_DrawString(LINE(24),10,"STM32F429-Oszi | UB | V:1.6 | 24.03.2014",&Arial_7x10,MENU_VG_COL,MENU_BG_COL); // Noah 20141013
  UB_Font_DrawString(LINE(24),10,"STM32F429-Noah | Oszi/UB|V:0.1| 2014.10.13",&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
}


//--------------------------------------------------------------
// draws submenu: "HELP"
//--------------------------------------------------------------
void p_menu_draw_BOT_HELP(void)
{
  UB_Font_DrawString(LINE(24),10,"CH1=PA5 | CH2=PA7 | TX=PA9 | 500Hz=PB2",&Arial_7x10,MENU_VG_COL,MENU_BG_COL); // Noah 20141013
//  UB_Font_DrawString(LINE(24),10,"CH1=PA3 | CH2=PA7 | TX=PA9 | 500Hz=PB2",&Arial_7x10,MENU_VG_COL,MENU_BG_COL);
}


//--------------------------------------------------------------
// Change of value (increment unsigned)
// data area: 0 to 4095
// automatically increases the increment
//--------------------------------------------------------------
uint16_t inc_uintval(uint16_t data, uint16_t startdata)
{
  uint16_t return_val=0;
  int16_t signed_int_data;
  static uint16_t inc_delay=0,inc_val=1;

  if(startdata>0) {
    inc_delay=0;
    inc_val=startdata;
  }
  else {
    inc_delay++;
    if(inc_delay>10) {
      inc_delay=0;
      inc_val+=10;
    }
  }

  signed_int_data=data;
  signed_int_data+=inc_val;

  if(signed_int_data>4095) signed_int_data=4095;
  return_val=(uint16_t)(signed_int_data);

  return(return_val);
}


//--------------------------------------------------------------
// Value change (decrement without sign)
// data range: 0 to 4095
// automatically increases the increment
//--------------------------------------------------------------
uint16_t dec_uintval(uint16_t data, uint16_t startdata)
{
  uint16_t return_val=0;
  int16_t signed_int_data;
  static uint16_t dec_delay=0,dec_val=1;

  if(startdata>0) {
    dec_delay=0;
    dec_val=startdata;
  }
  else {
    dec_delay++;
    if(dec_delay>10) {
      dec_delay=0;
      dec_val+=10;
    }
  }

  signed_int_data=data;
  signed_int_data-=dec_val;

  if(signed_int_data<0) signed_int_data=0;
  return_val=(uint16_t)(signed_int_data);

  return(return_val);
}


//--------------------------------------------------------------
// Change of value (increment with sign)
// data range: -200 to +200
// automatically increases the increment
//--------------------------------------------------------------
int16_t inc_intval(int16_t data, uint16_t startdata)
{
  int16_t return_val=0;
  int16_t signed_int_data;
  static uint16_t inc_delay=0,inc_val=1;

  if(startdata>0) {
    inc_delay=0;
    inc_val=startdata;
  }
  else {
    inc_delay++;
    if(inc_delay>10) {
      inc_delay=0;
      inc_val+=10;
    }
  }

  signed_int_data=data;
  signed_int_data+=inc_val;

  if(signed_int_data>200) signed_int_data=200;
  return_val=(int16_t)(signed_int_data);

  return(return_val);
}


//--------------------------------------------------------------
// Value change (decrement with sign)
// data range: -200 to +200
// automatically increases the increment
//--------------------------------------------------------------
int16_t dec_intval(int16_t data, uint16_t startdata)
{
  int16_t return_val=0;
  int16_t signed_int_data;
  static uint16_t dec_delay=0,dec_val=1;

  if(startdata>0) {
    dec_delay=0;
    dec_val=startdata;
  }
  else {
    dec_delay++;
    if(dec_delay>10) {
      dec_delay=0;
      dec_val+=10;
    }
  }

  signed_int_data=data;
  signed_int_data-=dec_val;

  if(signed_int_data<-200) signed_int_data=-200;
  return_val=(int16_t)(signed_int_data);

  return(return_val);
}


//--------------------------------------------------------------
// Conversion: float number into a string
//--------------------------------------------------------------
void P_FloatToDezStr(float data)
{
  int16_t vorkomma;
  uint16_t nachkomma;
  float rest;

  if((data>32767) || (data<-32767)) {
	  // number too big or too small
    sprintf(bval,"%s","OVF");
    return;
  }

  vorkomma=(int16_t)(data);
  if(data>=0.0) {
    rest = data-(float)(vorkomma);
    nachkomma = (uint16_t)(rest*(float)(STRING_FLOAT_FAKTOR)+0.5);
    sprintf(bval,STRING_FLOAT_FORMAT,vorkomma,nachkomma);
  }
  else {
    rest = (float)(vorkomma)-data;
    nachkomma = (uint16_t)(rest*(float)(STRING_FLOAT_FAKTOR)+0.5);
    if(data<=-1.0) {
      sprintf(bval,STRING_FLOAT_FORMAT,vorkomma,nachkomma);
    }
    else {
      sprintf(bval,STRING_FLOAT_FORMAT2,vorkomma,nachkomma);
    }
  }
}


//--------------------------------------------------------------
// Conversion: data in volts (depending on the prescaler)
//--------------------------------------------------------------
float P_Volt_to_Float(uint32_t faktor, int16_t pos)
{
  float return_val=0.0;

  switch(faktor) {
    case 0 : return_val=pos*VFAKTOR_5V;break;
    case 1 : return_val=pos*VFAKTOR_2V;break;
    case 2 : return_val=pos*VFAKTOR_1V;break;
    case 3 : return_val=pos*VFAKTOR_0V5;break;
    case 4 : return_val=pos*VFAKTOR_0V2;break;
    case 5 : return_val=pos*VFAKTOR_0V1;break;
    case 6 : return_val=pos*VFAKTOR_0V05;break;
  }

  return(return_val);
}


//--------------------------------------------------------------
// Conversion: data in time (depending on the timebase)
//--------------------------------------------------------------
float P_Time_to_Float(uint32_t faktor, uint16_t pos)
{
  float return_val=0.0;
  int16_t signed_data=0;

  signed_data=pos-2048;

  switch(faktor) {
    case 0 : return_val=(float)(signed_data)*TFAKTOR_5;break;
    case 1 : return_val=(float)(signed_data)*TFAKTOR_2;break;
    case 2 : return_val=(float)(signed_data)*TFAKTOR_1;break;
    case 3 : return_val=(float)(signed_data)*TFAKTOR_500;break;
    case 4 : return_val=(float)(signed_data)*TFAKTOR_200;break;
    case 5 : return_val=(float)(signed_data)*TFAKTOR_100;break;
    case 6 : return_val=(float)(signed_data)*TFAKTOR_50;break;
    case 7 : return_val=(float)(signed_data)*TFAKTOR_20;break;
    case 8 : return_val=(float)(signed_data)*TFAKTOR_10;break;
    case 9 : return_val=(float)(signed_data)*TFAKTOR_5;break;
    case 10 : return_val=(float)(signed_data)*TFAKTOR_2;break;
    case 11 : return_val=(float)(signed_data)*TFAKTOR_1;break;
    case 12 : return_val=(float)(signed_data)*TFAKTOR_500;break;
    case 13 : return_val=(float)(signed_data)*TFAKTOR_200;break;
    case 14 : return_val=(float)(signed_data)*TFAKTOR_100;break;
    case 15 : return_val=(float)(signed_data)*TFAKTOR_50;break;
    case 16 : return_val=(float)(signed_data)*TFAKTOR_25;break;
  }

  return(return_val);
}


//--------------------------------------------------------------
// Conversion: data in FFT (depending on the timebase)
//--------------------------------------------------------------
float P_FFT_to_Float(uint32_t faktor, uint16_t pos)
{
  float return_val=0.0;
  int16_t signed_data=0;

  signed_data=pos;

  switch(faktor) {
    case 0 : return_val=(float)(signed_data)*FFAKTOR_5s;break;
    case 1 : return_val=(float)(signed_data)*FFAKTOR_2s;break;
    case 2 : return_val=(float)(signed_data)*FFAKTOR_1s;break;
    case 3 : return_val=(float)(signed_data)*FFAKTOR_500m;break;
    case 4 : return_val=(float)(signed_data)*FFAKTOR_200m;break;
    case 5 : return_val=(float)(signed_data)*FFAKTOR_100m;break;
    case 6 : return_val=(float)(signed_data)*FFAKTOR_50m;break;
    case 7 : return_val=(float)(signed_data)*FFAKTOR_20m;break;
    case 8 : return_val=(float)(signed_data)*FFAKTOR_10m;break;
    case 9 : return_val=(float)(signed_data)*FFAKTOR_5m;break;
    case 10 : return_val=(float)(signed_data)*FFAKTOR_2m;break;
    case 11 : return_val=(float)(signed_data)*FFAKTOR_1m;break;
    case 12 : return_val=(float)(signed_data)*FFAKTOR_500u;break;
    case 13 : return_val=(float)(signed_data)*FFAKTOR_200u;break;
    case 14 : return_val=(float)(signed_data)*FFAKTOR_100u;break;
    case 15 : return_val=(float)(signed_data)*FFAKTOR_50u;break;
    case 16 : return_val=(float)(signed_data)*FFAKTOR_25u;break;
  }

  return(return_val);
}


//--------------------------------------------------------------
// for converting a "line number" into a "pixel position"
// n : [1...24]
//--------------------------------------------------------------
uint16_t LINE(uint16_t n)
{
  uint16_t return_val=0;

  return_val=LCD_MAXX-(n*FONT_H)-1;

  return(return_val);
}


//--------------------------------------------------------------
// for converting a "pixel position" into a "line number"
// xp : [0...249]
//--------------------------------------------------------------
uint16_t GET_LINE(uint16_t xp)
{
  uint16_t return_val=0;

  return_val=((LCD_MAXX-xp)/FONT_H)+1;

  return(return_val);
}
