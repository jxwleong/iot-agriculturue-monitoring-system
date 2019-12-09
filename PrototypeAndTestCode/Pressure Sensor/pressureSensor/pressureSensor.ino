#include <ESP8266WiFi.h>


// For ESP8266, the input voltage range of ADC
// is 3.3 V

// For the pressure sensor,
// 0.5 V -> 0 PSI
// 4.5 V -> 150 PSI
// But for the ADC the max input is 3.3V

#define ESP8266_ADC_MAX_VOLT  3.3
#define ESP8266_ADC_SCALE   1024
#define PRESSURE_SENSOR_MIN_ADC_VALUE   (0.5 / ESP8266_ADC_MAX_VOLT) * ESP8266_ADC_SCALE // 0.5 V in ADC value
#define PRESSURE_SENSOR_MAX_ADC_VALUE   (3.3 / ESP8266_ADC_MAX_VOLT) * ESP8266_ADC_SCALE // MAX V in ADC value

int analogPin = A0; // potentiometer wiper (middle terminal) connected to analog pin 3
                    // outside leads to ground and +5V
int adcVal = 0;     // variable to store the value read

void setup() {
  Serial.begin(115200);           //  setup serial
}

int timeSinceLastRead = 0;

void loop() {
  if(timeSinceLastRead > 5000) {
  adcVal = analogRead(analogPin);  // read the input pin
  int psi = ((adcVal-PRESSURE_SENSOR_MIN_ADC_VALUE)*150)/(PRESSURE_SENSOR_MAX_ADC_VALUE-PRESSURE_SENSOR_MIN_ADC_VALUE);
  Serial.println(psi);          // debug value
  timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}
