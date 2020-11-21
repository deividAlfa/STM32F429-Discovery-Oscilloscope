//--------------------------------------------------------------
// File     : stm32_ub_graphic2d.c
// Datum    : 25.11.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : STM32_UB_LCD_ILI9341, DMA2D
// Funktion : Grafik-LCD Funktionen (per DMA2D)
//            (Punkte, Linien, Rahmen, Flaechen)
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_graphic2d.h"
#include "stm32_ub_lcd_ili9341.h"

extern DMA2D_HandleTypeDef hdma2d;

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
int16_t P_Graphic2D_sgn(int16_t x);



//--------------------------------------------------------------
// l�scht den Bildschirm mit einer Farbe (per DMA2D)
//--------------------------------------------------------------
void UB_Graphic2D_ClearScreenDMA(uint32_t color)
{
	DMA2D_FillRect(0, 0, 0, LCD_MAXX, LCD_MAXY);

}


//--------------------------------------------------------------
// Zeichnet ein Pixel mit einer Farbe an x,y Position
// [x=0...LCD_MAXX, y=0...LCD_MAXY]
//--------------------------------------------------------------
void UB_Graphic2D_DrawPixelNormal(uint16_t xp, uint16_t yp, uint32_t color)
{
  // check auf Limit
  if(xp>=LCD_MAXX) return;
  if(yp>=LCD_MAXY) return;

  // Cursor setzen
  UB_LCD_SetCursor2Draw(xp,yp);
  // Pixel zeichnen
  UB_LCD_DrawPixel(color);
}


//--------------------------------------------------------------
// Zeichnet eine Linie mit einer Farbe
// von x1,y1 nach x2,y2  [x=0...LCD_MAXX, y=0...LCD_MAXY]
// (benutzt wird der Bresenham-Algorithmus)
//--------------------------------------------------------------
void UB_Graphic2D_DrawLineNormal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color)
{
  int16_t x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;

  // check auf Limit
  if(x1>=LCD_MAXX) x1=LCD_MAXX-1;
  if(y1>=LCD_MAXY) y1=LCD_MAXY-1;

  // check auf Limit
  if(x2>=LCD_MAXX) x2=LCD_MAXX-1;
  if(y2>=LCD_MAXY) y2=LCD_MAXY-1;

  // Entfernung in beiden Dimensionen berechnen
  dx = x2 - x1;
  dy = y2 - y1;

  // Vorzeichen des Inkrements bestimmen
  incx = P_Graphic2D_sgn(dx);
  incy = P_Graphic2D_sgn(dy);
  if(dx<0) dx = -dx;
  if(dy<0) dy = -dy;

  // feststellen, welche Entfernung gr��er ist
  if (dx>dy) {
    // x ist schnelle Richtung
    pdx=incx; pdy=0;    // pd. ist Parallelschritt
    ddx=incx; ddy=incy; // dd. ist Diagonalschritt
    es =dy;   el =dx;   // Fehlerschritte schnell, langsam
  }
  else {
    // y ist schnelle Richtung
    pdx=0;    pdy=incy; // pd. ist Parallelschritt
    ddx=incx; ddy=incy; // dd. ist Diagonalschritt
    es =dx;   el =dy;   // Fehlerschritte schnell, langsam
  }

  // Initialisierungen vor Schleifenbeginn
  x = x1;
  y = y1;
  err = (el>>1);
  UB_Graphic2D_DrawPixelNormal(x,y,color);

  // Pixel berechnen
  for(t=0; t<el; ++t) // t zaehlt die Pixel, el ist auch Anzahl
  {
    // Aktualisierung Fehlerterm
    err -= es;
    if(err<0) {
      // Fehlerterm wieder positiv (>=0) machen
      err += el;
      // Schritt in langsame Richtung, Diagonalschritt
      x += ddx;
      y += ddy;
    }
    else {
      // Schritt in schnelle Richtung, Parallelschritt
      x += pdx;
      y += pdy;
    }
    UB_Graphic2D_DrawPixelNormal(x,y,color);
  }
}

void DMA2D_FillRect(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height){

    HAL_DMA2D_DeInit(&hdma2d);
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.ColorMode = DMA2D_RGB565;
	hdma2d.Init.OutputOffset = LCD_MAXX - width;

	  // Foreground
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = 0;

	  // Background
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;

	  HAL_DMA2D_Init(&hdma2d);
	  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
	  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);

	HAL_DMA2D_Start(&hdma2d, color, LCD_CurrentFrameBuffer + 2*(x + y*LCD_MAXX),    width,    height);
	HAL_DMA2D_PollForTransfer(&hdma2d, 100);
}



