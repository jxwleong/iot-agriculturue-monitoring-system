/*
    ESP8266 Timer Example
    Hardware: NodeMCU
    Circuits4you.com
    2018
    LED Blinking using Timer
*/
// Original Author

#include <ESP8266WiFi.h>
#include <Ticker.h>

Ticker blinker;

#define LED D2  //On board LED

#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256

int interruptTimerInMilliS = 500;
//For references goto 
//1. https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h
//2. https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_timer.cpp


// @param : CPU frequency, frequency divider, timer in milliseconds
// @retval : number of ticks for achieve desired time
uint32_t getTimerTicks(uint32_t freq, int freqDivider, int milliSeconds){
  uint32_t  dividedFreq = 0;
  float period = 0, ticks = 0;
  dividedFreq = freq/ freqDivider;
  period = (float)(1) / (float)(dividedFreq);
  ticks = ((float)(milliSeconds)/(float)(1000) / period);
  // get the value in milliSeconds then div by period
  return ticks;
  }
  
void ICACHE_RAM_ATTR onTimerISR(){
    digitalWrite(LED,!(digitalRead(LED)));  //Toggle LED Pin
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV16, interruptTimerInMilliS)); // write 0.5s ticks
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED,OUTPUT);

    //Initialize Ticker every 0.5s
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV16, interruptTimerInMilliS)); // write 0.5s ticks
    // 1/F = period
    // Desired time / period = desired ticks
}

void loop()
{
    //timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV16, 100)); // write 0.5s ticks
  interruptTimerInMilliS = 200;
    // 80 M / 16  = F
}
