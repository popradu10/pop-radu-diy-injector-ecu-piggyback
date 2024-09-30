#include <TimerOne.h>

//delay percentage
const int DELAY_PERCENTAGE_LEVEL = 50;

//piggyback injector pins
uint8_t GPIO_InjectorIN = A0;
uint8_t GPIO_InjectorOUT = A2;

//variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long onFromECUInjectorMicroSeconds = 0;
unsigned long offFromECUInjectorMicroSeconds = 0;
unsigned long delayToOpenRealInjectorMicroSeconds = 1000;
volatile unsigned long microSecondsCount = 0;
volatile boolean delayToOpenTrigger = false;

boolean firstTimeOnInjectorEcu = true;
boolean firstTimeOffInjectorEcu = true;

unsigned long warnCount = 0;

void infoPrint();
unsigned long computeMiCroSecondsToDelay();
unsigned long readTimer();

void Timer1_ISR(void) {
  if (microSecondsCount == delayToOpenRealInjectorMicroSeconds) {
    delayToOpenTrigger = true;
  }
  microSecondsCount += 1;
}

void setup() {
  Timer1.initialize(10);  // Fire An Interrupt Every 0.01milliseconds
  Timer1.attachInterrupt(Timer1_ISR);

  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  delay(1200);
  infoPrint();
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);
  if (injectorIn == LOW) {
    //the input is negative, meaning that the ECU want's to open the injector
    if (firstTimeOnInjectorEcu) {
      offFromECUInjectorMicroSeconds = readAndResetTimer();

      //display debuging information
      Serial.print(",");
      Serial.print(offFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.println(warnCount);

      //first time on took place already
      firstTimeOnInjectorEcu=false;
      //reset the value for the first time off
      firstTimeOffInjectorEcu=true;
    }

    if (delayToOpenTrigger) {
      //open the real injector output (LOW) only after some time
      digitalWrite(GPIO_InjectorOUT, LOW);
      delayToOpenTrigger = false;
    }
    
  } else {
    if (firstTimeOffInjectorEcu) {
      //if the ECU closes the injector input, will also close the output
      digitalWrite(GPIO_InjectorOUT, HIGH);
      onFromECUInjectorMicroSeconds = readAndResetTimer();

      //if the real injector was not open at all because of the opening delay
      if (delayToOpenRealInjectorMicroSeconds > onFromECUInjectorMicroSeconds) {
        warnCount++;
      }

      //compute the next delay based on how many loops the injector ECU input was open
      delayToOpenRealInjectorMicroSeconds = computeMiCroSecondsToDelay();

      //display debuging information
      Serial.print(",");
      Serial.print(DELAY_PERCENTAGE_LEVEL);
      Serial.print("%,");
      Serial.print(onFromECUInjectorMicroSeconds);
      Serial.print(",");
      Serial.print(delayToOpenRealInjectorMicroSeconds);

      //reset the values
      firstTimeOnInjectorEcu=true;
      firstTimeOffInjectorEcu=false;
    }
  }
}

unsigned long readAndResetTimer() {
  //make sure we prepare for the next trigger
  delayToOpenTrigger = false;
  Timer1.stop();
  unsigned long tmp = microSecondsCount;
  microSecondsCount = 0;
  Timer1.start();
  return tmp;
}

unsigned long computeMiCroSecondsToDelay() {
  if (DELAY_PERCENTAGE_LEVEL > 0) {
    return (long)(onFromECUInjectorMicroSeconds * DELAY_PERCENTAGE_LEVEL) / 100;
  }
  return 0;
}


void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  Serial.println(",DelayPercentage,OnInjectorECU,ToDelayOnInjector,OffInjectorECU,WarnCounts");
}
