/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Turn on/ off relay using rpc on 
 *         ThingsBoard
 * Hardware : Wemos D1 R2
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <string.h>

// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

#define TOKEN "ADDRESS_TOKEN"

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;


// Assume relay are off 
boolean relayState[] = {false};

// GPIO definition
#define RELAY_IO    D3
#define RELAY_PIN   1    // Pin declared at Thingsboard Widget

/********************RPC functions*****************/
/*
 * @desc: bypass the json string and get command type in rpc remote shell
 * @param: message from rpc remote shell, bypass message length
 * @retval: pointer to the bypassed input message
 */
char *getRpcCommandInStr(char *str, int bypassLength){
  while(bypassLength != 0){
    str++;
    bypassLength--;
  }
  return str;
}

/*
 * @desc: convert rpc message from string to integer 
 * @param: rpc message from ThingsBoard
 * @retval: int value of rpc message
 */
int getRpcValInInt(char *str){
  // expect input string already bypass previous characters to command by
  // calling getRpcCommandInStr
  // eg "123"}}
  // return 123 in integer
  char *number;
  long rpcVal = 0;
  str++; // remove '"' character
  rpcVal = strtol(str, NULL, 10);
  return rpcVal;
  }

/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 * @ref: [1.]
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("\nMessage from server received.");

  char json[length + 1];
  char *command;
  long dataInInt;
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("json: ");
  Serial.println(json);

  // example of json received from rpc remote shell
  // {"method":"sendCommand","params":{"command":"1"}}
  // command are the variable that type in the rpc remote shell
  // since json is in string, we can bypass it by 44 characters to gt
  // "1"
  command = getRpcCommandInStr(json, 44);
  
  if(strstr(command, "turn on relay")){
    digitalWrite(RELAY_IO, 1);
    relayState[0] = HIGH;     // update relay status on ThingsBoard
    client.publish("v1/devices/me/attributes", get_relay_status().c_str());
    }
  else if(strstr(command, "turn off relay")){
    digitalWrite(RELAY_IO, 0);
    relayState[0] = LOW;     // update relay status on ThingsBoard
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
 * @ref: [1.]
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
    if ( client.connect("Relay Node", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
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
  Serial.begin(115200);
  // Set output mode for all GPIO pins
  pinMode(RELAY_IO, OUTPUT);
  delay(10);
  InitWiFi();    
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);
}

void loop() {
  StaticJsonBuffer<200> jsonBuffer;
  if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
}

// Reference
// [1.] ESP8266 GPIO control over MQTT using ThingsBoard 
//      https://thingsboard.io/docs/samples/esp8266/gpio/#provision-your-device
