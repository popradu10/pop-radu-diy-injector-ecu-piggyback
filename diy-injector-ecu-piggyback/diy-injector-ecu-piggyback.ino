#include <TimerOne.h>

int PERCENTAGE_LEVEL = 1;
const int LOW_RPM_PERCENTAGE_LEVEL = 5;
const int MIDDLE_RPM_PERCENTAGE_LEVEL = 12;
const int HIGH_RPM_PERCENTAGE_LEVEL = 15;
const int LOW_RPM_THRESHOLD = 3000;     //RPMs
const int MIDDLE_RPM_THRESHOLD = 5000;  //RPMs

//piggyback injector pins
uint8_t GPIO_InjectorIN = A0;
uint8_t GPIO_InjectorOUT = A2;

//variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long onFromECUInjectorMicroSeconds = 0;
unsigned long delayToCloseInjectorMicroSeconds = 0;
unsigned long offFromECUInjectorMicroSeconds = 0;

volatile unsigned long timerTriggerMicroSeconds = 0;
volatile unsigned long microSecondsCount = 0;
volatile boolean timerTrigger = false;

boolean firstTimeOnInjectorEcu = true;
boolean firstTimeOffInjectorEcu = true;

boolean computeRPMTrigger = true;
unsigned int rpmValue = 0;

unsigned long warnCount = 0;

void infoPrint();
unsigned long computeTriggerTimeDependingOnOperationMode();

void Timer1_ISR(void) {
  if (microSecondsCount == timerTriggerMicroSeconds) {
    timerTrigger = true;
  }
  microSecondsCount += 100;
}

void setup() {
  Timer1.initialize(100);  // Fire An Interrupt Every 0.01milliseconds
  Timer1.attachInterrupt(Timer1_ISR);
  Serial.begin(9600);
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, LOW);  // turn the injector OFF (HIGH)
  delay(1200);
  infoPrint();
  // testSerialSpeed();
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);
  if (injectorIn == LOW) {
    //the input is negative, meaning that the ECU want's to open the injector
    if (firstTimeOnInjectorEcu) {
      //open the real injector output (HIGH) right away
      digitalWrite(GPIO_InjectorOUT, HIGH);

      //read ECU injector off time
      offFromECUInjectorMicroSeconds = readResetAndStopTimer();
      //start timer as the timerTriggerMicroSeconds was already computed
      startTimer();

      //if the real injector wasn't closed at all
      if (timerTriggerMicroSeconds > offFromECUInjectorMicroSeconds) {
        warnCount++;
        //display debugging information
        Serial.print(",");
        Serial.print(PERCENTAGE_LEVEL);
        Serial.print("%,");
        Serial.print(onFromECUInjectorMicroSeconds);
        Serial.print(",");
        Serial.print(timerTriggerMicroSeconds);
        Serial.print(",0,");
        Serial.print(offFromECUInjectorMicroSeconds);
        Serial.print(",W");
        Serial.print(warnCount);
        Serial.println(",R0");
      }

      //first time on took place already
      firstTimeOnInjectorEcu = false;
      //reset the value for the first time off
      firstTimeOffInjectorEcu = true;
    }

  } else {
    if (firstTimeOffInjectorEcu) {
      //read the ECU injector on time
      onFromECUInjectorMicroSeconds = readResetAndStopTimer();
      //compute the next delay based on how many loops the injector ECU input was open
      timerTriggerMicroSeconds = computeTriggerTimeDependingOnOperationMode();
      //start timer only after the new timerTriggerMicroSeconds was computed
      startTimer();
      //reset the values
      firstTimeOnInjectorEcu = true;
      firstTimeOffInjectorEcu = false;
    }

    if (computeRPMTrigger) {
      computeRPMandChangePercentageLevel();
      computeRPMTrigger = false;
    }

    if (timerTrigger) {
      //close the real injector output (LOW) after a delay to increase the injector opening time
      digitalWrite(GPIO_InjectorOUT, LOW);

      //quick read the time without reseting the time, only the trigger
      Timer1.stop();
      timerTrigger = false;
      delayToCloseInjectorMicroSeconds = microSecondsCount;
      Timer1.start();

      //display debugging information
      Serial.print(",");
      Serial.print(PERCENTAGE_LEVEL);
      Serial.print("%,");
      Serial.print(onFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(timerTriggerMicroSeconds);
      Serial.print(",");
      Serial.print(delayToCloseInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(offFromECUInjectorMicroSeconds);
      Serial.print(",W");
      Serial.print(warnCount);
      Serial.print(",R");
      Serial.println(rpmValue);
      //trigger the rpm compute
      computeRPMTrigger = true;
    }

  }
}

void computeRPMandChangePercentageLevel() {
  //1 minute in microseconds * the total time for 2 rotation
  rpmValue = (long)(((float)60000000 / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds)) * 2);

  if (MIDDLE_RPM_THRESHOLD < rpmValue) {
    PERCENTAGE_LEVEL = HIGH_RPM_PERCENTAGE_LEVEL;
  } else if (LOW_RPM_THRESHOLD < rpmValue) {
    PERCENTAGE_LEVEL = MIDDLE_RPM_PERCENTAGE_LEVEL;
  } else {
    PERCENTAGE_LEVEL = LOW_RPM_PERCENTAGE_LEVEL;
  }
}

unsigned long readResetAndStopTimer() {
  //make sure we prepare for the next trigger
  Timer1.stop();
  //reset also the triggers when restart the counter
  timerTrigger = false;
  return microSecondsCount;
}

void startTimer() {
  microSecondsCount = 0;
  Timer1.start();
}


unsigned long computeTriggerTimeDependingOnOperationMode() {
  if (PERCENTAGE_LEVEL > 0 || PERCENTAGE_LEVEL < 100) {
    //for the increase operation mode: we need to compute the delay of closing time
    return (long)((onFromECUInjectorMicroSeconds * PERCENTAGE_LEVEL) / 10000) * 100;
  }
  return 0;
}


void infoPrint() {
  Serial.println("All good.");
  Serial.println(Serial.baud());
  Serial.println("Good luck.");
  Serial.println(",DelayPercentage,OnInjectorECU,ComputedDelayToInjector,DoubleCheckDelay,OffInjectorECU,WarnCounts,RPM");
}

void testSerialSpeed() {
  // Repeat a task multiple times
  int count = 0;
  for (int i = 0; i < 20; i++) {
    delay(100);
    readResetAndStopTimer();
    startTimer();
    Serial.print("Test Serial Speed ");
    Serial.print(i);
    Serial.print(" speed: ");
    Serial.println(readResetAndStopTimer());
  }
}
