/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Send sensor readings to a centralised node
 * Hardware : NodeMCU ESP8266
 *            Soil Moisture Sensor with LM393 Comparator
 *            DHT11 sensor
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 *  SDHT sensor library  : Version 2.0.0
 ****************************************************************/
 //-----------------------------LIBRARY---------------------------------
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <Ticker.h>
#include "SDHT.h"


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

#define DEFAULT_DATA_SAMPLING_RATE_MS  20000  // Sampling rate of sensor (software timer)

// Definition for ADC
#define ADC_10_BIT_RESOLUTION   1024     // resolution of ESP8266 ADC

// Definition for JSON
#define MAX_JSON_STRING_LENGTH      200 // Size of message to be sent to server node

// Definition for sensors
#define SOIL_MOSITURE_3V3_PIN   D0  // 3V3 pin for soil moisture sensor
#define SOIL_MOISTURE_PIN       A0  // Input pin for Soil Moisture Sensor 

#define DHT11_3V3_PIN      D1       // 3V3 pin for dht11 sensor
#define DHTPIN             D2       // Input Data pin for sensor
#define DHTTYPE    DHT11            // DHT 11


//--------------------------DATA STRUCTURE------------------------------
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
  
// Definition for RPC functions
typedef enum{
  TURN_ON_RELAY,
  TURN_OFF_RELAY,
  SLEEP,
  INVALID,
  SAMPLING
  }ControlOperation;

// Dht11 sensor data type
typedef struct {
  float humidity;
  float temperature;
  }DhtData;

// Combined sensor data
typedef struct {
  DhtData dht11Data;
  int soilMoistureData;
  }SensorData;

// Sensor mode (ON/ OFF)
typedef enum{
  SENSOR_OFF = LOW,
  SENSOR_ON = HIGH,
  }SensorMode;  

// Structure for time unit
typedef enum{
    s = 1,
    ms = 1000,
    us = 1000000,
    invalid,
}TimeUnit;

// Structure for rpc
typedef struct RpcType RpcType;
struct RpcType{
    long rpcVal;
    TimeUnit unit;
};


//-------------------------GLOBAL VARIABLE------------------------------
// WiFi variable
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;                   // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);         // ThingsBoard

// Timer variable
int interruptTimerInMilliS = 2000;      // For timer1 (hardware timer)
Ticker sendDataToServer;
TimeUnit unit = ms;

// MQTT variable
const char* mqtt_server = "ESP8266_SERVER_IP_ADDRESS";  // Ip address of Server Node

// Sleep variable
volatile SleepStatus sleepStatus = AWAKE;

// Sensor variable
SDHT dht;


//-----------------------------FUNCTIONS--------------------------------
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

// Init functions (setup function)
void InitSensor(){
  pinMode(DHT11_3V3_PIN, OUTPUT);
  pinMode(SOIL_MOSITURE_3V3_PIN, OUTPUT);
  digitalWrite(DHT11_3V3_PIN, 1);    // Turn on the sensor
  digitalWrite(SOIL_MOSITURE_3V3_PIN, 1); 
  }
  
/*
 * @desc: Create Json Object to store data and convert it into string
 * @param: Soil Moisture and Temperature
 * @retval: String that contain necessary info for server node
 */
char *createJsonStringForSensorReadings(int soilMoisture, float temperature){

  StaticJsonBuffer<MAX_JSON_STRING_LENGTH> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["from"] = "Sensor Node 1";
  object["to"] = "Server Node";
  object["method"] = "sendSensorReadings";
  object["soilMoisture"] = soilMoisture;
  object["temperature"] = temperature;

  Serial.println("\nJSON to be sent to Server:");
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
  Serial.println("\nPublishing message:");
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
  
//=============END OF SENSOR FUNCTION===============

/******************Power functios*****************/
/**
 * @desc: Turn off wifi connection
 */
void turnOffWiFi(){
  //WiFi.mode(WIFI_OFF);
  //if(WiFi.status()== WL_DISCONNECTED){
   // Serial.println("WiFi is disconnected.");
    //}
  //Serial.println("Going into deep sleep for 20 seconds");
  //ESP.deepSleep(20e6); // 20e6 is 20 microseconds
  //InitWiFi();
  }
  
/**
 * @desc: Switch between power modes
 * @param: Desired power modes
 */
void switchPowerMode(PowerMode powerMode){
    switch(powerMode){
        case NORMAL_MODE:   break;  // Do nothing
        case MODEM_SLEEP:   turnOffWiFi(); break;  // Turn off wifi
        case LIGHT_SLEEP:   break;  // Turn off System Clock
        case DEEP_SLEEP:   break;   // Everything off except RTC
        default: Serial.println("Invalid power mode chosen!");
    } // Rewrite timer1 number of ticks so to get the correct delay
      // Number of ticks may be decrement before this function called.  
      timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS));
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

//==============END OF POWER FUNCTION===============

/********************RPC functions*****************/
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
      return SAMPLING;      
  }

/*
 * @desc: Call Neccessary functions for rpc command from ThingsBoard server 
 */
