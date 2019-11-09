// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#include <ArduinoJson.h>
#include <stdlib.h>

void setup() {
  Serial.begin(115200);
  
}
char *createJsonStringForSensorReadings(int soilMoisture, int temperature){

  String soilMositureInStr = String(soilMoisture);
  String temperatureInStr = String(temperature);
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["From"] = "Sensor Node 1";
  object["To"] = "Server Node";
  object["Method"] = "sendSensorReadings";
  object["Soil Moisture"] = soilMositureInStr;
  object["Temperature"] = temperatureInStr;

  Serial.println("JSON string in function:");
  object.prettyPrintTo(Serial);
  char jsonChar[200];
  object.printTo((char*)jsonChar, object.measureLength() + 1);
  
  return jsonChar;
  }

void loop() {
  // not used in this example

delay(1000);

Serial.println("Return from function");
Serial.println(createJsonStringForSensorReadings(12,23));

}
