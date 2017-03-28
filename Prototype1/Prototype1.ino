#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include "Fonts/FreeMono24pt7b.h"

#include <Wire.h>
#include "Adafruit_Trellis.h"

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _rst 9
#define _dc 8

// Use hardware SPI for TFT
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

//Trellis
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);

typedef float (*Operation)(void);
char* charmap[] = {"7","8","9","/",
                  "4","5","6","x",
                  "1","2","3","-",
                  "0",".","=","+"};
int digitmap[] = {7,8,9,0,
                  4,5,6,0,
                  1,2,3,0,
                  0};
float a, b;
Operation op;

char lastshown[15];

void setup() {
    Serial.begin(9600);
    while (!Serial);

    //TFT
    tft.begin();
    tft.setRotation(1);
    tft.setFont(&FreeMono24pt7b);
    tft.fillScreen(ILI9340_BLACK);

    //Trellis
    pinMode(2, INPUT_PULLUP);
//    attachInterrupt(digitalPinToInterrupt(2), checkButtons, FALLING);
    trellis.begin(0x70);
    // light up all the LEDs in order
    for (uint8_t i=0; i<16; i++) {
      trellis.setLED(i);
      trellis.writeDisplay();    
      delay(50);
    }
    // then turn them off
    for (uint8_t i=0; i<16; i++) {
      trellis.clrLED(i);
      trellis.writeDisplay();    
      delay(50);
    }
}

void loop(void) {
    delay(30);
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<16; i++) {
//      if it was pressed, turn it on
        if (trellis.justPressed(i)) {
          trellis.setLED(i);
          processClick(i);
        } 
//      if it was released, turn it off
        if (trellis.justReleased(i)) {
          trellis.clrLED(i);
        }
      }
//    tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
}

void processClick(int key){
    Operation newOp;
    switch (key) {
        case 3:  newOp = &divide; break;
        case 7:  newOp = &times;  break;
        case 11: newOp = &minus;  break;
        case 14: newOp = &eval;   break;
        case 15: newOp = &plus;   break;
        default: digit(key);
    }
    if(newOp){
        if(op){
          int res=(op)();
          a=res;
          show(res);
        } else {
          a=b;
        }
        op = newOp;
        b=0;
    }
}

float divide(){ return a/b; }
float times(){ return a*b; }
float minus(){ return a-b; }
float plus(){ return a+b; }
float eval(){ return a; }  //unimplemented

void digit(int key){
    b=b*10+digitmap[key];
    show(b);
}

void show(float num){
    int16_t  x1, y1;
    uint16_t w, h;
    tft.setCursor(50, 50);
    tft.getTextBounds(lastshown, 50, 50, &x1, &y1, &w, &h);
    tft.fillRect(x1, y1, w, h, ILI9340_BLACK);
    tft.println(num);
    
    dtostrf(num,1,2,lastshown);
}

void addChar(char* newChar) {
    int16_t  x1, y1, curx=50, cury=50;
    uint16_t w, h;
    tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
    
    tft.getTextBounds(newChar, tft.getCursorX(), tft.getCursorY(), &x1, &y1, &w, &h);
    tft.fillRect(x1, y1, w, h, ILI9340_BLACK);
    tft.print(newChar);
}
