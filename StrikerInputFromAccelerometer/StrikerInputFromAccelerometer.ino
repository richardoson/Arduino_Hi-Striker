#include "Adafruit_WS2801.h"  //modified to include "Color()" command from neopixel library
#include "SPI.h"
#include "Time.h"

/*****************************************************************************
Coded by Aaron Crosby based off example sketch for driving Adafruit WS2801 pixels
Designed specifically to work with the Adafruit RGB Pixels!  Thanks Adafruit staff!
12mm Flat shape   ----> https://www.adafruit.com/products/738
NOTE:  Requires _modified_ Adafruit_WS2801 Library found 
https://github.com/richardoson/Arduino_Hi-Striker/tree/master/Adafruit-WS2801-Library-master
Also using Time library by Paul Stoffregen (thanks!) https://github.com/PaulStoffregen/Time
Accelerometer (ADXL377) https://www.adafruit.com/products/1413
attached with the following color code for a CAT6 network cable:
   VIN=orange
   3V=blue
   GND=brown
   T=green
   Z=brown/white
   Y=orange/white
   X=blue/white
*****************************************************************************/

// Define
#define NUM_LEDS 25
#define CLOCK_PIN 13
#define DATA_PIN 12
#define NUM_PATTERNS 4
#define CTR_THRESH 16


// Init Vars
uint32_t accelerometerThreshhold=100;     //minimum value returned from "hitVal" - noise reduction
uint32_t difficulty=165;                  //set this to the hardest hit you can get
const int sampleSize = 5;                 //original value was 10 (10 is best for metal striker, 5 for stapler)
const int potPin1 = A4;
const int potPin2 = A5;
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
int xRawMin = 512;
int xRawMax = 512;
int yRawMin = 512;
int yRawMax = 512;
int zRawMin = 512;
int zRawMax = 512;
uint8_t potRead1 = 0;
uint8_t potRead2 = 0;
uint8_t pattern=1;
uint8_t lastPix=0; 
uint8_t myPix=0;
uint8_t counter=0;
uint8_t counter2=0;
uint8_t color=200;
uint8_t colors[3];
uint8_t relayPin1=4;
uint8_t relayPin2=5;
uint8_t relayPin3=6;
uint8_t relayPin4=7;
uint8_t ledBrightBreathe=0;
uint8_t ledBrightBreatheMod=1;
uint32_t numLedsToLight=0;
uint32_t hitValue=0;
uint32_t xRaw=0;
uint32_t yRaw=0;
uint32_t zRaw=0;


// Start Strip
Adafruit_WS2801 strip = Adafruit_WS2801(NUM_LEDS, DATA_PIN, CLOCK_PIN);


void setup() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    pinMode(potPin1, INPUT);
    pinMode(potPin2, INPUT);
    pinMode(xInput, INPUT);
    pinMode(yInput, INPUT);
    pinMode(zInput, INPUT);
    pinMode(relayPin1, OUTPUT);
    pinMode(relayPin2, OUTPUT);
    pinMode(relayPin3, OUTPUT);
    pinMode(relayPin4, OUTPUT);
    analogReference(EXTERNAL); //required to not short-out arduino when connecting accelerometer 3v
    Serial.begin(9600);    
}

