/*
[nwazet Open Source Software & Open Source Hardware
Authors: Fabien Royer
Software License Agreement (BSD License)

Copyright (c) 2010, Nwazet, LLC. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nwazet, LLC. nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
* The names '[nwazet', 'nwazet', the ASCII hazelnut in the [nwazet logo and the color of the logo are Trademarks of nwazet, LLC. and cannot be used to endorse or promote products derived from this software or any hardware designs without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Parts of the [nwazet source code is derived from the MicroBuilder codebase.
The MicroBuilder license is provided below and is alos located under .\MicroBuilder\License.txt
*/

#include <xc.h>

#include "ILI9341.h"
#include "TimeDelay.h"
#include "driver/gpio.h"

enum {
  MemoryAccessControlNormalOrder,
  MemoryAccessControlReverseOrder
} MemoryAccessControlRefreshOrder;

enum {
  MemoryAccessControlColorOrderBGR,
  MemoryAccessControlColorOrderRGB
} MemoryAccessControlColorOrder;

enum {
    ColumnAddressSet = 0x2a,
    PageAddressSet = 0x2b,
    MemoryWrite = 0x2c,
    MemoryAccessControl = 0x36,
    WriteDisplayBrightness = 0x51
} ILI9341Register;

static lcdProperties_t  lcdProperties = { 240, 320, true, true, true };
static lcdOrientation_t lcdOrientation = LCD_ORIENTATION_PORTRAIT;

static unsigned char lcdPortraitConfig = 0;
static unsigned char lcdLandscapeConfig = 0;

void lcdWriteDataB(uint8_t data);

#define GPIO_LOW(port, pin)     (port) &= ~(0x1u << (pin))
#define GPIO_HIGH(port, pin)    (port) |=  (0x1u << (pin))

#define CS_LOW()                TFT_LCD_CS_PORT &= ~(0x1u << TFT_LCD_CS_PIN)
#define CS_HIGH()               TFT_LCD_CS_PORT |=  (0x1u << TFT_LCD_CS_PIN)

