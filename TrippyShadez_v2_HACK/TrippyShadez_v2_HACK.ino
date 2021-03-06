#include <Adafruit_NeoPixel.h>

#define NeopixelPin 4//chane to 1?
#define MicPin A0
#define ButtonPin 3 //interrupt is 0 even though GPIO #2
#define PotPin A5 //mic sensitivity pot

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(58, NeopixelPin);
  
uint32_t red = pixels.Color(255,0,0);
uint32_t orange = pixels.Color(255,65,0);
uint32_t yellow = pixels.Color(255,215,0);
uint32_t green = pixels.Color(0,255,0);
uint32_t blue = pixels.Color(0,0,255);
uint32_t purple = pixels.Color(255,0,200);
uint32_t pink = pixels.Color(255,0,80);
uint32_t white = pixels.Color(255,255,255);
uint32_t blank = pixels.Color(0,0,0);
int brightness = 0;
int pastBrightness = 0;
long debouncing_time = 15; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
unsigned int sample;
volatile byte mode = 0;
unsigned int lowBound = 100;
int lastNumLights = 0;

// color schemes
uint32_t listColors[] = {red, orange, yellow, green, blue, purple, white};
uint32_t rainbowColors[] = {red, orange, yellow, green, blue, purple, pink};
uint32_t holidayColors[] = {red, green};
uint32_t winterColors[] = {blue, purple, white};
uint32_t warmColors[] = {red, orange, yellow};
uint32_t brandColors[] = {yellow, green, blue, white};


void setup() 
{
  Serial.begin(9600);
  pixels.begin();
  
  pinMode(PotPin, INPUT);
  pinMode(MicPin, INPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  
  attachInterrupt(1, debounceInterrupt, FALLING); //Trinket
  randomSeed(analogRead(0));
}

void loop()
{
  // stabilize light flicker
  brightness = analogRead(PotPin);
  if (brightness != pastBrightness) {
    uint16_t level = uint16_t(map(brightness,0,1023,0, 100));
    pixels.setBrightness(level);
  }
  
  if(mode==0) {
    lastNumLights = SoundReactiveOneWay(10, lastNumLights);
    //SoundReactiveFillRight(5, 80);
  } else if(mode==9) {
    lastNumLights = SoundReactiveReverse(10, lastNumLights);
  } else if(mode==1) {
    SparklePulse(5);
  } else if(mode==2) {
    SoundReactiveSparkle(10);
  } else if(mode==3) {
    rainbow(10);
  } else if(mode==4) {
    CylonBounce(6, 30, 50);
  } else if(mode==5) {
    Sparkle(10, 30);
  } else if (mode==6) {
    StrangerThings(30, 100);
  } else if (mode==7) {
    FillRight(10);
  } else if (mode==8) {
    Fire(55,120,15);
  } else if(mode==-1) {
    TheaterChaseRainbow(100);
  } else {
    mode = 0;
  }
  pastBrightness = brightness;
}

// cycle between red, blue, green
void calibrate() {
  allSet(red);
  delay(500);
  allSet(blue);
  delay(500);
  allSet(green);
  delay(500);
}

void Snake(int wait) {
  for (int j = 0; j < 6; j++) {
    // forward
    for (int i = 0; i < pixels.numPixels()/2; i++) {
      pixels.setPixelColor(i,listColors[j]);
      pixels.setPixelColor(pixels.numPixels()-1-i,listColors[j]);
      pixels.show();
      delay(wait);
    }

    // backward
    for (int i = pixels.numPixels()/2 - 1; i >= 0; i--) {
      pixels.setPixelColor(i,blank);
      pixels.setPixelColor(pixels.numPixels()-1-i,blank);
      pixels.show();
      delay(wait);
    }
  }
}

void SnakeReverse(int wait) {
  for (int j = 0; j < 6; j++) {
    // forward
    for (int i = pixels.numPixels()/2 - 1; i >= 0; i--) {
      pixels.setPixelColor(i,listColors[j]);
      pixels.setPixelColor(pixels.numPixels()-1-i,listColors[j]);
      pixels.show();
      delay(wait);
    }

    // backward
    for (int i = 0; i < pixels.numPixels()/2; i++) {
      pixels.setPixelColor(i,blank);
      pixels.setPixelColor(pixels.numPixels()-1-i,blank);
      pixels.show();
      delay(wait);
    }
  }
}

void pixelsSet(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, c);
  }
  // move the show outside the loop
  pixels.show();
  delay(wait);
}

