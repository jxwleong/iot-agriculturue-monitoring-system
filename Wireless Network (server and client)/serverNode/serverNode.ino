/****************************************************************
 * Original Author  : Uteh Str (https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s)
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Server node
 * Hardware : LoLin NodeMCU V3 ESP8266
 ****************************************************************/

#include <ESP8266WiFi.h>

const char* ssid = "YOUR_WIFI_SSID";                  // Your wifi Name       
const char* password = "YOUR_WIFI_PASSWORD"; // Your wifi Password
const char* Commands_Reply;                 // The command variable that is sent to the client

const char * host = "YOUR_CLIENT_IP_ADDRESS";          // IP Client

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  pinMode(D2, OUTPUT);                    // Declare the GPIO pin D2 as an output
  Serial.begin(115200);                     // initialize serial:
  delay(10);

  Serial.println("");
  Serial.println("Server-------------------------------");
  Serial.print("Configuring access point");
  WiFi.begin(ssid, password);

  // Waiting to connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");

  // Start the server
    server.begin();
    Serial.println("Server started");

  // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    Serial.println("-------------------------------------");
    Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("Server-------------------------------");
  Serial.println("New client");
  Serial.print("From client = ");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request -------------------------------------
   String req = client.readStringUntil('\r');
   Serial.println(req);
   client.flush();

   //Command RELAY -------------------------------------------------------------
    if (req.indexOf("RELAY_OFF") != -1){
      Commands_Reply = "Relay Status : Off";
      Serial.print("Server send = ");
      Serial.println(Commands_Reply);
      client.print(String("GET ") + Commands_Reply + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
      digitalWrite(D2, LOW);
    }
   else if (req.indexOf("RELAY_ON") != -1){
      Commands_Reply = "Relay Status : On";
      Serial.print("Server send = ");
      Serial.println(Commands_Reply);
      client.print(String("GET ") + Commands_Reply + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
      digitalWrite(D2, HIGH);
    }
   else {
     Serial.println("invalid request");
     client.stop();
     return;
    }

   client.flush();
   Serial.println("Client disonnected");
   Serial.println("-------------------------------------");
   Serial.println("");
}
