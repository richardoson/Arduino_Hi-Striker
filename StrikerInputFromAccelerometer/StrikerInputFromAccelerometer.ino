#include "Adafruit_WS2801.h"
#include "SPI.h"  
/*****************************************************************************
Coded by Aaron Crosby based off example sketch for driving Adafruit WS2801 pixels
Designed specifically to work with the Adafruit RGB Pixels!  Thanks Adafruit staff!
12mm Flat shape   ----> https://www.adafruit.com/products/738
NOTE:  Requires _modified_ Adafruit_WS2801 Library found HERE %%%%%GITHUB LINK%%%%%%%
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
#define CLOCK_PIN 3
#define DATA_PIN 2
#define BTN_PIN 0
#define BTN_DELAY 250
#define NUM_PATTERNS 9
#define CTR_THRESH 16


// Init Vars
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
const int sampleSize = 10;
int xRawMin = 512;
int xRawMax = 512;
int yRawMin = 512;
int yRawMax = 512;
int zRawMin = 512;
int zRawMax = 512;
uint8_t j = 0;
uint8_t pattern=1;
uint8_t buttonState=0;
uint8_t lastPix=0; 
uint8_t myPix=0;
uint8_t direction=1;
uint8_t counter=0;
uint8_t colors[3];
uint8_t relayPin1=4;
uint8_t relayPin2=5;
uint8_t relayPin3=6;
uint8_t relayPin4=7;
uint32_t setColor=0;
uint32_t xRaw=0;
uint32_t yRaw=0;
uint32_t zRaw=0;
uint32_t ledBrightBreathe=0;
uint32_t ledBrightBreatheMod=1;
uint32_t accelerometerThreshhold=100;
uint32_t difficulty=180;
long int numLedsToLight=0;
uint32_t hitValue=0;
unsigned long mark;

// Start Strip
Adafruit_WS2801 strip = Adafruit_WS2801(NUM_LEDS, DATA_PIN, CLOCK_PIN);

void setup() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
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
    //basic logic of hi-striker:
    //value from sensor is read (accelerometer)
    //value is filtered and averaged turned into variable
    //if value is legit, striker goes off standby mode for 3 minutes (?)
    //turns off relays - DONE!
    //stops all animations - DONE!
    //striker matches value of hit mapped to number of LEDs in strip - DONE!
    //white LED quickly crawls up the line - (fix, work with gravity values, quicker)
    //value remains on board for a delay period - DONE!
    //fade leds with wipe - DONE!
    //back into ready mode until no hits registered for 3 minutes (?)
    //standby mode activated, relays turn on, animations start - DONE!
    //loop

    //each time the system reads a value, it changes the LED brightness.  
    //This makes it very responsive to input from accelerometer while in ready mode.
    ledBrightBreathe = ledBrightBreathe + ledBrightBreatheMod;
    if (ledBrightBreathe <= 0 ) { ledBrightBreatheMod = 1; }
    if (ledBrightBreathe >= 255) { ledBrightBreatheMod = -1; }
    strip.setPixelColor(0, 0, ledBrightBreathe, 0);
    strip.setPixelColor(1, 0, ledBrightBreathe, 0);
    strip.show();
    
    //set timer here - if no activity for (1 minute?) it goes into standby mode

    // Read and average raw values into variable
    int xRaw = ReadAxis(xInput);
    int yRaw = ReadAxis(yInput);
    int zRaw = ReadAxis(zInput);

    //math to get acceleration data into single value
    int hitValue = sqrt(xRaw*xRaw + yRaw*yRaw + zRaw*zRaw);
    //Serial.print("using math: square root of (");
    //Serial.print(xRaw);Serial.print("*");Serial.print(xRaw);Serial.print(") + (");
    //Serial.print(yRaw);Serial.print("*");Serial.print(yRaw);Serial.print(") + (");
    //Serial.print(zRaw);Serial.print("*");Serial.print(zRaw);Serial.print(") = ");
    //Serial.println(hitValue);


    //display values if over threshhold
    if(hitValue > accelerometerThreshhold) {
      Serial.print("Nice hit!  Registered a "); Serial.print(hitValue); Serial.println(" on the scale!");
      numLedsToLight = map(hitValue, accelerometerThreshhold, difficulty, 0, NUM_LEDS); //how hard = how many lights
      hitAnimation(numLedsToLight, 50);
      //delay(1000);
      hitValue = 0;
      Serial.println("**********READY!**********");
    }    
    
        
    delay(2); //Minimum delay of 2 milliseconds between sensor reads (500 Hz) 
    

    

    /**************************************************************************************************
     ***************************commented out while working on accelerometer***************************
     **************************************************************************************************
     
    readSerialHitValue();
        
    if (hitValue == 0) { // this means no hits were registered
      //standbyMode(1); //standby mode is ON //turned off for testing purposes
      Serial.println("no hits registered, entering standby mode...");      
    }
    
    if (hitValue > 0) { // this means a hit was registered
      standbyMode(0); //standby mode is OFF
      Serial.println("hit registered, turning off standby mode...");
      numLedsToLight = map(hitValue, 0, 25, 0, NUM_LEDS); //how hard = how many pixels
      hitAnimation(numLedsToLight, 50); //animation that shows hit with bright lights
      delay(1000);
      hitValue = 0;
      //standbyMode(1); //standby mode is ON  //turned off for testing
      Serial.println("READY!");
    }
    ***************************commented out while working on accelerometer***************************
    **************************************************************************************************
    **************************************************************************************************/
} //end of Void Loop


