/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Turn on Relay
 * Hardware : Wemos D1 R2
 ****************************************************************/

void setup() {
  // put your setup code here, to run once:
  // configure pin D3 as output
  pinMode(D3, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // set pin D3 to high
  digitalWrite(D3, HIGH);
}
