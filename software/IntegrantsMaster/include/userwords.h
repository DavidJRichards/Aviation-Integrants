/*
*  Copyright PeterForth 2022 
*  use freely according MIT license
*  redistribution only possible with
*  acknowledgment to the author 
*  more information of the project https://esp32.forth2020.org
*  tft libraries of Bodmer https://github.com/Bodmer/TFT_eSPI
*  download ESP32forth from Brad Nelson 
*  https://esp32forth.appspot.com/ESP32forth.html
*/

#define nn0 ((uint16_t *) tos)
#define nn1 (*(uint16_t **) &n1)

#include <SPI.h>
#include <TFT_eSPI.h> 
//#include <XPT2046_Touchscreen.h>


extern TFT_eSPI gfx;  // screen declared in ...menu.cpp
#define tft gfx       // this file expects tft instead


//TFT_eSPI tft = TFT_eSPI();
//SPIClass touchSpi = SPIClass(HSPI);
//XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_HEIGHT 16 // Height of text to be printed and scrolled
#define BOT_FIXED_AREA 0 // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define TOP_FIXED_AREA 16 // Number of lines in top fixed area (lines counted from top of screen)
#define YMAX 320 // Bottom of screen area

// The initial y coordinate of the top of the scrolling area
uint16_t yStart = TOP_FIXED_AREA;
// yArea must be a integral multiple of TEXT_HEIGHT
uint16_t yArea = YMAX-TOP_FIXED_AREA-BOT_FIXED_AREA;
// The initial y coordinate of the top of the bottom text line
uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;

// Keep track of the drawing x coordinate
uint16_t xPos = 0;

// For the byte we read from the serial port
byte data = 0;

// A few test variables used during debugging
bool change_colour = 1;
bool selected = 1;

// We have to blank the top line each time the display is scrolled, but this takes up to 13 milliseconds
// for a full width line, meanwhile the serial buffer may be filling... and overflowing
// We can speed up scrolling of short text lines by just blanking the character we drew
int blank[19]; // We keep all the strings pixel lengths to optimise the speed of the top line blanking

void scrollAddress(uint16_t vsp);
void setupScrollArea(uint16_t tfa, uint16_t bfa);
int scroll_line();

void setuptouch(void) {
  uint16_t calData[5] = { 235, 3234, 504, 3066, 0 };
  tft.setTouch(calData);
}
void setuptftdemo(void) {
  uint16_t calData[5] = { 235, 3234, 504, 3066, 0 };
  tft.setTouch(calData);  
  
  //tft.init();
  tft.fillScreen(TFT_BLACK);  
  
  tft.setCursor(20, 10, 4);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.println("White Text\n");
  tft.println("Next White Text");
  
  tft.setCursor(10, 100);
  tft.setTextFont(2);
  tft.setTextColor(TFT_RED, TFT_WHITE);
  tft.println("Red Text, White Background");
  
  tft.setCursor(10, 140, 4);
  tft.setTextColor(TFT_GREEN);
  tft.println("Green text");
  
  tft.setCursor(70, 180);
  tft.setTextColor(TFT_BLUE, TFT_YELLOW);
  tft.println("Blue text");
 
  tft.setCursor(50, 220);
  tft.setTextFont(4);
  tft.setTextColor(TFT_YELLOW);
  tft.println("2020-06-16");
 
  tft.setCursor(50, 260);
  tft.setTextFont(7);
  tft.setTextColor(TFT_PINK);
  tft.println("20:35");
}

// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
int scroll_line() {
  int yTemp = yStart; // Store the old yStart, this is where we draw the next line
  // Use the record of line lengths to optimise the rectangle size we need to erase the top line
  tft.fillRect(0,yStart,blank[(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT],TEXT_HEIGHT, TFT_BLACK);

  // Change the top of the scroll area
  yStart+=TEXT_HEIGHT;
  // The value must wrap around as the screen memory is a circular buffer
  if (yStart >= YMAX - BOT_FIXED_AREA) yStart = TOP_FIXED_AREA + (yStart - YMAX + BOT_FIXED_AREA);
  // Now we can scroll the display
  scrollAddress(yStart);
  return  yTemp;
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void setupScrollArea(uint16_t tfa, uint16_t bfa) {
//  tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  tft.writedata(tfa >> 8);           // Top Fixed Area line count
  tft.writedata(tfa);
  tft.writedata((YMAX-tfa-bfa)>>8);  // Vertical Scrolling Area line count
  tft.writedata(YMAX-tfa-bfa);
  tft.writedata(bfa >> 8);           // Bottom Fixed Area line count
  tft.writedata(bfa);
}

// ##############################################################################################
// Setup the vertical scrolling start address pointer
// ##############################################################################################
void scrollAddress(uint16_t vsp) {
//  tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling pointer
  tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling pointer
  tft.writedata(vsp>>8);
  tft.writedata(vsp);
}

void setuptft() {
  // Setup the TFT display
  //touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  //ts.begin(touchSpi);

  tft.init();
  tft.setRotation(0); // Must be setRotation(0) for this sketch to work correctly
  tft.fillScreen(TFT_BLACK);

#if 1
  // boost gamma value, increases contrast in dark region.
  tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
  tft.writedata(2);
  delay(120);
  tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
  tft.writedata(1);   
  //SPI_setup();
#endif

  // Setup baud rate and draw top banner
//  Serial.begin(5340);
  
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.fillRect(0,0,240,16, TFT_BLUE);
  tft.drawCentreString(" Serial Terminal - 9600 baud ",120,0,2);

  // Change colour for scrolling zone text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Setup scroll area
  setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);

  // Zero the array
  for (byte i = 0; i<18; i++) blank[i]=0;
}


void tftWrite(char data)
{

  if (data == '\r' || xPos>231) {
    xPos = 0;
    yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
  }
  if (data > 31 && data < 128) {
    xPos += tft.drawChar(data,xPos,yDraw,2);
    blank[(18+(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT)%19]=xPos; // Keep a record of line lengths
  }
 
}

void tftType(char *string, int count)
{
  
  while ( count-- > 0)
  {
    data = *string++;
    if (data == '\r' || xPos>231) {
      xPos = 0;
      yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
    }
    if (data > 31 && data < 128) {
      xPos += tft.drawChar(data,xPos,yDraw,2);
      blank[(18+(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT)%19]=xPos; // Keep a record of line lengths
    }
  }

}

void looptft(void) {
  //  These lines change the text colour when the serial buffer is emptied
  //  These are test lines to see if we may be losing characters
  //  Also uncomment the change_colour line below to try them
  //
  //  if (change_colour){
  //  change_colour = 0;
  //  if (selected == 1) {tft.setTextColor(TFT_CYAN, TFT_BLACK); selected = 0;}
  //  else {tft.setTextColor(TFT_MAGENTA, TFT_BLACK); selected = 1;}
  //}

  while (Serial2.available()) {
    data = Serial2.read();
    // If it is a CR or we are near end of line then scroll one line
    if (data == '\r' || xPos>231) {
      xPos = 0;
      yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
    }
    if (data > 31 && data < 128) {
      xPos += tft.drawChar(data,xPos,yDraw,2);
      blank[(18+(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT)%19]=xPos; // Keep a record of line lengths
    }
    //change_colour = 1; // Line to indicate buffer is being emptied
  }
}

extern float get_menuindex(int idx);
extern int values[16];
extern float fvalues[16];
#include "src_menu.h"
extern int encoder_pos;


#define USER_WORDS \
  Y(encoder, DUP; tos = (cell_t) & encoder_pos) \
  Y(eepromload, menuMgr.load();) \
  Y(menuidx, *++fp = get_menuindex(n0); DROP) \
  Y(putvalue, values[n0]=n1; DROPn(2)) \
  Y(getvalue, n0=values[n0];) \
  Y(fgetvalue, *++fp = fvalues[n0]; DROP) \
  Y(fputvalue, fvalues[n0] = *fp--; DROP) \
  \
  Y(tftsetup, setuptft();) \
  Y(tfttype, tftType(c1, n0); DROP) \
  Y(tftwrite, tftWrite(n0); DROP) \
  Y(tftloop, looptft(); ) \
  Y(tftdemo, setuptftdemo();) \
  Y(tftinit, tft.init();) \
  Y(tftcls, tft.fillScreen(TFT_BLACK);) \
  Y(tftcursor, tft.setCursor(n1, n0); DROPn(2)) \
  Y(tftcursorink, tft.setCursor(n2,n1, n0); DROPn(2)) \
  Y(tftTextFont, tft.setTextFont(n0); DROP) \
  Y(tftTextColor, tft.setTextColor(n1,n0); DROPn(2)) \
  Y(tftprintln, tft.println(c0); DROP) \
  Y(tftprint, tft.print(c0); DROP) \
  Y(tftNum, tft.print(n0); DROP) \
  Y(tftNumln, tft.println(n0); DROP) \
  Y(tftCircle, tft.drawCircle(n3,n2,n1,n0); DROPn(3)) \
  Y(tftPixel, tft.drawPixel(n2,n1,n0); DROPn(3)) \
  Y(tftLine, tft.drawLine(n4,n3,n2,n1,n0); DROPn(5)) \
  Y(tftfillRect, tft.fillRect(n4,n3,n2,n1,n0); DROPn(5)) \
  Y(tftfillRRect, tft.fillRoundRect(n5,n4,n3,n2,n1,n0); DROPn(6)) \
  Y(tftRect, tft.drawRect(n4,n3,n2,n1,n0); DROPn(5)) \
  Y(tftHLine, tft.drawFastHLine(n3,n2,n1,n0); DROPn(4)) \
  Y(tftVLine, tft.drawFastVLine(n3,n2,n1,n0); DROPn(4)) \
  Y(tftfillCircle, tft.fillCircle(n3,n2,n1,n0); DROPn(4)) \
  Y(tftRotation,  tft.setRotation(n0); DROP) \
  Y(tftfillscreen, tft.fillScreen(n0); DROP) \
  Y(tftEllipse,  tft.drawEllipse(n4,n3,n2,n1,n0); DROPn(5)) \
  Y(tftfillEllipse,  tft.fillEllipse(n4,n3,n2,n1,n0); DROPn(5)) \
  Y(tftTriangle, tft.drawTriangle(n6,n5,n4,n3,n2,n1,n0); DROPn(7)) \
  Y(tftfillTriangle, tft.fillTriangle(n6,n5,n4,n3,n2,n1,n0); DROPn(7)) \
  
  //Y(tfttouch, ts.getTouch(nn1, nn0); DROPn(2)) \
  //Y(tftinittouch, setuptouch; DROP)

 
