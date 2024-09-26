
//delay percentage 
const int DELAY_PERCENTAGE_LEVEL = 50;

//default delay percentage if no switch or jumper is added to the pins bellow.
unsigned int delayPercentage = DELAY_PERCENTAGE_LEVEL;

//piggyback injector pins
uint8_t GPIO_InjectorIN = 25;
uint8_t GPIO_InjectorOUT = 26;

//loops variables used to count how much time the injector is open by ECU
//and how much to delay the injector output
unsigned long loopsOnFromECUInjectorCount = 0;
unsigned long loopsDelayToOpenRealInjectorCount = 1000;
unsigned long loopsOffFromECUInjectorCount = 0;

//onboard led and variables used for debuging
unsigned long warnCount = 0;

void infoPrint();
unsigned long computeLoopsToDelay();

void setup() {
  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorIN, INPUT_PULLUP);
  pinMode(GPIO_InjectorOUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1200);
  digitalWrite(LED_BUILTIN, LOW);
  infoPrint();
}

void loop() {
  int injectorIn = digitalRead(GPIO_InjectorIN);
  if (injectorIn == LOW) {
    //the input is negative, meaning that the ECU want's to open the injector
    if (loopsOnFromECUInjectorCount == 0) {
      digitalWrite(LED_BUILTIN, HIGH);

      //display debuging information
      Serial.print(",");
      Serial.print(loopsOffFromECUInjectorCount);
      Serial.print(",");
      Serial.println(warnCount);
      
      //reset the values for the off and delay
      loopsOffFromECUInjectorCount = 0;
      loopsDelayToOpenRealInjectorCount = 0;
    }
    if (loopsDelayToOpenRealInjectorCount == loopsOnFromECUInjectorCount) {
      //open the real injector output (LOW) only after some time
      digitalWrite(GPIO_InjectorOUT, LOW);
    } 
    //count how many loops the ECU injector pin open
    loopsOnFromECUInjectorCount++;
  } else {
    if (loopsOffFromECUInjectorCount == 0) {
      //if the ECU closes the injector input, will also close the output
      digitalWrite(GPIO_InjectorOUT, HIGH);
      digitalWrite(LED_BUILTIN, LOW);     
      
      //if the real injector was not open at all because of the opening delay
      if (loopsDelayToOpenRealInjectorCount < loopsOnFromECUInjectorCount){
        warnCount++;
      }

      //display debuging information
      Serial.print(",");
      Serial.print(delayPercentage);
      Serial.print("%,");
      Serial.print(loopsOnFromECUInjectorCount);
      Serial.print(",");
      Serial.print(loopsDelayToOpenRealInjectorCount);
      
      //compute the next delay based on how many loops the injector ECU input was open
      loopsDelayToOpenRealInjectorCount = computeLoopsToDelay();
      //reset the value for the on injector ECU input
      loopsOnFromECUInjectorCount = 0;
    }
   
    //count how many loops the injector was closed
    loopsOffFromECUInjectorCount++;
  }
}

unsigned long computeLoopsToDelay() {
  if (delayPercentage > 0) {
    return (long)(loopsOnFromECUInjectorCount * delayPercentage) / 100;
  }
  return 0;
}


void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
  Serial.println(",DelayPercentage,LoopOnInjectorECU,LoopToDelayOnInjector,LoopOffInjectorECU,WarnCounts");
}
