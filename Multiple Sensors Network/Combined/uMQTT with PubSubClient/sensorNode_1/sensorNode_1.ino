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
#include "SDHT.h"


// Definition for WiFi
#define WIFI_AP "HUAWEI nova 2i"         // WiFi SSID
#define WIFI_PASSWORD "pdk47322"         // WiFi PASSWORD

const char * host = "IP_ADDRESS_SERVER";     // IP Server

// Definition for ADC
#define ADC_10_BIT_RESOLUTION   1024     // resolution of ESP8266 ADC

// Global variable
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;                   // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);         // ThingsBoard

const char* mqtt_server = "192.168.43.21";

#define MAX_MQTT_MESSAGE_LENGTH     128   // Size of message to be sent to server node


// Definition for sensors
#define SOIL_MOSITURE_3V3_PIN   D0  // 3V3 pin for soil moisture sensor
#define SOIL_MOISTURE_PIN       A0  // Input pin for Soil Moisture Sensor 

#define DHT11_3V3_PIN      D1 // 3V3 pin for dht11 sensor
#define DHTPIN             D2 // Input Data pin for sensor
#define DHTTYPE    DHT11      // DHT 11

SDHT dht;
typedef struct {
  float humidity;
  float temperature;
  }DhtData;

typedef struct {
  DhtData dht11Data;
  int soilMoistureData;
  }SensorData;
  
typedef enum{
  SENSOR_OFF = LOW,
  SENSOR_ON = HIGH,
  }SensorMode;  

Ticker sendDataToServer;

#define MAX_JSON_STRING_LENGTH  200

// Definition for timer
#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

#define DEFAULT_DATA_SAMPLING_RATE_MS  2000

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

  if (dht.read(DHT11, DHTPIN)){ // If no eror then status return 1
      dhtData.temperature= (double(dht.celsius) / 10);
      dhtData.humidity = (double(dht.humidity) / 10);
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

/*
 * @desc: Turn on/off 3V3 supply for sensors
 * @param: sensorMode (on/ off)
 */
void sensorPowerSwitch(SensorMode sensorMode){
  digitalWrite(DHT11_3V3_PIN, sensorMode);    // Switch sensor mode
  digitalWrite(SOIL_MOSITURE_3V3_PIN, sensorMode);
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
    String clientId = "Sensor2-";
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
/*******************Timer functions****************/
/*
 * @desc: Calcuate the number of ticks to write into timer
 * @param: CPU frequency, frequency divider, timer in milliseconds
 * @retval: number of ticks for achieve desired time
 */
uint32_t getTimerTicks(uint32_t freq, int freqDivider, int milliSeconds){
  uint32_t  dividedFreq = 0;
  float period = 0, ticks = 0;
  dividedFreq = freq/ freqDivider;
  period = (float)(1) / (float)(dividedFreq);
  ticks = ((float)(milliSeconds)/(float)(1000) / period);
  // get the value in milliSeconds then div by period
  return ticks;
  }

void ISR_FUNC(){
  SensorData sensorData;
  Serial.println("Times up!");
  sensorData = getSensorData();
  sendSensorReadingsToServerNode(sensorData.soilMoistureData, sensorData.dht11Data.temperature);
  }
  

// Init functions
void InitSensor(){
  pinMode(DHT11_3V3_PIN, OUTPUT);
  pinMode(SOIL_MOSITURE_3V3_PIN, OUTPUT);
  digitalWrite(DHT11_3V3_PIN, 1);    // Turn on the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  }
  
// Main functions
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //delay(10);
  InitWiFi();
  InitSensor();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  sendDataToServer.attach_ms(DEFAULT_DATA_SAMPLING_RATE_MS,  ISR_FUNC);

}

void loop() {
  // put your main code here, to run repeatedly:
 if ( !client.connected() ) {
    reconnect();
  }

  client.loop();

}

// References 
// [1.] Arduino | Communication Two LoLin NodeMCU V3 ESP8266 (Client Server) for Controlling LED
//      https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s
// [2.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
// [3.] Arduino and Soil Moisture Sensor -Interfacing Tutorial
//      http://www.circuitstoday.com/arduino-soil-moisture-sensor
