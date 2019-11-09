#include <DHT.h>
#include <DHT_U.h>

#define DHT11_3V3_PIN      D1 // 3V3 pin for sensor
#define DHTPIN             D2 // Input Data pin for sensor
#define DHTTYPE    DHT11      // DHT 11

DHT dht(DHTPIN, DHTTYPE);

typedef struct {
  float humidity;
  float temperature;
  }DhtData;

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
  Serial.println("");
  Serial.print(F("Humidity: "));
  Serial.print(dhtData.humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(dhtData.temperature);
  Serial.print(F("°C "));

  return dhtData; // return the result
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DHT11_3V3_PIN, OUTPUT);
  
 // pinMode(DHTPIN, INPUT);
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DHT11_3V3_PIN, 1); // Turn on the sensor
  delay(2000);
  getDhtSensorReadings();
  digitalWrite(DHT11_3V3_PIN, 0); // Turn off the sensor
  
  
}
