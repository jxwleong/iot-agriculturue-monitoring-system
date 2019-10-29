/****************************************************************
 * Original Author  : Uteh Str (https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s)
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Server node
 * Hardware : LoLin NodeMCU V3 ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 *  uMQTTBroker : https://github.com/martin-ger/uMQTTBroker
 ****************************************************************/
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <MQTT.h>
#include <uMQTTBroker.h>

/*
 * uMQTTBroker demo for Arduino (C++-style)
 * 
 * The program defines a custom broker class with callbacks, 
 * starts it, subscribes locally to anything, and publishs a topic every second.
 * Try to connect from a remote client and publish something - the console will show this as well.
 */

#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"

/*
 * Your WiFi config here
 */
char ssid[] = "HUAWEI nova 2i";     // your network SSID (name)
char pass[] = "pdk47322"; // your network password
bool WiFiAP = false;      // Do yo want the ESP as AP?

/*
 * Custom broker class with overwritten callback functions
 */
class myMQTTBroker: public uMQTTBroker
{
public:
    virtual bool onConnect(IPAddress addr, uint16_t client_count) {
      Serial.println(addr.toString()+" connected");
      return true;
    }
    
    virtual bool onAuth(String username, String password) {
      Serial.println("Username/Password: "+username+"/"+password);
      return true;
    }
    
    virtual void onData(String topic, const char *data, uint32_t length) {
      char data_str[length+1];
      os_memcpy(data_str, data, length);
      data_str[length] = '\0';
      
      Serial.println("received topic '"+topic+"' with data '"+(String)data_str+"'");
    }
};

myMQTTBroker myBroker;

/*
 * WiFi init stuff
 */
void startWiFiClient()
{
  Serial.println("Connecting to "+(String)ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void startWiFiAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  Serial.println("AP started");
  Serial.println("IP address: " + WiFi.softAPIP().toString());
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Start WiFi
  if (WiFiAP)
    startWiFiAP();
  else
    startWiFiClient();

  // Start the broker
  Serial.println("Starting MQTT broker");
  myBroker.init();

/*
 * Subscribe to topic
 */
  myBroker.subscribe("inTopic");
  myBroker.subscribe("outTopic");
  myBroker.subscribe("client1");

}

int counter = 0;

void loop()
{
/*
 * Publish the counter value as String
 */
   myBroker.publish("inTopic", "This message is for every clients...");
  // wait a second
  delay(1000);
   //myBroker.publish("client1", "This message is for client1");
}