void lcdInit(void)
{
  lcdPortraitConfig = lcdBuildMemoryAccessControlConfig(
                                                    MemoryAccessControlNormalOrder, // rowAddressOrder
                                                    MemoryAccessControlReverseOrder, // columnAddressOrder
                                                    MemoryAccessControlNormalOrder, // rowColumnExchange
                                                    MemoryAccessControlNormalOrder, // verticalRefreshOrder
                                                    MemoryAccessControlColorOrderRGB, // colorOrder
                                                    MemoryAccessControlNormalOrder); // horizontalRefreshOrder

  lcdLandscapeConfig = lcdBuildMemoryAccessControlConfig(
                                                    MemoryAccessControlNormalOrder, // rowAddressOrder
                                                    MemoryAccessControlNormalOrder, // columnAddressOrder
                                                    MemoryAccessControlReverseOrder, // rowColumnExchange
                                                    MemoryAccessControlReverseOrder, // verticalRefreshOrder
                                                    MemoryAccessControlColorOrderRGB, // colorOrder
                                                    MemoryAccessControlReverseOrder); // horizontalRefreshOrder
  DelayMs(120);

  lcdReset();

  GPIO_LOW(TFT_LCD_CS_PORT, TFT_LCD_CS_PIN);

  lcdWriteCommand(0x01);
  DelayMs(10);
  lcdWriteCommand(0x28);

  lcdWriteCommand(0xcf);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x83);
  lcdWriteDataB(0x30);

  lcdWriteCommand(0xed);
  lcdWriteDataB(0x64);
  lcdWriteDataB(0x03);
  lcdWriteDataB(0x12);
  lcdWriteDataB(0x81);

  lcdWriteCommand(0xe8);
  lcdWriteDataB(0x85);
  lcdWriteDataB(0x01);
  lcdWriteDataB(0x79);

  lcdWriteCommand(0xcb);
  lcdWriteDataB(0x39);
  lcdWriteDataB(0x2c);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x34);
  lcdWriteDataB(0x02);

  lcdWriteCommand(0xf7);
  lcdWriteDataB(0x20);

  lcdWriteCommand(0xea);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x00);

  lcdWriteCommand(0xc0);
  lcdWriteDataB(0x26);

  lcdWriteCommand(0xc1);
  lcdWriteDataB(0x11);

  lcdWriteCommand(0xc5);
  lcdWriteDataB(0x35);
  lcdWriteDataB(0x3e);

  lcdWriteCommand(0xc7);
  lcdWriteDataB(0xbe);

  lcdWriteCommand(MemoryAccessControl);
  lcdWriteDataB(lcdPortraitConfig);

  lcdWriteCommand(0x3a);
  lcdWriteDataB(0x55);

  lcdWriteCommand(0xb1);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x1B);

  lcdWriteCommand(0xf2);
  lcdWriteDataB(0x08);

  lcdWriteCommand(0x26);
  lcdWriteDataB(0x01);

  lcdWriteCommand(0xe0);
  lcdWriteDataB(0x1f);
  lcdWriteDataB(0x1a);
  lcdWriteDataB(0x18);
  lcdWriteDataB(0x0a);
  lcdWriteDataB(0x0f);
  lcdWriteDataB(0x06);
  lcdWriteDataB(0x45);
  lcdWriteDataB(0x87);
  lcdWriteDataB(0x32);
  lcdWriteDataB(0x0a);
  lcdWriteDataB(0x07);
  lcdWriteDataB(0x02);
  lcdWriteDataB(0x07);
  lcdWriteDataB(0x05);
  lcdWriteDataB(0x00);

  lcdWriteCommand(0xe1);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x25);
  lcdWriteDataB(0x27);
  lcdWriteDataB(0x05);
  lcdWriteDataB(0x10);
  lcdWriteDataB(0x09);
  lcdWriteDataB(0x3a);
  lcdWriteDataB(0x78);
  lcdWriteDataB(0x4d);
  lcdWriteDataB(0x05);
  lcdWriteDataB(0x18);
  lcdWriteDataB(0x0d);
  lcdWriteDataB(0x38);
  lcdWriteDataB(0x3a);
  lcdWriteDataB(0x1f);

  lcdWriteCommand(0x2a);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0xEF);

  lcdWriteCommand(0x2b);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x00);
  lcdWriteDataB(0x01);
  lcdWriteDataB(0x3F);

  lcdWriteCommand(0xb7);
  lcdWriteDataB(0x07);

  lcdWriteCommand(0xb6);
  lcdWriteDataB(0x0a);
  lcdWriteDataB(0x82);
  lcdWriteDataB(0x27);
  lcdWriteDataB(0x00);

  lcdWriteCommand(0x11);
  DelayMs(100);
  lcdWriteCommand(0x29);
  DelayMs(100);
  lcdWriteCommand(MemoryWrite);

}
void lcdSetCursor(unsigned short x, unsigned short y) {
  lcdSetWindow(x, x, y, y);
}
void lcdSetWindow(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
  lcdWriteCommand(ColumnAddressSet);
  lcdWriteDataB((x0 >> 8) & 0xFF);
  lcdWriteDataB(x0 & 0xFF);
  lcdWriteDataB((y0 >> 8) & 0xFF);
  lcdWriteDataB(y0 & 0xFF);
  lcdWriteCommand(PageAddressSet);
  lcdWriteDataB((x1 >> 8) & 0xFF);
  lcdWriteDataB(x1 & 0xFF);
  lcdWriteDataB((y1 >> 8) & 0xFF);
  lcdWriteDataB(y1 & 0xFF);
  lcdWriteCommand(MemoryWrite);
}
void lcdHome(void){
  lcdSetWindow(0, lcdProperties.width-1, 0, lcdProperties.height-1);
}
void lcdTest(void) {
  int x;
  int y;
  lcdSetOrientation(LCD_ORIENTATION_PORTRAIT);

  for (y = 0; y < 320; y++) {
    for (x = 0; x < 240; x++) {
      if (y > 279) lcdWriteData(COLOR_WHITE);
      else if (y > 239) lcdWriteData(COLOR_BLUE);
      else if (y > 199) lcdWriteData(COLOR_GREEN);
      else if (y > 159) lcdWriteData(COLOR_CYAN);
      else if (y > 119) lcdWriteData(COLOR_RED);
      else if (y > 79) lcdWriteData(COLOR_MAGENTA);
      else if (y > 39) lcdWriteData(COLOR_YELLOW);
      else lcdWriteData(COLOR_BLACK);
    }
  }
}
void lcdFillRGB(uint16_t color) {
  lcdSetWindow(0,lcdProperties.width-1,0,lcdProperties.height-1);
  int dimensions = lcdProperties.width * lcdProperties.height;
  while(dimensions--){
    lcdWriteData(color);
  }
}
void lcdDrawPixel(uint16_t x, uint16_t y, uint16_t color) {
  lcdSetWindow(x, x, y, y);
  lcdWriteData(color);
}
void lcdDrawPixels(uint16_t x, uint16_t y, uint16_t *data, uint32_t dataLength) {
  uint32_t i = 0;
  lcdSetWindow(x,lcdProperties.width-1,y,lcdProperties.height-1);
  do {
    lcdWriteData(data[i++]);
  } while (i<dataLength);
}
void lcdDrawHLine(uint16_t x0, uint16_t x1, uint16_t y, uint16_t color) {
  // Allows for slightly better performance than setting individual pixels
  uint16_t x, pixels;
  if (x1 < x0) {
    // Switch x1 and x0
    x = x1;
    x1 = x0;
    x0 = x;
  }
  // Check limits
  if (x1 >= lcdProperties.width) {
    x1 = lcdProperties.width - 1;
  }
  if (x0 >= lcdProperties.width) {
    x0 = lcdProperties.width - 1;
  }
  lcdSetWindow(x0, x1, y, y);
  for (pixels = 0; pixels < x1 - x0 + 1; pixels++) {
    lcdWriteData(color);
  }
}
void lcdDrawVLine(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color) {
  int line;
  
  if (y1 < y0) {
    uint16_t y = y1;
    y1 = y0;
    y0 = y;
  }

  if (x >= lcdProperties.width) {
    x = lcdProperties.width - 1;
  }

  if (y0 >= lcdProperties.height) {
    y0 = lcdProperties.height - 1;
  }
  if (y1 >= lcdProperties.height) {
    y1 = lcdProperties.height - 1;
  }

  for(line = y0; line <= y1; line++){
    lcdDrawPixel(x,line,color);
  }
}
uint16_t lcdGetPixel(uint16_t x, uint16_t y) {
  return 0;
}
void lcdSetOrientation(lcdOrientation_t value) {
  lcdOrientation = value;
  lcdWriteCommand(MemoryAccessControl);
  if (lcdOrientation == LCD_ORIENTATION_PORTRAIT) {
	  lcdWriteDataB(lcdPortraitConfig);
      lcdProperties.width = 240;
      lcdProperties.height = 320;
  } else {
	  lcdWriteDataB(lcdLandscapeConfig);
      lcdProperties.width = 320;
      lcdProperties.height = 240;
  }
  lcdWriteCommand(MemoryWrite);
  lcdSetWindow(0, lcdProperties.width-1, 0, lcdProperties.height-1);
}
lcdOrientation_t lcdGetOrientation(void) {
  return lcdOrientation;
}
uint16_t lcdGetWidth(void) {
  return lcdProperties.width;
}
uint16_t lcdGetHeight(void) {
  return lcdProperties.height;
}
void lcdScroll(int16_t pixels, uint16_t fillColor) {
}
uint16_t lcdGetControllerID(void) {
  return 0x9341;
}
lcdProperties_t lcdGetProperties(void) {
  return lcdProperties;
}
void lcdReset(void){
  GPIO_HIGH(TFT_LCD_RD_PORT, TFT_LCD_RD_PIN);
  GPIO_HIGH(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
  GPIO_HIGH(TFT_LCD_CS_PORT, TFT_LCD_CS_PIN);
  GPIO_HIGH(TFT_LCD_RESET_PORT, TFT_LCD_RESET_PIN);

  DelayMs(1);
  GPIO_LOW(TFT_LCD_RESET_PORT, TFT_LCD_RESET_PIN);

  DelayMs(5);
  GPIO_HIGH(TFT_LCD_RESET_PORT, TFT_LCD_RESET_PIN);
  DelayMs(120);
}

static inline void writeByte(uint8_t byte) {
    TFT_LCD_DATA_PORT = byte;
}

void lcdWriteB(uint8_t data) {
  writeByte(data);
  GPIO_LOW(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
  GPIO_HIGH(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
}

// Write an 8 bit command to the IC driver
void lcdWriteCommand(unsigned char command){
  GPIO_LOW(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN);
  lcdWriteB(command);
  GPIO_HIGH(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN);
}

void lcdWriteDataB(uint8_t data){
  GPIO_HIGH(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN);
  lcdWriteB(data);
}

// Write an 16 bit data word to the IC driver
void lcdWriteData(unsigned short data){
  GPIO_HIGH(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN);
  lcdWrite(data);
}

// Places a 16 bit word on the port connected to the IC driver and pulses the write pin
void lcdWrite(unsigned short data){
  writeByte((data >> 8) & 0xFF);
  GPIO_LOW(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
  GPIO_HIGH(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
//delay(1);
  writeByte((uint8_t)data);
  GPIO_LOW(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
  GPIO_HIGH(TFT_LCD_WR_PORT, TFT_LCD_WR_PIN);
}

unsigned char lcdBuildMemoryAccessControlConfig(
                        bool rowAddressOrder,
                        bool columnAddressOrder,
                        bool rowColumnExchange,
                        bool verticalRefreshOrder,
                        bool colorOrder,
                        bool horizontalRefreshOrder){
  unsigned char value = 0;
  if(horizontalRefreshOrder) value |= 0x0004;
  if(colorOrder) value |= 0x0008;
  if(verticalRefreshOrder) value |= 0x0010;
  if(rowColumnExchange) value |= 0x0020;
  if(columnAddressOrder) value |= 0x0040;
  if(rowAddressOrder) value |= 0x0080;
  return value;
}
