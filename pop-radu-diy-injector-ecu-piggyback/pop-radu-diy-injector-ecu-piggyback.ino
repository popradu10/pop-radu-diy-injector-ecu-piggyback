#include "esp_system.h"
#include "rom/ets_sys.h"
#include "esp32-hal-timer.h"

//delay percentage we can select one of this value using switch or jumper
const int DELAY_PERCENTAGE_LEVEL_0 = 25; 
const int DELAY_PERCENTAGE_LEVEL_1 = 35;
const int DELAY_PERCENTAGE_LEVEL_2 = 45;
//default delay percentage if no switch or jumper is added to the pins bellow.
unsigned int delayPercentage = DELAY_PERCENTAGE_LEVEL_0; 

//piggyback injector pins
uint8_t GPIO_InjectorIN = 25;
uint8_t GPIO_InjectorOUT = 26;

//this pin will activate the DELAY_PERCENTAGE_LEVEL_1 or DELAY_PERCENTAGE_LEVEL_2
uint8_t GPIO_DELAY_LEVEL_1 = 13; 
uint8_t GPIO_DELAY_LEVEL_2 = 14; 

//loops variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long loopsOnInjectorCount = 0;
unsigned long loopsToDelayInjectorCount = 0;
unsigned long loopsOffInjectorCount = 0;


//using a hw timer for tracking/debuging purposes
hw_timer_t *HwTimer = NULL; 
uint64_t timerMicro = 0;

//onboard led and variables used for debuging
uint8_t GPIO_Led = 2;
unsigned long warnCount = 0;

void infoPrint();
void checkPercentageBTN();
unsigned long computeLoopsToDelay();

void setup() {
  Serial.begin(115200); /* initialise serial communication */
  HwTimer = timerBegin(80);
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_DELAY_LEVEL_1, INPUT_PULLUP);
  pinMode(GPIO_DELAY_LEVEL_2, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  pinMode(GPIO_Led, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  digitalWrite(GPIO_Led, HIGH);
  delay(1200);
  digitalWrite(GPIO_Led, LOW);
  infoPrint();
  checkPercentageBTN();
  timerStart(HwTimer);
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);

  if (injectorIn == LOW) {  
    //the input is negative, meaning that the ECU opens the injector
    if (loopsOnInjectorCount == 0) {
      //open the injector output (LOW)
      digitalWrite(GPIO_InjectorOUT, LOW);
      digitalWrite(GPIO_Led, HIGH);
      //this timer will represent how much time the injector output was off
      timerMicro = timerReadMicros(HwTimer);
      //restart the timer so will know how much time the injector ECU is open
      timerRestart(HwTimer);
      if (loopsToDelayInjectorCount > loopsOffInjectorCount) {
        //means that the loopsToDelayInjectorCount was not added
        Serial.print(",");
        Serial.print(loopsToDelayInjectorCount);
        Serial.print(",0,");
        Serial.print(loopsOffInjectorCount);
        Serial.print(timerMicro);
        Serial.print(",");
        Serial.print(",WARN,");
        //will increment the warning if the injector output was not closed between 2 cycles
        warnCount++;
      } else {
        Serial.print(",");
        Serial.print(loopsOffInjectorCount);
        Serial.print(",");
        //how much time the ECU closes the injector in microseconds
        Serial.print(timerMicro);
        Serial.print(",OK,");
      }
      Serial.println(warnCount);
      loopsOffInjectorCount = 0;
      loopsToDelayInjectorCount = 0;
    }
    //count how many loops the injector was open
    loopsOnInjectorCount++;
  } else {
    //even if the ECU closes the injector input, will not close the output
    if (loopsOffInjectorCount == 0) {
      digitalWrite(GPIO_Led, LOW);
      timerMicro = timerReadMicros(HwTimer);
      timerRestart(HwTimer);
      //compute the delay based on how many loops the injector was open
      loopsToDelayInjectorCount = computeLoopsToDelay();
      Serial.print(",");
      Serial.print(delayPercentage);
      Serial.print("%,");
      Serial.print(loopsOnInjectorCount);
      Serial.print(",");
      //how much time the ECU opens the injector in microseconds
      Serial.print(timerMicro);
      loopsOnInjectorCount = 0;
    }
    //delay closing of the injector until the delay is not apply
    if (loopsToDelayInjectorCount == loopsOffInjectorCount) {
      //close the injector after a delays (HIGH)
      digitalWrite(GPIO_InjectorOUT, HIGH);
      //log how many microseconds the delay took
      timerMicro = timerReadMicros(HwTimer);
      Serial.print(",");
      Serial.print(loopsToDelayInjectorCount);
      Serial.print(",");
      //how much time we delayed closing of the injector in microseconds
      Serial.print(timerMicro);
      //check if we change the delay percentage switch or jumper
      checkPercentageBTN();
    }
    //count how many loops the injector was closed
    loopsOffInjectorCount++;
  }
}

unsigned long computeLoopsToDelay() {
  if (delayPercentage > 0) {
    return (long)(loopsOnInjectorCount * delayPercentage) / 100;
  }
  return 0;
}

void checkPercentageBTN() {
  //check if the boost buttons were pusshed
  if (digitalRead(GPIO_DELAY_LEVEL_1) == LOW) {
    delayPercentage = DELAY_PERCENTAGE_LEVEL_1;
  } else if (digitalRead(GPIO_DELAY_LEVEL_2) == LOW) {
    delayPercentage = DELAY_PERCENTAGE_LEVEL_2;
  } else {
    delayPercentage = DELAY_PERCENTAGE_LEVEL_0;
  }
}

void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  Serial.println(",Percentage,LoopONInjectorECU,TimeONInjectorECUMicro,LoopDelayOutput,TimeDelayOutputMicro,LoopOffInjectorECU,TimerOffInjectorECUMicro,Status,WarnCounts");
}