void loop() {

    //read adjustable knobs to determine difficulty/threshhold
    int potRead1 = analogRead(potPin1);
    accelerometerThreshhold = map(potRead1, 0, 1023, 100, 150); // min:100, max:150
    int potRead2 = analogRead(potPin2);
    difficulty = map(potRead2, 0, 1023, 151, 190); // min:151, max: 190    

    
    //display values if over threshhold
    if(hitValue > accelerometerThreshhold) {
      relayControl(0); //relays are OFF
      ledsOff();
      pattern = 1;
      Serial.print("Accelerometer Threshhold = "); Serial.print(accelerometerThreshhold); Serial.print("\t");
      Serial.print("Difficulty = "); Serial.println(difficulty);
      Serial.print("Nice hit!  Registered a "); Serial.print(hitValue); Serial.println(" on the scale!");      
      numLedsToLight = map(hitValue, accelerometerThreshhold, difficulty, 0, NUM_LEDS); //how hard = how many lights
      hitAnimation(numLedsToLight, 10); //lower number at end is faster animation sequence
      hitValue = 0;
      Serial.println("**********READY!**********"); Serial.println();
    } else if (hitValue > accelerometerThreshhold/2) {      
      pattern = 1;
      Serial.print("Accelerometer Threshhold = "); Serial.print(accelerometerThreshhold); Serial.print("\t");
      Serial.print("Difficulty = "); Serial.println(difficulty);
      Serial.print("Meh hit.  Registered a "); Serial.print(hitValue); Serial.println(" on the scale."); Serial.println();         
    }
    
        
    // Timer Function and Pattern Picker           
    if(now() <= 30) {            
      relayControl(0); //relays are OFF
      pattern=1;
      //Serial.println("pattern = 1");
    }

    if(now() >= 30 && minute() <= 2) {
      relayControl(1); //relays are ON
      pattern=2;             
    }

    if(minute() >= 3 && minute() <= 5) {
      relayControl(1); //relays are ON
      pattern=3;            
    }

    if(minute() >= 6 && minute() <= 8) {
      relayControl(1); //relays are ON
      pattern=4;      
    }
        
    if(minute() > 9) {
      setTime(31); //resets timer back to 31 second (skipping pattern #1)
    }

//***************************************************************************
//*********Green LEDS fade in and out in between readings********************
//***************************************************************************
    //each time the system reads a value, it changes the LED brightness.  
if(pattern == 1) {
    ledBrightBreathe = ledBrightBreathe + ledBrightBreatheMod;
    if (ledBrightBreathe <= 0 ) { ledBrightBreatheMod = 1; }
    if (ledBrightBreathe >= 255) { ledBrightBreatheMod = -1; } 
    strip.setPixelColor(0, 0, ledBrightBreathe, 0);
    strip.setPixelColor(1, 0, ledBrightBreathe, 0);    
    strip.show();
    hitValue = readSensorVal();
}

//***************************************************************************
//**********************Red Theater Crawl Up*********************************
//***************************************************************************    
if(pattern == 2) {
    float in,level;    
    for (in = 0; in < 6.283; in = in + .050) {  //last value was .025, higher is faster crawl
      hitValue = readSensorVal();
      if(hitValue > accelerometerThreshhold/2) { break; }       
      for(int i=0; i< strip.numPixels(); i++) {      
        if (hitValue > accelerometerThreshhold/2) { break; }         
        //level = sin(i+in) * 127 + 128; // "plus" sign makes it crawl down       
        level = sin(i-in) * 127 + 128; // "minus" sign makes it crawl up        
        strip.setPixelColor(i,(int)level,0,0); // set color level (red)       
      }
        strip.show();
    }
}

//***************************************************************************
//**********************Purple Theater Crawl Down****************************
//***************************************************************************    
if(pattern == 4) {
    float in,level;
    for (in = 0; in < 6.283; in = in + .050) { //last value was .025, higher is faster crawl
      hitValue = readSensorVal();
      if(hitValue > accelerometerThreshhold/2) { break; }       
      for(int i=0; i< strip.numPixels(); i++) {      
        if (hitValue > accelerometerThreshhold/2) { break; }         
        level = sin(i+in) * 127 + 128; // "plus" sign makes it crawl down       
        //level = sin(i-in) * 127 + 128; // "minus" sign makes it crawl up 
        strip.setPixelColor(i,(int)level,0,(int)level); // set color level (red)       
      }
        strip.show();
    }
}

//***************************************************************************
//******************Rainbow Cycle Crawl Down*********************************
//***************************************************************************
if(pattern == 3) {  
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      hitValue = readSensorVal();
      if(hitValue > accelerometerThreshhold/2) { break; }
      strip.setPixelColor(i, Wheel(((i * 256 / NUM_LEDS) + counter2) & 255));
    }
    counter2 = counter2 + 8; //how fast the cycle of colors - 5 seems to be a slow value    
    if (counter2 > 257) { counter2 = 0; } //reset's variable "counter2" for another spin through the color wheel
    strip.show();    
}

//***************************************************************************
//******************Lights Up & Down (in progress)***************************
//***************************************************************************
if(pattern == 5) {
  //set all lights to black
 ledsOff();
 //light them up one by one
 
 for(int iii=0; iii<NUM_LEDS; iii++) {
   strip.setPixelColor(iii, Wheel(iii*25));
//   strip.setPixelColor((iii-1), Wheel(iii*25));
//   strip.setPixelColor((iii-2), Wheel(iii*25));
//   strip.setPixelColor((iii-3), Wheel(iii*25));
//   strip.setPixelColor((iii-4), Wheel(iii*25));
   for(int ivi=0; ivi<NUM_LEDS; ivi++) {
    if (ivi == iii) { break; }
    strip.setPixelColor(ivi,0,0,0);
   }
   delay(20);
   strip.show();   // write all the pixels out 
 }

}
//***************************************************************************

   delay(2); //Minimum delay of 2 milliseconds between sensor reads (500 Hz)
} //end of Void Loop


//******************************************************
//**********ACCELEROMETER reading***********************
//******************************************************
int readSensorVal() {

    int sensorValue = 0;
    long readingx = 0;
    long readingy = 0;
    long readingz = 0;
    
    for (int i=0; i<sampleSize; i++)
    {
      readingx += analogRead(xInput);
    }
    int xRaw = readingx/sampleSize;        
    for (int i=0; i<sampleSize; i++)
    {
      readingy += analogRead(yInput);
    }
    int yRaw = readingy/sampleSize;   
    for (int i=0; i<sampleSize; i++)
    {
      readingz += analogRead(zInput);
    }
    int zRaw = readingz/sampleSize;    
    sensorValue = sqrt(xRaw*xRaw + yRaw*yRaw + zRaw*zRaw);
    if (sensorValue > accelerometerThreshhold) {
      strip.setPixelColor(0, 0, 0, 0);  //to remove green LEDS from pattern #1
      strip.setPixelColor(1, 0, 0, 0);  //to remove green LEDS from pattern #1
    }
    return sensorValue;
}

