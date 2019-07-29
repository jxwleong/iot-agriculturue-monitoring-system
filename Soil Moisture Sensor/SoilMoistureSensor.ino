int sensorPin = A0;
int outputVal;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Reading from the soil moisture sensor...");
  delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  outputVal = analogRead(sensorPin);
  outputVal = map(outputVal,1024,0,0,100);
  Serial.print("\nMoisutre : ");
  Serial.print(outputVal);
  Serial.println("%");
  delay(1000);
}
