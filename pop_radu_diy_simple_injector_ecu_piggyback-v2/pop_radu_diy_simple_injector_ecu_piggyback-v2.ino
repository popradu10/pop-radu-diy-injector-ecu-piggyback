
//delay percentage we can select one of this value using switch or jumper
const int DELAY_PERCENTAGE_LEVEL = 40;
//default delay percentage if no switch or jumper is added to the pins bellow.
unsigned int delayPercentage = DELAY_PERCENTAGE_LEVEL;

//piggyback injector pins
uint8_t GPIO_InjectorIN = 25;
uint8_t GPIO_InjectorOUT = 26;

//loops variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long loopsOnInjectorCount = 0;
unsigned long loopsToDelayInjectorCount = 0;
unsigned long loopsOffInjectorCount = 0;

//onboard led and variables used for debuging
uint8_t GPIO_Led = 2;
unsigned long warnCount = 0;

void infoPrint();
unsigned long computeLoopsToDelay();

void setup() {
  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  pinMode(GPIO_Led, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  digitalWrite(GPIO_Led, HIGH);
  delay(1200);
  digitalWrite(GPIO_Led, LOW);
  infoPrint();
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);
  if (injectorIn == LOW) {
    //the input is negative, meaning that the ECU opens the injector
    //make sure we open the injector first think. injector output (LOW)
    // digitalWrite(GPIO_InjectorOUT, LOW);
    // digitalWrite(GPIO_Led, HIGH);
    if (loopsOnInjectorCount == 0) {
      //open the injector output (LOW)
      digitalWrite(GPIO_InjectorOUT, LOW);
      digitalWrite(GPIO_Led, HIGH);

      if (loopsToDelayInjectorCount > loopsOffInjectorCount) {
        //means that the loopsToDelayInjectorCount was not achieve, we keeped the injector open for a 100% dutty cycle
        Serial.print(",");
        Serial.print(loopsToDelayInjectorCount);
        Serial.print(",");
        Serial.print(loopsOffInjectorCount);
        Serial.print(",0,");
        Serial.print(",WARN,");
        //will increment the warning if the injector output was not closed between 2 cycles (injector open for 100% dutty cycle warning)
        warnCount++;
      } else {
        Serial.print(",");
        Serial.print(loopsOffInjectorCount);
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
      //compute the delay based on how many loops the injector was open
      loopsToDelayInjectorCount = computeLoopsToDelay();
      Serial.print(",");
      Serial.print(delayPercentage);
      Serial.print("%,");
      Serial.print(loopsOnInjectorCount);
      loopsOnInjectorCount = 0;
    }
    //delay closing of the injector until the delay is not apply
    if (loopsToDelayInjectorCount == loopsOffInjectorCount) {
      //close the injector after a delays (HIGH)
      digitalWrite(GPIO_InjectorOUT, HIGH);
      digitalWrite(GPIO_Led, LOW);      
      Serial.print(",");
      Serial.print(loopsToDelayInjectorCount);
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


void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  Serial.println(",DelayPercentage,LoopONInjectorECU,LoopDelayOutput,LoopInjectorOutputOff,Status,WarnCounts");
}
