/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Turn on Relay
 * Hardware : Wemos D1 R2
 ****************************************************************/

void setup() {
  // put your setup code here, to run once:
  // configure pin D2 as output
  pinMode(D2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // set pin D2 to high
  digitalWrite(D2, HIGH);
}
