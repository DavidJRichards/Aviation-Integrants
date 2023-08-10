#include "Compass.h"

TFT_eSPI tft = TFT_eSPI();      // Invoke custom library

#define TFT_GREY 0x5AEB
float sx = 0, sy = 1;       // Saved H, M, S x & y multipliers
uint16_t osx=120, osy=120;  // Saved H, M, S x & y coords
float sxb = 0, syb = 1;       // Saved H, M, S x & y multipliers
uint16_t osxb=120, osyb=120;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint8_t ss=0;

void compass_init(void){

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK, true);  // Adding a background colour erases previous text automatically

  // Draw clock face
  tft.fillCircle(120, 120, 118, TFT_GREY);
  tft.fillCircle(120, 120, 110, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 5) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*110+120;
    yy0 = sy*110+120;
    x1 = sx*108+120;
    yy1 = sy*108+120;

    tft.drawLine(x0, yy0, x1, yy1, TFT_WHITE);
  }
  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*114+120;
    yy0 = sy*114+120;
    x1 = sx*100+120;
    yy1 = sy*100+120;

    tft.drawLine(x0, yy0, x1, yy1, TFT_WHITE);
  }


  // Draw 60 dots
  for(int i = 0; i<360; i+= 10) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*102+120;
    yy0 = sy*102+120;
     // Draw minute markers
    tft.drawPixel(x0, yy0, TFT_WHITE);

    // Draw main quadrant dots
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
  }

//  tft.fillCircle(120, 121, 3, TFT_RED);
  Serial.println("compass");
}


void draw_needle(float sdeg, float tdeg)
{
    char buf[8];
    // Pre-compute hand degrees, x & y coords for a fast screen update
    sxb = cos((tdeg-90)*0.0174532925);    
    syb = sin((tdeg-90)*0.0174532925);

    // un draw old hands
    tft.drawLine(osxb-1, osyb, 120-1, 121, TFT_BLACK);
    tft.drawLine(osxb+1, osyb, 120+1, 121, TFT_BLACK);

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sx = cos((sdeg-90)*0.0174532925);    
    sy = sin((sdeg-90)*0.0174532925);

    // un draw old hands
    tft.drawLine(osx-1, osy, 120-1, 121, TFT_BLACK);
    tft.drawLine(osx+1, osy, 120+1, 121, TFT_BLACK);

    // then draw new hands
    osxb = sxb*99+121;    
    osyb = syb*99+121;
    tft.drawLine(osxb-1, osyb, 120-1, 121, TFT_YELLOW);
    tft.drawLine(osxb+1, osyb, 120+1, 121, TFT_YELLOW);

    // then draw new hands
    osx = sx*99+121;    
    osy = sy*99+121;
    tft.drawLine(osx-1, osy, 120-1, 121, TFT_RED);
    tft.drawLine(osx+1, osy, 120+1, 121, TFT_RED);

    // centre dot
    tft.fillCircle(120, 121, 3, TFT_RED);

    // bottom text
    sprintf(buf," %05.1f ",sdeg);
    tft.drawCentreString(buf,120,160,4);

    // top text
    sprintf(buf," %05.1f ",tdeg);
    tft.drawCentreString(buf,120,60,4);



}
