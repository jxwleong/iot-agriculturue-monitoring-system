#include <ACS712.h>

#define ANALOG_IN   A0
#define RESISTOR_1  30000.0 // ohm 
#define RESISTOR_2  7500.0  // ohm

#define REFERENCE_VOLT  3.3
#define ADC_SCALE       1024.0

// Definition of Mux
#define S0    D6
#define S1    D5
#define S2    D4

#define ACS712_MUX_CHANNEL  4
#define VOLTAGE_SENSOR_MUX_CHANNEL  6


// Variable for Mux
int outS0 = 0;
int outS1 = 0;
int outS2 = 0;

int zero = 0; // offset ADC val
int noOfSampling = 300;

float averageVolt=0;
float averageCurrent=0;
float averagePower=0;

// Offset obtained after running the main loop once
// when there is no current and voltage
float voltageOffset;
float currentOffset;

ACS712 sensor(ACS712_05B, A0);
/*
 * @desc: Setup the I/Os for selector of mux
 */
void muxInIt(int s0_pin, int s1_pin, int s2_pin){
  pinMode(s0_pin, OUTPUT);
  pinMode(s1_pin, OUTPUT);
  pinMode(s2_pin, OUTPUT);
}

/*
 * @desc: Select the analog input channel of Mux
 * @param: Number of analog channel
 */
void selectMuxChannel(int channel){
  // Get the bit to write on individual digital output
  // Eg
  // Channel 1 = (b001)&1  = 1 (Write 1 to S0)
  //           = (b001 >> 1 = bx00)&1 = 0 (Write 0 to S1)
  //           = (b001 >> 2 = bxx0)&1 = 0 (Write 0 to S2)
  outS0 = channel & 0x1;          
  outS1 = (channel >> 1) & 0x1;   
  outS2 = (channel >> 2) & 0x1;  
  

  digitalWrite(S0, outS0);
  digitalWrite(S1, outS1);
  digitalWrite(S2, outS2);
  } 

/*
 * @desc: Get voltage reading from sensor
 */
float getVoltageDC(){
  int adcVal = analogRead(ANALOG_IN);
  float vout = (adcVal *REFERENCE_VOLT)/ADC_SCALE;
  float voltage = vout/ (RESISTOR_2/ (RESISTOR_1 + RESISTOR_2));
  return voltage;
  }
  
/*
 * @desc: Calculate the offset voltage when there's no voltage
 */
float getVoltageOffset(){
  float averageVoltage = 0;
  // Get the average current
  selectMuxChannel(VOLTAGE_SENSOR_MUX_CHANNEL); // Read voltage sensor analog reading
  for(int i =0; i<noOfSampling;i++){
    delay(50);
    averageVoltage = averageVoltage + getVoltageDC();
  }
  averageVoltage = averageVoltage / noOfSampling;
  return averageVoltage;
  }  

/*
 * @desc: Calculate the offset current when there's no current
 */
float getCurrentOffset(){
  float averageCurrent = 0;
  // Get the average current
  selectMuxChannel(ACS712_MUX_CHANNEL); // Read current sensor analog reading
  for(int i =0; i<noOfSampling;i++){
    delay(50);
    averageCurrent = averageCurrent + sensor.getCurrentDC();
  }
  averageCurrent = averageCurrent / noOfSampling;
  return averageCurrent;
  }  

/*
 * @desc:get user input character from serial monitor
 * @retval: return character input by user
 */
char getCharFromSerialMonitor() {
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();
    Serial.println("\nReceived character: ");
    Serial.print(receivedChar);
    return receivedChar;
  }
  else
    return '\0';
}
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  muxInIt(S0, S1, S2);
  pinMode(A0, INPUT);

  // Make sure sensors are disconnected
  selectMuxChannel(ACS712_MUX_CHANNEL);
  sensor.setVoltageReference(3.3);
  zero = sensor.calibrate();

  // Obatin the offset voltage and current
  // when there's no power source
  //voltageOffset = getVoltageOffset();
  //currentOffset = getCurrentOffset();
  voltageOffset = 0;
  currentOffset = 0;
  //Serial.println("\nVoltage Offset: ");
  //Serial.print(voltageOffset);
  //Serial.println("\nCurrent Offset: ");
  //Serial.print(currentOffset);


}

void loop() {
 // delay(10000);
  // put your main code here, to run repeatedly:

  if(getCharFromSerialMonitor() == 'y'){
    for(int i =0; i<noOfSampling;i++){
    delay(1000);
    selectMuxChannel(VOLTAGE_SENSOR_MUX_CHANNEL);
    Serial.println("\nReading from Channel 6:");
    Serial.print("Voltage: ");
    float volt = getVoltageDC()-voltageOffset;
    averageVolt = averageVolt + volt;
    Serial.print(volt);
    Serial.print(" V");

    selectMuxChannel(ACS712_MUX_CHANNEL);
    Serial.println("\nReading from Channel 4:");
    Serial.print("Current: ");
    float current = sensor.getCurrentDC()-currentOffset;
    averageCurrent = averageCurrent + current;
    Serial.print(current);
    Serial.print(" A");
  
    Serial.println("\n Power obtained: ");
    float pwr = volt * current;
    Serial.print(pwr);
    averagePower = averagePower + pwr;
    Serial.print(" W");
    }
    Serial.println("\n Average voltage:");
    Serial.print(averageVolt/noOfSampling);
    Serial.print(" V");
  
    Serial.println("\n Average current:");
    Serial.print(averageCurrent/noOfSampling);
    Serial.print(" A");
    Serial.println("\n Average power:");
    Serial.print(averagePower/noOfSampling);
    Serial.print(" W");
    }
}
