/****************************************************************
 * Original Author  : Uteh Str (https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s)
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Client node
 * Hardware : LoLin NodeMCU V3 ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/

#include <ESP8266WiFi.h>

const char* ssid = "YOUR_WIFI_SSID_HERE";                  // Your wifi Name       
const char* password = "YOUR_WIFI_PASSWORD_HERE"; // Your wifi Password

const char * host = "IP_ADDRESS_SERVER";        // IP Server

const int httpPort = 80;

const char* Commands;                       // The command variable that is sent to the server                     

int button = 5;                             // push button is connected
bool btn_press = true;                      // The variable to detect the button has been pressed
int con = 0;                                // Variables for mode

void setup() {
  // put your setup code here, to run once:
  // pin D0 suppy 3.3 V so can't use
  pinMode(D1, INPUT);   // pin D1
  pinMode(D2, INPUT);   // pin D2
  Serial.begin(115200);                     // initialize serial:

  Serial.println("");
  Serial.println("Client-------------------------------");
  Serial.print("Connecting to Network");
  WiFi.mode(WIFI_STA);                      // Mode Station
  WiFi.begin(ssid, password);               // Matching the SSID and Password
  delay(1000);

  // Waiting to connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Successfully Connecting");  
  Serial.println("Status : Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("-------------------------------------");
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  
    Serial.println("Client-------------------------------");
    Serial.print("Send Command = ");
    // Check for button pressed
      if(digitalRead(D1) == LOW && digitalRead(D2)==HIGH){
        Commands = "RELAY_OFF";
        Serial.println(Commands);
        send_commands();
        }  
      else if(digitalRead(D1) == HIGH && digitalRead(D2)==LOW){
        Commands = "RELAY_ON";
        Serial.println(Commands);
        send_commands();
        }
        else{
         Commands = "IDLE";
        Serial.println(Commands);
        send_commands();
          }
  delay(100);
}


void send_commands(){
  Serial.println("Sending command...");
  Serial.println("Don't press the button for now...");
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // We now create a URI for the request  
  Serial.print("Requesting URL : ");
  Serial.println(Commands);

  // This will send the request to the server
  client.print(String("GET ") + Commands + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: Close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {      
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  Serial.print("Server Reply = "); 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("Now you can press the button ...");
  Serial.println("-------------------------------------");
  Serial.println("");
}
