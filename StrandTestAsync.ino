#include <Adafruit_NeoPixel.h>
#include "StrandTestAsync_lib.h"

#define PIN 4
#define LEDS 36

Pattern strand = Pattern(LEDS, PIN, NEO_GRB + NEO_KHZ800, &PatternComplete);

void setup() {
  Serial.begin(9600);
  strand.begin();
  strand.show();
  strand.Color2 = strand.Color(0,255,0);
  strand.Color1 = strand.Color(255,0,0);
  strand.Init();
//  strand.Clap(strand.Color(255, 0, 0), strand.Color(0,255,0), 50, 3);

//Test Patters
//  strand.lockPattern = true;

  // Testing CircleFade pattern
//  strand.CircleFade(strand.Color(255, 0, 0), strand.Color(0,255,0), 50, 8, true);

//    Testing Clap pattern
//  strand.Clap(strand.Color(255, 0, 0), strand.Color(0,255,0), 50, 3);
}

void loop() {
  strand.Update();
}

void PatternComplete() {
  
}

