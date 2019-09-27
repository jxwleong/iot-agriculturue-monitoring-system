/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Read Soil Moisture Sensor
 * Hardware : NodeMCU ESP8266 
 *****************************************************************/

// Global variable
int sensorPin = A0;
int outputVal;

// Main functions
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Reading from the soil moisture sensor...");
  delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  outputVal = analogRead(sensorPin);
  // The adc on NodeMCU is 10-bit thus the resolution is
  // 1024
  outputVal = map(outputVal,1024,0,0,100);
  Serial.print("\nMoisutre : ");
  Serial.print(outputVal);
  Serial.println("%");
  delay(1000);
}
