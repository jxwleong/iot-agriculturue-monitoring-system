/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Send sensor readings to a centralised node
 * Hardware : NodeMCU ESP8266
 ****************************************************************/
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <Ticker.h>

// Definition for WiFi
#define WIFI_AP "HUAWEI nova 2i"
#define WIFI_PASSWORD "pdk47322"

const char * host = "192.168.43.90";        // IP Server

// Definition for ADC
#define ADC_10_BIT_RESOLUTION   1024

// Global variable
int status = WL_IDLE_STATUS;

WiFiClient client;

const int httpPort = 80;

/*
 * @desc: Read soil moisture sensor reading and return
 *        it in percentage
 * @param: Adc resoltuion, I/O pin connected to sensor
 * @retval: percentage of soil moisture
 */
int getSoilMoisturePercentage(int resolution, int sensorPin)
{
  int adcVal = 0;
  Serial.println("\nCollecting soil moisture data...");

  // Read the Soil Mositure Sensor readings
  adcVal = analogRead(sensorPin);
  adcVal = map(adcVal,1024,0,0,100);

  Serial.print("Soil Moisture: ");
  Serial.print(adcVal);
  Serial.print(" %\t");
  return adcVal;
}

/*
 * @desc: send soil moisture percentage to server node
 */
void sendSoilMoistureToServerNode(int soilMoisture){
  Serial.println("Sending data...");
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // We now create a URI for the request  
  Serial.print("Requesting URL : ");
  Serial.println(soilMoisture);

  // This will send the request to the server
  client.print(soilMoisture);
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {      
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    Serial.print("Server Reply = "); 
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
  
/*
 * @desc: Read soil moisture sensor reading and sent to
 *        server node
 */
void getAndSendSoilMoistureToServerNode()
{
  int soilMoisture = 0;
  int sensorPin = A0;

  soilMoisture = getSoilMoisturePercentage(ADC_10_BIT_RESOLUTION, sensorPin);
  sendSoilMoistureToServerNode(soilMoisture);
  
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
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
   
  }
}

// Main functions
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  InitWiFi();

}

void loop() {
  // put your main code here, to run repeatedly:
 
  getAndSendSoilMoistureToServerNode();
  

}


// References 
// [1.] Arduino | Communication Two LoLin NodeMCU V3 ESP8266 (Client Server) for Controlling LED
//      https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s
// [2.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
// [3.] Arduino and Soil Moisture Sensor -Interfacing Tutorial
//      http://www.circuitstoday.com/arduino-soil-moisture-sensor
