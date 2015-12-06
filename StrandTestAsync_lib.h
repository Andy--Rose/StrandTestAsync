enum pattern { NONE, THEATER_CHASE, RAINBOW_CYCLE, COLOR_WIPE, CIRCLE_FADE };
enum direction { FORWARD, REVERSE };

#define MIN_CHASE_SPEED 60
#define MAX_CHASE_SPEED 30
#define CHASE_SPEED_INTERVAL 2
#define CHASE_INTERVAL_MILIS 300
#define WIPE_INTERVAL_MILIS 500
#define RAINBOW_INTERVAL_MILIS 2

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
    unsigned long lastUpdate;   // last update of position

    uint32_t Color1, Color2;
    uint32_t CycleTime_Seconds = 30;
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
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case CIRCLE_FADE:
                    CircleFadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }

        // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
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
            if (Index <= 0)
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
        switch(ActivePattern) {
            case THEATER_CHASE:
                Interval = RAINBOW_INTERVAL_MILIS;
                ActivePattern = RAINBOW_CYCLE;
                break;
            case RAINBOW_CYCLE:
                Interval = WIPE_INTERVAL_MILIS;
                ActivePattern = COLOR_WIPE;
                break;
            case COLOR_WIPE:
                Interval = CHASE_INTERVAL_MILIS;
                ActivePattern = THEATER_CHASE;
                break;
            default:
                break;
        } 
      }
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        Serial.print("Begin ColorWipe");
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }

    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      Serial.print("Begin RainbowCycle");
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
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        Serial.print("Begin TheaterChase");
        ActivePattern = THEATER_CHASE;
        Interval = interval;
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
            if ((i + Index) % Interval == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a Circle Fade
    void CircleFade(uint32_t color1, uint32_t color2, uint16_t count, uint16_t fadeLength, direction dir = FORWARD)
    {
        Serial.print("Begin CircleFade");
        ActivePattern = CIRCLE_FADE;
        CircleFadeLength = fadeLength;
        Interval = count;     // for this pattern, interval is the number of circles
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }

   // Update the Theater Chase Pattern
    void CircleFadeUpdate()
    {
      for(int i=0; i< numPixels(); i++)
      {
        uint16_t line = i / CircleFadeLength;
        if (line > Interval) 
        {
          setPixelColor(i, 0);
        }
        else
        {
          uint32_t color = Color1;
          if ( line % 2 == 1) { color = Color2; }
          
          uint16_t pixel = i % CircleFadeLength;
          double percent = pixel / CircleFadeLength;

          setPixelColor(i, DimColorPercent(color, percent));
        }
      }
      show();
      Increment();
    }
    
  private:
    unsigned long this_time = millis();
    unsigned long last_time = this_time;
  
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
      uint32_t dimColor = Color(Red(color) * percent, Green(color) * percent, Blue(color) * percent);
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
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }

    void ChangeSpeed() {
      if (speedChange) {
        if (accelerating) {
          Interval -= CHASE_SPEED_INTERVAL;
          if (Interval < MAX_CHASE_SPEED) {
            accelerating = false;
            Interval += CHASE_SPEED_INTERVAL;
          }
        }
        else {
          Interval += CHASE_SPEED_INTERVAL;
          if (Interval > MIN_CHASE_SPEED) {
            accelerating = true;
            Interval -= CHASE_SPEED_INTERVAL;
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
