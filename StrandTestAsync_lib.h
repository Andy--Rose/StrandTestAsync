enum pattern { NONE, THEATER_CHASE, RAINBOW_CYCLE, COLOR_WIPE, CIRCLE_FADE, CLAP };
enum direction { FORWARD, REVERSE };

#define MIN_CHASE_SPEED 60
#define MAX_CHASE_SPEED 30
#define CHASE_INTERVAL_MILIS 500
#define WIPE_INTERVAL_MILIS 50
#define RAINBOW_INTERVAL_MILIS 5
#define FADE_INTERVAL_MILIS 50
#define CLAP_INTERVAL_MILIS 50

class Pattern : public Adafruit_NeoPixel
{
  public:
    // Member Variables:
    pattern ActivePattern;
    direction Direction;

    bool speedChange = false;   // must be overridden
    bool accelerating = false;  // must be overridden
    bool lockPattern = false;

    unsigned long Interval;     // milliseconds between updates

    uint32_t Color1 = Color(0,255,0);
    uint32_t Color2 = Color(0,0,255);
    uint32_t CycleTime_Seconds = 30;
    uint16_t SplitSize = 2;
    uint16_t TotalSteps;        // total number of steps in the pattern
    uint16_t Index;

    // pattern-specific variables
    uint16_t CircleFadeLength = 6;

    void (*OnComplete)();       // callback

    // Constructor - calls base-class constructor to initialize strip
    Pattern(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }

