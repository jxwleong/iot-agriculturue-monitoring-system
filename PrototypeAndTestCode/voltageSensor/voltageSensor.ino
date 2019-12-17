#define ANALOG_IN   A0

#define RESISTOR_1  30000.0 // ohm 
#define RESISTOR_2  7500.0  // ohm

#define REFERENCE_VOLT  3.3
#define ADC_SCALE       1024.0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
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

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  /*
  int adcVal = analogRead(A0);
  Serial.print("\n ADC scale: ");
  Serial.print(ADC_SCALE);
  Serial.print(" ADC Value: ");
  Serial.print(adcVal);
  float vout = (adcVal *REFERENCE_VOLT)/ADC_SCALE;
  Serial.print(" Vref: ");
  Serial.print(REFERENCE_VOLT);
  Serial.print(" Resistor 1: ");
  Serial.print(RESISTOR_1);  Serial.print(" Resistor 2: ");
  Serial.print(RESISTOR_2);

  float voltage = vout/ (7500/ (37500));*/

  Serial.println(getVoltageDC());
}
