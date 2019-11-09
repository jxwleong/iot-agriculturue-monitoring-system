/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Two sensors node (Soil Moisture and Humidity) with JSON
 * Hardware : NodeMCU ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 *  DHT sensor library  : Version 1.3.8
 ****************************************************************/
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

#define MAX_JSON_STRING_LENGTH  200

#define SOIL_MOSITURE_3V3_PIN   D0  // 3V3 pin for soil moisture sensor
#define SOIL_MOISTURE_PIN       A0  // Input pin for Soil Moisture Sensor 

#define DHT11_3V3_PIN      D1 // 3V3 pin for dht11 sensor
#define DHTPIN             D2 // Input Data pin for sensor
#define DHTTYPE    DHT11      // DHT 11

DHT dht(DHTPIN, DHTTYPE);

typedef struct {
  float humidity;
  float temperature;
  }DhtData;

typedef struct {
  DhtData dht11Data;
  int soilMoistureData;
  }SensorData;

/*
 * @desc: Read DHT11's humidity and temperature readings
 * @retval: Humidity and Temperature
 */    
DhtData getDhtSensorReadings(){
  DhtData dhtData;

  // Read sensor readings
  dhtData.humidity = dht.readHumidity();
  dhtData.temperature = dht.readTemperature();

  // Check whether the reading is valid or not
  if (isnan(dhtData.humidity) || isnan(dhtData.temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    //return; had to return DhtData type
  }
  return dhtData; // return the result
}

/*
 * @desc: Read soil moisture sensor reading and return
 *        it in percentage
 * @retval: percentage of soil moisture
 */
int geSoilMoistureData(){
  int soilMoisture = 0;

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  soilMoisture = map(soilMoisture,1024,0,0,100);

  return soilMoisture;
}

/*
 * @desc: Call functions to get all sensors reading
 * @retval: Soil Moisture, Humidity and Temperature
 */
SensorData getSensorData(){
  SensorData sensorData;
  // Get all sensor data
  sensorData.dht11Data = getDhtSensorReadings();
  sensorData.soilMoistureData = geSoilMoistureData();

  // Display the data
  Serial.println("");
  Serial.print("Soil Moisture: ");
  Serial.print(sensorData.soilMoistureData);
  Serial.print(" %\t");
  
  Serial.print("Temperature: ");
  Serial.print(sensorData.dht11Data.temperature);
  Serial.print(" C\t");

  return sensorData;
  }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DHT11_3V3_PIN, OUTPUT);
  pinMode(SOIL_MOSITURE_3V3_PIN, OUTPUT);
  dht.begin();
}

/*
 * @desc: Create Json Object to store data and convert it into string
 * @param: Soil Moisture and Temperature
 * @retval: String that contain necessary info for server node
 */
char *createJsonStringForSensorReadings(int soilMoisture, float temperature){

  StaticJsonBuffer<MAX_JSON_STRING_LENGTH> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["From"] = "Sensor Node 1";
  object["To"] = "Server Node";
  object["Method"] = "sendSensorReadings";
  object["Soil Moisture"] = soilMoisture;
  object["Temperature"] = temperature;

  Serial.println("\nJSON string in function:");
  object.prettyPrintTo(Serial);
  char jsonChar[MAX_JSON_STRING_LENGTH];
  object.printTo((char*)jsonChar, object.measureLength() + 1);
  return (char *)jsonChar;
  }

/*
 * @desc: Parse JSON format string and extract necessary info
 * @param: String in JSON format
 */ 
void extractDataFromJsonString(char *jsonStr){
  // need to use char array to parse else the duplication will occur and jsonBuffer will be full.
  // Ref:https://github.com/bblanchon/ArduinoJson/blob/5.x/examples/JsonParserExample/JsonParserExample.ino#L28
  char jsonBuff[MAX_JSON_STRING_LENGTH];
  strncpy (jsonBuff, jsonStr, MAX_JSON_STRING_LENGTH+1);
  
  DynamicJsonBuffer jsonBuffer;
  Serial.println("jsonString in func:");
  Serial.print(jsonBuff);
  JsonObject& root = jsonBuffer.parseObject(jsonBuff);  

  // Decode data from jsonString
  Serial.println("Data from json string: ");
  const char* from = root["From"]; 
  Serial.println(from); 
  const char* to = root["To"]; 
  Serial.println(to);
  const char* Method = root["Method"]; 
  Serial.println(Method);
  int soilMoisture = root["Soil Moisture"];
  Serial.println(soilMoisture); 
  float temperature = root["Temperature"];
  Serial.println(temperature);
  }
    
void loop() {
  // put your main code here, to run repeatedly:
  SensorData sensorData;
  char *str;
  char str1[MAX_JSON_STRING_LENGTH];
  digitalWrite(DHT11_3V3_PIN, 1);    // Turn on the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  delay(2000);
  sensorData = getSensorData();
  str = createJsonStringForSensorReadings(sensorData.soilMoistureData, sensorData.dht11Data.temperature);
  extractDataFromJsonString(str);
   // delay(1000);
  digitalWrite(DHT11_3V3_PIN, 0);    // Turn off the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 0); 
  delay(1000);

  
}