        // Update the pattern
    void Update()
    {
        if (!lockPattern) { ChangePattern(); }
        if((millis() - lastUpdate) > Interval) // time to update
        {
            ChangeSpeed();
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
//                case RAINBOW_CYCLE:
//                    RainbowCycleUpdate();
//                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case CIRCLE_FADE:
                    CircleFadeUpdate();
                    break;
                case CLAP:
                    ClapUpdate();
                    break;
                default:
                    break;
            }
            Increment();
        }
    }

        // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index > TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index < 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }

    void ChangePattern() {
      this_time = millis();
      if((this_time - last_time) > (CycleTime_Seconds * 1000)) {
        last_time = millis();
        show();
        switch(ActivePattern) {
            case THEATER_CHASE:
//                RainbowCycle(RAINBOW_INTERVAL_MILIS);
//                break;
//            case RAINBOW_CYCLE:
                ColorWipe(Color1, Color2, WIPE_INTERVAL_MILIS);
                break;
            case COLOR_WIPE:
                CircleFade(Color1, Color2, FADE_INTERVAL_MILIS, 8, true);
                break;
            case CIRCLE_FADE:
                Clap(Color1, Color2, CLAP_INTERVAL_MILIS, 3);
                break;
            case CLAP:
                TheaterChase(Color1, Color2, CHASE_INTERVAL_MILIS, 3);
                break;
            default:
                break;
        } 
      }
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        Serial.println("Begin ColorWipe");
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        wipeColor = Color1;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
      setPixelColor(Index, wipeColor);
      show();
      if (Index + 1 == TotalSteps)
      {
        if (wipeColor == Color1)
        {
          wipeColor = Color2;
        }
        else
        {
          wipeColor = Color1;
        }
      }
    }

    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      Serial.println("Begin RainbowCycle");
      ActivePattern = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, uint16_t count, direction dir = FORWARD)
    {
        Serial.println("Begin TheaterChase");
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        SplitSize = count;               // SplitSize here will be the length of Color1
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }

   // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
      for(int i=0; i< numPixels(); i++)
      {
          if ((i + Index) % SplitSize == 0)
          {
              setPixelColor(i, Color1);
          }
          else
          {
              setPixelColor(i, Color2);
          }
      }
      show();
    }

    // Initialize for a Circle Fade
    void CircleFade(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t fadeLength, bool doubletone = false, direction dir = FORWARD)
    {
        Serial.println("Begin CircleFade");
        ActivePattern = CIRCLE_FADE;
        CircleFadeLength = fadeLength;
        Interval = interval;
        circleFadeDouble = doubletone;
        TotalSteps = numPixels() + 1;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }

   // Update the Theater Chase Pattern
    void CircleFadeUpdate()
    {
      CircleFadeSet(Index, Color1);
      if (circleFadeDouble)
      {
        uint32_t start = (Index + (TotalSteps / 2)) % TotalSteps;
        CircleFadeSet(start, Color2);
      }
      show();
    }

    void CircleFadeSet(uint32_t start, uint32_t color)
    {
      for (int i=0; i < CircleFadeLength; i++)
      {
        int point = start - i;
        if (point < 0) { point = TotalSteps + point; }
//        Serial.println(point);
        double percent = (float)i/(float)CircleFadeLength;
//        Serial.println(percent);
        uint32_t colorDimmed = DimColorPercent(color, percent);
        setPixelColor(point, colorDimmed);
      }
      int point = start - CircleFadeLength;
      if (point < 0) { point = TotalSteps + point; }
      setPixelColor(point, 0); 
    }

        // Initialize for a Circle Fade
    void Clap(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t len)
    {
        Serial.println("Begin Clap");
        ActivePattern = CLAP;
        Interval = interval;
        SplitSize = len;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
   }

   // Update the Theater Chase Pattern
    void ClapUpdate()
    {
      show();
      if (Index >= (TotalSteps / 2))
      {
//        Serial.println("REVERSE");
        Reverse();
      }

      // Set the ON lights
      for (int i=0; i < SplitSize; i++)
      {
        int point = Index - i;
        if (Direction == REVERSE)
        {
          point = Index + i;
        }
        ClapSet(point, Color1, Color2);
      }

      // Set the OFF lights
      int point = Index - SplitSize;
      if (Direction == REVERSE)
      {
        point = Index + SplitSize;
      }
      ClapSet(point, 0, 0);
      show();
    }

    void ClapSet(int point, int32_t colorOne, int32_t colorTwo)
    {
      if (point >= 0)
      {
        if (point <= TotalSteps / 2)
        {
          setPixelColor(point, colorOne);
          int oppositePoint = TotalSteps - point;
          setPixelColor(oppositePoint, colorTwo);
        }
        else
        {
          if (colorOne == 0 && colorTwo == 0)
          {
            setPixelColor(point, colorOne);
            int oppositePoint = TotalSteps - point;
            setPixelColor(oppositePoint, colorTwo);
          }
        }
      }
    }
    
  private:
    unsigned long lastUpdate;   // last update of position
    unsigned long this_time = millis();
    unsigned long last_time = this_time;
    bool circleFadeDouble = false;

    uint32_t wipeColor;
  
    // Common Utility Functions
        // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }

    // Return color, dimmed by 75% (used by scanner)
    uint32_t DimColor(uint32_t color)
    {
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    uint32_t DimColorPercent(uint32_t color, double percent)
    {
      if (percent == 0) { return color; }
      uint32_t redPart = FlipColor((uint32_t)(Red(color)*percent));
      uint32_t greenPart = FlipColor((uint32_t)(Green(color)*percent));
      uint32_t bluePart = FlipColor((uint32_t)(Blue(color)*percent));
      uint32_t dimColor = Color(redPart, greenPart, bluePart);
      return dimColor;
    }

    uint32_t FlipColor(uint32_t color)
    {
      if (color != 0)
      {
        return 255 - color;
      }
      return color;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }

    // Reverse direction of the pattern
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
        }
        else
        {
            Direction = FORWARD;
        }
    }

    void ChangeSpeed() {
      if (speedChange) {
        if (accelerating) {
          Interval -= CHASE_INTERVAL_MILIS;
          if (Interval < MAX_CHASE_SPEED) {
            accelerating = false;
            Interval += CHASE_INTERVAL_MILIS;
          }
        }
        else {
          Interval += CHASE_INTERVAL_MILIS;
          if (Interval > MIN_CHASE_SPEED) {
            accelerating = true;
            Interval -= CHASE_INTERVAL_MILIS;
          }
        }
      }
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
};