//--------------------------------------------------------------
// Draws a straight line with one color (via DMA2D)
// Start = xp, yp [x = 0 ... LCD_MAXX, y = 0 ... LCD_MAXY]
// Length = l (is limited to the display limit)
// Direction = d [LCD_DIR_HORIZONTAL, LCD_DIR_VERTICAL]
//--------------------------------------------------------------
void UB_Graphic2D_DrawStraightDMA(uint16_t xp, uint16_t yp, uint16_t l, LCD_DIR_t d, uint32_t color){
	color = (((0xF800 & color) >> 11)<<16) |(((0x07E0 & color) >> 5)<<8) | ((0x001F & color)<<3);
	HAL_DMA2D_DeInit(&hdma2d);
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.ColorMode = DMA2D_RGB565;
	// check auf Limit
	if(xp>=LCD_MAXX) xp=LCD_MAXX-1;
	if(yp>=LCD_MAXY) yp=LCD_MAXY-1;
	if(l==0) return;
	if(LCD_DISPLAY_MODE==LANDSCAPE) {
		// richtung drehen
		if(d==LCD_DIR_HORIZONTAL) {
			d=LCD_DIR_VERTICAL;
		}
		else {
	 	d=LCD_DIR_HORIZONTAL;
		}
	}

	if(d == LCD_DIR_HORIZONTAL) {
	// check auf Limit
	if((xp+l)>LCD_MAXX) l=LCD_MAXX-xp;
	hdma2d.Init.OutputOffset = 0;
	}
	else {
	// check auf Limit
	if((yp+l)>LCD_MAXY) l=LCD_MAXY-yp;
	hdma2d.Init.OutputOffset = LCD_MAXX - 1;
	}

	  // Foreground
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = 0;

	  // Background
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;

	  HAL_DMA2D_Init(&hdma2d);
	  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
	  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);

	HAL_DMA2D_Start(&hdma2d, color, LCD_CurrentFrameBuffer + 2*(LCD_MAXX*yp + xp), 1, l);
	HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}


//--------------------------------------------------------------
// Draws a rectangle with one color (via DMA2D)
// corner = xp, yp [x = 0 ... LCD_MAXX, y = 0 ... LCD_MAXY]
// width = w
// height = h
//--------------------------------------------------------------
void UB_Graphic2D_DrawRectDMA(uint16_t xp, uint16_t yp, uint16_t w, uint16_t h, uint32_t color)
{
  int16_t d;

  // check auf Limit
  if(xp>=LCD_MAXX) xp=LCD_MAXX-1;
  if(yp>=LCD_MAXY) yp=LCD_MAXY-1;
  if(w==0) return;
  if(h==0) return;

  if(LCD_DISPLAY_MODE==LANDSCAPE) {
    // richtung drehen
    d=w;
    w=h;
    h=d;
  }

  // check auf Limit
  if((xp+w)>LCD_MAXX) w=LCD_MAXX-xp;
  if((yp+h)>LCD_MAXY) h=LCD_MAXY-yp;

  if(LCD_DISPLAY_MODE==LANDSCAPE) {
    UB_Graphic2D_DrawStraightDMA(xp, yp, h, LCD_DIR_HORIZONTAL, color);
    UB_Graphic2D_DrawStraightDMA((xp+w-1), yp, h, LCD_DIR_HORIZONTAL, color);
    UB_Graphic2D_DrawStraightDMA(xp, yp, w, LCD_DIR_VERTICAL, color);
    UB_Graphic2D_DrawStraightDMA(xp, yp+h-1, w, LCD_DIR_VERTICAL, color);
  }
  else {
    UB_Graphic2D_DrawStraightDMA(xp, yp, w, LCD_DIR_HORIZONTAL, color);
    UB_Graphic2D_DrawStraightDMA(xp, (yp+ h-1), w, LCD_DIR_HORIZONTAL, color);
    UB_Graphic2D_DrawStraightDMA(xp, yp, h, LCD_DIR_VERTICAL, color);
    UB_Graphic2D_DrawStraightDMA((xp + w-1), yp, h, LCD_DIR_VERTICAL, color);
  }
}


