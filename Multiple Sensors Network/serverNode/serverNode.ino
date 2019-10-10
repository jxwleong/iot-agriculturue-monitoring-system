/****************************************************************
 * Original Author  : Uteh Str (https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s)
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Server node
 * Hardware : LoLin NodeMCU V3 ESP8266
 ****************************************************************/
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>

// Definition for WiFi
#define WIFI_AP "HUAWEI nova 2i"         // WiFi SSID
#define WIFI_PASSWORD "pdk47322"         // WiFi PASSWORD


#define MAX_CLIENTS   10          // Maximum no of clients to be connected to server
#define MAX_DEVICES   10

#define MAX_TOKEN_LENGTH    32

const char * host = "192.168.43.221";       // IP Client
String TOKEN = "rB8vcDdciynDrDTfK5wY";      // Device's Token address created on ThingsBoard
String PARAMETER = "Server Node";           // Parameter of device's widget on ThingsBoard

char thingsboardServer[] = "demo.thingsboard.io";   // ip or host of ThingsBoard 

// Global variable
int status = WL_IDLE_STATUS;
unsigned long lastSend;

WiFiServer server(80);              // Server with port(declared at clients node)

WiFiClient wifiClient;              // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);    // ThingsBoard
ThingsBoard tb(wifiClient);

WiFiClient clients[MAX_CLIENTS];

const char Commands_Reply[] = "Received data";                // The command message that is sent to the client
char rpc_Reply[] = "No rpc command from ThingsBoard";   // Message to sent to the client with Commands_Reply
// char* : cause unexpected behaviour
int i = 0;
  
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

/********************RPC functions*****************/
/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("\nMessage from ThingsBoard received.");

  char json[length + 1];
  char *data;
  //RpcType dataReceived;
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("json: ");
  Serial.println(json);

  strcpy(rpc_Reply,json);
  Serial.println("\nSent rpc command to clients");
  Serial.println(rpc_Reply);
  Serial.println("-------------------------------------");
  Serial.println("");
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
    if ( client.connect("ESP8266 Device", TOKEN.c_str(), NULL)  ) {
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
    Serial.println(Commands_Reply);
    Serial.println(rpc_Reply);
    clients[i].print(Commands_Reply);       // Send reply messages to client
    clients[i].print(rpc_Reply);
    changeServerReplyMessageRpc("No rpc command from ThingsBoard");  // Change the reply messages(rpc message will change
                                                                     // on callback function thus need to reset default message
    clients[i].flush();           // Wait until all data was sent
    Serial.println("-------------------------------------");
    Serial.println("");


  //clients[i].stop();
  

  client.loop();
}