void rpcCommandOperation(char *command){
  if(command != NULL){
    switch(getCommandOperation(command)){
      //case TURN_ON_RELAY: digitalWrite(RELAY_IO, 1);
      //                    relayState[0] = HIGH;     // update relay status on ThingsBoard
      //                    client.publish("v1/devices/me/attributes", get_relay_status().c_str());
      //                   break;  // Turn on Relay and update status
                        
      //case TURN_OFF_RELAY: digitalWrite(RELAY_IO, 0);
      //                     relayState[0] = LOW;     // update relay status on ThingsBoard
      //                     client.publish("v1/devices/me/attributes", get_relay_status().c_str());
      //                     break;  // Turn off Relay and update status
                         
      case SLEEP:    switchPowerMode(getPowerMode(command));
                     sleepStatus = SLEEPING;
                     Serial.println("I'm going to sleep...");
                     break;           // Choose power mode
      case SAMPLING:    RpcType dataReceived = getRpcValInInt(command);  
                        Serial.println("Rpc val: ");
                        Serial.print(dataReceived.rpcVal);
                        Serial.println("Rpc unit: ");
                        Serial.print(dataReceived.unit);
                        interruptTimerInMilliS =  dataReceived.rpcVal;
                        unit = dataReceived.unit;
                        break;
                
     // default: Serial.println("Invalid operation chosen!");
    }
  }
}

/*
 * @desc: get command type in rpc remote shell
 * @param: message from rpc remote shell, desired command
 * @retval: pointer input message
 */
char *getRpcCommandInStr(char *command, char *subStr){
  Serial.print("Command");
  Serial.println(command);
  Serial.print("SubString");
  Serial.println(subStr);
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

void skipWhiteSpaces(char **str){
    while(**str == ' ')
        ++*str;
}

/*
 * @desc: convert rpc message from string to integer 
 * @param: rpc message from ThingsBoard
 * @retval: int value of rpc message
 */
RpcType getRpcValInInt(char *str){
  char *number;
  char *unit;
  long rpcVal = 0;
  RpcType ans;
  
  ans.rpcVal = strtol(str, &unit, 10);
  ans.unit = getTimeUnitInStr(unit);
  return ans;
  }

//===============END OF RPC FUNCTION================
  
/*******************MQTT functions*****************/
/*
 * @desc: Parse JSON format string,extract necessary info 
 *        from sever and take action
 * @param: String in JSON format
 */ 
void extractAndProcessDataFromServer(char *jsonStr){
  // need to use char array to parse else the duplication will occur and jsonBuffer will be full.
  // Ref:https://github.com/bblanchon/ArduinoJson/blob/5.x/examples/JsonParserExample/JsonParserExample.ino#L28
  char jsonBuff[MAX_JSON_STRING_LENGTH];
  char *command;
  strncpy (jsonBuff, jsonStr, MAX_JSON_STRING_LENGTH+1);
  Serial.println(jsonStr);
  DynamicJsonBuffer jsonBuffer;
  Serial.println("JSON received from Server");
  Serial.print(jsonBuff);
  JsonObject& root = jsonBuffer.parseObject(jsonBuff);  

  // Decode data from jsonString
  Serial.println("Method :");
  const char* Method = root["method"]; 
  Serial.println(Method);
   strncpy (jsonBuff, jsonStr, MAX_JSON_STRING_LENGTH+1);
  Serial.println(jsonBuff);
  command = getRpcCommandInStr(jsonBuff, "command");
  Serial.println(command);
  rpcCommandOperation(command);
  }

/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 */
void callback(char* topic, byte* payload, unsigned int length) {
  char messageBuffer[length + 1];
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

//=============END OF TIMER FUNCTION================
  
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
  
/*
 * @desc: get the time unit from rpc command
 * @param: string of rpc command
 */
TimeUnit getTimeUnitInStr(char *str){
  // example of json received from rpc remote shell (string)
  // {"method":"sendCommand","params":{"command":"1 us"}}
  //                                         str---^
    skipWhiteSpaces(&str);
    if(*str == 's'){
        return s;
    }
    else if(*str == 'm'){
        return ms;
    }    
    else if(*str == 'u'){
        return us;
    }    
    else
        return invalid;
}

//=============END OF TIMER FUNCTION================

/*************Interrupt Service Routine************/
/*
 * @desc: Interrupt Service Routine when desired time is achieved,
 *        Software timer
 */  
void ISR_FUNC(){
  if(sleepStatus == AWAKE){
  SensorData sensorData;
  Serial.println("\n========================================");
  Serial.println("  ISR to collect and send sensor data.  ");
  Serial.println("========================================");
  Serial.println("Collecting sensor data now...");
  sensorData = getSensorData();
  sendSensorReadingsToServerNode(sensorData.soilMoistureData, sensorData.dht11Data.temperature);
  } 
}

/*
 * @desc: Interrupt Service Routine when desired time is achieved,
 *        Timer1 (hardware timer)
 */  
void ICACHE_RAM_ATTR onTimerISR(){
  SensorData sensorData;
  Serial.println("\n========================================");
  Serial.println("  ISR to collect and send sensor data.  ");
  Serial.println("========================================");
  Serial.println("Collecting sensor data now...");
  sensorData = getSensorData();
  sendSensorReadingsToServerNode(sensorData.soilMoistureData, sensorData.dht11Data.temperature);
  
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS)); 
}
//==================END OF ISR======================

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
    String clientId = "SensorNode1-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("toServer", "I'm connected");
      // ... and resubscribe
      client.subscribe("fromServer");
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
  //delay(10);
  InitWiFi();
  InitSensor();
  // Initialise timer
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS));
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //sendDataToServer.attach_ms(DEFAULT_DATA_SAMPLING_RATE_MS,  ISR_FUNC);

}

void loop() {
  // put your main code here, to run repeatedly:
 if ( !client.connected() && sleepStatus== AWAKE) {
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
