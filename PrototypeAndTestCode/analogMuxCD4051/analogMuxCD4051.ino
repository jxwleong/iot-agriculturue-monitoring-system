
// Definition of Mux
#define S0    D6
#define S1    D5
#define S2    D4

// Variable for Mux
int outS0 = 0;
int outS1 = 0;
int outS2 = 0;

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

  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);

  muxInIt(S0, S1, S2);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  selectMuxChannel(0);
  Serial.println("\nReading from Channel 0:");
  Serial.print(analogRead(A0));
  
  delay(1000);
  selectMuxChannel(1);
  Serial.println("\nReading from Channel 1:");
  Serial.print(analogRead(A0));

  delay(1000);
  selectMuxChannel(2);
  Serial.println("\nReading from Channel 2:");
  Serial.print(analogRead(A0));

}
