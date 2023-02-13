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
    Serial.println(F(""));
    PrintMainMenu();
  }

}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    char err = 1;

    // Runtime
    EscLocate(66,1);
    EscInverse(1);
    PrintRunTime();
    //EscInverse(0);    
    
    // Realtime
    
    //EscColor(bgBlueB);
    //EscColor(fgBlack);
    EscLocate(61,24);
    PrintDateTime();
    Serial.print(F(" "));
    //EscColor(0);
    EscInverse(0);    

    //Read EZO's
    if (EzoDoNext() == 1){
      // All read
      err = PrintWaterVals(5);
      PrintAVGs(err + 1);
    }
    
  }

  if (Serial.available()){
    Serial.read();
    PrintMainMenu();
  }


}