void SimpleWave(float rate,int cycles, int wait) {
   float pos=0.0;
  // cycle through x times
  for(int x=0; x<cycles; x++)
  {
    pos = pos+rate;
    for(int i=0; i<pixels.numPixels(); i++) {
      // sine wave, 3 offset waves make a rainbow!
      float level = sin(i+pos) * 127 + 128;
      // set color level 
      pixels.setPixelColor(i,(int)level*(random(0,11)/10),(int)level*(random(0,11)/10),(int)level*(random(0,11)/10));
    }
    pixels.show();
    delay(wait);
  }
}

int getPeakToPeak() {
  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak;
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  //50 is the same size, 50ms
  while (millis() - startMillis < 50) {
    sample = analogRead(MicPin); 
    if (sample < 1024) { // toss out spurious readings
        if (sample > signalMax) {
          signalMax = sample;  // save just the max levels
        }
        else if (sample < signalMin) {
          signalMin = sample;  // save just the min levels
        }
     }
  }
 
  peakToPeak = signalMax - signalMin;
  
  return peakToPeak;
}

void SoundReactive(int wait) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();

  if (peakToPeak > lowBound) {
    int halfLights = pixels.numPixels() / 2;
    float divisor = float((maxSound - lowBound) / halfLights);
    int numToLight = int((peakToPeak - lowBound) / divisor);
    
    for (int i = 0; i < numToLight; i++) {
      pixels.setPixelColor(i,listColors[random(7)]);
      pixels.setPixelColor(pixels.numPixels()-1-i,listColors[random(7)]);
    }

    if ((halfLights - numToLight) > 0) {
      for (int i = halfLights; i >= numToLight; i--) {
        pixels.setPixelColor(i,blank);
        pixels.setPixelColor(pixels.numPixels()-1-i,blank);
      }
    }
    
    pixels.show();
  } else {
    allSet(blank);
  }
  delay(wait);
}

int SoundReactiveReverse(int wait, int lastLights) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();
  int halfLights = pixels.numPixels() / 2;
  int numToLight;

  if (peakToPeak > lowBound) {
    float divisor = float((maxSound - lowBound) / halfLights);
    numToLight = int((peakToPeak - lowBound) / divisor);
  } else {
    numToLight = 0;
  }

  if (numToLight < lastLights) {
    numToLight = lastLights - 2;
  }
  
  if (numToLight > 0)  {
    for (int i = halfLights; i >= (halfLights - numToLight); i--) {
      pixels.setPixelColor(i,listColors[random(7)]);
      pixels.setPixelColor(pixels.numPixels()-1-i,listColors[random(7)]);
    }

    if ((halfLights - numToLight) > 0) {
      for (int i = 0; i < (halfLights - numToLight); i++) {
        pixels.setPixelColor(i,blank);
        pixels.setPixelColor(pixels.numPixels()-1-i,blank);
      }
    }
    
    pixels.show();
  } else {
    allSet(blank);
  }    
  delay(wait);
  return numToLight;
}

