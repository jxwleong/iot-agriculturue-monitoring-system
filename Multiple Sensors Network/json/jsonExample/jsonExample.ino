// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License
// Source: https://github.com/bblanchon/ArduinoJson/blob/5.x/examples/JsonParserExample/JsonParserExample.ino
// This is just an test .ino to understand ArduinoJson v5.13.5
#include <ArduinoJson.h>
#include <stdlib.h>

#define MAX_JSON_STRING_LENGTH    200

void setup() {
  Serial.begin(115200);
  
}
char *createJsonStringForSensorReadings(int soilMoisture, float temperature){
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["From"] = "Sensor Node 1";
  object["To"] = "Server Node";
  object["Method"] = "sendSensorReadings";
  object["Soil Moisture"] = soilMoisture;
  object["Temperature"] = temperature;

  Serial.println("JSON string in function:");
  object.prettyPrintTo(Serial);
  char jsonChar[MAX_JSON_STRING_LENGTH];
  object.printTo((char*)jsonChar, object.measureLength() + 1);
  
  return jsonChar;
  }

void loop() {
  // not used in this example

delay(1000);

Serial.println("Return from function");
Serial.println(createJsonStringForSensorReadings(12,23.2));

const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3) + 60;
DynamicJsonBuffer jsonBuffer(capacity);

const char* json = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.75608,2.302038]}";

JsonObject& root = jsonBuffer.parseObject(json);

const char* sensor = root["sensor"]; // "gps"
long time = root["time"]; // 1351824120

Serial.println(sensor);

}
