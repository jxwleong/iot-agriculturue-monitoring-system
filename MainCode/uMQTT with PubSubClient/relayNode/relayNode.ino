/****************************************************************
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Server node
 * Hardware : LoLin NodeMCU V3 ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/
//-----------------------------LIBRARY---------------------------------
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <Ticker.h>


//----------------------------DEFINITION--------------------------------
// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"                   // WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"         // WiFi PASSWORD

// Definition for timer
#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

// GPIO definition
#define RELAY_IO    D3
#define RELAY_PIN   1    // Pin declared at Thingsboard Widget

// Boundary of soil moisture for optimal growth
#define MAX_SOIL_MOISTURE   30
#define MIN_SOIL_MOISTURE   10

// JSON definition
#define MAX_JSON_STRING_LENGTH  200


//-------------------------GLOBAL VARIABLE------------------------------
// WiFi variable
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;              // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);    // ThingsBoard
ThingsBoard tb(wifiClient);

// MQTT variable
const char* mqtt_server = "ESP8266_SERVER_IP_ADDRESS";  // Ip address of Server Node

// Timer variable
Ticker callbackFlag;
int interruptTimerInMilliS = 5000;

int i = 0;

// GPIO variable
// Assume relay are off 
boolean relayState[] = {false};

// RPC variable
char *command;  // Command from rpc remote shell


//--------------------------DATA STRUCTURE------------------------------
// Definition for RPC functions
typedef enum{
  TURN_ON_RELAY,
  TURN_OFF_RELAY,
  SLEEP,
  INVALID,
  }ControlOperation;

// MQTT definition and function
typedef struct SensorInfo SensorInfo;
struct SensorInfo{
    int sensorNodeNumber; // Specific number for each sensor node.
    int sensorReading;    // Sensor reading extract from sensor node publish
                          // message.
};

// Sensor parameter used to publish telemetry to ThingsBoard
typedef struct SensorParameter SensorParameter;
struct SensorParameter{
   String soilMoistureParam = "soilMoisture";       // Soil Moisture Sensor Parameter
   String dht11Param = "temperature";               // DHT11 Sensor Parameter
  };


//-----------------------------FUNCTIONS--------------------------------
/******************Sensor functions****************/
/*
 * @desc: Compare soil moisture reading received and return
 *        respective value
 * @param: Soil Moisture from sensor nodes
 * @retval: Return correspond attributes
 */
int isSoilMoistureOptimum(int soilMoisture){
       if(soilMoisture < MAX_SOIL_MOISTURE &&\
          soilMoisture > MIN_SOIL_MOISTURE)
        return 1;
       else
        return 0; 
  }  

//=============END OF SENSOR FUNCTION===============

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

//=============END OF TIMER FUNCTION================

/*****************Relay functions****************/  
/*
 * @desc: Get the relay pin status
 */ 
String getRelayStatus() {
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
void setRelayStatus(int pin, boolean enabled) {
  if (pin == RELAY_PIN) {
    // Output Relay state
    digitalWrite(RELAY_IO, enabled ? HIGH : LOW);
    // Update Relay state
    relayState[0] = enabled;
  }
}

//==============END OF RELAY FUNCTION===============

/*******************MQTT functions****************/ 
/*
 * @desc: Create Json Object to store data and convert it into string
 * @param: Relay status
 * @retval: String that contain necessary info for server node
 */
char *createJsonStringToUpdateRelayStatus(int attribute){

  StaticJsonBuffer<MAX_JSON_STRING_LENGTH> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["from"] = "Relay Node";
  object["to"] = "Server Node";
  object["method"] = "relayCommand";
  object["attribute"] = attribute; // Relay Status
  
  Serial.println("\nJSON to be sent to Server:");
  object.prettyPrintTo(Serial);
  char jsonChar[MAX_JSON_STRING_LENGTH];
  object.printTo((char*)jsonChar, object.measureLength() + 1);
  return (char *)jsonChar;
}

/*
 * @desc: Take action with command from server
 * @param: Command from server node, attribute (variable respect to command)
 */ 
void processCommandFromServer(char *command, int attribute){
  if(strstr(command, "setRelayStatus")){
    Serial.println("Toggle relay pin now.");
    setRelayStatus(RELAY_PIN, attribute);
    char *replyToServer = createJsonStringToUpdateRelayStatus((int)relayState[0]);
    client.publish("toServer", replyToServer);
    // resubscribe
    client.subscribe("toRelay");
  }

  else if(strstr(command, "getRelayStatus")){
    char *replyToServer = createJsonStringToUpdateRelayStatus((int)relayState[0]);
    client.publish("toServer", replyToServer);
    // resubscribe
    client.subscribe("toRelay");
    }
  else if(strstr(command, "soilMoistureReadingFromSensors")){
      if(!isSoilMoistureOptimum(attribute)) // Soil Moisture is not ideal
         setRelayStatus(RELAY_PIN, attribute);
          char *replyToServer = createJsonStringToUpdateRelayStatus((int)relayState[0]);
          client.publish("toServer", replyToServer);
          // resubscribe
          client.subscribe("toRelay");
    }
}

/*
 * @desc: Get the command from server node and call functions to operate
 * @param: JsonObject, method
 */
void operatedBasedOnMethod(JsonObject& root, const char *Method){
  if(strstr(Method, "relayCommand")){  
    Serial.println("\nDecoded JSON received from server:");
    const char* from = root["from"]; 
    Serial.print("From: ");
    Serial.print(from); 
    Serial.println("");
  
    const char* to = root["to"]; 
    Serial.print("To: ");
    Serial.print(to); 
    Serial.println("");
    
    const char* command = root["command"];
    Serial.print("Command: ");
    Serial.print(command);  
    Serial.println("");

    int attribute = root["attribute"];
    Serial.print("Attribute: ");
    Serial.print(attribute);  
    Serial.println(""); 

    processCommandFromServer((char *)command, attribute);
  }
  else if(strstr(Method, "sendSensorReadingsFromServer")){
    int soilMoisture = root["soilMoisture"];
    Serial.println("Soil Moisture: ");
    Serial.print(soilMoisture);

    processCommandFromServer("soilMoistureReadingFromSensors", soilMoisture);
    }
}

/*
 * @desc: Get method from Json sent from server and call function to operate.
 * @param: Json String sent from server
 */
void extractAndProcessDataFromServer(char *jsonStr){
  // need to use char array to parse else the duplication will occur and jsonBuffer will be full.
  // Ref:https://github.com/bblanchon/ArduinoJson/blob/5.x/examples/JsonParserExample/JsonParserExample.ino#L28
  char jsonBuff[MAX_JSON_STRING_LENGTH];
  strncpy (jsonBuff, jsonStr, MAX_JSON_STRING_LENGTH+1);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonBuff);  

  Serial.println("\n========================================");
  Serial.println("      Received message from server.      ");
  Serial.println(  "========================================");  

  // Decode method from jsonString
  const char* Method = root["method"]; 
  Serial.print("Method: ");
  Serial.print(Method); 
  Serial.println("");

  operatedBasedOnMethod(root, Method);
}

