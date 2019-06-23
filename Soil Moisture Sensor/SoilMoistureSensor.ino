int sensorPin = A0;
int outputVal;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Reading from the soil moisture sensor...");
  delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  outputVal = analogRead(sensorPin);
  outputVal = map(outputVal,550,0,0,100);
  Serial.print("Moisutre : ");
  Serial.print(outputVal);
  delay(1000);
}
