/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Send sensor readings to a centralised node
 * Hardware : NodeMCU ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 *  DHT sensor library  : Version 1.3.8
 ****************************************************************/
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <Ticker.h>
#include <DHT.h>
#include <DHT_U.h>


// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"         // WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"         // WiFi PASSWORD

#define TOKEN  "TOKEN"              	       // Device's Token address created on ThingsBoard
#define PARAMETER_SOIL_MOSITURE  "SoilMoisture"       // Parameter of device's widget on ThingsBoard
#define PARAMETER_TEMPERATURE  "Temperature"  

const char * host = "IP_ADDRESS_SERVER";     // IP Server


// Definition for ADC
#define ADC_10_BIT_RESOLUTION   1024     // resolution of ESP8266 ADC

// Global variable
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;                   // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);         // ThingsBoard
ThingsBoard tb(wifiClient);

const int httpPort = 80;                 // client port

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";   // ip or host of ThingsBoard 

char rpcCommand[128] ;          // rpc message key-in at ThingsBoard RPC remote shell
String replyFromServer;         // Reply message from server after received data

const char* mqtt_server = "ESP8266_SERVER_IP_ADDRESS";

#define MAX_MQTT_MESSAGE_LENGTH     128   // Size of message to be sent to server node



// Definition for sensors
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
  
#define MAX_JSON_STRING_LENGTH  200

/******************Sensor functions***************/  
/*
 * @desc: Read soil moisture sensor reading and return
 *        it in percentage
 * @param: Adc resoltuion, I/O pin connected to sensor
 * @retval: percentage of soil moisture
 */
int getSoilMoisturePercentage(int resolution, int sensorPin)
{
  int soilMoisture = 0;

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(sensorPin);
  soilMoisture = map(soilMoisture,resolution,0,0,100);

  return soilMoisture;
}

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
 * @desc: Call functions to get all sensors reading
 * @retval: Soil Moisture, Humidity and Temperature
 */
SensorData getSensorData(){
  SensorData sensorData;
  
  // Get all sensor data
  sensorData.dht11Data = getDhtSensorReadings();
  sensorData.soilMoistureData = getSoilMoisturePercentage(ADC_10_BIT_RESOLUTION, SOIL_MOISTURE_PIN);

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
 * @desc: Send data to server in json format
 * @param: Soil Moisture and Temperature
 */
void sendSensorReadingsToServerNode(int soilMoisture,  float temperature){

  char *message = createJsonStringForSensorReadings(soilMoisture, temperature);
  Serial.println("Publishing message:");
  Serial.println(message);
  client.publish("toServer", message);
      // ... and resubscribe
  client.subscribe("fromServer");
  }
  
/*******************WiFi functions****************/  
/*
 * @desc: Connect device to WiFi
 */  
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

/*
 * @desc: Connect device to ThingsBoard/ Reconnect to WiFi
 */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("toServer", "I'm connected");
      // ... and resubscribe
      client.subscribe("fromServer");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  client.publish("toServer", "Received command from server");
      // ... and resubscribe
  client.subscribe("fromServer");

}
// Main functions
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //delay(10);
  InitWiFi();
  pinMode(DHT11_3V3_PIN, OUTPUT);
  pinMode(SOIL_MOSITURE_3V3_PIN, OUTPUT);

  digitalWrite(DHT11_3V3_PIN, 1);    // Turn on the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
 if ( !client.connected() ) {
    reconnect();
  }

  client.loop();

 delay(5000);
  sendSensorReadingsToServerNode(12, 30.5);
}


// References 
// [1.] Arduino | Communication Two LoLin NodeMCU V3 ESP8266 (Client Server) for Controlling LED
//      https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s
// [2.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
// [3.] Arduino and Soil Moisture Sensor -Interfacing Tutorial
//      http://www.circuitstoday.com/arduino-soil-moisture-sensor
