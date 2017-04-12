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

#define MAX_DISPLAY_DECIMALS 5
typedef double (*Operation)(void);
char* charmap[] = {"7","8","9","/",
                  "4","5","6","x",
                  "1","2","3","-",
                  "0",".","=","+"};
int digitmap[] = {7,8,9,0,
                  4,5,6,0,
                  1,2,3,0,
                  0};
double a, b;
Operation op;
int decimalPlace;

unsigned long eqDown;

int16_t  lastX, lastY;
uint16_t lastW, lastH;
char tmpStr[15];

void setup() {
    Serial.begin(9600);
    while (!Serial);

    //TFT
    tft.begin();
    tft.setRotation(1);
    tft.setFont(&FreeMono24pt7b);
    tft.setTextColor(ILI9340_WHITE);
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
      if(trellis.justPressed(14)){
        eqDown = millis();
      }
//    tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
    if(trellis.isKeyPressed(14) && millis()-eqDown>1000){
        trellis.clrLED(14);
        trellis.writeDisplay();
        cls();
    }
}

void processClick(int key){
    Operation newOp;
    switch (key) {
        case 3:  newOp = &divide; break;
        case 7:  newOp = &times;  break;
        case 11: newOp = &minus;  break;
        case 13: decimalPlace = 1;  break;
        case 14: newOp = &eval;   break;
        case 15: newOp = &plus;   break;
        default: digit(key);
    }
    if(newOp){
        if(op){
          double res=(op)();
          a=res;
          
          //convert result for display
          dtostrf(res, 1, MAX_DISPLAY_DECIMALS, tmpStr);
          boolean hasDecimal = false, consec = true;
          uint8_t trimSpot = strlen(tmpStr);
          for(int i = strlen(tmpStr)-1; i >= 0; i--){
              if(tmpStr[i] == '.'){
                hasDecimal = true;
              }
              if(consec && tmpStr[i] == '0'){
                trimSpot = i;
              } else {
                consec = false;
              }
          }
          // Only trim if there's a decimal point in the number
          if(hasDecimal){
            if(tmpStr[trimSpot-1] == '.'){
              // If the zeroes after the decimal edge right up against the decimal, trim off the decimal point too
              tmpStr[trimSpot-1] = 0;
            } else { 
              // Otherwise just trim the zeroes
              tmpStr[trimSpot] = 0;
            }
          }
          show(tmpStr);
        } else {
          a=b;
        }
        op = newOp;
        b=0;
        decimalPlace = 0;
    }
}

double divide(){ return a/b; }
double times(){ return a*b; }
double minus(){ return a-b; }
double plus(){ return a+b; }
double eval(){ return a; }  //unimplemented
void cls(){ a=0; b=0; op = NULL; show(0); }

void digit(int key){
    if(decimalPlace != 0){
        b+=digitmap[key]*pow(.1,decimalPlace);
        show(dtostrf(b,1,decimalPlace,tmpStr));
        decimalPlace++;
    } else {
        b=b*10+digitmap[key];
        show(dtostrf(b,1,0,tmpStr));
    }
}

void show(char* str){
    tft.setCursor(50, 50);
//    tft.fillScreen(ILI9340_BLACK);

    tft.fillRect(lastX, lastY, lastW + 100, lastH, ILI9340_BLACK);
//    dtostrf(num,1,decs,lastshown);
    char *period;
    int index;
    size_t str_length;
    str_length = strlen(str);
    period = strchr(str, '.');
    index = (int) (period - str);
    
    if(index < 1){
      tft.println(str);
    } else{
      String sub1;
      for(int p = 0; p < index; p++){
        sub1 = sub1 + str[p];
      }
      String sub2;
      for(int p = index + 1; p < (int) str_length; p++){
         sub2 = sub2 + str[p];
      }
      Serial.print("Sub 1:");
      Serial.println(sub1);
      Serial.print("Sub 2:");
      Serial.println(sub2);
      tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
      tft.print(sub1);
      tft.setTextColor(ILI9340_RED);  tft.setTextSize(2);
      int16_t cursorX = tft.getCursorX() - 18;
      int16_t cursorY = tft.getCursorY(); 
      tft.setCursor(cursorX, cursorY);
      tft.print('.');
      cursorX = tft.getCursorX() - 18;
      cursorY = tft.getCursorY(); 
      tft.setCursor(cursorX, cursorY);
      tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
      tft.println(sub2);
      
    }

    
    
    //Serial.println(index);
    //Serial.println(str_length);
    
    tft.getTextBounds(str, 50, 50, &lastX, &lastY, &lastW, &lastH);
    
}

void addChar(char* newChar) {
    int16_t  x1, y1, curx=50, cury=50;
    uint16_t w, h;
    tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
    
    tft.getTextBounds(newChar, tft.getCursorX(), tft.getCursorY(), &x1, &y1, &w, &h);
    tft.fillRect(x1, y1, w, h, ILI9340_BLACK);
    tft.print(newChar);
}
