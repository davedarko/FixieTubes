/*
  Shift Register Example
  Turning on the outputs of a 74HC595 using an array

 Hardware:
 * 74HC595 shift register 
 * LEDs attached to each of the outputs of the shift register

https://github.com/JChristensen/DS3232RTC
http://playground.arduino.cc/Code/Time

 */
 
#include "stuff.h"
#include <DS3232RTC.h>
#include <Wire.h>
#include <Time.h>

char formatted[] = "00-00-00 00:00:00x";

//Pin connected to ST_CP of 74HC595
int latchPin = 12;
//Pin connected to SH_CP of 74HC595
int clockPin = 8;
////Pin connected to DS of 74HC595
int dataPin = 11;
int pwmPin = 3;

int ft_year = 2014;
int ft_month = 01;
int ft_day = 24;
int ft_hours = 23;
int ft_minut = 45;
int ft_seconds = 40;

long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;           // interval at which to blink (milliseconds)

//holders for infromation you're going to pass to shifting function
byte data1;
byte data2;
byte data3;
byte data4;

void setup() {
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()) {
    processCommand();
  }
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    update_data();
    if (
      (ft_hours < 8) || (ft_hours > 22))
    {
      digitalWrite(pwmPin, LOW);  
    }
    else
    {
      analogWrite(pwmPin, 12);
    }
  }

  int h2 = ft_hours%10;
  int h1 = (ft_hours-h2)/10;

  int m2 = ft_minut%10;
  int m1 = (ft_minut-m2)/10;

  for (int j = 0; j < 5; j++) {
     //load the light sequence you want from array
    data1 = cipherArrayBIG[h1][j];
    data2 = cipherArrayBIG[h2][j];
    data3 = cipherArrayBIG[m1][j];
    data4 = cipherArrayBIG[m2][j];
    
    //ground latchPin and hold low for as long as you are transmitting
    digitalWrite(latchPin, 0);
    //move 'em out
    shiftOut(dataPin, clockPin, data1);
    shiftOut(dataPin, clockPin, data2);
    shiftOut(dataPin, clockPin, data3);
    shiftOut(dataPin, clockPin, data4);    
    //return the latch pin high to signal chip that it 
    //no longer needs to listen for information
    digitalWrite(latchPin, 1);
    delayMicroseconds(20);
  }
  /*
  for (int i = 0; i < 16; i++) {
    digitalWrite(latchPin, 0);
    //move 'em out
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);    
    //return the latch pin high to signal chip that it 
    //no longer needs to listen for information
    digitalWrite(latchPin, 1);
    delayMicroseconds(100);
  }*/
}


// the heart of the program
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {  
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}


//blinks the whole register based on the number of times you want to 
//blink "n" and the pause between them "d"
//starts with a moment of darkness to make sure the first blink
//has its full visual effect.
void blinkAll_2Bytes(int n, int d) {
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, 0);
  shiftOut(dataPin, clockPin, 0);
  digitalWrite(latchPin, 1);
  delay(200);
  for (int x = 0; x < n; x++) {
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 255);
    shiftOut(dataPin, clockPin, 255);
    digitalWrite(latchPin, 1);
    delay(d);
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    digitalWrite(latchPin, 1);
    delay(d);
  }
}

void processCommand() {
  if(!Serial.available()) { return; }
  char command = Serial.read();
  tmElements_t tm;
  int in,in2;
  switch(command)
  {
    case 'q':
      RTC.squareWave(SQWAVE_1_HZ);
      Serial.println("Square wave output set to 1Hz");
    break;
    
    case 'h':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Hour = in;
      RTC.write(tm);  
      print_data();    
    break;

    case 'i':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Minute = in;
      RTC.write(tm); 
      print_data();     
    break;    
    
    case 's':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Second = in;
      RTC.write(tm); 
      print_data();     
    break;
    
    case 'y':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Year = in+ 30;
      RTC.write(tm);      
      print_data();
    break;

    case 'm':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Month = in;
      RTC.write(tm);      
      print_data();
    break;    
    
    case 'd':
      in=SerialReadPosInt();
      RTC.read(tm);
      tm.Day = in;
      RTC.write(tm);
      print_data();
    break;

    case 'g':
      print_data();
    break;
    
    default:
      Serial.println(" h## - set Hours       d## - set Date");
      Serial.println(" i## - set mInutes     m## - set Month");
      Serial.println(" s## - set Seconds     y## - set Year");
      Serial.println(" q   - SQW/OUT = 1Hz");
      Serial.println();
    break;
  }//switch on command
  
}

void update_data() 
{
  time_t myTime;
  myTime = RTC.get();
  tmElements_t tm;
  RTC.read(tm);
  
  ft_year = 1970+tm.Year;
  ft_month = tm.Month;
  ft_day = tm.Day;
  ft_hours = tm.Hour;
  ft_minut = tm.Minute;
  ft_seconds = tm.Second;
}

void print_data()
{
  update_data();
  Serial.println();
  Serial.print(ft_year);
  Serial.print("-");
  Serial.print(ft_month);
  Serial.print("-");
  Serial.print(ft_day);
  Serial.print(" ");
  Serial.print(ft_hours);
  Serial.print(":");
  Serial.print(ft_minut);
  Serial.print(":");
  Serial.print(ft_seconds);
  Serial.println();
}

//read in numeric characters until something else
//or no more data is available on serial.
int SerialReadPosInt() 
{
  int i = 0;
  boolean done=false;
  while(Serial.available() && !done)
  {
    char c = Serial.read();
    if (c >= '0' && c <='9')
    {
      i = i * 10 + (c-'0');
    }
    else 
    {
      done = true;
    }
  }
  return i;
}
