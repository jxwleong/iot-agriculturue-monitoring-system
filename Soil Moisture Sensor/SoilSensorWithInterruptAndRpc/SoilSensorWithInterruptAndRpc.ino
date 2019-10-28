/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Control sensor sampling rate using rpc at 
 *         ThingsBoard
 * Hardware : NodeMCU ESP8266       
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/

#include <ThingsBoard.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <string.h>
#include <Ticker.h>

// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

#define TOKEN "ADDRESS_TOKEN"

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
ThingsBoard tb(wifiClient);

// Definition for GPIO
#define GPIO0 D2  //On board GPIO0
#define GPIO0_PIN D2

// Definition for timer
#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

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

// Global variable
int status = WL_IDLE_STATUS;

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false, false};

int interruptTimerInSecs = 500;
int sensorPin = A0;
int soilMoisture;
TimeUnit unit = ms;



/*
 * @desc: read soil moisture sensor and upload to ThingsBoard
 */
void getAndSendSoilMoistureData()
{

  // Read the Soil Mositure Sensor readings
  soilMoisture = analogRead(sensorPin);
  soilMoisture = map(soilMoisture,1024,0,0,100);

  tb.sendTelemetryFloat("Soil Moisture", soilMoisture);
}  


/********************RPC functions*****************/
/*
 * @desc: skip empty spaces in string
 * @param: String to skip spaces
 */
void skipWhiteSpaces(char **str){
    while(**str == ' ')
        ++*str;
}

/*
 * @desc: get the time unit from rpc command
 * @param: string of rpc command
 */
TimeUnit getTimeUnitInStr(char *str){
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

 /*
 * @desc: bypass the json string and get command type in rpc remote shell
 * @param: message from rpc remote shell, bypass message length
 * @retval: pointer to the bypassed input message
 */
char *getRpcCommandInStr(char *str, int bypassLength){
  while(bypassLength != 0){
    str++;
    bypassLength--;
  }
  return str;
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
  
  str++; // remove '"' character
  ans.rpcVal = strtol(str, &unit, 10);
  ans.unit = getTimeUnitInStr(unit);
  return ans;
  }

/*
 * @desc: The callback for when a PUBLISH message is received from the server.
 */
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("\nMessage from server received.");

  char json[length + 1];
  char *data;
  RpcType dataReceived;
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  // example of json received from rpc remote shell
  // {"method":"sendCommand","params":{"command":"1"}}
  // command are the variable that type in the rpc remote shell
  // since json is in string, we can bypass it by 44 characters to gt
  // "1"
  data = getRpcCommandInStr(json, 44);
  if(strstr(data, "turn on led")){
    digitalWrite(GPIO0_PIN, 1);
    }
  else if(strstr(data, "turn off led")){
    digitalWrite(GPIO0_PIN, 0);
    }
  else{
  dataReceived = getRpcValInInt(data);
  Serial.println("\n Data received in int:");
  }
  interruptTimerInSecs =  dataReceived.rpcVal;
  unit = dataReceived.unit;
}



/*******************Timer functions****************/
/*
 * @desc: Calcuate the number of ticks to write into timer
 * @param: CPU frequency, frequency divider, timer in milliseconds
 * @retval: number of ticks for achieve desired time
 */
uint32_t getTimerTicks(uint32_t freq, int freqDivider, int seconds){
  uint32_t  dividedFreq = 0;
  float period = 0, ticks = 0;
  dividedFreq = freq/ freqDivider;
  period = (float)(1) / (float)(dividedFreq);
  ticks = ((float)(seconds)/(float)(unit) / period);
  // get the value in Seconds then div by period
  return ticks;
  }
  
/*
 * @desc: Interrupt Service Routine when desired time is achieved
 */  
void ICACHE_RAM_ATTR onTimerISR(){
    digitalWrite(GPIO0,!(digitalRead(GPIO0)));  //Toggle GPIO0 Pin
    getAndSendSoilMoistureData(); // Upload sensor reading to ThingsBoard
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInSecs));
}


/*******************WiFi functions****************/  
/*
 * @desc: Connect device to WiFi
 * @ref: [4.]
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
 * @ref: [4.]
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
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
      // Sending current GPIO status
     // Serial.println("Sending current GPIO status ...");
      //client.publish("v1/devices/me/attributes", get_gpio_status().c_str());
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
void setup()
{
    Serial.begin(115200);

    pinMode(GPIO0,OUTPUT);

    //Initialize Ticker every 0.5s
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInSecs));
    // 1/F = period
    // Desired time / period = desired ticks
    // maximum ticks 8388607
    // Since the max ticks is 8388607 stated from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h#L114
    // Thus the max time for TIM_DIV16 is limited around 1.6s
    // 1600 / (1/5M) = 8000000
    // To get more time range for the interrupt, TIM_DIV265 is used
    // Time = ticks * (1 / CPU_FREQ)
    //      = 8000000 * (1 / 80M/256)
    //      = 25.6s

    InitWiFi();
    client.setServer( thingsboardServer, 1883 );
    client.setCallback(on_message);
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
}

//References 
// [1.] Arduino/cores/esp8266/Arduino.h 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h
// [2.] Arduino/cores/esp8266/core_esp8266_timer.cpp 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_timer.cpp
// [3.] ESP8266 Timer and Ticker Example 
//      https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
// [4.] Temperature upload over MQTT using ESP8266 and DHT22 sensor
//      https://thingsboard.io/docs/samples/esp8266/temperature/