//--------------------------------------------------------------
// Draws a filled rectangle with one color (via DMA2D)
// Start corner = xp, yp [x = 0 ... LCD_MAXX, y = 0 ... LCD_MAXY]
// width = w
// height = h
//--------------------------------------------------------------
void UB_Graphic2D_DrawFullRectDMA(uint16_t xp, uint16_t yp, uint16_t w, uint16_t h, uint32_t color)
{
  uint32_t  Xaddress;
  int16_t d;
  color = (((0xF800 & color) >> 11)<<16) |(((0x07E0 & color) >> 5)<<8) | ((0x001F & color)<<3);
 // color = 0xff;
  // check auf Limit
  if(xp>=LCD_MAXX) xp=LCD_MAXX-1;
  if(yp>=LCD_MAXY) yp=LCD_MAXY-1;
  if(w==0) return;
  if(h==0) return;

  if(LCD_DISPLAY_MODE==LANDSCAPE) {
    // richtung drehen
    d=w;
    w=h;
    h=d;
  }

  // check auf Limit
  if((xp+w)>LCD_MAXX) w=LCD_MAXX-xp;
  if((yp+h)>LCD_MAXY) h=LCD_MAXY-yp;

  Xaddress = LCD_CurrentFrameBuffer + 2*(LCD_MAXX*yp + xp);

  HAL_DMA2D_DeInit(&hdma2d);

  hdma2d.Instance = DMA2D;
  hdma2d.Init.ColorMode = DMA2D_RGB565;
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.OutputOffset = (LCD_MAXX - w);
  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_Start(&hdma2d,color,Xaddress, w, h );

  while(__HAL_DMA2D_GET_FLAG(&hdma2d,DMA2D_FLAG_TC) == RESET);
}