int SoundReactiveOneWay(int wait, int lastLights) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();
  int lights = pixels.numPixels();
  int numToLight;

  if (peakToPeak > lowBound) {
    float divisor = float((maxSound - lowBound) / lights);
    numToLight = int((peakToPeak - lowBound) / divisor);
  } else {
    numToLight = 0;
  }

  if (numToLight < lastLights) {
    lastLights = lastLights - 1;
  } else {
    lastLights = numToLight;
  }
  
  if (numToLight > 0)  {
    for (int i = 0; i < (numToLight - 1); i++) {
      pixels.setPixelColor(i,listColors[random(7)]);
    }

    if ((lights - numToLight) > 0) {
      for (int i = numToLight; i < lights; i++) {
        pixels.setPixelColor(i,blank);
      }
    }
    pixels.setPixelColor(lastLights, white);
    
    pixels.show();
  } else {
    if (lastLights > 0) {
      for (int i = 0; i < lastLights; i++) {
        pixels.setPixelColor(i, blank);
      }
      pixels.setPixelColor(lastLights, white);

      for (int i = (lastLights+1); i < lights; i++) {
        pixels.setPixelColor(i, blank);
      }
      pixels.show();
    } else {
      allSet(blank);
    }
  }    
  delay(wait);
  return lastLights;
}

void SoundReactiveBrightness(int wait) {
  int peakToPeak = getPeakToPeak();
  int bound = 30;

  if (peakToPeak > bound) {
    int bright = map(peakToPeak, bound, 1023, 0, 255);
    for (int i = 0; i < pixels.numPixels(); i++) {
      if ((i % 4 == 3) || (i % 4 == 2)) {
        // green
        pixels.setPixelColor(i,pixels.Color(0,bright,0));
      } else {
        // red
        pixels.setPixelColor(i,pixels.Color(bright,0,0));
      }
      
    }
    pixels.show();
  } else {
    allSet(blank);
  }
  delay(wait);
}

void SoundReactiveColor(int wait) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();
  int bound = 30;

  if (peakToPeak > bound) {
    // start with red => green
    int colorSum = map(peakToPeak, lowBound, 1023, 0, 255*2);
    uint32_t color;
    if (colorSum < 255) {
      color = pixels.Color(255-colorSum, 0, 0);
    } else {
      color = pixels.Color(255, 255*2-colorSum, 0);
    }
    allSet(color);
  } else {
    allSet(blank);
  }    
  delay(wait);
}

void SoundReactiveSparkle(int wait) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();

  if (peakToPeak > lowBound) {
    float divisor = float((maxSound - lowBound) / (pixels.numPixels()-10));
    int numToLight = int((peakToPeak - lowBound) / divisor);

    int isOn[pixels.numPixels()] = {0};

    // turn on a random selection of lights based on loudness
    for (int i = 0; i < numToLight; i++) {
      bool litLight = false;
      while (litLight == false) {
        int turnOn = random(pixels.numPixels());
        if (isOn[turnOn] == 0) {
          pixels.setPixelColor(turnOn,listColors[random(7)]);
          isOn[turnOn] = 1;
          litLight = true;
        }
      }
    }

    // turn the rest of the lights off
    for (int i = 0; i < pixels.numPixels(); i++) {
      if (isOn[i] == 0) {
        pixels.setPixelColor(i, blank);
      }
    }
    
    pixels.show();
    delay(wait);
  } else {
    allSet(blank);
  }
}

void SparklePulse(int wait) {
  allSet(blank);
  int isOn[pixels.numPixels()] = {0};

  // slowly turn all lights on
  for (int i = 0; i < pixels.numPixels(); i++) {
    bool litLight = false;
    while (litLight == false) {
      int turnOn = random(pixels.numPixels());
      if (isOn[turnOn] == 0) {
        pixels.setPixelColor(turnOn,rainbowColors[random(7)]);
        isOn[turnOn] = 1;
        litLight = true;
      }
    }
    pixels.show();
    delay(wait);
  }

  delay(wait*3);
  // slowly turn all lights off
  // slowly turn all lights on
  for (int i = 0; i < pixels.numPixels(); i++) {
    bool litLight = true;
    while (litLight == true) {
      int turnOff = random(pixels.numPixels());
      if (isOn[turnOff] == 1) {
        pixels.setPixelColor(turnOff, blank);
        isOn[turnOff] = 0;
        litLight = false;
      }
    }
    pixels.show();
    delay(wait);
  }
  delay(wait*3);
}

