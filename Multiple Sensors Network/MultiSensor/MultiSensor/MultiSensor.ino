#include "DHT.h"
#include <DHT_U.h>


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

  // Display the sensor value
  /*
  Serial.println("");
  Serial.print(F("Humidity: "));
  Serial.print(dhtData.humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(dhtData.temperature);
  Serial.print(F("°C "));
*/
  return dhtData; // return the result
  
}

int geSoilMoistureData(){
  int soilMoisture = 0;

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  soilMoisture = map(soilMoisture,1024,0,0,100);

  return soilMoisture;
}

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
  }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DHT11_3V3_PIN, OUTPUT);
  pinMode(SOIL_MOSITURE_3V3_PIN, OUTPUT);
 // pinMode(DHTPIN, INPUT);
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DHT11_3V3_PIN, 1);    // Turn on the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  delay(2000);
  getSensorData();
  digitalWrite(DHT11_3V3_PIN, 0);    // Turn off the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  
  
}
