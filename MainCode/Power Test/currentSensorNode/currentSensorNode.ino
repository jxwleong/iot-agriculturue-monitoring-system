/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Original Author : ahmadSum1(https://github.com/ahmadSum1)
 * Contact : jason9829@live.com
 * Title : Measure the current of load by using ACS712 current
 *         sensor
 * Hardware : Wemos D1 R2 
 *            ACS712 current sensor (Required 5V)
 * Library Version:
 *  Modified Library: From: (https://github.com/rkoptev/ACS712-arduino)
 *                    To: (https://github.com/jason9829/Arduino_ACS712)
 ****************************************************************/
//-----------------------------LIBRARY---------------------------------
#include <ACS712.h>


//----------------------------DEFINITION--------------------------------
// GPIO pin connected to current sensor
#define CURRENT_SENSOR_PIN    A0


//-------------------------GLOBAL VARIABLE------------------------------
float current = 0;     // variable to store current
float volts = 0;
ACS712 sensor(ACS712_05B, CURRENT_SENSOR_PIN);


// Main functions
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // calibrate() method calibrates zero point of sensor,
  // It is not necessary, but may positively affect the accuracy
  // Ensure that no current flows through the sensor at this moment
  // If you are not sure that the current through the sensor will not leak during calibration - comment out this method
  Serial.println("\nCalibrating... Ensure that no current flows through the sensor at this moment");
  int zero = sensor.calibrate();
  
  sensor.setSensitivity(0.185);
  sensor.setVoltageReference(3.1);
  Serial.println("Done!");
  Serial.println(String("Zero point for this sensor = ") + zero);
}

void loop() {

  // put your main code here, to run repeatedly:
  current = sensor.getCurrentDC();
  volts = sensor.getVoltage();
  // Send it to serial
  Serial.println(String("I = ") + current + " A" + String("\tV = ") + volts + " V");
 
  // Wait a second before the new measurement
  delay(1000);
  
}
