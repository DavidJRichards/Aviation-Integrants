#include <math.h>             // initial build using floating point, intent to rewrite with fixed point
#include <SPI.h>
#include <TFT_eSPI.h>         // Hardware-specific library


extern void compass_init(void);
extern void draw_needle(float);