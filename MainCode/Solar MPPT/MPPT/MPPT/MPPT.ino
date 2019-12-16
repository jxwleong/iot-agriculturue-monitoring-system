/****************************************************************
 * Author : Jason Leong Xie Wei
 * Conctact : jason9829@live.com
 * Title : Use Hill-climbing technique to find the MPPT
 *         (Maximum Power Point Tracking) of the solar panel.
 * Hardware : 5V, 200mA Solar Panel
 *            A multimeter
 * Library Version:            
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/
//-----------------------------LIBRARY---------------------------------
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>


//----------------------------DEFINITION-------------------------------
// Definition for WiFi
#define WIFI_AP "YOUR_WIFI_SSID_HERE"                   // WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"         // WiFi PASSWORD

// Definition for ThingsBoard
#define TOKEN "ol4soSBG3ZyWpfMSh6zI"
#define VOLTAGE_PARAMETER_TB    "Voltage"   // Parameter in widget of ThingsBoard
#define CURRENT_PARAMETER_TB    "Current"
#define POWER_PARAMETER_TB      "Power"
#define MAX_POWER_PARAMETER_TB      "MPPT"

// Definition for Solar
#define MAX_ARRAY_SIZE   1024


//--------------------------DATA STRUCTURE-----------------------------
// Data to store information about solar panel
typedef struct {
  float volt;
  float current;
  float power;
  }SolarData;


//-------------------------GLOBAL VARIABLE------------------------------
// WiFi variable
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;                   // Wifi clients created to connect to internet and 
PubSubClient client(wifiClient);         // ThingsBoard

// ThingsBoard variable
ThingsBoard tb(wifiClient);
char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP_HERE";

// Solar variable
SolarData solarDataArr[MAX_ARRAY_SIZE];   // Array to store solar panel's info
float maxPowerDetected = 0;       // Mximum power detected
float previousMaxPower = 0;

int arrayIndex = 0;    // i of Array (solarData)


//-----------------------------FUNCTIONS--------------------------------
/*
 * @desc: Calculated the power of solar
 * @param: Voltage, Current
 * @retval: power in float
 */
float getSolarPower(float volt, float current){
  return (volt*current);
  } 
  
/*
 * @desc: Insert the solar data into the array
 * @param: Power, arrayIndex
 */
void insertPowerIntoArray(float power){
  if(arrayIndex == 0){  // If the array is empty
    maxPowerDetected = power;
  } 
  solarDataArr[arrayIndex].power = power;
}  

/*
 * @desc: Insert Voltage and current into array
 * @param: Voltage, Current
 */
void insertVoltageAndCurrentIntoArray(float volt, float current){
  solarDataArr[arrayIndex].volt = volt;
  solarDataArr[arrayIndex].current = current;
  } 

/*
 * @desc: Get Max power from the array
 * @retval: Power (float)
 */
float getMaxPowerInArr(){

  if(arrayIndex != 0){
    float maxPower = solarDataArr[0].power;   // Assume first element is the largest
  
    for(int i = 0; i < arrayIndex; i++){
     if(maxPower < solarDataArr[i].power)    // If there's larger power in the array
        maxPower = solarDataArr[i].power;     // Replace it and continue the iterations
      }
   return maxPower;
    }
} 

/*
 * @desc: Prevent same max power sent to ThingsBoard
 * @retval: true if max power is same or vice versa
 */
boolean isMaxPowerRepeat(){
  if(maxPowerDetected == previousMaxPower)
    return true;
  else
    return false;  
  }
  
/*
 * @desc: Upload solar data to ThingsBoard
 * @param: Solar data (Voltage, Current and Power)
 */
void uploadSolarDataToThingsBoard(SolarData solarData){
  tb.sendTelemetryFloat(VOLTAGE_PARAMETER_TB, solarData.volt);
  tb.sendTelemetryFloat(CURRENT_PARAMETER_TB, solarData.current);
  tb.sendTelemetryFloat(POWER_PARAMETER_TB, solarData.power);
  if(!isMaxPowerRepeat())
    tb.sendTelemetryFloat(MAX_POWER_PARAMETER_TB, maxPowerDetected);
  }
  
/*
 * @desc: Main function of MPPT, it will call neccessary
 *        functions to perform MPPT
 */
void MPPT(){
  // Read Voltage function
  float volt = random(0,5);
  // Read Current function
  float current = 0.1;
  insertVoltageAndCurrentIntoArray(volt, current);
  // Get power
  float power = getSolarPower(volt, current);
  insertPowerIntoArray(power);

  maxPowerDetected = getMaxPowerInArr();
  uploadSolarDataToThingsBoard(solarDataArr[arrayIndex]);
  arrayIndex++;
  previousMaxPower = maxPowerDetected;
  }

/*
 * @desc: Connect device to WiFi
 * @ref: [1.]
 */
void InitWiFi()
{
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
    if ( client.connect("Solar Node", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
}

void loop() {
  // put your main code here, to run repeatedly:
  
 if ( !client.connected()) {
    reconnect();
  }
  delay(1000);
  MPPT();
  client.loop();
}
