/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Control Relay switch and show status on ThingsBoard
 * Hardware : Wemos D1 R2
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"         // WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"         // WiFi PASSWORD

#define TOKEN  "ADDRESS_TOKEN"              // Device's Token address created on ThingsBoard

// GPIO definition

#define RELAY_IO    D3
#define RELAY_PIN   1    // Pin declared at Thingsboard Widget

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";   // ip or host of ThingsBoard 

WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

// Assume relay are off 
boolean relayState[] = {false};



/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("On message");

  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject((char*)json);

  if (!data.success())
  {
    Serial.println("parseObject() failed");
    return;
  }

  // Check request method
  String methodName = String((const char*)data["method"]);

  if (methodName.equals("getRelayStatus")) {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_relay_status().c_str());
  } else if (methodName.equals("setRelayStatus")) {
    // Update GPIO status and reply
    set_relay_status(data["params"]["pin"], data["params"]["enabled"]);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_relay_status().c_str());
    client.publish("v1/devices/me/attributes", get_relay_status().c_str());
  }
}

/*******************GPIO functions****************/  
/*
 * @desc: Get the relay pin status
 */ 
String get_relay_status() {
  // Prepare gpio JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data[String(RELAY_PIN)] = relayState[0] ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get relay pin status: ");
  Serial.println(strPayload);
  return strPayload;
}

/*
 * @desc: Set the relay pin status
 * @param: Relay pin, data to write (HIGH/ LOW)
 */ 
void set_relay_status(int pin, boolean enabled) {
  if (pin == RELAY_PIN) {
    // Output Relay state
    digitalWrite(RELAY_IO, enabled ? HIGH : LOW);
    // Update Relay state
    relayState[0] = enabled;
  }
}
/*******************WiFi functions****************/  
/*
 * @desc: Connect device to WiFi
 */ 
void InitWiFi(){
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
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("RELAY NODE", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
      // Sending current GPIO status
      Serial.println("Sending current GPIO status ...");
      client.publish("v1/devices/me/attributes", get_relay_status().c_str());
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Set output mode for all GPIO pins
  pinMode(RELAY_IO, OUTPUT);
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
}