//******************************************************
//***********ACCELEROMETER sampling input***************
//******************************************************
// Read "sampleSize" samples and report the average

int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

//******************************************************
//***********ACCELEROMETER Calibration******************
//******************************************************
// Find the extreme raw readings from each axis
//
void AutoCalibrate(int xRaw, int yRaw, int zRaw)
{
  Serial.println("Calibrate");
  if (xRaw < xRawMin)
  {
    xRawMin = xRaw;
  }
  if (xRaw > xRawMax)
  {
    xRawMax = xRaw;
  }
  
  if (yRaw < yRawMin)
  {
    yRawMin = yRaw;
  }
  if (yRaw > yRawMax)
  {
    yRawMax = yRaw;
  }
 
  if (zRaw < zRawMin)
  {
    zRawMin = zRaw;
  }
  if (zRaw > zRawMax)
  {
    zRawMax = zRaw;
  }
}
//******************************************************
// **********Animation Selection Code*******************
//******************************************************
/* pick a pattern */
void pickPattern(uint8_t var) {
      switch (var) {
        case 1:
          // scanner, color and delay - RGB
          for(int i=0; i<6; i++) {
            Serial.print("Animation 1 - Scanner - run #");
            Serial.println(i);
            scanner(strip.Color(random(255), random(100), random(255)),90);            
          }                    
        break;
        case 2:
          // color wipe random RGB
          for(int i=0; i<26; i++) {
            Serial.print("Animation 2 - colorWipe - run #");
            Serial.println(i);
            colorWipe(strip.Color(random(200), random(200), random(200)),50);            
          }
        break;
        case 3:
          // rainbow firefly, 1px at random
          for(int i=0; i<400; i++) {
          Serial.print("Animation 3 - colorFirefly - run #");
          Serial.println(i);
          colorFirefly(60);
          counter++;          
          }
        break;
        case 4:
          // rainbow solid
          for(int i=0; i<100; i++) {
          Serial.print("Animation 4 - rainbow - run #");
          Serial.println(i);
          rainbow(10);
          counter++;          
          }
        break;
        case 5:
           // bounce in and out
           // tail len, counter, delay
           for(int i=0; i<500; i++) {
           Serial.print("Animation 5 - bounceInOut - run #");
           Serial.println(i);
           bounceInOut(4,counter,20);
           counter++;           
           }
        break;
        case 6:
           // color wipe from center
           for(int i=0; i<21; i++) {
           Serial.print("Animation 6 - colorWipeCenter - run #");
           Serial.println(i);
           colorWipeCenter(strip.Color(255,0,0),100);
           colorWipeCenter(strip.Color(255,64,0),100);           
           }
        break;
        case 7:
           // solid color
           for(int i=0; i<256; i++) {
           Serial.print("Animation 7 - colorFast - run #");
           Serial.println(i);
           colorFast(strip.Color(random(255), random(255), random(255)),50);
           //colorFast(strip.Color(255,0,255),50);           
           //colorFast(strip.Color(255,255,0),50);           
           //colorFast(strip.Color(0,255,0),50);           
           }
        break;
        case 8:
          // fade even or odd
          // 0-359 Hue value, even/odd, delay
          for(int i=0; i<10; i++) {
          Serial.print("Animation 8 - fadeEveOdd - run #");
          Serial.println(i);
          //fadeEveOdd(200,0,20);
          fadeEveOdd(random(200), 0, random(20));          
          //fadeEveOdd(300,1,20);
          fadeEveOdd(random(300), 1, random(20));          
          }
        break;
        case 9:
          // show rainbow
          for(int i=0; i<100; i++) {
          Serial.print("Animation 9 - rainbowCycle - run #");
          Serial.println(i);
          rainbowCycle(10);          
          }
         break; 
      } 
} 


