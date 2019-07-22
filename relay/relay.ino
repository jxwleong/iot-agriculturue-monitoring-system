void setup() {
  // put your setup code here, to run once:
  // configure the GPIO pin
  pinMode(5, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // set the GPIO pin to HIGH or LOW to turn on/
  // off the relay
  digitalWrite(5, HIGH);
}
