#include "esp_system.h"
#include "rom/ets_sys.h"
#include "esp32-hal-timer.h"

//configure output data levels
const bool logVerbose = true;
const bool enableInjectorDutyCyclePercentageAvg = true;


//using a hw timer for tracking/debuging purposes
hw_timer_t *HwTimer = NULL;
uint64_t timerMicro = 0;
uint64_t onTimeMicroToComputeInjectorDutyCycleAvg = 0;
uint64_t offTimeMicroToComputeInjectorDutyCycleAvg = 0;
uint16_t indexToComputeInjectorDutyCycleAvg = 0;
uint16_t injectorDutyCyclePercentageAvg = 0;

//delay percentage we can select one of this value using switch or jumper
const int DELAY_PERCENTAGE_LEVEL_0 = 45;
const int DELAY_PERCENTAGE_LEVEL_1 = 25;
const int DELAY_PERCENTAGE_LEVEL_2 = 35;
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

//onboard led and variables used for debuging
uint8_t GPIO_Led = 2;
unsigned long warnCount = 0;

void infoPrint();
void checkPercentageBTN();
unsigned long computeLoopsToDelay();
void computeInjectorDutyCyclePercentageAvg();

void setup() {
  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_DELAY_LEVEL_1, INPUT_PULLUP);
  pinMode(GPIO_DELAY_LEVEL_2, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  pinMode(GPIO_Led, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  digitalWrite(GPIO_Led, HIGH);
  delay(1200);
  digitalWrite(GPIO_Led, LOW);
  HwTimer = timerBegin(0, 80, true);
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
        //means that the loopsToDelayInjectorCount was not achieve, we keeped the injector open for a 100% dutty cycle
        if (enableInjectorDutyCyclePercentageAvg) {
          onTimeMicroToComputeInjectorDutyCycleAvg += timerMicro;
        }
        if (logVerbose) {
          Serial.print(",");
          Serial.print(loopsToDelayInjectorCount);
          Serial.print(timerMicro);
          Serial.print(",");
          Serial.print(loopsOffInjectorCount);
          Serial.print(",0,");
          Serial.print(",WARN,");
        }
        //will increment the warning if the injector output was not closed between 2 cycles (injector open for 100% dutty cycle warning)
        warnCount++;
      } else {
        if (enableInjectorDutyCyclePercentageAvg) {
          offTimeMicroToComputeInjectorDutyCycleAvg += timerMicro;
        }
        if (logVerbose) {
          Serial.print(",");
          Serial.print(loopsOffInjectorCount);
          Serial.print(",");
          //how much time the ECU closes the injector in microseconds
          Serial.print(timerMicro);
          Serial.print(",OK,");
        }
      }
      if (logVerbose) {
        Serial.println(warnCount);
      }
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
      if (enableInjectorDutyCyclePercentageAvg && indexToComputeInjectorDutyCycleAvg >= 100) {
        computeInjectorDutyCyclePercentageAvg();
      }

      if (enableInjectorDutyCyclePercentageAvg) {
        onTimeMicroToComputeInjectorDutyCycleAvg += timerMicro;
      }
      //compute the delay based on how many loops the injector was open
      loopsToDelayInjectorCount = computeLoopsToDelay();
      Serial.print(",");
      Serial.print(delayPercentage);
      Serial.print("%,");
      if (enableInjectorDutyCyclePercentageAvg) {
        Serial.print(injectorDutyCyclePercentageAvg);
        Serial.print("%,");
      }
      if (logVerbose) {
        Serial.print(loopsOnInjectorCount);
        Serial.print(",");
        //how much time the ECU opens the injector in microseconds
        Serial.print(timerMicro);
      } else {
        Serial.println(warnCount);
      }
      loopsOnInjectorCount = 0;
    }
    //delay closing of the injector until the delay is not apply
    if (loopsToDelayInjectorCount == loopsOffInjectorCount) {
      //close the injector after a delays (HIGH)
      digitalWrite(GPIO_InjectorOUT, HIGH);
      //log how many microseconds the delay took
      timerMicro = timerReadMicros(HwTimer);
      timerRestart(HwTimer);
      if (enableInjectorDutyCyclePercentageAvg) {
        onTimeMicroToComputeInjectorDutyCycleAvg += timerMicro;
        indexToComputeInjectorDutyCycleAvg += 1;
      }
      if (logVerbose) {
        Serial.print(",");
        Serial.print(loopsToDelayInjectorCount);
        Serial.print(",");
        //how much time we delayed closing of the injector in microseconds
        Serial.print(timerMicro);
      }
      //check if we change the delay percentage switch or jumper
      checkPercentageBTN();
    }
    //count how many loops the injector was closed
    loopsOffInjectorCount++;
  }
  // Serial.println("---");
  // Serial.print("---");
  // Serial.print(injectorDutyCyclePercentageAvg);
  // Serial.print("%,");
  // Serial.print("---");
  // Serial.print(onTimeMicroToComputeInjectorDutyCycleAvg);
  // Serial.print("---");
  // Serial.print(offTimeMicroToComputeInjectorDutyCycleAvg);
  // Serial.print("---");
  // Serial.print(indexToComputeInjectorDutyCycleAvg);
  // Serial.print("---");
}

unsigned long computeLoopsToDelay() {
  if (delayPercentage > 0) {
    return (long)(loopsOnInjectorCount * delayPercentage) / 100;
  }
  return 0;
}

void computeInjectorDutyCyclePercentageAvg() {
  if (onTimeMicroToComputeInjectorDutyCycleAvg + offTimeMicroToComputeInjectorDutyCycleAvg != 0
      && indexToComputeInjectorDutyCycleAvg != 0) {
    onTimeMicroToComputeInjectorDutyCycleAvg = onTimeMicroToComputeInjectorDutyCycleAvg / indexToComputeInjectorDutyCycleAvg;
    offTimeMicroToComputeInjectorDutyCycleAvg = offTimeMicroToComputeInjectorDutyCycleAvg / indexToComputeInjectorDutyCycleAvg;
    injectorDutyCyclePercentageAvg = (int)round(((((float) onTimeMicroToComputeInjectorDutyCycleAvg) / (onTimeMicroToComputeInjectorDutyCycleAvg + offTimeMicroToComputeInjectorDutyCycleAvg))) * 100);
  }
  onTimeMicroToComputeInjectorDutyCycleAvg = 0;
  offTimeMicroToComputeInjectorDutyCycleAvg = 0;
  indexToComputeInjectorDutyCycleAvg = 0;
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
  if (HwTimer == NULL) {
    Serial.println("HwTimer could not be init.");
  } else {
    Serial.println("All good.");
  }
  Serial.println("Good luck.");
  if (logVerbose) {
    if (enableInjectorDutyCyclePercentageAvg) {
      Serial.println(",DelayPercentage,NewDuttyCycle%,LoopONInjectorECU,TimeONInjectorECUMicro,LoopDelayOutput,TimeDelayOutputMicro,LoopInjectorOutputOff,TimeInjectorOutpurOffMicro,Status,WarnCounts");
    } else {
      Serial.println(",DelayPercentage,LoopONInjectorECU,TimeONInjectorECUMicro,LoopDelayOutput,TimeDelayOutputMicro,LoopInjectorOutputOff,TimeInjectorOutpurOffMicro,Status,WarnCounts");
    }
  } else {
    if (enableInjectorDutyCyclePercentageAvg) {
      Serial.println(",DelayPercentage,NewDuttyCycle%,WarnCounts");
    } else {
      Serial.println(",DelayPercentage,WarnCounts");
    }
  }
}
