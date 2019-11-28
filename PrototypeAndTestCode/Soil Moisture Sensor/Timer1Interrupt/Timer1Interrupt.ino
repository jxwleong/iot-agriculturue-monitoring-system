/****************************************************************
 * Author  : Jason Leong Xie Wei
 * Contact : jason9829@live.com
 * Title : Control LED blink using timer1 interrupt
 * Hardware : NodeMCU ESP8266
 * Library Version:
 *  ArduinoJson : Version 5.13.5
 *  ThingsBoard : Version 0.2.0
 *  PubSubClient : Version 2.7.0
 ****************************************************************/

#include <ESP8266WiFi.h>
#include <Ticker.h>

// Definition for GPIO
#define LED D2  //On board LED

// Definition for timer
Ticker blinker;

#define CPU_FREQ_80M    80000000
#define CPU_FREQ_160M   160000000
#define TIM_FREQ_DIV1   1
#define TIM_FREQ_DIV16   16
#define TIM_FREQ_DIV256   256


int interruptTimerInMilliS = 500;

/*
 * @desc: Calcuate the number of ticks to write into timer
 * @param: CPU frequency, frequency divider, timer in milliseconds
 * @retval: number of ticks for achieve desired time
 */
uint32_t getTimerTicks(uint32_t freq, int freqDivider, int milliSeconds){
  uint32_t  dividedFreq = 0;
  float period = 0, ticks = 0;
  dividedFreq = freq/ freqDivider;
  period = (float)(1) / (float)(dividedFreq);
  ticks = ((float)(milliSeconds)/(float)(1000) / period);
  // get the value in milliSeconds then div by period
  return ticks;
  }


/*
 * @desc: Interrupt Service Routine when desired time is achieved
 */  
void ICACHE_RAM_ATTR onTimerISR(){
    Serial.println("500ms is up!");
    digitalWrite(LED,!(digitalRead(LED)));  //Toggle LED Pin
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS)); // write 0.5s ticks
}


// Main functions
void setup()
{
    Serial.begin(115200);

    pinMode(LED,OUTPUT);

    //Initialize Ticker every 0.5s
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV256, interruptTimerInMilliS)); // write 0.5s ticks
    // 1/F = period
    // Desired time / period = desired ticks
}

void loop()
{
    //timer1_write(getTimerTicks(CPU_FREQ_80M, TIM_FREQ_DIV16, 100)); // write 0.5s ticks
  interruptTimerInMilliS = 500;
    // 80 M / 16  = F
}


// References 
// [1.] Arduino/cores/esp8266/Arduino.h 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h
// [2.] Arduino/cores/esp8266/core_esp8266_timer.cpp 
//      https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_timer.cpp
//[3.] ESP8266 Timer and Ticker Example 
//     https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
