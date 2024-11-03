#include <TimerOne.h>

int PERCENTAGE_LEVEL = 0;
const int LOW_RPM_PERCENTAGE_LEVEL = 1;
const int MIDDLE_RPM_PERCENTAGE_LEVEL = 15;
const int LOW_RPM_THRESHOLD = 3500; //RPMs

//piggyback injector pins
uint8_t GPIO_InjectorIN = A0;
uint8_t GPIO_InjectorOUT = A2;

//variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long onFromECUInjectorMicroSeconds = 0;
unsigned long offFromECUInjectorMicroSeconds = 0;
unsigned long delayToCloseRealInjectorMicroSeconds = 0;
unsigned int rpmValue = 0;

volatile unsigned long timerTriggerMicroSeconds = 0;
volatile unsigned long microSecondsCount = 0;
volatile boolean timerTrigger = false;

byte newDutyCycle = 0;

boolean firstTimeOnInjectorEcu = true;
boolean firstTimeOffInjectorEcu = true;

unsigned long warnCount = 0;

void infoPrint();
unsigned long computeTriggerTimeDependingOnOperationMode();
byte computeNewDutyCycle();
unsigned long computeRPM();

void Timer1_ISR(void) {
  if (microSecondsCount == timerTriggerMicroSeconds) {
    timerTrigger = true;
  }
  microSecondsCount += 10;
}

void setup() {
  Timer1.initialize(10);  // Fire An Interrupt Every 0.01milliseconds
  Timer1.attachInterrupt(Timer1_ISR);

  //Arduino Leonardo, Micro, and Other Boards with Native USB (e.g., Due)
  //Since these boards use a native USB port (instead of a USB-to-serial chip), they can support much higher baud rates.
  //For these, the maximum rate can be set theoretically up to 1,000,000 bps or even 2,000,000 bps, depending on the application and the USB host’s capability.
  c(2000000); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, LOW);  // turn the injector OFF (HIGH)
  delay(800);
  testSerialSpeed();
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
      offFromECUInjectorMicroSeconds = readResetAndStopTimer();
      //start timer as the timerTriggerMicroSeconds was already computed
      startTimer();

      //compute the RPM value based on ON and OFF value and change the percentage level accordingly
      computeRPMandChangePercentageLevel();

      //if the real injector wasn't closed at all
      if (timerTriggerMicroSeconds > offFromECUInjectorMicroSeconds) {
        warnCount++;
        //display debugging information
        Serial.print(",");
        Serial.print(offFromECUInjectorMicroSeconds);
        Serial.print(",100%,");  //the new duty cycle
        Serial.println(warnCount);
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

      //display debugging information
      Serial.print(",");
      Serial.print(PERCENTAGE_LEVEL);
      Serial.print("%,");
      Serial.print(onFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(microSecondsCount);
      Serial.print(",");
      Serial.print(timerTrigger);


      //reset the values
      firstTimeOnInjectorEcu = true;
      firstTimeOffInjectorEcu = false;
    }

    if (timerTrigger) {
      //close the real injector output (LOW) after a delay to increase the injector opening time
      digitalWrite(GPIO_InjectorOUT, LOW);
      //display debugging information
      Serial.print(",");
      Serial.print(timerTriggerMicroSeconds);
      Serial.print(",");
      Serial.print(microSecondsCount);

      timerTrigger = false;

      //compute the new duty cycle
      newDutyCycle = computeNewIncreaseDutyCycle();

      //display debugging information
      Serial.print(",");
      Serial.print(offFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(microSecondsCount);
      Serial.print(",");
      Serial.print(newDutyCycle);
      Serial.print("%,");
      Serial.print(rpmValue);
      Serial.print(",");
      Serial.println(warnCount);
    }
  }
}

void computeRPMandChangePercentageLevel() {
  //compute RPM
  rpmValue = computeRPM();
  if (LOW_RPM_THRESHOLD > rpmValue) {
    PERCENTAGE_LEVEL = LOW_RPM_PERCENTAGE_LEVEL;
  } else {
    PERCENTAGE_LEVEL = MIDDLE_RPM_PERCENTAGE_LEVEL;
  }
}

byte computeECUDutyCycle() {
  if ((offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) != 0) {
    return (byte)(((float)onFromECUInjectorMicroSeconds) / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds) * 100);
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
    return (long)((onFromECUInjectorMicroSeconds * PERCENTAGE_LEVEL) / 1000) * 10;
  }
  return 0;
}

unsigned long computeRPM() {
  //1 minute in microseconds * the total time for 2 rotation
  rpmValue = (long)(((float)60000000 / (offFromECUInjectorMicroSeconds + onFromECUInjectorMicroSeconds)) * 2);
}


void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  Serial.println(",DelayPercentage,OnInjectorECU,AddedDelayOnToInjector,OffInjectorECU,WarnCounts");
}

void testSerialSpeed() {
  // Repeat a task 10 times
  for (int i = 0; i < 10; i++) {
    delay(1000);
    Serial.println("Baud-rate = 2000000");
  }
}
