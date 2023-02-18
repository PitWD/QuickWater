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

  RTC_GetDateTime();

strcpy(RunningTimers[0].name, "Light Flower-1");
RunningTimers[0].onTime = 30600;
RunningTimers[0].offTime = 27000;
RunningTimers[0].offset = 0;
RunningTimers[0].state.automatic = 1;
RunningTimers[0].state.dayTimer = 0;
RunningTimers[0].state.interrupted = 0;
RunningTimers[0].tempUntil = 0;
strcpy(RunningTimers[1].name, "Light Flower-2");
RunningTimers[1].onTime = 30600;
RunningTimers[1].offTime = 27000;
RunningTimers[1].offset = 900;
RunningTimers[1].state.automatic = 1;
RunningTimers[1].state.dayTimer = 0;
RunningTimers[1].state.interrupted = 0;
RunningTimers[1].tempUntil = 0;
strcpy(RunningTimers[2].name, "Light Flower-3");
RunningTimers[2].onTime = 30600;
RunningTimers[2].offTime = 27000;
RunningTimers[2].offset = 1800;
RunningTimers[2].state.automatic = 1;
RunningTimers[2].state.dayTimer = 0;
RunningTimers[2].state.interrupted = 0;
RunningTimers[2].tempUntil = 0;
strcpy(RunningTimers[3].name, "Light Flower-4");
RunningTimers[3].onTime = 30600;
RunningTimers[3].offTime = 27000;
RunningTimers[3].offset = 27000;
RunningTimers[3].state.automatic = 1;
RunningTimers[3].state.dayTimer = 0;
RunningTimers[3].state.interrupted = 0;
RunningTimers[3].tempUntil = 0;

strcpy(RunningTimers[4].name, "Light Vega-1");
RunningTimers[4].onTime = 37800;
RunningTimers[4].offTime = 16200;
RunningTimers[4].offset = 60;
RunningTimers[4].state.automatic = 1;
RunningTimers[4].state.dayTimer = 0;
RunningTimers[4].state.interrupted = 0;
RunningTimers[4].tempUntil = 0;

strcpy(RunningTimers[5].name, "Light Vega-?");
RunningTimers[5].onTime = 37800;
RunningTimers[5].offTime = 16200;
RunningTimers[5].offset = 60;
RunningTimers[5].state.automatic = 0;
RunningTimers[5].state.dayTimer = 0;
RunningTimers[5].state.interrupted = 0;
RunningTimers[5].tempUntil = 0;

strcpy(RunningTimers[6].name, "Light Clone-1");
RunningTimers[6].onTime = 37800;
RunningTimers[6].offTime = 16200;
RunningTimers[6].offset = 120;
RunningTimers[6].state.automatic = 1;
RunningTimers[6].state.dayTimer = 0;
RunningTimers[6].state.interrupted = 0;
RunningTimers[6].tempUntil = 0;
strcpy(RunningTimers[7].name, "Light Clone-2");
RunningTimers[7].onTime = 37800;
RunningTimers[7].offTime = 16200;
RunningTimers[7].offset = 1320;
RunningTimers[7].state.automatic = 1;
RunningTimers[7].state.dayTimer = 0;
RunningTimers[7].state.interrupted = 0;
RunningTimers[7].tempUntil = 0;
strcpy(RunningTimers[8].name, "Light Clone-3");
RunningTimers[8].onTime = 37800;
RunningTimers[8].offTime = 16200;
RunningTimers[8].offset = 2520;
RunningTimers[8].state.automatic = 1;
RunningTimers[8].state.dayTimer = 0;
RunningTimers[8].state.interrupted = 0;
RunningTimers[8].tempUntil = 0;

strcpy(RunningTimers[9].name, "Water Flower-1");
RunningTimers[9].onTime = 37800;
RunningTimers[9].offTime = 16200;
RunningTimers[9].offset = 2520;
RunningTimers[9].state.automatic = 1;
RunningTimers[9].state.dayTimer = 0;
RunningTimers[9].state.interrupted = 0;
RunningTimers[9].tempUntil = 0;
strcpy(RunningTimers[10].name, "Water Flower-2");
RunningTimers[10].onTime = 37800;
RunningTimers[10].offTime = 16200;
RunningTimers[10].offset = 2520;
RunningTimers[10].state.automatic = 1;
RunningTimers[10].state.dayTimer = 0;
RunningTimers[10].state.interrupted = 0;
RunningTimers[10].tempUntil = 0;

strcpy(RunningTimers[11].name, "Water Vega-1");
RunningTimers[11].onTime = 37800;
RunningTimers[11].offTime = 16200;
RunningTimers[11].offset = 2520;
RunningTimers[11].state.automatic = 1;
RunningTimers[11].state.dayTimer = 0;
RunningTimers[11].state.interrupted = 0;
RunningTimers[11].tempUntil = 0;



EscCls();
EscLocate(1,1);
Serial.print(F("Last  :"));
EscLocate(1,2);
Serial.print(F(" Now  :"));
EscLocate(1,3);
Serial.print(F("Inter :"));
EscLocate(1,4);
Serial.print(F("Next  :"));

int i = 1;

while (i){

  if (DoTimer()){
    EscLocate(9,1);
    Serial.print(LastInterval(myTime, 60, 60, 0));
    Serial.print(F("  "));
    EscLocate(9,2);
    Serial.print(IntervalTimer(myTime, 60, 60, 0));
    Serial.print(F("  "));
    EscLocate(9,3);
    Serial.print(InterruptedIntervalTimer(myTime, 60, 60, 0, 5, 5));
    Serial.print(F("  "));
    EscLocate(9,4);
    Serial.print(NextInterval(myTime, 60, 60, 0));
    Serial.print(F("  "));
  }

  if (Serial.available()){
    Serial.read();
    i = 0;
  }
}


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