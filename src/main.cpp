#include "quicklib.h"
#include "myMenu.h"
#define WIRE Wire

//void myToRom();
//void myFromRom();

// global RunTime Timing
byte myRunSec = 0;
byte myRunMin = 0;
byte myRunHour = 0;
uint16_t myRunDay = 0;
byte myRunTime = 0;

// global RealTime Timing
byte mySec = 0;
byte myMin = 0;
byte myHour = 0;
byte myDay = 1;
byte myMonth = 1;
uint16_t myYear = 2023;
uint32_t myTime = 0;

void setup() {
  // put your setup code here, to run once:

  myFromRom();

  Serial.begin(mySpeed);

  Wire.setClock(31000L);
  Wire.begin();

  delay(300);

  RTC_GetDateTime();

  delay(300);

  if (myDefault == 1 && myCnt && myCnt <= EZO_MAX_PROBES){ 
    DefaultProbesFromRom();
    ezoCnt = myCnt;
    PrintLoopMenu();
  }
  else{
    EzoScan();
    PrintMainMenu();
  }
  
}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    byte err = 1;

    // Print Runtime
    EscLocate(67,1);
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