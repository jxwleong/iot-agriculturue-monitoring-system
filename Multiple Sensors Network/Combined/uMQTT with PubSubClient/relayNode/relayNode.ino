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
 ****************************************************************/
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <Ticker.h>
#include <MQTT.h>
#include <uMQTTBroker.h>

// Definition for WiFi
#define WIFI_AP "HUAWEI nova 2i"         // WiFi SSID
#define WIFI_PASSWORD "pdk47322"         // WiFi PASSWORD


String TOKEN = "yIXyjYSSWZupo67gw1p4";      // Device's Token address created on ThingsBoard

char thingsboardServer[] = "demo.thingsboard.io";   // ip or host of ThingsBoard 

// Global variable
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;              // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);    // ThingsBoard
ThingsBoard tb(wifiClient);


#define MAX_JSON_STRING_LENGTH  200

int i = 0;
  
// Definition for timer
#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

int interruptTimerInMilliS = 5000;

Ticker callbackFlag;

// Assume relay are off 
boolean relayState[] = {false};

// GPIO definition
#define RELAY_IO    D3
#define RELAY_PIN   1    // Pin declared at Thingsboard Widget

// Definition for power saving functions
typedef enum{
  NORMAL_MODE,  // Use power for all neccessary peripehrals
  MODEM_SLEEP,  // Turn off WiFi
  LIGHT_SLEEP,  // Turn off System Clock
  DEEP_SLEEP,   // Everything off except RTC
  }PowerMode;
  
typedef enum{
  AWAKE,
  SLEEPING,
  }SleepStatus;  
  
volatile SleepStatus sleepStatus = AWAKE;
// Definition for RPC functions
typedef enum{
  TURN_ON_RELAY,
  TURN_OFF_RELAY,
  SLEEP,
  INVALID,
  }ControlOperation;
  
char *command;  // Command from rpc remote shell

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


// Variable to makesure callback function for RPC only run
// once because callback function called multiple times
// (Due to "Device is offline")
static boolean callbackCalled = false;

// MACROs for callback function
#define isCallbackFuncCalled      callbackCalled
#define setCallbackFuncFlag       callbackCalled = true
#define resetCallbackFuncFlag     callbackCalled = false  

static int DELAY_FOR_CALLBACK = 0;    // in ms
/*
 * @desc: bypass the characters of str based on length
 * @param: str to be bypass, the length of bypass
 */
void bypassCharactersInStr(char **str, int length){
    while(length != 0){
        ++*str;
        length--;
    }
}

/*
 * @desc: Skip any whitespaces of str
 * @param: Str with whitespaces to bypass
 */
void skipWhiteSpaces(char **str){
    while(**str == ' ')
        ++*str;
}
/*
 * @desc: Get the sensor node number from MQTT client message
 * @param: Message from client, parameter (keyboard) to search
 * @retval: Sensor node number
 */
int getSensorNodeNumber(char *message, char *parameter){
    int sensorNodeNumber;
    int parameterLength = strlen(parameter);
    
    // move to the last character of matched string        
    bypassCharactersInStr(&message, parameterLength); // -1 minus the '/0'
    skipWhiteSpaces(&message);
    // -0 to get int based on ASCII table
    sensorNodeNumber = *message - '0';
    return sensorNodeNumber;
}

SensorParameter getSensorParameterFromClient(const char *from){
  SensorParameter sensorParameter;
  String myString = String(from);
  char sensorNumber = myString.charAt(12);  // Get the sensor node number 0-9 MAX since
                                            // uMQTT broker allow 8 clients
  // Concatenate paramter with sensor node number
  sensorParameter.soilMoistureParam = sensorParameter.soilMoistureParam + sensorNumber;
  sensorParameter.dht11Param = sensorParameter.dht11Param + sensorNumber;
  
  return sensorParameter;
    
}
/*
 * @desc: Parse JSON format string and extract necessary info
 * @param: String in JSON format
 */ 
