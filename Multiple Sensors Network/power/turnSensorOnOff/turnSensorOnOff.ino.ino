
#define SOIL_MOISTURE_PIN   D0 // Soil Moisture sensor
#define DHT11_PIN           D1 // Temperature sensor
/*
 * @desc: Toggle the power of sensors connected to I/O pin
 * @param: Configuration of which sensor to turn on/off
 * Example:
 *    D0: Soil Mositure Sensor
 *    D1: Temperature Sensor
 *           (MSB) (LSB)
 *            D1    D0
 *            1     1     (Both on) configuration should be 3
 *                                  because in binary 2'b11 is 3
 *            1     0     Configuration should be 2 because                      
 *                        2'b10 is 2 in decimal
 */
void toggleSensorsPower(int configuration){
  digitalWrite(SOIL_MOISTURE_PIN, configuration & 1);  // LSB
  digitalWrite(DHT11_PIN, configuration & 2);  // MSB
  
  }

void setup() {
  // put your setup code here, to run once:
pinMode(SOIL_MOISTURE_PIN, OUTPUT);
pinMode(DHT11_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
toggleSensorsPower(1);
delay(1000);
toggleSensorsPower(0);
delay(1000);
}
