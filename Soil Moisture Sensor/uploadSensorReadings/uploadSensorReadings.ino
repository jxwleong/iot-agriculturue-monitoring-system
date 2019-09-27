/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Send Soil Moisture readings to ThingsBoard
 * Hardware : NodeMCU ESP8266
 ****************************************************************/

#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

#define TOKEN "ADDRESS_TOKEN"

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

ThingsBoard tb(wifiClient);

// Global variable
int status = WL_IDLE_STATUS;
unsigned long lastSend;

int sensorPin = A0;
int soilMoisture;


/*
 * @desc: Read soil moisture sensor reading and upload to
 *        ThingsBoard
 * @ref: [1.], [2.]
 */
void getAndSendSoilMoistureData()
{
  Serial.println("Collecting soil moisture data.");

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(sensorPin);
  soilMoisture = map(soilMoisture,1024,0,0,100);

  Serial.println("Sending data to ThingsBoard:");
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(" %\t");

  tb.sendTelemetryFloat("Soil Moisture", soilMoisture);
}

/*
 * @desc: Connect device to WiFi
 * @ref: [1.]
 */
void InitWiFi()
{
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
 * @ref: [1.]
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
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

// Main functions
void setup()
{
  Serial.begin(115200);
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}

void loop()
{
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendSoilMoistureData();
    lastSend = millis();
  }

  tb.loop();
}

// References
// [1.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
// [2.] Arduino and Soil Moisture Sensor -Interfacing Tutorial
//      http://www.circuitstoday.com/arduino-soil-moisture-sensor
