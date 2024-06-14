/*
  Use the Qwiic Scale to read load cells and scales
  By: Nathan Seidle @ SparkFun Electronics
  Date: March 3rd, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).

  The Qwiic Scale is an I2C device that converts analog signals to a 24-bit
  digital signal. This makes it possible to create your own digital scale
  either by hacking an off-the-shelf bathroom scale or by creating your
  own scale using a load cell.

  This example merely outputs the raw data from a load cell. For example, the
  output may be 25776 and change to 43122 when a cup of tea is set on the scale.
  These values are unitless - they are not grams or ounces. Instead, it is a
  linear relationship that must be calculated. Remeber y = mx + b?
  If 25776 is the 'zero' or tare state, and 43122 when I put 15.2oz of tea on the
  scale, then what is a reading of 57683 in oz?

  (43122 - 25776) = 17346/15.2 = 1141.2 per oz
  (57683 - 25776) = 31907/1141.2 = 27.96oz is on the scale

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15242

  Hardware Connections:
  Plug a Qwiic cable into the Qwiic Scale and a RedBoard Qwiic
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 9600 baud to see the output
*/
#define CURRENT 26
#define VOLTAGE 27
#define bit2g 372
#define ESC_PIN 29
#define SERVO_MIN 1100
#define SERVO_MAX 2000
#define MAX_CURRENT 30.0
#define BATT_LOW_TIMER 10


#include <Wire.h>
#include <Servo.h>
#include <Adafruit_INA219.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"


NAU7802 myScale; //Create instance of the NAU7802 class

Adafruit_INA219 pmeter;

Servo servo;
int servo_signal = SERVO_MIN;

bool end_test = false;

long zeroThrust;
unsigned long counter= 0;
unsigned long mtimer = 0;
unsigned long ptimer = 0;

void setup()
{
  Serial.begin(9600);
  delay(2000);
  Serial.println("Starting");

  Wire.begin();
  
  if (myScale.begin() == false)
  {
    Serial.println("Scale not detected.");
    while (1);
  }

  if (pmeter.begin() == false){
    Serial.println("Power meter not detected.");
    while(1);
  }

  servo.attach(ESC_PIN);
  
  zeroThrust = 0;
  while (myScale.available() == false) {
    Serial.println("zeroing scale ...");
    delay(200);
  }
  zeroThrust = myScale.getReading();
  Serial.println(zeroThrust);

  servo.writeMicroseconds(0);
  delay(2000);
}

void loop()
{
  //user input
  if (Serial.read() == 'x'){
    end_test = true;
  }
  else if (Serial.read() == 'a'){
    zeroThrust = myScale.getReading();
    servo_signal = SERVO_MIN;
    servo.writeMicroseconds(servo_signal);
    end_test = false;
  }
  
  //update PWM @ 20 Hz
  // was 100
  if (millis() - ptimer > 200){
    ptimer = millis();
    if (end_test == true){
      if (servo_signal != 0){
        servo_signal = 0;
        servo.writeMicroseconds(servo_signal);
      }
    }
    else if (servo_signal == SERVO_MAX){
      end_test = true;
    }
    else{
      servo_signal += 1;
      servo.writeMicroseconds(servo_signal);
    }
  }
    
  //update MEASUREMENTS @ 10 Hz
  if (millis() - mtimer > 100){
    counter ++;
    mtimer = millis();
    
    if (myScale.available() == true)
    {
      //Calculate Thrust
      long rawThrust = myScale.getReading() - zeroThrust;
      int thrustg = rawThrust / bit2g;
      float busVoltage = 0;
      float current = 0;
      float power = 0;
    
      busVoltage = pmeter.getBusVoltage_V();
      current = pmeter.getCurrent_mA() / 50;
      power = pmeter.getPower_mW() / 50;

      if (end_test == false){
        Serial.print(counter);
        Serial.print(" th "); Serial.print(thrustg);
        Serial.print(" v "); Serial.print(busVoltage);
        Serial.print(" c "); Serial.print(current);
        Serial.print(" p "); Serial.print(power);
        Serial.print(" s "); Serial.println(servo_signal);
      }
    
      if (current >= MAX_CURRENT){
        end_test = true;
      }
    }
  }
}
