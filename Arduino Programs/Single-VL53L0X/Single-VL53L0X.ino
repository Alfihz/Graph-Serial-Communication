/* This example shows how to get single-shot range
 measurements from the VL53L0X. The sensor can optionally be
 configured with different ranging profiles, as described in
 the VL53L0X API user manual, to get better performance for
 a certain application. This code is based on the four
 "SingleRanging" examples in the VL53L0X API.

 The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;


// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

//#define HIGH_SPEED
#define HIGH_ACCURACY

const int buttonPin = 2;
const int ledPin = 8;
int buttonState = 0;
bool inLoop = false;
int data;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  data = 0;

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  sensor.init();
  sensor.setTimeout(500);

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
  //sensor.setMeasurementTimingBudget(000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(33000);
  //sensor.setMeasurementTimingBudget(900000);
#endif
}

void loop()
{
  buttonState = digitalRead(buttonPin);

  for(int i = 0; i < 15; i++)
    data += sensor.readRangeSingleMillimeters();

  data /= 15;
  String temp = "sData,";
  String sData = temp + data;
  Serial.print(sData);
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.println();

  if (buttonState == HIGH && inLoop == false)
  {
    Serial.print("btnState,true");
    Serial.println();
    //Serial.print(sData);
    //Serial.println();
    digitalWrite(ledPin, HIGH);
    inLoop = true;
  }
  else if (buttonState == LOW && inLoop == true)
  {
    Serial.print("btnState,false");
    Serial.println();
    digitalWrite(ledPin, LOW);
    inLoop = false;
  }
}
