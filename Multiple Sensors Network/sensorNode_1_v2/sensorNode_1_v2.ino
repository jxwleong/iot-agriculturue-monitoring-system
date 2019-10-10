/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Send sensor readings to a centralised node
 * Hardware : NodeMCU ESP8266
 ****************************************************************/
#include <ESP8266WiFi.h>
#include <string.h>

const char* ssid = "HUAWEI nova 2i";                  // Your wifi Name       
const char* password = "pdk47322"; // Your wifi Password

const char * host = "192.168.43.90";        // IP Server

const int httpPort = 80;

char* Commands;                       // The command variable that is sent to the server                     

int button = 5;                             // push button is connected
bool btn_press = true;                      // The variable to detect the button has been pressed
int con = 0;     

  int soilMoisture = 0;
  int sensorPin = A0;
  
/*
 * @desc: Read soil moisture sensor reading and upload to
 *        ThingsBoard
 * @ref: [1.], [2.], [3.]
 */
void getAndSendSoilMoistureToServerNode()
{
    // Use WiFiClient class to create TCP connections
  WiFiClient client;

  Serial.println("Collecting soil moisture data.");

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(sensorPin);
  soilMoisture = map(soilMoisture,1024,0,0,100);

  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(" %\t");
  //itoa (soilMoisture, Commands, 10);
  //sprintf(Commands, "%i", soilMoisture);
  Commands = "1";
  send_commands();

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
  Serial.println(soilMoisture);

  // This will send the request to the server
  client.print(String(" ") + soilMoisture + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: Close\r\n\r\n");
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
}

// Main function
void setup() {
  // put your setup code here, to run once:
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
 
  getAndSendSoilMoistureToServerNode();

}


// References 
// [1.] Arduino | Communication Two LoLin NodeMCU V3 ESP8266 (Client Server) for Controlling LED
//      https://www.youtube.com/watch?v=O-aOnZViBzs&t=317s
// [2.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
// [3.] Arduino and Soil Moisture Sensor -Interfacing Tutorial
//      http://www.circuitstoday.com/arduino-soil-moisture-sensor
