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
#define CPU_FREQ_80M      80000000
#define CPU_FREQ_160M     160000000
#define TIM_FREQ_DIV1     1
#define TIM_FREQ_DIV16    16
#define TIM_FREQ_DIV256   256

// GPIO definition
#define RELAY_IO    D3
#define RELAY_PIN   1    // Pin declared at Thingsboard Widget
#define PRESSURE_SENSOR_PIN   A0

// JSON definition
#define MAX_JSON_STRING_LENGTH  200

// ESP8266 definition
#define ESP8266_ADC_REF_VOLT  5
#define ESP8266_ADC_SCALE   1024

// Pressure sensor definition
#define MAX_PRESSURE_LIMIT    100   // Max boundary of pressure (psi)
#define MIN_PRESSURE_LIMIT    10   // Max boundary of pressure (psi)

#define PRESSURE_SENSOR_MIN_ADC_VALUE   (0.5 / ESP8266_ADC_REF_VOLT) * ESP8266_ADC_SCALE // 0.5 V in ADC value
#define PRESSURE_SENSOR_MAX_ADC_VALUE   (4.5 / ESP8266_ADC_REF_VOLT) * ESP8266_ADC_SCALE // MAX V in ADC value


//--------------------------DATA STRUCTURE------------------------------
// Enum definition for RPC functions
typedef enum{
  TURN_ON_RELAY,
  TURN_OFF_RELAY,
  SLEEP,
  INVALID,
  }ControlOperation;

// Enum definition for RPC functions
typedef enum{
  BELOW_MINIMUM = 0,
  OPTIMUM = 1,
  ABOVE_MAXIMUM = 0,
  }PressureStatus;


//-------------------------GLOBAL VARIABLE------------------------------
// WiFi variable
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;              // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);    // ThingsBoard
ThingsBoard tb(wifiClient);

// MQTT variable
const char* mqtt_server = "192.168.43.140";  // Ip address of Server Node

// Timer variable
Ticker callbackFlag;
int interruptTimerInMilliS = 10000;

int i = 0;

// GPIO variable
// Assume relay are off 
boolean relayState[] = {false};

// RPC variable
char *command;  // Command from rpc remote shell

// ADC variable (Pressure sensor)
int adcVal = 0;     // variable to store the value read


//-----------------------------FUNCTIONS-------------------------------
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

/************Pressure Sensor functions***********/  
/*
 * @desc: Read pressure sensor reading 
 * @param: Analog input (A0)
 * @retval: Pressure in PSI
 */ 
int getPressureSensorReading(int pin){
  adcVal = analogRead(PRESSURE_SENSOR_PIN);  // read the input pin
  int psi = ((adcVal-PRESSURE_SENSOR_MIN_ADC_VALUE)*150)/(PRESSURE_SENSOR_MAX_ADC_VALUE-PRESSURE_SENSOR_MIN_ADC_VALUE);
  return psi;
  }

/*
 * @desc: Determine whether pressure is suitable for watering
 * @param: Pressure (PSI)
 * @retval: PressureStatus
 */ 
PressureStatus getPressureStatus(int psi){
  if(psi < MIN_PRESSURE_LIMIT)
    return BELOW_MINIMUM;    // 0 in enum
  else if (psi > MAX_PRESSURE_LIMIT)
    return ABOVE_MAXIMUM;     // 0 in enum
  else 
    return OPTIMUM;   // 1 in enum  
} 

  /*
 * @desc: Create Json Object to store data and convert it into string
 * @param: Pressure sensor reading (psi)
 * @retval: String that contain necessary info for server node
 */
char *createJsonStringForSensorReading(int psi){

  StaticJsonBuffer<MAX_JSON_STRING_LENGTH> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["from"] = "Relay Node";
  object["to"] = "Server Node";
  object["method"] = "sendSensorReading";
  object["waterPressure"] = psi;

  Serial.println("\nJSON to be sent to Server:");
  object.prettyPrintTo(Serial);
  char jsonChar[MAX_JSON_STRING_LENGTH];
  object.printTo((char*)jsonChar, object.measureLength() + 1);
  return (char *)jsonChar;
  }
  
/*
 * @desc: Send data to server in json format
 * @param: Pressure sensor reading (psi)
 */
void sendSensorReadingToServerNode(int psi){

  char *message = createJsonStringForSensorReading(psi);
  Serial.println("\nPublishing message:");
  Serial.println(message);
  client.publish("toServer", message);
      // ... and resubscribe
  client.subscribe("fromServer");
  }  
  
//========END OF PRESSURE SENSOR FUNCTION===========  

/*****************Relay functions****************/
/*  
 * @desc: Check whether relay is suitable to turn on
 * @param: PressureStatus, attribute (on/ off)
 * @return: true/fase
 */
boolean isRelaySuitableToTurnOn(PressureStatus status, int attribute){
  if(status == OPTIMUM && attribute == 1)
    return true;
  else
    return false;  
  } 
  
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

/*************Interrupt Service Routine************/
/*
 * @desc: Interrupt Service Routine when desired time is achieved,
 *        Timer1 (hardware timer)
 */  
void ICACHE_RAM_ATTR onTimerISR(){
  Serial.println("\n========================================");
  Serial.println("  ISR to collect and send sensor data.  ");
  Serial.println("========================================");
  Serial.println("Collecting sensor data now...");

  sendSensorReadingToServerNode(getPressureSensorReading(PRESSURE_SENSOR_PIN));
  
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS)); 
}
//==================END OF ISR======================


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
    //if(isRelaySuitableToTurnOn(getPressureStatus(getPressureSensorReading(A0)), attribute)){
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
  else if(strstr(Method,"sendCommand")){
    Serial.println("Received command from CMD!");
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
    // Initialise timer
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS));
  
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
