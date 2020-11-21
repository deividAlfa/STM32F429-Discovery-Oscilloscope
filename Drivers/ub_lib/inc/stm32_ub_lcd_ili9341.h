//--------------------------------------------------------------
// File     : stm32_ub_lcd_ili9341.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_LCD_ILI9341_H
#define __STM32F4_UB_LCD_ILI9341_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "main.h"


//--------------------------------------------------------------
// Standard Farbdatae
// 16bit Farbdata (R5G6B5)
// Red   (5bit) -> Bit15-Bit11
// Green (6bit) -> Bit10-Bit5
// Blue  (5bit) -> Bit4-Bit0
//--------------------------------------------------------------
/*#define  RGB_COL_BLACK          (uint32_t)0x000000
#define  RGB_COL_BLUE           (uint32_t)0x0000FF
#define  RGB_COL_GREEN          (uint32_t)0x00FF00
#define  RGB_COL_RED            (uint32_t)0xFF0000
#define  RGB_COL_WHITE          (uint32_t)0xFFFFFF

#define  RGB_COL_CYAN           (uint32_t)0x00FFFF
#define  RGB_COL_MAGENTA        (uint32_t)0xFF00FF
#define  RGB_COL_YELLOW         (uint32_t)0xFFFF00
#define  RGB_COL_GREY           (uint32_t)0xF0F0F0
*/
#define  RGB_COL_BLACK          0x0000
#define  RGB_COL_BLUE           0x001F
#define  RGB_COL_GREEN          0x07E0
#define  RGB_COL_RED            0xF800
#define  RGB_COL_WHITE          0xFFFF

#define  RGB_COL_CYAN           0x07FF
#define  RGB_COL_MAGENTA        0xF81F
#define  RGB_COL_YELLOW         0xFFE0
#define  RGB_COL_ORANGE         0xFC00

#define  RGB_COL_GREY           0xF7DE



//--------------------------------------------------------------
// Display Mode
//--------------------------------------------------------------
typedef enum {
  PORTRAIT =0,
  LANDSCAPE
}LCD_MODE_t;
LCD_MODE_t  LCD_DISPLAY_MODE;


//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
extern volatile uint32_t LCD_CurrentFrameBuffer;   // aktuelle Adresse zum zeichnen
extern volatile uint32_t LCD_CurrentLayer;         // 0=Hintergrund, 1=Vodergrund
extern volatile uint32_t LCD_CurrentOrientation;   // 0=0Grad, 1=180Grad
extern volatile uint32_t LCD_MenuFrameBuffer;      // Adressse vom Menu
extern volatile uint32_t LCD_ADCFrameBuffer;       // Adressse vom ADC


//--------------------------------------------------------------
// Defines vom Display
//--------------------------------------------------------------
#define  LCD_MAXX           ((uint16_t)240)      // Pixel in X-Richtung
#define  LCD_MAXY           ((uint16_t)320)      // Pixel in Y-Richtung
#define  LCD_PIXEL  LCD_MAXX*LCD_MAXY

#define  LCD_INIT_PAUSE   200  // kleine Pause beim Init
#define  LCD_SPI_PAUSE    10   // kleine Pause bei SPI



//--------------------------------------------------------------
// Defines f�r das Display-RAM
//--------------------------------------------------------------
#define  LCD_FRAME_BUFFER   ((uint32_t)0xD0000000)         // Start vom SDRAM
#define  LCD_FRAME_OFFSET   ((uint32_t)(LCD_PIXEL*2)) // gr��e von einem Puffer (in Byte)

//--------------------------------------------------------------
// Register vom ILI9341
//--------------------------------------------------------------
#define LCD_SLEEP_OUT            0x11   // Sleep out register
#define LCD_GAMMA                0x26   // Gamma register
#define LCD_DISPLAY_OFF          0x28   // Display off register
#define LCD_DISPLAY_ON           0x29   // Display on register
#define LCD_COLUMN_ADDR          0x2A   // Colomn address register
#define LCD_PAGE_ADDR            0x2B   // Page address register
#define LCD_GRAM                 0x2C   // GRAM register
#define LCD_MAC                  0x36   // Memory Access Control register
#define LCD_PIXEL_FORMAT         0x3A   // Pixel Format register
#define LCD_WDB                  0x51   // Write Brightness Display register
#define LCD_WCD                  0x53   // Write Control Display register
#define LCD_RGB_INTERFACE        0xB0   // RGB Interface Signal Control
#define LCD_FRC                  0xB1   // Frame Rate Control register
#define LCD_BPC                  0xB5   // Blanking Porch Control register
#define LCD_DFC                  0xB6   // Display Function Control register
#define LCD_POWER1               0xC0   // Power Control 1 register
#define LCD_POWER2               0xC1   // Power Control 2 register
#define LCD_VCOM1                0xC5   // VCOM Control 1 register
#define LCD_VCOM2                0xC7   // VCOM Control 2 register
#define LCD_POWERA               0xCB   // Power control A register
#define LCD_POWERB               0xCF   // Power control B register
#define LCD_PGAMMA               0xE0   // Positive Gamma Correction register
#define LCD_NGAMMA               0xE1   // Negative Gamma Correction register
#define LCD_DTCA                 0xE8   // Driver timing control A
#define LCD_DTCB                 0xEA   // Driver timing control B
#define LCD_POWER_SEQ            0xED   // Power on sequence register
#define LCD_3GAMMA_EN            0xF2   // 3 Gamma enable register
#define LCD_INTERFACE            0xF6   // Interface control register
#define LCD_PRC                  0xF7   // Pump ratio control register




//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_LCD_Init(void);
void UB_LCD_LayerInit_Fullscreen(void);
void UB_LCD_SetLayer_1(void);
void UB_LCD_SetLayer_2(void);
void UB_LCD_SetLayer_Menu(void);
void UB_LCD_SetLayer_ADC(void);
void UB_LCD_SetLayer_Back(void);
void UB_LCD_FillLayer(uint32_t color);
void UB_LCD_SetTransparency(uint8_t data);
void UB_LCD_SetCursor2Draw(uint16_t xpos, uint16_t ypos);
void UB_LCD_DrawPixel(uint32_t color);
void UB_LCD_SetMode(LCD_MODE_t mode);
void UB_LCD_Rotate_0(void);
void UB_LCD_Rotate_180(void);
void UB_LCD_Copy_Layer1_to_Layer2(void);
void UB_LCD_Copy_Layer2_to_Layer1(void);
void UB_LCD_Refresh(void);
void DMA2D_FillRect(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);


//--------------------------------------------------------------
#endif // __STM32F4_UB_LCD_ILI9341_H
