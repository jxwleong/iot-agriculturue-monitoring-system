/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Control server and sensor nodes/relay using rpc on 
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
#include <Ticker.h>
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

// Definition for timer

#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

int interruptTimerInMilliS = 5000;

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
 * @desc: Interrupt Service Routine when desired time is achieved,
 *        Reset the sleeping status to awake
 */  
void ICACHE_RAM_ATTR onTimerISR(){
    if(sleepStatus == SLEEPING){
    sleepStatus = AWAKE;
    Serial.println("I'm awake!");
    }
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS)); // write 0.5s ticks
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
                         
      case SLEEP:    switchPowerMode(getPowerMode(command));
                     sleepStatus = SLEEPING;
                     Serial.println("I'm going to sleep...");
                     break;           // Choose power mode
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
  } 
  else if (methodName.equals("setRelayStatus")) {
    // Update GPIO status and reply
    set_relay_status(data["params"]["pin"], data["params"]["enabled"]);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");     
    client.publish(responseTopic.c_str(), get_relay_status().c_str());
    client.publish("v1/devices/me/attributes", get_relay_status().c_str());
  }
  else if(methodName.equals("sendCommand")){
    Serial.println(fullRpcMessage);
   command = getRpcCommandInStr(fullRpcMessage, "command"); // get command from command Str
   Serial.println(command);
   rpcCommandOperation(command);                  // Operate based on command
  }
}
/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 * @ref: [1.]
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("\nMessage from server received.");

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


/******************Power functios*****************/
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

/**
 * @desc: Turn off wifi connection
 */
void turnOffWiFi(){
  WiFi.mode(WIFI_OFF);
  if(WiFi.status()== WL_DISCONNECTED){
    Serial.println("WiFi is disconnected.");
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
    if ( client.connect("Server Node", TOKEN, NULL) ) {
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
  //Initialize Ticker every 0.5s
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS));
 // delay(10);
  InitWiFi();    
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);

}

void loop() {
  StaticJsonBuffer<200> jsonBuffer;
  if ( !client.connected()&& sleepStatus == AWAKE) {
      reconnect();
  }
  //switchPowerMode(MODEM_SLEEP);
  client.loop();
}

// Reference
// [1.] ESP8266 GPIO control over MQTT using ThingsBoard 
//      https://thingsboard.io/docs/samples/esp8266/gpio/#provision-your-device
// [2.] Making the ESP8266 Low-Powered with Deep Sleep
//      https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep
// [3.] Arduino/cores/esp8266/Arduino.h 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h
// [4.] Arduino/cores/esp8266/core_esp8266_timer.cpp 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_timer.cpp
// [5.] ESP8266 Timer and Ticker Example 
//     https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