//--------------------------------------------------------------
// Draws a circle with one color
// center point = xp, yp [x = 0 ... LCD_MAXX, y = 0 ... LCD_MAXY]
// radius = r
// (the Bresenham algorithm is used)
//--------------------------------------------------------------
void UB_Graphic2D_DrawCircleNormal(uint16_t xp, uint16_t yp, uint16_t r, uint32_t color)
{
  int16_t f=1-r, ddF_x=0, ddF_y=-2*r, x=0, y=r;

  // check auf Limit
  if(xp>=LCD_MAXX) xp=LCD_MAXX-1;
  if(yp>=LCD_MAXY) yp=LCD_MAXY-1;
  if(r==0) return;

  UB_Graphic2D_DrawPixelNormal(xp, yp + r, color);
  UB_Graphic2D_DrawPixelNormal(xp, yp - r, color);
  UB_Graphic2D_DrawPixelNormal(xp + r, yp, color);
  UB_Graphic2D_DrawPixelNormal(xp - r, yp, color);

  while(x < y) {
    if(f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;

    UB_Graphic2D_DrawPixelNormal(xp + x, yp + y, color);
    UB_Graphic2D_DrawPixelNormal(xp - x, yp + y, color);
    UB_Graphic2D_DrawPixelNormal(xp + x, yp - y, color);
    UB_Graphic2D_DrawPixelNormal(xp - x, yp - y, color);
    UB_Graphic2D_DrawPixelNormal(xp + y, yp + x, color);
    UB_Graphic2D_DrawPixelNormal(xp - y, yp + x, color);
    UB_Graphic2D_DrawPixelNormal(xp + y, yp - x, color);
    UB_Graphic2D_DrawPixelNormal(xp - y, yp - x, color);
  }
}


//--------------------------------------------------------------
// Zeichnet einen gefuellten Kreis mit einer Farbe (per DMA2D)
// Mittelpunkt   = xp,yp [x=0...LCD_MAXX, y=0...LCD_MAXY]
// Radius = r
//--------------------------------------------------------------
void UB_Graphic2D_DrawFullCircleDMA(uint16_t xp, uint16_t yp, uint16_t r, uint32_t color)
{
  int32_t  D;    
  uint32_t  CurX;
  uint32_t  CurY; 
  int16_t x,y,l;
  LCD_DIR_t m=LCD_DIR_VERTICAL;
  
  // check auf Limit
  if(xp>=LCD_MAXX) xp=LCD_MAXX-1;
  if(yp>=LCD_MAXY) yp=LCD_MAXY-1;
  if(r==0) return;

  if(LCD_DISPLAY_MODE==LANDSCAPE) {
    // richtung drehen
    m=LCD_DIR_HORIZONTAL;
  }

  D = 3 - (r << 1);
  
  CurX = 0;
  CurY = r;
  
  while (CurX <= CurY) {
    if(CurY > 0) {
      x=xp - CurX;
      y=yp - CurY;
      l=2*CurY;
      if(y<0) {
       	l+=y;
       	y=0;
      }
      UB_Graphic2D_DrawStraightDMA(x, y, l, m, color);
      x=xp + CurX;
      y=yp - CurY;
      l=2*CurY;
      if(y<0) {
       	l+=y;
       	y=0;
      }
      UB_Graphic2D_DrawStraightDMA(x,y ,l , m, color);
    }
    
    if(CurX > 0) {
      x=xp - CurY;
      y=yp - CurX;
      l=2*CurX;
      if(y<0) {
        l+=y;
        y=0;
      }
      UB_Graphic2D_DrawStraightDMA(x,y , l, m, color);
      x=xp + CurY;
      y=yp - CurX;
      l=2*CurX;
      if(y<0) {
        l+=y;
        y=0;
      }
      UB_Graphic2D_DrawStraightDMA(x,y , l, m, color);
    }

    if (D < 0) { 
      D += (CurX << 2) + 6;
    }
    else {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  
  UB_Graphic2D_DrawCircleNormal(xp, yp, r, color);
}



//--------------------------------------------------------------
// copied from an image (from the flash)
// into the graphics RAM (via DMA2D)
// -> Image must be passed with & operator
// If there are errors with the coordinates, nothing is drawn
//--------------------------------------------------------------
void UB_Graphic2D_CopyImgDMA(UB_Image *img, DMA2D_Koord koord)
{
  uint32_t  dest_address = 0;
  uint32_t  source_address = 0;
  uint32_t offset;
  uint32_t  picture_width;
  uint32_t  picture_height;
  HAL_DMA2D_DeInit(&hdma2d);
  // Destination address in the display RAM
  dest_address = LCD_CurrentFrameBuffer + 2*(LCD_MAXX*koord.dest_yp + koord.dest_xp);

  picture_width=img->width;
  picture_height=img->height;

  // Check limits
  if(koord.source_w==0) return;
  if(koord.source_h==0) return;
  if(koord.source_xp+koord.source_w>picture_width) return;
  if(koord.source_yp+koord.source_h>picture_height) return;
  if(koord.dest_xp+koord.source_w>LCD_MAXX) return;
  if(koord.dest_yp+koord.source_h>LCD_MAXY) return;

  // Source address from the image
  offset=(picture_width*koord.source_yp + koord.source_xp);
  source_address  = (uint32_t)&img->table[offset];


  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = LCD_MAXX-koord.source_w;
  // Foreground
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = picture_width-koord.source_w;
  // Background
  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;
  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);
  HAL_DMA2D_Start(&hdma2d, source_address, dest_address, koord.source_w, koord.source_h);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}


//--------------------------------------------------------------
// copies a complete screen (240 x 320 pixels)
// from one address from the SD-RAM to another (via DMA2D)
//
// Source address = LCD_MenuFrameBuffer
// Target address = LCD_CurrentFrameBuffer
//--------------------------------------------------------------
void UB_Graphic2D_Copy1DMA(void){

	  HAL_DMA2D_DeInit(&hdma2d);
	  hdma2d.Instance = DMA2D;
	  hdma2d.Init.Mode = DMA2D_M2M;
	  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
	  hdma2d.Init.OutputOffset = 0;

	  // Foreground
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = 0;
	  HAL_DMA2D_Init(&hdma2d);
	  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
	  HAL_DMA2D_Start(&hdma2d, LCD_MenuFrameBuffer, LCD_CurrentFrameBuffer, LCD_MAXX, LCD_MAXY);
	  HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}


//--------------------------------------------------------------
// Creates a new screen from two complete screens (240 x 320 pixels)
// from two addresses from the SD-RAM using (BLEND effect)
// and copies it to a target address in the SD-RAM (using DMA2D)
//
// Source address-1 = LCD_MenuFrameBuffer
// Source address-2 = LCD_ADCFrameBuffer
// Target address = LCD_CurrentFrameBuffer
//
// transparency [0 ... 255] specifies the transparency
//--------------------------------------------------------------
void UB_Graphic2D_Copy2DMA(uint32_t tranzparenz){

	HAL_DMA2D_DeInit(&hdma2d);
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_M2M_BLEND;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
	hdma2d.Init.OutputOffset = 0;

	// Foreground
	hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_COMBINE_ALPHA;
	hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;
	hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha = 255;

	// Background
	hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;
	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);
	HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
	HAL_DMA2D_BlendingStart(&hdma2d, LCD_MenuFrameBuffer, LCD_ADCFrameBuffer, LCD_CurrentFrameBuffer, LCD_MAXX, LCD_MAXY);
	HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}



//--------------------------------------------------------------
// interne Funktion
// Signum funktion
//  Return_data
//    1 ,wenn x > 0
//    0 ,wenn x = 0
//   -1 ,wenn x < 0
//--------------------------------------------------------------
int16_t P_Graphic2D_sgn(int16_t x)
{
  return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}
