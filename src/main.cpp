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


memset(runningTimers, 0, RUNNING_TIMERS_CNT * sizeof(TimerSTRUCT));

strcpy(runningTimers[0].name, "Light Flower-1");
runningTimers[0].onTime = 30600;
runningTimers[0].offTime = 27000;
runningTimers[0].state.automatic = 1;
strcpy(runningTimers[1].name, "Light Flower-2");
runningTimers[1].onTime = 30600;
runningTimers[1].offTime = 27000;
runningTimers[1].offset = 900;
runningTimers[1].state.automatic = 1;
strcpy(runningTimers[2].name, "Light Flower-3");
runningTimers[2].onTime = 30600;
runningTimers[2].offTime = 27000;
runningTimers[2].offset = 1800;
runningTimers[2].state.automatic = 1;
strcpy(runningTimers[3].name, "Light Flower-4");
runningTimers[3].onTime = 30600;
runningTimers[3].offTime = 27000;
runningTimers[3].offset = 27000;
runningTimers[3].state.automatic = 1;

strcpy(runningTimers[4].name, "Light Vega-1");
runningTimers[4].onTime = 37800;
runningTimers[4].offTime = 16200;
runningTimers[4].offset = 60;
runningTimers[4].state.automatic = 1;

strcpy(runningTimers[5].name, "Light Vega-?");
runningTimers[5].onTime = 37800;
runningTimers[5].offTime = 16200;
runningTimers[5].offset = 60;

strcpy(runningTimers[6].name, "Light Clone-1");
runningTimers[6].onTime = 37800;
runningTimers[6].offTime = 16200;
runningTimers[6].offset = 120;
runningTimers[6].state.automatic = 1;
strcpy(runningTimers[7].name, "Light Clone-2");
runningTimers[7].onTime = 37800;
runningTimers[7].offTime = 16200;
runningTimers[7].offset = 1320;
runningTimers[7].state.automatic = 1;
strcpy(runningTimers[8].name, "Light Clone-3");
runningTimers[8].onTime = 37800;
runningTimers[8].offTime = 16200;
runningTimers[8].offset = 2520;
runningTimers[8].state.automatic = 1;

strcpy(runningTimers[9].name, "Water Flower-1");
runningTimers[9].onTime = 37800;
runningTimers[9].offTime = 16200;
runningTimers[9].offset = 2520;
runningTimers[9].state.automatic = 1;
strcpy(runningTimers[10].name, "Water Flower-2");
runningTimers[10].onTime = 37800;
runningTimers[10].offTime = 16200;
runningTimers[10].offset = 2520;
runningTimers[10].state.automatic = 1;

strcpy(runningTimers[11].name, "Water Vega-1");
runningTimers[11].onTime = 37800;
runningTimers[11].offTime = 16200;
runningTimers[11].offset = 2520;


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

    byte err = 1;

    err = RunTimers();

    // Print Runtime
    EscLocate(66,1);
    EscInverse(1);
    PrintRunTime();    
    // Print Realtime
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