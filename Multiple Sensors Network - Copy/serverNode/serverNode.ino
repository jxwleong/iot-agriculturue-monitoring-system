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
#define WIFI_AP "HUAWEI nova 2i"
#define WIFI_PASSWORD "pdk47322"


#define MAX_CLIENTS   10
#define MAX_DEVICES   10

#define MAX_TOKEN_LENGTH    32

const char * host = "192.168.43.221";          // IP Client
String TOKEN = "rB8vcDdciynDrDTfK5wY";
String PARAMETER = "Server Node";

char thingsboardServer[] = "demo.thingsboard.io";

// Global variable
int status = WL_IDLE_STATUS;
unsigned long lastSend;

WiFiServer server(80);

WiFiClient wifiClient;
PubSubClient client(wifiClient);
ThingsBoard tb(wifiClient);

WiFiClient clients[MAX_CLIENTS];

const char* Commands_Reply;                 // The command variable that is sent to the client
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

  clients[i].print(json);
  clients[i].connect(clients[i].remoteIP(), 80);
  if(!clients[i].connected())
    Serial.println("Client is disconnected!");

  Serial.println("Sent json");
  clients[i].stop();
  // example of json received from rpc remote shell
  // {"method":"sendCommand","params":{"command":"1"}}
  // command are the variable that type in the rpc remote shell
  // since json is in string, we can bypass it by 44 characters to gt
  // "1"

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
    Serial.print("Connecting to ThingsBoard node ...");
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


  // Check if a client has connected
  clients[i] = server.available();
  if (!clients[i]) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("Server-------------------------------");
  Serial.println("New client");
  Serial.print("From client = ");
  Serial.print(clients[i].remoteIP());
  while(!clients[i].available()){
    delay(1);
  }

   setThingsBoardDevice(TOKEN, PARAMETER, Sensor1);
  // Read the first line of the request -------------------------------------
   String req = clients[i].readStringUntil('\r');
   Serial.print("Data from client: ");
   Serial.println(req);
   int soilMoisture = req.toInt();

   Serial.println("Uploading soil sensor to ThingsBoard...");
  tb.sendTelemetryFloat(PARAMETER.c_str(), soilMoisture);
   //clients[i].flush();

   //Command -------------------------------------------------------------
    Commands_Reply = "Received data";
    Serial.print("Server send = ");
    Serial.println(Commands_Reply);
    clients[i].print(Commands_Reply);
 
    //clients[i].flush();
    Serial.println("Client disonnected");
    Serial.println("-------------------------------------");
    Serial.println("");

  if ( !tb.connected() ) {
    reconnect();
  }
  clients[i].stop();
  setThingsBoardDevice(TOKEN, PARAMETER, ServerNode);
  
  //tb.disconnect();
     if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
  //tb.loop();
}