//******************************************************
//********This turns all the relays on or off***********
//******************************************************
void relayControl(uint8_t relay) {
  if(relay == 1) {
    digitalWrite(relayPin1, HIGH);
    digitalWrite(relayPin2, HIGH);
    digitalWrite(relayPin3, HIGH);
    digitalWrite(relayPin4, HIGH);
    //Serial.println("Relays are set to ON");
    
  } if (relay == 0) {
    digitalWrite(relayPin1, LOW);
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin3, LOW);
    digitalWrite(relayPin4, LOW);
    //Serial.println("Relays are set to OFF");
   }
 }

//******************************************************
// **********Animation For Winner!**********************
//******************************************************
// used for the "congrats!" animation for hitting max load
// sine wave, low (0-359),high (0-359), rate of change, wait
// entered from hitAnimation routine:  wavey(200,240,0.06,0)
void wavey(int low,int high,float rt,uint8_t wait) {
  float in,out;
  int congrats=0;
  int congratsMod=1;  
  int diff=high-low;
  int step = diff/strip.numPixels();  
  for (in = 0; in < 6; in = in + rt) {  
       for(int i=0; i< strip.numPixels(); i++) {
           out=sin(in+i*(6.283/strip.numPixels())) * diff + low;
           HSVtoRGB(out,255,255,colors);
           strip.setPixelColor(i,colors[0],colors[1],colors[2]);
       }           
       congrats = congrats + congratsMod;
       if (congrats <= 0 ) { congratsMod = 1; }
       if (congrats >= NUM_LEDS) { congratsMod = -1; }
       strip.setPixelColor(congrats,255,255,255);
       strip.setPixelColor((congrats-1),255,255,255);
       strip.setPixelColor((congrats+1),255,255,255);
       strip.show();
       delay(wait);       
  }
}
 
//******************************************************
// hitAnimation(numLedsToLight, 20);
// this animation plays when the striker is hit
//******************************************************
void hitAnimation(uint32_t lights, uint8_t wait) {
  if (lights >= NUM_LEDS) {
    for (int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor(i, 255, 255, 255);
    strip.show();
    delay(wait); 
    }
    // color wave - Hue/Sat/Bright
    // hue low (0-359), high (0-359),rate,extra delay
    for (int i=0; i<4; i++) {
      wavey(200,240,0.06,0);
      Serial.println("wavey(200,240,0.06,0);");
    }
  } else {  
    for (int i=0; i<lights; i++) {
    strip.setPixelColor(i, 255, 255, 255);
    strip.show();
    delay(wait);    
  }
  delay(1500);
  }
  ledsFadeOff(20);
  setTime(0);  
}

//******************************************************
// This turns all LEDS off at once
//******************************************************
void ledsOff() {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show(); // write all the pixels out
}

//******************************************************
// This wipes all LEDs from top to bottom
//******************************************************
void ledsFadeOff(uint8_t wait) {
  int i;
  
  for (i=strip.numPixels(); i > -1; i--) {
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
    delay(wait);
  }
}

//******************************************************
/* Helper functions */
//******************************************************

uint32_t Wheel(byte WheelPos) {
    if (WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

// HSV to RGB colors
// hue: 0-359, sat: 0-255, val (lightness): 0-255
// adapted from http://funkboxing.com/wordpress/?p=1366
void HSVtoRGB(int hue, int sat, int val, uint8_t * colors) {
    int r, g, b, base;
    if (sat == 0) { // Achromatic color (gray).
        colors[0] = val;
        colors[1] = val;
        colors[2] = val;
    } else {
        base = ((255 - sat) * val) >> 8;
        switch (hue / 60) {
        case 0:
            colors[0] = val;
            colors[1] = (((val - base) * hue) / 60) + base;
            colors[2] = base;
            break;
        case 1:
            colors[0] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            colors[1] = val;
            colors[2] = base;
            break;
        case 2:
            colors[0] = base;
            colors[1] = val;
            colors[2] = (((val - base) * (hue % 60)) / 60) + base;
            break;
        case 3:
            colors[0] = base;
            colors[1] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            colors[2] = val;
            break;
        case 4:
            colors[0] = (((val - base) * (hue % 60)) / 60) + base;
            colors[1] = base;
            colors[2] = val;
            break;
        case 5:
            colors[0] = val;
            colors[1] = base;
            colors[2] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            break;
        }

    }
}


