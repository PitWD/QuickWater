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
uint32_t myRunTime = 0;

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

  memset(tooLowSince, 0, sizeof(tooLowSince));
  memset(lowSince, 0, sizeof(lowSince));
  memset(highSince, 0, sizeof(highSince));
  memset(tooHighSince, 0, sizeof(tooHighSince));
  memset(lastAction, 0, sizeof(lastAction));

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
    long timeSinceHLP = 0;

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

      // Check High/Low of AVGs 
      // compare timeOuts with timing-setting
      for (byte i = 0; i < 8; i++){
        switch (GetAvgState(avgVal[i], tooLow[i], low[i], high[i], tooHigh[i])){
        case fgCyan:
          // tooLow
          timeSinceHLP = (myTime - tooLowSince[i] - lastAction[i]);
          if (!tooLowSince[i]){
            // 1st time tooLow recognized
            tooLowSince[i] = myTime;
          }
          else if (timeSinceHLP > delayTimes[i]){
            // Action Valid
            if ((timeSinceHLP - delayTimes[i]) > actionTooLow[i]){
              // ActionTime done...
              lastAction[i] = myTime;
              tooLowSince[i] = 0;
            }
            else{
              // Do Action
            }
          }
          break;
        case fgBlue:
          // Low
          timeSinceHLP = (myTime - lowSince[i] - lastAction[i]);
          if (!lowSince[i]){
            // 1st time tooLow recognized
            lowSince[i] = myTime;
          }
          else if (timeSinceHLP > delayTimes[i]){
            // Action Valid
            if ((timeSinceHLP - delayTimes[i]) > actionTooLow[i]){
              // ActionTime done...
              lastAction[i] = myTime;
              lowSince[i] = 0;
            }
            else{
              // Do Action
            }
          }
          break;
        case fgYellow:
          // High
          timeSinceHLP = (myTime - highSince[i] - lastAction[i]);
          if (!highSince[i]){
            // 1st time High recognized
            highSince[i] = myTime;
          }
          else if (timeSinceHLP > delayTimes[i]){
            // Action Valid
            if ((timeSinceHLP - delayTimes[i]) > actionTooLow[i]){
              // ActionTime done...
              lastAction[i] = myTime;
              highSince[i] = 0;
            }
            else{
              // Do Action
            }
          }
          break;
        case fgRed:
          // tooHigh
          timeSinceHLP = (myTime - tooHighSince[i] - lastAction[i]);
          if (!tooHighSince[i]){
            // 1st time tooLow recognized
            tooHighSince[i] = myTime;
          }
          else if (timeSinceHLP > delayTimes[i]){
            // Action Valid
            if ((timeSinceHLP - delayTimes[i]) > actionTooLow[i]){
              // ActionTime done...
              lastAction[i] = myTime;
              tooHighSince[i] = 0;
            }
            else{
              // Do Action
            }
          }
          break;
        default:
          // OK
          // Reset ...Since Vars
          tooLowSince[i] = 0;
          lowSince[i] = 0;
          highSince[i] = 0;
          tooHighSince[i] = 0;
          break;
        }
      }
    } 
  }

  if (Serial.available()){
    Serial.read();
    PrintMainMenu();
  }

}