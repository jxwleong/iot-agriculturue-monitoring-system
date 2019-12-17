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

// Variable for Mux
int outS0 = 0;
int outS1 = 0;
int outS2 = 0;

int zero = 0; // offset ADC val
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
    
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  muxInIt(S0, S1, S2);
  pinMode(A0, INPUT);
  selectMuxChannel(4);
  sensor.setVoltageReference(3.3);
  zero = sensor.calibrate();

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  selectMuxChannel(6);
  Serial.println("\nReading from Channel 6:");
  Serial.print("Voltage: ");
  Serial.print(getVoltageDC());
  Serial.print(" V");
  
  delay(3000);
  selectMuxChannel(4);
  Serial.println("\nReading from Channel 4:");
  Serial.print("Current: ");
  float val = analogRead(A0);
  Serial.print(sensor.getCurrentDC());
  Serial.print(" A");

}
