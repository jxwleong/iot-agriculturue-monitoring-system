// current sensor connected to A0, 5volt and ground

float numReadings = 64; // number of readings for averaging
unsigned int total; // total A/D readings
float offset = 511.49; // >>>calibrate zero current here<<<
float span = 0.0651; // >>>calibrate span here<<<
float amps; // final current

void setup() {
  Serial.begin(9600);
}

void loop() {
  total = 0; // reset
  for (int x = 0; x < numReadings; x++) { // read sensor multiple times
    total = total + analogRead(A0); // add each value
  }
  amps = ((total / numReadings) - offset) * span;
  Serial.println(amps);
  delay(100); // use  millis() timing in final code
}
