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
#include <MQTT.h>
#include <uMQTTBroker.h>

// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"         // WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"         // WiFi PASSWORD


#define MAX_CLIENTS   10          // Maximum no of clients to be connected to server
#define MAX_DEVICES   10

#define MAX_TOKEN_LENGTH    32

#define NUMBER_OF_CLIENTS    2 // Maximum number of sensor nodes to connect to server
//const char * host = "IP_ADDRESS_CLIENT";    // IP Client
String TOKEN = "ADDRESS_TOKEN";      // Device's Token address created on ThingsBoard
String PARAMETER = "PARAMETER";           // Parameter of device's widget on ThingsBoard

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";   // ip or host of ThingsBoard 

// Global variable
int status = WL_IDLE_STATUS;
unsigned long lastSend;

WiFiServer server(80);              // Server with port(declared at clients node)

WiFiClient wifiClient;              // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);    // ThingsBoard
ThingsBoard tb(wifiClient);

WiFiClient clients[MAX_CLIENTS];


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


char Commands_Reply[] = "Received data";                // The command message that is sent to the client
char rpc_Reply[] = "No rpc command from ThingsBoard";   // Message to sent to the client with Commands_Reply
// char* : cause unexpected behaviour
int i = 0;
  
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

// Device List Name for String Array
typedef enum{
  Sensor1,
  Sensor2,
  ServerNode,
  }DeviceLists;

// String array represent TOKEN ADDRESS of
// respective devices on ThingsBoard
String deviceToken[MAX_DEVICES] = {
  [Sensor1] = "9XsTWu7cnzt2Ntso9X67",
  [Sensor2] = "uUca2JippVzlqxuQ3V8B",
  [ServerNode] = "rB8vcDdciynDrDTfK5wY",
  }; 

// String array represent parameter of widget
// on ThingsBoard (use for sending/ receiving data)  
String deviceParam[MAX_DEVICES] = {
  [Sensor1] = "Soil Sensor 1",
  [Sensor2] = "Soil Sensor 2",
  [ServerNode] = "Server Node",
  }; 

// MQTT definition and function
typedef struct SensorInfo SensorInfo;
struct SensorInfo{
    int sensorNodeNumber; // Specific number for each sensor node.
    int sensorReading;    // Sensor reading extract from sensor node publish
                          // message.
};

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

/*
 * @desc: Get the sensor reading from MQTT client message
 * @param: Message from client, parameter (keyboard) to search[modified]
 * @retval: Sensor reading
 */
int getSensorReading(char *message, char *parameter){
    int i = 0;
    int sensorReading;
    int parameterLength = strlen(parameter);
    char temp[8];    // to store sensor reading in char first
    // move to the last character of matched string        
    bypassCharactersInStr(&message, parameterLength); // -1 minus the '/0'
    skipWhiteSpaces(&message);
    while(*message != ','){
        if(isdigit(*message)){
            temp[i] = *message;
            i++;
            message++;
        }
    }
        temp[i] = '\0'; // end of string
        sensorReading = strtol(temp, NULL, 10);
    
}


/*
 * @desc: Get the sensor node number and reading from MQTT client message
 * @param: Message from client, parameter (keyboard) to search
 * @retval: Senor node number, sensor reading
 */
SensorInfo getSensorInfoInStr(char *message, char *parameter){
    SensorInfo sensorInfo;
    int i = 0;
    int parameterLength = strlen(parameter);
    char temp[20];   // change the parameter with sensor 
                                                // node number to find sensor reading
    char *exist = strstr(message, parameter); 
    
    // If the paramter is found
    if(exist != NULL){
    
        // get the sensor node number    
        sensorInfo.sensorNodeNumber = getSensorNodeNumber(exist, parameter); 
        //Example "Soil Mositure 1:"
        //-------------^         ^-----
        //         (parameter)    (sensorNodeNumber)
        sprintf(temp, "%s %i:", parameter,sensorInfo.sensorNodeNumber); // new parameter to get 
                                                                        // sensor reading
        sensorInfo.sensorReading = getSensorReading(exist, temp);                                         
    }
    return sensorInfo;
}

