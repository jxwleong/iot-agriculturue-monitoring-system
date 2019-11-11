#include "SDHT.h"

SDHT dht;

#define DHT11_3V3_PIN      D1 // 3V3 pin for dht11 sensor
#define DHTPIN             D2 // Input Data pin for sensor

void setup() {
  // put your setup code here, to run once:
  pinMode(DHT11_3V3_PIN, OUTPUT);
  digitalWrite(DHT11_3V3_PIN, HIGH);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (dht.read(DHT11, DHTPIN)) layout();
  delay(2000);
}

void layout() {
  Serial.print("   Celsius => ");
  Serial.println(String(double(dht.celsius) / 10, 1));
  Serial.print("   Humdity => ");
  Serial.println(String(double(dht.humidity) / 10, 1));
}
