#include "LPD8806.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma
#ifdef __AVR_ATtiny85__
 #include <avr/power.h>
#endif

// Number of RGB LEDs in strand:
int nLEDs = 59;
int ledvs [59][3];
bool held [59];
// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 2;
int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);
byte incomingByte;
byte note;
byte velocity;
int chan1 = 1;  //specify what MIDI chan1nel we're listing to
int chan2 = 2;
int action=2; //0 =note off ; 1=note on ; 2= null
const int lowNote = 36; //what's the first note?  36 is C1 in Ableton

void setup() {
Serial.begin(115200);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();
}

void loop() {
  delay(1);
if(Serial.available() > 0 ){

  if (Serial.available() > 0) {
// read the incoming byte:
    incomingByte = Serial.read();
 
    // wait for as status-byte, chan1nel 1, note on or off
    if (incomingByte== 143+chan1){ // note on message starting starting
      action=1;
    }else if (incomingByte== 127+chan1){ // note off message starting
      action=0;
 
    }else if ( (action==0)&&(note==0) ){ // if we received a "note off", we wait for which note (databyte)
      note=incomingByte;

      setColor(note,0,127,0);
      strip.show();   // write all the pixels out
      held[note] = false;
      note=0;
      velocity=0;
      action=2;
    }else if ( (action==1)&&(note==0) ){ // if we received a "note on", we wait for the note (databyte)
      note=incomingByte;
    }else if ( (action==1)&&(note!=0) ){ // ...and then the velocity
      velocity=incomingByte;
      setColor(note,127,velocity,velocity);
      strip.show();   // write all the pixels out
      held[note] = true;
      
      note=0;
      velocity=0;
      action=0;
      
    }


    // wait for as status-byte, chan1nel 2, note on or off
    else if (incomingByte== 143+chan2){ // note on message starting starting
      action=4;
    }else if (incomingByte== 127+chan2){ // note off message starting
      action=3;
 
    }else if ( (action==3)&&(note==0) ){ // if we received a "note off", we wait for which note (databyte)
      note=incomingByte;

      
      note=0;
      velocity=0;
      action=5;
    }else if ( (action==4)&&(note==0) ){ // if we received a "note on", we wait for the note (databyte)
      note=incomingByte;
    }else if ( (action==4)&&(note!=0) ){ // ...and then the velocity
      velocity=incomingByte;
      
      if (note == 36)
        setSection(0,30,0,50,50);
      else if (note == 38)
        setSection(30,59,50,0,0);  
      else if (note == 42)
        setSection(25,35,50,25,0);          
      strip.show();   // write all the pixels out
      
      note=0;
      velocity=0;
      action=3;
    }else{
      //nada
    }
    
  }

  
}

  //Fade out
  for (int i = 0; i < nLEDs; i++) {
    
    if (ledvs[i][0] > 0 && !held[i]) 
        ledvs[i][0]--;
    if (ledvs[i][1] > 0 && !held[i]) 
        ledvs[i][1]--;
    if (ledvs[i][2] > 0 && !held[i]) 
        ledvs[i][2]--;
    setColor(i);
   }
    strip.show();   // write all the pixels out

}

void setColor(int light, int r, int g, int b){
      if(light<2 || light > nLEDs)
        return;
  
      ledvs [light][0] = r;
      ledvs [light][1] = g;
      ledvs [light][2] = b;
      strip.setPixelColor(light, strip.Color(r,g,b));        

  }
  void addColor(int light, int r, int g, int b){
    if(light<2 || light > nLEDs)
        return;
  
      ledvs [light][0] = min((r+ ledvs[light][0]), 255);
      ledvs [light][1] = min((g+ ledvs[light][1]), 255);
      ledvs [light][2] = min((b+ ledvs[light][2]), 255);

      setColor(light);
  }
void setColor(int light){
   if(light<2 || light > nLEDs)
        return;
  
        strip.setPixelColor(light, strip.Color(ledvs [light][0],ledvs [light][1],ledvs [light][2]));        

  }

void setStrip(int r, int g, int b){
    for (int i = 0; i < nLEDs; i++) {
       setColor(i,r,g,b);        
    }
}

 
void setSection(int start, int e, int r, int g, int b){
  if (start < 0 || e > nLEDs || e < start)
    return;
  for (int i = start; i < e; i++) {
      if(!held[i])
       setColor(i,r,g,b);        
    }
}