void extractAndProcessDataFromClient(char *jsonStr){
  // need to use char array to parse else the duplication will occur and jsonBuffer will be full.
  // Ref:https://github.com/bblanchon/ArduinoJson/blob/5.x/examples/JsonParserExample/JsonParserExample.ino#L28
  char jsonBuff[MAX_JSON_STRING_LENGTH];
  strncpy (jsonBuff, jsonStr, MAX_JSON_STRING_LENGTH+1);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonBuff);  

  // Decode data from jsonString
  Serial.println("\nDecoded JSON received from clients:");
  const char* from = root["from"]; 
  Serial.print("From: ");
  Serial.print(from); 
  Serial.println("");
  
  const char* to = root["to"]; 
  Serial.print("To: ");
  Serial.print(to); 
  Serial.println("");
  
  const char* Method = root["method"]; 
  Serial.print("Method: ");
  Serial.print(Method); 
  Serial.println("");
  
  int soilMoisture = root["soilMoisture"];
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);  
  Serial.print(" %");
  Serial.println("");
  
  float temperature = root["temperature"];
  Serial.print("Temperature: ");
  Serial.print(temperature); 
  Serial.print(" C");
  Serial.println("");
  
  SensorParameter sensorParameter = getSensorParameterFromClient(from);
  Serial.println("Uploading sensor data to ThingsBoard...");
  uploadReadingsToThingsBoard(soilMoisture,(char *)((sensorParameter.soilMoistureParam).c_str()));
  uploadReadingsToThingsBoard(temperature, (char *)((sensorParameter.dht11Param).c_str()));
  }
  
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

      Serial.println("\n========================================");
      Serial.println("      Received message from clients.      ");
      Serial.println(  "========================================");

      Serial.println("received topic '"+topic+"' with data '"+(String)data_str+"'");
      extractAndProcessDataFromClient(data_str);
    }
};

myMQTTBroker myBroker;


/*
 * @desc: Upload sensor reading to ThingsBoard and display on widget
 * @param: sensor reading, parameter created on ThingsBoard
 */
void uploadReadingsToThingsBoard(float sensorData, char *deviceParameter){
  tb.sendTelemetryFloat(deviceParameter, sensorData);   // Upload data to ThingsBoard
  }  
  

/********************RPC functions*****************/

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
                          client.publish("v1/devices/me/attributes", get_relay_status().c_str());
                          break;  // Turn on Relay and update status
                        
      case TURN_OFF_RELAY: digitalWrite(RELAY_IO, 0);
                           relayState[0] = LOW;     // update relay status on ThingsBoard
                           client.publish("v1/devices/me/attributes", get_relay_status().c_str());
                           break;  // Turn off Relay and update status
                         
      //case SLEEP:    switchPowerMode(getPowerMode(command));
       //              sleepStatus = SLEEPING;
       //              Serial.println("I'm going to sleep...");
        //             break;           // Choose power mode
      default: Serial.println("Invalid operation chosen!");
    }
  }
}

/*
 * @desc: Call functions to operate based on request from ThingsBoard Server
 * @param: Method to process (SendCommand, setRelayPin,...), Data (json object)
 *         Subsribe topic, Full command request from rpc remote shell.
 */
void processRequestFromThingsBoard(String methodName, JsonObject& data, const char* topic, char *fullRpcMessage){
  if (methodName.equals("getRelayStatus")) {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_relay_status().c_str());
    DELAY_FOR_CALLBACK = 0;
  } 
  else if (methodName.equals("setRelayStatus")) {
    // Update GPIO status and reply
    set_relay_status(data["params"]["pin"], data["params"]["enabled"]);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");     
    client.publish(responseTopic.c_str(), get_relay_status().c_str());
    client.publish("v1/devices/me/attributes", get_relay_status().c_str());
    DELAY_FOR_CALLBACK = 0;
  }
  else if(methodName.equals("sendCommand")){
    myBroker.publish("fromServer", fullRpcMessage);
    Serial.println(fullRpcMessage);
    command = getRpcCommandInStr(fullRpcMessage, "command"); // get command from command Str
    Serial.println(command);
    rpcCommandOperation(command);                  // Operate based on command
    DELAY_FOR_CALLBACK = 5000;
  }
}
/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  if(!isCallbackFuncCalled){
    Serial.println("\n========================================");
    Serial.println("      Received message from server.      ");
    Serial.println(  "========================================");

    char json[length + 1];
    char fullRpcMessage[length + 1];
    long dataInInt;
    strncpy (json, (char*)payload, length);
    json[length] = '\0';

    strncpy (fullRpcMessage, json, length); // Another copy of json because
    fullRpcMessage[length] = '\0';          // json will be decode later to find
                                          // request method
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("json: ");
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
    Serial.println(methodName);
    processRequestFromThingsBoard(methodName, data, topic, fullRpcMessage);
    setCallbackFuncFlag;
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
    Serial.println("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("server Node", TOKEN.c_str(), NULL)  ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println( "Subsribed to RPC requests" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

void resetCallBackFuncFlag(){
  // If the flag is set
  if(isCallbackFuncCalled){
    delay(DELAY_FOR_CALLBACK);
    Serial.println("Resetting callback function flag.");
    resetCallbackFuncFlag;
  }
}
  
// Main functions
void setup() {
   // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAY_IO, OUTPUT);
  InitWiFi();
  
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);

  callbackFlag.attach_ms(2000, resetCallBackFuncFlag);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( !client.connected()) {
    reconnect();
  }
  client.loop();
}