/*
 * @desc: Callback function when there's publish from mqtt server
 * @param: mqtt topic, payload(data), payload length
 */
void callback(char* topic, byte* payload, unsigned int length) {
  char messageBuffer[length + 1];
  Serial.println("Payload length: ");
  Serial.print(length);
  strncpy (messageBuffer, (char*)payload, length);
  messageBuffer[length] = '\0';
  
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
  Serial.println("Message buffer:");
  Serial.println(messageBuffer);
  extractAndProcessDataFromServer(messageBuffer);

}

//==============END OF MQTT FUNCTION================

/********************RPC functions*****************/
/*
 * @desc: get command type in rpc remote shell
 * @param: message from rpc remote shell, desired command
 * @retval: pointer input message
 */
char *getRpcCommandInStr(char *command, char *subStr){
    // Example
    //{"method":"sendCommand","params":{"command":"turn on relay"}}
    // If subStr is found in command, it will return the first
    // character of subStr in command else return NULL
    char *exist = strstr(command, subStr);  
    if(exist != NULL){
        return exist+10; // Get the command in str after "
    }
    return NULL;
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
 * @desc: Get the operation for rpc command from remote shell of ThingsBoard
 * @param: Return the operation
 */
ControlOperation getCommandOperation(char *command){
    if(strstr(command,"turn on relay"))
      return TURN_ON_RELAY;
    else if(strstr(command, "turn off relay")) 
      return TURN_OFF_RELAY;
    else if(strstr (command, "sleep"))
      return SLEEP;
    else 
      return INVALID;      
  }

/*
 * @desc: Get the power mode for rpc command from remote shell of ThingsBoard
 * @param: Command from rpc remote shell on ThingsBoard
 * @retval: Return the power mode
 */
PowerMode getPowerMode(char *command){
    if(strstr(command,"modem sleep"))
      return MODEM_SLEEP;
    else if(strstr(command, "light sleep"))
      return LIGHT_SLEEP;
    else if(strstr(command, "deep sleep")) 
      return DEEP_SLEEP;
    else
      return NORMAL_MODE;   
  }  

/*
 * @desc: Call Neccessary functions for rpc command from ThingsBoard server 
 */
void rpcCommandOperation(char *command){
  if(command != NULL){
    switch(getCommandOperation(command)){
      case TURN_ON_RELAY: digitalWrite(RELAY_IO, 1);
                          relayState[0] = HIGH;     // update relay status on ThingsBoard
                          client.publish("v1/devices/me/attributes", getRelayStatus().c_str());
                          break;  // Turn on Relay and update status
                        
      case TURN_OFF_RELAY: digitalWrite(RELAY_IO, 0);
                           relayState[0] = LOW;     // update relay status on ThingsBoard
                           client.publish("v1/devices/me/attributes", getRelayStatus().c_str());
                           break;  // Turn off Relay and update status
                         
      //case SLEEP:    switchPowerMode(getPowerMode(command));
       //              sleepStatus = SLEEPING;
       //              Serial.println("I'm going to sleep...");
        //             break;           // Choose power mode
      default: Serial.println("Invalid operation chosen!");
    }
  }
}

//===============END OF RPC FUNCTION================

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
    String clientId = "relayNode-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("toServer", "I'm connected");
      // ... and resubscribe
      client.subscribe("toRelay");
    } else {
      Serial.println("Client_ID: ");
      Serial.print(clientId.c_str());
      Serial.println("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//==============END OF WIFI FUNCTION================
  
// Main functions
void setup() {
   // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAY_IO, OUTPUT);
  InitWiFi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  // put your main code here, to run repeatedly:
  if ( !client.connected()) {
    reconnect();
  }
  client.loop();
}