/*  
 * @desc: Change the TOKEN to different devices TOKEN,
 *        The TOKEN will used to communicate the device
 *        created on ThingsBoard
 * @param: token(string), deviceToken(string)
 */
void changeTokenAddress(String &des, String src){ // '&' so param val will changed
   Serial.println("Before Token :");
   Serial.println(TOKEN);
   des = src;
   Serial.println("After Token :");
   Serial.println(TOKEN);
} 

/*  
 * @desc: Change the parameter to different devices parameter,
 *        The parameter will used to upload readings 
 *        to ThingsBoard
 * @param: parameter(string), deviceParam(string),
 */
void changeDeviceParam(String &des, String src){
   Serial.println("Before parameter :");
   Serial.println(PARAMETER);
   des = src;
   Serial.println("After parameter :");
   Serial.println(PARAMETER);  
}

/*  
 * @desc: Change the parameter and token to different devices parameter,
 *        to communicate with ThingsBoard
 * @param: token(string),deviceParam(string), DeviceLists(enum)
 */
void setThingsBoardDevice(String &token, String &param, DeviceLists device){
  Serial.print("\n");
  Serial.print("Changing device from ");
  Serial.print(PARAMETER);
  token = deviceToken[device];
  param = deviceParam[device];
  Serial.print(" to ");
  Serial.print(PARAMETER);
  }

/*
 * @desc: Change server reply message to client
 * @param: commandStr, rpcStr
 */
void changeServerReplyMessageRpc(char *rpcReply){
  Serial.println("\nChanging replay");
  Serial.println("From ");
  Serial.print(rpc_Reply);
  strcpy(rpc_Reply,rpcReply);
  Serial.print(" to ");
  Serial.print(rpcReply);
  Serial.print("\n");
}

/*
 * @desc: Receive data from connected clients and reply 
 *        with message
 */
void getDataFromClientsAndReply(){
  // Check if a client has connected
  clients[i] = server.available();
  if (!clients[i]) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("Server-------------------------------");
  Serial.println("New client");
  Serial.print("From client = ");
  Serial.print(clients[i].remoteIP());      // Display client IP address
  while(!clients[i].available()){           // Wait until client available
    delay(1);
  }

  // Read the first line of the request -------------------------------------
   String req = clients[i].readStringUntil('\n');   // Read the message request(data) from client node
   Serial.print("\nData from client: ");            // Print message from client node
   Serial.print(req);
   Serial.print("%\n");
   int soilMoisture = req.toInt();          // Convert the data from string to integer 
    

   //Command -------------------------------------------------------------
    Serial.print("Server send = ");
    sprintf(Commands_Reply, "Received data");  // Reply to client after receive data.
    //Serial.println(Commands_Reply);
    Serial.println(rpc_Reply);
    //clients[i].print(Commands_Reply);       // Send reply messages to client
    clients[i].print(rpc_Reply);
    changeServerReplyMessageRpc("No rpc command from ThingsBoard");  // Change the reply messages(rpc message will change
                                                                     // on callback function thus need to reset default message
    
    clients[i].flush();           // Wait until all data was sent
    Serial.println("-------------------------------------");
    Serial.println("");
}

/*
 * @desc: Call function to get data from clients
 * @param: Number of clients(sensor nodes)
 */
void clientHandler(int noOfClients){
  for(int j=0; j<noOfClients; j++){
      getDataFromClientsAndReply(); // get the data from clients
      i++;
    }
   i = 0;   // Reset the client counter
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
  myBroker.publish("fromServer", fullRpcMessage);
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
  
  // Start the server
  server.begin();
  Serial.println("Server started");
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


// Main functions
void setup() {
   // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAY_IO, OUTPUT);
  InitWiFi();

  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS));
  
  // Start the broker
  Serial.println("Starting MQTT broker");
  myBroker.init();
  
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);

  // Subsribe to topic
  myBroker.subscribe("fromServer");  
  myBroker.subscribe("toServer"); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( !client.connected() && sleepStatus == AWAKE) {
    reconnect();
  }
  clientHandler(NUMBER_OF_CLIENTS);


  client.loop();
}
