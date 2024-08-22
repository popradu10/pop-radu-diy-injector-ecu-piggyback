// #include "esp_system.h"

//injector pins
uint8_t GPIO_InjectorOUT = 26;

//onboard led and variables used for debuging
uint8_t GPIO_Led = 2;

hw_timer_t *HwTimer = NULL;
uint64_t timerMicro = 0;
const uint64_t timerMicroOn =  300000;
const uint64_t timerMicroOff = 500000;


void setup() {
  Serial.begin(115200); /* initialise serial communication */
  pinMode(GPIO_InjectorOUT, OUTPUT);
  pinMode(GPIO_Led, OUTPUT);
  digitalWrite(GPIO_InjectorOUT, HIGH);  // turn the injector OFF (HIGH)
  digitalWrite(GPIO_Led, HIGH);
  delay(1200);
  digitalWrite(GPIO_Led, LOW);
  HwTimer = timerBegin(0, 80, true);
  infoPrint();
  timerStart(HwTimer);
}

void loop() {
  timerMicro = timerReadMicros(HwTimer);
  if (timerMicro <= timerMicroOn) {
      //open the injector output (LOW)
      digitalWrite(GPIO_InjectorOUT, LOW);
      digitalWrite(GPIO_Led, HIGH);
  } else if (timerMicro <= timerMicroOn + timerMicroOff) {
      //close the injector (HIGH)
      digitalWrite(GPIO_InjectorOUT, HIGH);
      digitalWrite(GPIO_Led, LOW);
  } else {
    timerRestart(HwTimer);
  }
}

void infoPrint() {
  Serial.println("All good.");
  Serial.println("Good luck.");
}