//******************************************************
// **********Animation Case #3 FIREFLY******************
//******************************************************
void colorFirefly(int wait) {
        if(myPix != lastPix) {
          if(counter<CTR_THRESH) {
            float colorV = sin((6.28/30)*(float)(counter)) *255;
            HSVtoRGB((359/CTR_THRESH)*counter, 255, colorV, colors);
            strip.setPixelColor(myPix, colors[0], colors[1], colors[2]);
            //myPix2=random(0,strip.numPixels());
            //strip.setPixelColor(myPix2, colors[0], colors[1], colors[2]);
            //myPix3=random(0,strip.numPixels());
            //strip.setPixelColor(myPix3, colors[0], colors[1], colors[2]);
            //strip.setPixelColor((myPix - 3), colors[0], colors[1], colors[2]); //trying to add pixels
            //strip.setPixelColor((myPix + 3), colors[0], colors[1], colors[2]); //trying to add pixels
           strip.show();
           delay(wait);           
          } else {
            lastPix=myPix;
            counter=0;
            colorFast(0,0);
          }
        } else {
          myPix=random(0,strip.numPixels());          
        }
}

//******************************************************
// **********Animation Case #2 COLORWIPE****************
//******************************************************
// Fill the dots one after the other with a color
// Modified from Neopixel sketch to break on button press
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=strip.numPixels(); i>0; i--) {
      readSerialHitValue();
      if(hitValue>0) { break; }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

//******************************************************
// **********Animation Case #6 COLORWIPECENTER**********
//******************************************************
// color wipe from center
void colorWipeCenter(uint32_t c, uint8_t wait) {
  uint8_t mid=strip.numPixels()/2;
  strip.setPixelColor(mid,c);
  for(uint16_t i=0; i<=strip.numPixels()/2; i++) {
      readSerialHitValue();
      if(hitValue>0) { break; }
      strip.setPixelColor(mid+i, c);
      strip.setPixelColor(mid-i, c);
      strip.show();
      delay(wait);
  }
}

//******************************************************
// **********Animation Case #7 COLORFAST****************
//******************************************************
// fast version 
void colorFast(uint32_t c, uint8_t wait) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        readSerialHitValue();
        if(hitValue>0) { break; }
    }
    strip.show();
    delay(wait);
}

//******************************************************
// **********Animation Case #9 RAINBOWCYCLE*************
//******************************************************
// Rainbow Cycle, modified from Neopixel sketch to break on button press
void rainbowCycle(uint8_t wait) {
    uint16_t i;

    //  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      readSerialHitValue();
      if(hitValue>0) { break; }
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    // }
}

//******************************************************
// **********Animation Case #4 RAINBOW******************
//******************************************************
void rainbow(uint8_t wait) {
    uint16_t i;

    //for(j=0; j<256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      readSerialHitValue();
      if(hitValue>0) { break; }
      //strip.setPixelColor(i, Wheel((i + j) & 255)); //original line
      strip.setPixelColor(i, Wheel((i + j) & 255));
      Serial.print("variable j =");
      Serial.println(j);
    }
    strip.show();
    delay(wait);
    // }
}

