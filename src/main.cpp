#include "myMenu.h"
#include <EEPROM.h>

#define WIRE Wire

// Seconds between RTC sync's (0 disables sync)
#define syncRTCinterval 0


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  Wire.setClock(31000L);
  Wire.begin();

  EzoScan();

  if (ezoCnt){
    // at least 1 ezo got found
    Serial.println("");
    Serial.println("Press a key to enter menu...");
    Serial.read();
    PrintMainMenu();
  }
  
}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
  // A Second is over...

  }

  if (Serial.available()){
    Serial.read();
    PrintMainMenu();
  }


}