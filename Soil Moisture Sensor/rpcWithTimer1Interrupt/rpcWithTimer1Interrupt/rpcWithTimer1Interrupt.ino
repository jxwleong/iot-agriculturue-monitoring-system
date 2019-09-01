#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <string.h>
#include <Ticker.h>

#define WIFI_AP "HUAWEI nova 2i"
#define WIFI_PASSWORD "pdk47322"

#define TOKEN "lqT7NtbuQVLP08sOhCIa"

#define GPIO0 D2  //On board GPIO0
#define GPIO0_PIN D2

#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

typedef enum{
    s = 1,
    ms = 1000,
    us = 1000000,
    invalid,
}TimeUnit;

typedef struct RpcType RpcType;
struct RpcType{
    long rpcVal;
    TimeUnit unit;
};
char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false, false};

int interruptTimerInSecs = 500;
TimeUnit unit = ms;
//For references goto 
//1. https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h
//2. https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_timer.cpp


// @param : CPU frequency, frequency divider, timer in seconds
// @retval : number of ticks for achieve desired time
uint32_t getTimerTicks(uint32_t freq, int freqDivider, int seconds){
  uint32_t  dividedFreq = 0;
  float period = 0, ticks = 0;
  dividedFreq = freq/ freqDivider;
  period = (float)(1) / (float)(dividedFreq);
  ticks = ((float)(seconds)/(float)(unit) / period);
  // get the value in Seconds then div by period
  return ticks;
  }
  
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


// expect input string already bypass previous characters to command by
// calling getRpcCommandInStr
// eg "123"}}
void skipWhiteSpaces(char **str){
    while(**str == ' ')
        ++*str;
}

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

// The callback for when a PUBLISH message is received from the server.
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("\nMessage from server received.");

  char json[length + 1];
  char *data;
  RpcType dataReceived;
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  //Serial.print("Topic: ");
  //Serial.println(topic);
  //Serial.print("Message: ");
  Serial.print("json: ");
  Serial.println(json);
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

 // bypass the json string and get command type in rpc remote shell
char *getRpcCommandInStr(char *str, int bypassLength){
  while(bypassLength != 0){
    str++;
    bypassLength--;
  }
  return str;
}



 
void ICACHE_RAM_ATTR onTimerISR(){
    digitalWrite(GPIO0,!(digitalRead(GPIO0)));  //Toggle GPIO0 Pin
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInSecs));
}

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