//******************************************************
// **********Animation Case #1 SCANNER******************
//******************************************************
// scanner
void scanner(uint32_t c,uint8_t wait) {
        for(int i=0; i< strip.numPixels(); i++) {
            readSerialHitValue();
            if(hitValue>0) { break; }
            colorFast(0,0);
            strip.setPixelColor(i,c);
            strip.show();
            delay(wait);
        }
        for(int i=strip.numPixels(); i>0; i--) {
           readSerialHitValue();
           if(hitValue>0) { break; }
           colorFast(0,0);
           strip.setPixelColor(i,c);
           strip.show();
           delay(wait);
        }    
}

//******************************************************
// **********Animation Case #5 BOUNCEINOUT**************
//******************************************************
// scanner to midpoint
void bounceInOut(uint8_t num, uint8_t start,uint8_t wait) {
  colorFast(0,0);
  uint8_t color=200;
  for(int q=0; q < num; q++) {
    readSerialHitValue();
    if(hitValue>0) { break; }
    if(strip.numPixels()-start >= 0 && start < NUM_LEDS) {
          strip.setPixelColor(start+q, strip.Color(0,color,0));
          strip.setPixelColor(strip.numPixels()-start-q, strip.Color(0,color,0));
      }   
    color=round(color/2.0);
    strip.show();
    delay(wait);
  }
  if(counter > strip.numPixels()) { counter=0; }
}

//******************************************************
// **********Animation Case #8 FADEEVEODD***************
//******************************************************
void fadeEveOdd(int c1,byte rem,uint8_t wait) {
              for(int j=0; j < CTR_THRESH; j++) {
                      for(int i=0; i< strip.numPixels(); i++) {
                        if(i % 2== rem) {
                           HSVtoRGB(c1,255,(255/CTR_THRESH)*j,colors);
                           strip.setPixelColor(i,colors[0],colors[1],colors[2]);
                         }
                      }           
                      readSerialHitValue();
                      if(hitValue>0) { break; }
                      strip.show();
                      delay(wait);
                }
                for(int j=CTR_THRESH; j >= 0; j--) {
                      for(int i=0; i< strip.numPixels(); i++) {
                        if(i % 2== rem) {
                           HSVtoRGB(c1,255,(255/CTR_THRESH)*j,colors);
                           strip.setPixelColor(i,colors[0],colors[1],colors[2]);
                         }
                      }             
                     readSerialHitValue();
                     if(hitValue>0) { break; }
                      strip.show();
                      delay(wait);
                } 
}

//******************************************************
// **********Animation Case #?? TWINKLERAND*************
//******************************************************
// twinkle random number of pixels
void twinkleRand(int num,uint32_t c,uint32_t bg,int wait) {
	// set background
	 colorFast(bg,0);
	 // for each num
	 for (int i=0; i<num; i++) {
    readSerialHitValue();
    if(hitValue>0) { break; }
	  strip.setPixelColor(random(strip.numPixels()),c);
	 }
	strip.show();
	delay(wait);
}

//******************************************************
// **********Animation Case #0 WAVEY********************
//******************************************************
// used for the "congrats!" animation for hitting max load
// sine wave, low (0-359),high (0-359), rate of change, wait
void wavey(int low,int high,float rt,uint8_t wait) {
  float in,out;
  int diff=high-low;
  int step = diff/strip.numPixels();
  for (in = 0; in < 6.283; in = in + rt) {
       for(int i=0; i< strip.numPixels(); i++) {
           out=sin(in+i*(6.283/strip.numPixels())) * diff + low;
           HSVtoRGB(out,255,255,colors);
           strip.setPixelColor(i,colors[0],colors[1],colors[2]);
       }
           strip.show();
           delay(wait);
           //if(hitValue>0) { break; } //commented out as this is "congrats" animation for hit value
  }
}

