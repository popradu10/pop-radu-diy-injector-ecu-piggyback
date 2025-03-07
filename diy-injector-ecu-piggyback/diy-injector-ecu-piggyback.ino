#include <TimerOne.h>

int PERCENTAGE_LEVEL = 1;
// Percentage of additional fuel injection at low RPM
const int LOW_RPM_PERCENTAGE_LEVEL = 1;
// Percentage of additional fuel injection at middle RPM
const int MIDDLE_RPM_PERCENTAGE_LEVEL = 15;
// Percentage of additional fuel injection at high RPM
const int HIGH_RPM_PERCENTAGE_LEVEL = 25;

// Threshold for low RPM range (RPM value below which LOW_RPM_PERCENTAGE_LEVEL is used)
const int LOW_RPM_THRESHOLD = 3000;     //RPMs
// Threshold for middle RPM range (RPM value above LOW_RPM_THRESHOLD but below this value)
const int MIDDLE_RPM_THRESHOLD = 5000;  //RPMs

// Piggyback injector pins
uint8_t GPIO_InjectorIN = A0;
uint8_t GPIO_InjectorOUT = A2;

// Variables to count how long the injector is open (on) and delay for closing
long onFromECUInjectorMicroSeconds = 0;
long delayToCloseInjectorMicroSeconds = 0;
long offInjectorMicroSeconds = 0;

// Timer variables
volatile long timerTriggerMicroSeconds = 0;
volatile long microSecondsCount = 0;
volatile boolean timerTrigger = false;

// Flags for state tracking
boolean firstTimeOnInjectorEcu = true;
boolean firstTimeOffInjectorEcu = true;

// RPM computation trigger and value
boolean computeRPMTrigger = true;
unsigned int rpmValue = 0;

// Debugging counter for warnings
unsigned long warnCount = 0;

// Define DEBUG mode
#define DEBUG_MODE 1 // Set to 0 to disable debug messages

// Define DEBUG and DEBUGLN macros
#if DEBUG_MODE
  #define DEBUG(x) Serial.print(x)
  #define DEBUGLN(x) Serial.println(x)
#else
  #define DEBUG(x)    // No operation
  #define DEBUGLN(x)  // No operation
#endif

void infoPrint();
unsigned long computeDelayTriggerTime();

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
  delay(200);
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
      offInjectorMicroSeconds = readResetAndStopTimer();
      //start timer to count on injector time
      startTimer();

      //if the real injector wasn't closed the delayToCloseInjectorMicroSeconds for this cycle was not set
      if (delayToCloseInjectorMicroSeconds == -1) {
        warnCount++;
        DEBUG(",W");
        DEBUGLN(warnCount);
      }

      //first time on took place already
      firstTimeOnInjectorEcu = false;
      //reset the value for the first time off
      firstTimeOffInjectorEcu = true;
    }

  } else {
    //when the ECU injector is OFF

    if (firstTimeOffInjectorEcu) {
      //read the ECU injector on time
      onFromECUInjectorMicroSeconds = readResetAndStopTimer();
      //compute the next delay based on how many loops the injector ECU input was open
      timerTriggerMicroSeconds = computeDelayTriggerTime();
      //flag the delay to close with -1 to check later on if this happen
      delayToCloseInjectorMicroSeconds = -1;
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
      //read the real delay time
      delayToCloseInjectorMicroSeconds = readResetAndStopTimer();
      //start timer to count how much time the real injector is off
      startTimer();

      //display debugging information
      DEBUG(",");
      DEBUG(PERCENTAGE_LEVEL);
      DEBUG(",");
      DEBUG(onFromECUInjectorMicroSeconds);
      DEBUG(",");
      DEBUG(delayToCloseInjectorMicroSeconds);
      DEBUG(",");
      DEBUG(offInjectorMicroSeconds);
      DEBUG(",");
      DEBUG(warnCount);
      DEBUG(",");
      DEBUGLN(rpmValue);
      //trigger the rpm compute
      computeRPMTrigger = true;
    }

  }
}

void computeRPMandChangePercentageLevel() {
  //1 minute in microseconds * the total time for 2 rotation
  rpmValue = (long)(((float)60000000 / (offInjectorMicroSeconds + delayToCloseInjectorMicroSeconds + onFromECUInjectorMicroSeconds)) * 2);

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
  //don't set any trigger value when restarting the counter
  timerTriggerMicroSeconds = -1;
  //reset the trigger flag when restarting the counter
  timerTrigger = false;
  return microSecondsCount;
}

void startTimer() {
  microSecondsCount = 0;
  Timer1.start();
}


unsigned long computeDelayTriggerTime() {
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
  DEBUGLN(",DelayPercentage,OnInjectorECU,AddedDelayToInjector,OffInjector,WarnCounts,RPM");
}

