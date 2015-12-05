#include <Adafruit_NeoPixel.h>
#include "StrandTestAsync_lib.h"

#define PIN 4
#define LEDS 36

Pattern strand = Pattern(LEDS, PIN, NEO_GRB + NEO_KHZ800, &PatternComplete);

void setup() {
  strand.begin();

  strand.RainbowCycle(1);
  strand.Color1 = strand.Color(0,255,0);
  strand.Color2 = strand.Color(255,0,0);
}

void loop() {
  strand.Update();
}

void PatternComplete() {
  
}

