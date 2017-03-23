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

void setup() {
    Serial.begin(9600);
    while (!Serial);

    //TFT
    tft.begin();
    tft.setRotation(1);
    tft.setFont(&FreeMono24pt7b);
    testText();

    //Trellis
    trellis.begin(0x70);
}

void loop(void) {
    delay(30);
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<16; i++) {
        // if it was pressed, turn it on
        if (trellis.justPressed(i)) {
          Serial.print("v"); Serial.println(i);
          trellis.setLED(i);
        } 
        // if it was released, turn it off
        if (trellis.justReleased(i)) {
          Serial.print("^"); Serial.println(i);
          trellis.clrLED(i);
        }
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
}

unsigned long testText() {
  tft.fillScreen(ILI9340_BLACK);
  unsigned long start = micros();
  tft.setCursor(50, 50);
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
  tft.print(1234);
  tft.setTextColor(ILI9340_RED);  tft.setTextSize(3);
  tft.print('.');
  tft.setTextColor(ILI9340_WHITE);  tft.setTextSize(1);
  tft.print(56);
  return micros() - start;
}
