/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Turn on/ off LED or get command(int) using rpc on 
 *         ThingsBoard
 * Hardware : NodeMCU ESP8266
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

// Definition for GPIO
#define GPIO0 D2
#define GPIO1 D3
#define GPIO2 D1

#define GPIO0_PIN D2
#define GPIO1_PIN D3
#define GPIO2_PIN D1

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false, false};

// Variable to makesure callback function for RPC only run
// once because callback function called multiple times
// (Due to "Device is offline")
static boolean callbackCalled = false;

// MACROs for callback function
#define isCallbackFuncCalled      callbackCalled
#define setCallbackFuncFlag       callbackCalled = true
#define resetCallbackFuncFlag     callbackCalled = false
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

  if(!isCallbackFuncCalled){
    Serial.println("\nMessage from server received.");

    char json[length + 1];
    char fullRpcMessage[length + 1];
    char *data;
    long dataInInt;
    strncpy (json, (char*)payload, length);
    json[length] = '\0';
  
    strncpy (fullRpcMessage, json, length); // Another copy of json because
    fullRpcMessage[length] = '\0'; 

    Serial.print("json: ");
    Serial.println(json);
    // Decode JSON request
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& dataJson = jsonBuffer.parseObject((char*)json);

    if (!dataJson.success())
    {
     Serial.println("parseObject() failed");
     return;
   }
    char *reqID = getRpcCommandInStr((char*)topic, 26);
     int rpcVal = strtol(reqID, NULL, 10);
    Serial.println(rpcVal);
    // Check request method
    String methodName = String((const char*)dataJson["method"]);
    Serial.println(methodName);
    // example of json received from rpc remote shell
    // {"method":"sendCommand","params":{"command":"1"}}
    // command are the variable that type in the rpc remote shell
    // since json is in string, we can bypass it by 44 characters to gt
    // "1"
    data = getRpcCommandInStr(fullRpcMessage, 44);
    if(strstr(data, "turn on led")){
      digitalWrite(GPIO2_PIN, 1);
     }
   else if(strstr(data, "turn off led")){
      digitalWrite(GPIO2_PIN, 0);
     }
   else{
    dataInInt = getRpcValInInt(data);
    //String reqID = "$request_id";
     //char *rpcResponse= "{ \"ok\": true,\"platform\": \"os.platform()\",\"type\":\" os.type()\",\"release\": \"os.release()\" }"; 
    char rpcResponse[128];
    sprintf(rpcResponse, "{\"done\": \"true\", \"requestId\" : %i, \"data\": [{\"stdout\": \"Hello\"}]}", rpcVal);
    Serial.println(rpcResponse);
    Serial.println("\n Data received in int:");
    Serial.println(dataInInt);
    String responseTopic = String(topic);
    //Serial.println(responseTopic);
    responseTopic.replace("request", "response");  
    Serial.println(responseTopic);   
    client.publish(responseTopic.c_str(), rpcResponse);
    //client.publish("v1/devices/me/attributes", get_relay_status().c_str());
    //client.subscribe("v1/devices/me/rpc/response/+");
    //client.publish("v1/devices/me/rpc/response/$request_id", "{\"done\": \"true\", \"data\": [{\"stdout\": \"Hello\"}]}");
    ////client.subscribe("v1/devices/me/rpc/request/+");
    setCallbackFuncFlag;
    }
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
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
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
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO1, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  delay(10);
  InitWiFi();    
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }
  resetCallbackFuncFlag;
  client.loop();
}

// Reference
// [1.] ESP8266 GPIO control over MQTT using ThingsBoard 
//      https://thingsboard.io/docs/samples/esp8266/gpio/#provision-your-device