//******************************************************
// **********Animation Case #?? WAVEINTENSITY***********
//******************************************************
// sine wave, intensity
void waveIntensity(float rt,uint8_t wait) {
  float in,level;
  for (in = 0; in < 6.283; in = in + rt) {
       for(int i=0; i< strip.numPixels(); i++) {
        if(hitValue>0) { break; }
        // sine wave, 3 offset waves make a rainbow!
        level = sin(i+in) * 127 + 128;
        // set color level 
        strip.setPixelColor(i,(int)level,0,0);
       }
           strip.show();
           delay(wait);
           readSerialHitValue();
           if(hitValue>0) { break; }
  }
} 
//******************************************************
// hitAnimation(numLedsToLight, 20);
// this animation plays when the striker is hit
//******************************************************
void hitAnimation(uint32_t lights, uint8_t wait) {
  if (lights == NUM_LEDS) {
    for (int i=0; i<lights; i++) {
    strip.setPixelColor(i, 255, 255, 255);
    strip.show();
    delay(wait); 
    }
    // color wave - Hue/Sat/Bright
    // hue low (0-359), high (0-359),rate,extra delay
    for (int i=0; i<12; i++) {
      wavey(200,240,0.06,0);
      Serial.println("wavey(200,240,0.06,0);");
    }
  } else {  
    for (int i=0; i<lights; i++) {
    strip.setPixelColor(i, 255, 255, 255);
    strip.show();
    delay(wait);    
  }
  delay(1000);
  }
  ledsFadeOff(20);
}
//******************************************************
// Standby Mode
// 0 = standbyMode off, hi-striker is active
// 1 = standbyMode on, playing animations
//******************************************************
void standbyMode(uint8_t standbyModeVal) {
  if(standbyModeVal == 0) {
    relayControl(0);
    //ledsOff();
        
  } if(standbyModeVal == 1) {
    relayControl(1);
    pickPattern(pattern);
    pattern++;
    // if pattern greater than #pattern reset  
    if (pattern > NUM_PATTERNS) { pattern = 1; }
    // set direction
    if (direction == 1) { j++;  } else {  j--; }
    if (j > 254) { direction = 0; }
    if (j < 1) { direction = 1; }
  }
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
// This turns all the relays on or off
// relayControl(1); OR relayControl(0);
//******************************************************
void relayControl(uint8_t relay) {
  if(relay == 1) {
    digitalWrite(relayPin1, HIGH);
    digitalWrite(relayPin2, HIGH);
    digitalWrite(relayPin3, HIGH);
    digitalWrite(relayPin4, HIGH);
    Serial.println("Relays are set to ON");
    
  } if (relay == 0) {
    digitalWrite(relayPin1, LOW);
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin3, LOW);
    digitalWrite(relayPin4, LOW);
    Serial.println("Relays are set to OFF");
   }
 }


//******************************************************
// Do math to make accelerometers even more cool
//******************************************************

void doMathAccelerometer() {
    // Convert raw values to 'milli-Gs"
    long xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
    long yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
    long zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);
  
    // re-scale to fractional Gs
    float xAccel = xScaled / 1000.0;
    float yAccel = yScaled / 1000.0;
    float zAccel = zScaled / 1000.0;

    //numLedsToLight = sqrt(xAccel*xAccel + yAccel*yAccel + zAccel*zAccel); //original math
}
//******************************************************
// check if value received from serial monitor (hit value)
//******************************************************
void readSerialHitValue() {
    if (Serial.available()) {
      hitValue = Serial.parseInt();
      Serial.print("Captured: ");
      Serial.println(hitValue);
    }
}
// CODE NOTE:
// Use this to break for loops:  if(hitValue>0) { break; }

//******************************************************
// check button state  (useful for figuring out how the original author did it)
//******************************************************
boolean chkBtn(int buttonState) {
//   if (buttonState == HIGH && (millis() - mark) > BTN_DELAY) { //original
   if (buttonState == LOW && (millis() - mark) > BTN_DELAY) {     
       j = 0;
       mark = millis();
       pattern++;
       return true;
    } 
    else { return false; }
}
// CODE NOTE:
// if(chkBtn(digitalRead(BTN_PIN))) { break; }
// WAS CHANGED TO:
// if(hitValue>0) { break; }
// FOR ALL ANIMATIONS EXCEPT "wavey", which has no animation breaker

//******************************************************
/* Helper functions *///  DON'T TOUCH! (I don't know what they do!)
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