void SoundReactiveFillRight(int minWait, int maxWait) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();

  allSet(blank);
  int lit = pixels.numPixels();
  for (int i = 0; i < pixels.numPixels(); i++) {
    uint32_t color = rainbowColors[random(7)];
    // send pixel from one side to other
    for (int j = 0; j < lit; j++) {
      pixels.setPixelColor(j, color);
      
      // set all other pixels blank (except ones already filled)
      if (j > 0) {
        for(int k = 0; k < j; k++) {
          pixels.setPixelColor(k, blank);
        }
      }
      for(int k = j+1; k < lit; k++) {
        pixels.setPixelColor(k, blank);
      }
      pixels.show();
      delay(getSoundDelay(minWait, maxWait));
    }
    delay(getSoundDelay(minWait, maxWait));
    lit--;
  }
}

void FillRight(int wait) {
  allSet(blank);
  int lit = pixels.numPixels();

  for (int i = 0; i < pixels.numPixels(); i++) {
    uint32_t color = rainbowColors[random(7)];
    // send pixel from one side to other
    for (int j = 0; j < lit; j++) {
      pixels.setPixelColor(j, color);
      
      // set all other pixels blank (except ones already filled)
      if (j > 0) {
        for(int k = 0; k < j; k++) {
          pixels.setPixelColor(k, blank);
        }
      }
      for(int k = j+1; k < lit; k++) {
        pixels.setPixelColor(k, blank);
      }
      pixels.show();
      delay(wait);
    }
    delay(wait);
    lit--;
  }
}

void CylonBounce(int EyeSize, int SpeedDelay, int ReturnDelay){
  int number = 0;
  // forward
  for(int i = 0; i < pixels.numPixels()-EyeSize+1; i++) {
    for(int j = 0; j < EyeSize; j++) {
      pixels.setPixelColor(i+j,listColors[j]);
    }

    // set all other pixels blank
    if (i > 0) {
      for(int j = 0; j < i; j++) {
        pixels.setPixelColor(j, blank);
      }
    }
    if (i+EyeSize < pixels.numPixels()) {
      for(int j = i+EyeSize; j < pixels.numPixels(); j++) {
        pixels.setPixelColor(j, blank);
      }
    }
    
    pixels.show();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);

  // backward
  for(int i = pixels.numPixels()-EyeSize-2; i > 0; i--) {
    for(int j = 0; j < EyeSize; j++) {
      pixels.setPixelColor(i+j,listColors[j]);
    }

    // set all other pixels blank
    if (i > 0) {
      for(int j = 0; j < i; j++) {
        pixels.setPixelColor(j, blank);
      }
    }
    if (i+EyeSize < pixels.numPixels()) {
      for(int j = i+EyeSize; j < pixels.numPixels(); j++) {
        pixels.setPixelColor(j, blank);
      }
    }
    
    pixels.show();
    delay(SpeedDelay);
  } 
  delay(ReturnDelay);
}

void allSet(uint32_t c)
{
  for(int i=0;i<pixels.numPixels();i++) {
    pixels.setPixelColor(i,c);
  }
  pixels.show();
}

void travellingDot()
{
  int number = 0;
  for(int i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i,listColors[random(7)]);
    pixels.show();
    delay(30);
    allSet(blank);
  }
  for(int j=pixels.numPixels()-1; j>=0; j--) {
    pixels.setPixelColor(j,listColors[random(7)]);
    pixels.show();
    delay(30);
    allSet(blank);
  }
}

void debounceInterrupt() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) 
  {
    Interrupt();
    last_micros = micros();
  }
}

void Interrupt() {
  mode++;
}

