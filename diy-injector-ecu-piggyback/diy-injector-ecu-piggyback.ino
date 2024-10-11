#include <TimerOne.h>

enum OPERATIONAL_MODE {
  DECREASE_INJECTOR_ON_TIME,
  INCREASE_INJECTOR_ON_TIME,
};

const OPERATIONAL_MODE MODE = INCREASE_INJECTOR_ON_TIME;
const int PERCENTAGE_LEVEL = 25;
const int INIT_DELAY_ON_MICRO_SECONDS = 1000;

//TODO const int MAX_ON_INJECTOR_MICRO_SECONDS = 12000;

//piggyback injector pins
uint8_t GPIO_InjectorIN = A0;
uint8_t GPIO_InjectorOUT = A2;

//variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long onFromECUInjectorMicroSeconds = 0;
unsigned long offFromECUInjectorMicroSeconds = 0;
unsigned long timerTriggerMicroSeconds = INIT_DELAY_ON_MICRO_SECONDS;
unsigned long delayToCloseRealInjectorMicroSeconds = 0;
volatile unsigned long microSecondsCount = 0;
volatile boolean timerTrigger = false;
//TODO volatile boolean closeInjectorEarlierTrigger = false;
byte newDutyCycle = 0;

boolean firstTimeOnInjectorEcu = true;
boolean firstTimeOffInjectorEcu = true;

unsigned long warnCount = 0;

void infoPrint();
unsigned long computeTriggerTimeDependingOnOperationMode();
unsigned long readTimer();
byte computeNewDutyCycle();

void Timer1_ISR(void) {
  if (microSecondsCount == timerTriggerMicroSeconds) {
    timerTrigger = true;
  }
  microSecondsCount += 10;
}

void setup() {
  Timer1.initialize(10);  // Fire An Interrupt Every 0.01milliseconds
  Timer1.attachInterrupt(Timer1_ISR);

  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, LOW);  // turn the injector OFF (HIGH)
  delay(1200);
  infoPrint();
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);
  if (injectorIn == LOW) {
    //the input is negative, meaning that the ECU want's to open the injector
    if (firstTimeOnInjectorEcu) {
      //open the real injector output (HIGH) right away
      digitalWrite(GPIO_InjectorOUT, HIGH);

      //read ECU injector off time
      offFromECUInjectorMicroSeconds = readAndResetTimer();

      if (MODE == INCREASE_INJECTOR_ON_TIME) {
        //if the real injector wasn't closed at all
        if (timerTriggerMicroSeconds > offFromECUInjectorMicroSeconds) {
          warnCount++;
          //display debugging information
          Serial.print(",");
          Serial.print(offFromECUInjectorMicroSeconds);
          Serial.print(",100%,");  //the new dutty cycle
          Serial.println(warnCount);
        }
      }

      //first time on took place already
      firstTimeOnInjectorEcu = false;
      //reset the value for the first time off
      firstTimeOffInjectorEcu = true;
    }

    if (MODE == DECREASE_INJECTOR_ON_TIME) {
      if (timerTrigger) {
        //close the real injector output (LOW) ahead of time
        digitalWrite(GPIO_InjectorOUT, LOW);
        timerTrigger = false;

        //compute the new duty cycle
        newDutyCycle = computeNewDecreaseDutyCycle();

        //display debugging information
        Serial.print(",");
        Serial.print(offFromECUInjectorMicroSeconds);
        Serial.print(",");
        Serial.print(newDutyCycle);
        Serial.print("%,");
        Serial.println(warnCount);
      }
    }

  } else {
    if (firstTimeOffInjectorEcu) {
      if (MODE == DECREASE_INJECTOR_ON_TIME) {
        //if the ECU closes the injector input, will also close the output
        digitalWrite(GPIO_InjectorOUT, LOW);
      }
      //read the ECU injector on time
      onFromECUInjectorMicroSeconds = readAndResetTimer();

      if (MODE == DECREASE_INJECTOR_ON_TIME) {
        //if the real injector wasn't closed before the ECU wanted to close
        if (timerTriggerMicroSeconds > onFromECUInjectorMicroSeconds) {
          warnCount++;
          //display debugging information
          newDutyCycle = computeECUDutyCycle();
          Serial.print(",");
          Serial.print(offFromECUInjectorMicroSeconds);
          Serial.print(",");
          Serial.print(newDutyCycle);
          Serial.print("%,");
          Serial.println(warnCount);
        }
      }

      //compute the next delay based on how many loops the injector ECU input was open
      timerTriggerMicroSeconds = computeTriggerTimeDependingOnOperationMode();

      //display debugging information
      Serial.print(",");
      Serial.print(PERCENTAGE_LEVEL);
      Serial.print("%,");
      Serial.print(onFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(timerTriggerMicroSeconds);

      //reset the values
      firstTimeOnInjectorEcu = true;
      firstTimeOffInjectorEcu = false;
    }

    if (MODE == INCREASE_INJECTOR_ON_TIME) {
      if (timerTrigger) {
        //close the real injector output (LOW) after a delay to increase the injector opening time
        digitalWrite(GPIO_InjectorOUT, LOW);
        timerTrigger = false;

        //compute the new duty cycle
        newDutyCycle = computeNewIncreaseDutyCycle();

        //display debugging information
        Serial.print(",");
        Serial.print(offFromECUInjectorMicroSeconds);
        Serial.print(",");
        Serial.print(newDutyCycle);
        Serial.print("%,");
        Serial.println(warnCount);
      }
    }
  }
}

byte computeECUDutyCycle() {
  if ((offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) != 0) {
    return (byte)(((float)onFromECUInjectorMicroSeconds) / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) * 100);
  } else {
    return 0;
  }
}

byte computeNewDecreaseDutyCycle() {
  if ((offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) != 0) {
    return (byte)((((float)timerTriggerMicroSeconds) / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds)) * 100);
  } else {
    return 0;
  }
}

byte computeNewIncreaseDutyCycle() {
  if ((offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) != 0) {
    return (byte)((((float)(onFromECUInjectorMicroSeconds + timerTriggerMicroSeconds)) / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds)) * 100);
  } else {
    return 0;
  }
}

unsigned long readAndResetTimer() {
  //make sure we prepare for the next trigger
  Timer1.stop();
  //reset also the triggers when restart the counter
  timerTrigger = false;
  unsigned long tmp = microSecondsCount;
  microSecondsCount = 0;
  Timer1.start();
  return tmp;
}

unsigned long computeTriggerTimeDependingOnOperationMode() {
  if (PERCENTAGE_LEVEL > 0 || PERCENTAGE_LEVEL < 100) {
    //make sure to round up at 10s of microseconds
    if (MODE == DECREASE_INJECTOR_ON_TIME) {
      //for the decrease operation mode: we need to compute the remaining open time
      return (long)((onFromECUInjectorMicroSeconds * (100 - PERCENTAGE_LEVEL)) / 1000) * 10;
    } else {
      //for the increase operation mode: we need to compute the delay of closing time
      return (long)((onFromECUInjectorMicroSeconds * PERCENTAGE_LEVEL) / 1000) * 10;
    }
  }
  return 0;
}


void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  if (MODE == DECREASE_INJECTOR_ON_TIME) {
    Serial.println(",DelayPercentage,OnInjectorECU,ActualOnInjector,OffInjectorECU,WarnCounts");
  } else {
    Serial.println(",DelayPercentage,OnInjectorECU,AddedDelayOnToInjector,OffInjectorECU,WarnCounts");
  }
}