int getSoundDelay(int minWait, int maxWait) {
  int maxSound = 1024;
  int peakToPeak = getPeakToPeak();
  int wait;

  if (peakToPeak > lowBound) {
    float multiplier = float((peakToPeak - lowBound) / (maxSound - lowBound));
    wait = maxWait - int(maxWait * multiplier) + minWait;
  } else {
    wait = maxWait;
  }
  return wait;
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[60];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < pixels.numPixels(); i++) {
    cooldown = random(0, ((Cooling * 10) / pixels.numPixels()) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= pixels.numPixels() - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < pixels.numPixels(); j++) {
    setPixelHeatColor(j, heat[j] );
  }

  pixels.show();
  delay(SpeedDelay);
}

void FireSparkle(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[60];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < pixels.numPixels(); i++) {
    cooldown = random(0, ((Cooling * 10) / pixels.numPixels()) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= pixels.numPixels() - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < pixels.numPixels(); j++) {
    setPixelHeatColor(j, heat[j] );
  }

  pixels.show();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    pixels.setPixelColor(Pixel, random(0,256), random(0,256), random(0,256));
  } else if( t192 > 0x40 ) {             // middle
    pixels.setPixelColor(Pixel, random(0,256), random(0,256), 0);
  } else {                               // coolest
    pixels.setPixelColor(Pixel, heatramp, 0, 0);
  }
}

void setPixelSparkleColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    pixels.setPixelColor(Pixel, random(0,256), random(0,256), random(0,256));
  } else if( t192 > 0x40 ) {             // middle
    pixels.setPixelColor(Pixel, random(0,256)/2, random(0,256)/2, random(0,256)/2);
  } else {                               // coolest
    pixels.setPixelColor(Pixel, random(0,256)/4, random(0,256)/4, random(0,256)/4);
  }
}

void Sparkle(int Count, int wait) {
  //create zero array of lights
  int isOn[pixels.numPixels()] = {0};

  // light random lights, but only if not turned on already
  for (int i=0; i<Count; i++) {
    bool litLight = false;
    while (litLight == false) {
      int turnOn = random(pixels.numPixels());
      if (isOn[turnOn] == 0) {
        pixels.setPixelColor(turnOn,listColors[random(7)]);
        isOn[turnOn] = 1;
        litLight = true;
      }
    }
  }

  // turn the rest of the lights off
  for (int i = 0; i < pixels.numPixels(); i++) {
    if (isOn[i] == 0) {
      pixels.setPixelColor(i, blank);
    }
  }
  
  pixels.show();
  delay(wait);
}

void StrangerThings(int Count, int wait) {
  // set background
  allSet(blank);

  //create zero array of lights
  int isOn[pixels.numPixels()] = {0};

  // light random lights, but only if not turned on already
  for (int i=0; i<Count; i++) {
    bool litLight = false;
    while (litLight == false) {
      int turnOn = random(pixels.numPixels());
      if (isOn[turnOn] == 0) {
        pixels.setPixelColor(turnOn,listColors[random(7)]);
        isOn[turnOn] = 1;
        litLight = true;
      }
    }
    pixels.show();
    delay(wait);
  }
  allSet(blank); 
  delay(wait);
}

void colorWipe(uint32_t c, uint8_t wait) 
{
  for(int i=0; i<pixels.numPixels(); i++) 
  {
      pixels.setPixelColor(i, c);
      pixels.show();
      delay(wait);
  }
}
void rainbow(uint8_t wait) 
{
  uint16_t i, j;

  for(j=0; j<256; j++) 
  {
    for(i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}
void TheaterChaseRainbow(uint8_t wait) 
{
  for (int j=0; j < 256; j++) 
  {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) 
    {
        for (int i=0; i < pixels.numPixels(); i=i+3) 
        {
          pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        pixels.show();
       
        delay(wait);
       
        for (int i=0; i < pixels.numPixels(); i=i+3) 
        {
          pixels.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}
void randomCycle(int count)
{
  //uint32_t c;
  int pixel = random(0,62);
  byte color = random(7);
  allSet(blank);
  pixels.show();
  pixels.setPixelColor(pixel,listColors[color]);
  pixels.show();
  delay(20); 
  
}
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
