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

  // RTC_GetDateTime();

  delay(300);

  //memset(tooLowSince, 0, sizeof(tooLowSince));
  //memset(lowSince, 0, sizeof(lowSince));
  //memset(highSince, 0, sizeof(highSince));
  //memset(tooHighSince, 0, sizeof(tooHighSince));
  //memset(okSince, 0, sizeof(okSince));
  //memset(lastAction, 0, sizeof(lastAction));

  // OutPorts
  for (byte i = 2; i < 18; i++){
    if (i < 14){
      pinMode(i, OUTPUT);
      digitalWrite(i, LOW);
    }
    else{
      pinMode(i, INPUT_PULLUP);    // Level tooLow, low, high, tooHigh
    }
  }

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

uint32_t ValidTimeSince(uint32_t valIN){
  if (!valIN){
    return 0;
  }
  return myTime - valIN;
}
uint32_t checkAction(uint32_t valIN, uint32_t actionTime, byte ezotype, byte isLowPort, byte *backSet){

  uint32_t r = valIN;
  *backSet = 0;

  // If something is OnAction
  if (ValidTimeSince(valIN) > delayTimes[ezotype]){
    // Action Valid
    if ((ValidTimeSince(valIN) - delayTimes[ezotype]) > actionTime){
      // ActionTime done
      lastAction[ezotype] = myTime;
      r = 0;
    }
    else{
      // DoAction
      *backSet = 1;
    }
  }
  else{
    // NoAction
  }
  
  // Port 2-6 - Action ports for high / tooHigh
  // Port 7-11 - Action ports for low / tooLow
  digitalWrite(ezotype + 2 + (isLowPort * 6), *backSet);
  
  return r;

}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    byte err = 1;
    byte printAction = 0;

    uint32_t preToo = 0;

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

    // Check High/Low of AVGs 
    // compare timeOuts with timing-setting
    for (byte i = 0; i < 6; i++){

      printAction = 0;

      // Check On needed/pending actions
      preToo = tooLowSince[i];
      tooLowSince[i] = checkAction(tooLowSince[i], actionTooLow[i], i, 1, &err);
      if (!err){
        // TooLow isn't in Action...
        lowSince[i] = checkAction(lowSince[i], actionLow[i], i, 1, &err);
        if (preToo != tooLowSince[i]){ 
          // after finished tooXYZ-Action - reset lowSince, too
          lowSince[i] = 0;
          err = 0;
        }      
        if (err){
          // Low in Action
          strcpy_P(strHLP, (PGM_P)F("\0"));
        }
      }
      else{
        //  TooLow in Action
        strcpy_P(strHLP, (PGM_P)F("Too"));
      }
      if (err){
        // something is in action
        strcpy_P(&strHLP[strlen(strHLP)], (PGM_P)F("LowAction-"));
        printAction = 1;
      }
      
      preToo = tooHighSince[i];
      tooHighSince[i] = checkAction(tooHighSince[i], actionTooHigh[i], i, 0, &err);
      if (!err){
        // TooHigh isn't in Action...
        highSince[i] = checkAction(highSince[i], actionHigh[i], i, 0, &err);
        if (preToo != tooHighSince[i]){ 
          // after finished tooXYZ-Action - reset highSince, too
          highSince[i] = 0;
          err = 0;
        }
        if (err){
          // High in Action
          strcpy_P(strHLP, (PGM_P)F("\0"));
        }
      }
      else{
        //  TooHigh in Action
        strcpy_P(strHLP, (PGM_P)F("Too"));
      }
      if (err){
        // something is in action
        strcpy_P(&strHLP[strlen(strHLP)], (PGM_P)F("HighAction-"));
        printAction = 1;
      }
      if (printAction){
        // Print Action
        strcpy_P(&strHLP[strlen(strHLP)], (PGM_P)pgm_read_word(&(ezoStrType[i])));
        PrintErrorOK(0, strlen(strHLP), strHLP);
      }
      
      // Set / Reset Since-Variables depending on high/low state...
      switch (GetAvgState(avgVal[i], tooLow[i], low[i], high[i], tooHigh[i])){
      case fgCyan:
        // tooLow
        highSince[i] = 0;
        tooHighSince[i] = 0;
        if (!tooLowSince[i]){
          // 1st time tooLow recognized
          tooLowSince[i] = myTime;
          if (!lowSince[i]){
            // If tooXYZ is active... regular state becomes active too
            lowSince[i] = myTime;
          }  
        }
        break;
      case fgBlue:
        // Low
        highSince[i] = 0;
        tooHighSince[i] = 0;
        tooLowSince[i] = 0;
        if (!lowSince[i]){
          // 1st time Low recognized
          lowSince[i] = myTime;
        }
        break;
      case fgRed:
        // tooHigh
        lowSince[i] = 0;
        tooLowSince[i] = 0;
        if (!tooHighSince[i]){
          // 1st time tooLow recognized
          tooHighSince[i] = myTime;
          if (!highSince[i]){
            // If tooXYZ is active... regular state becomes active too
            highSince[i] = myTime;
          }
        }
        break;
      case fgYellow:
        // High
        lowSince[i] = 0;
        tooLowSince[i] = 0;
        tooHighSince[i] = 0;
        if (!highSince[i]){
          // 1st time High recognized
          highSince[i] = myTime;
        }
        break;
      default:
        // OK
        // Reset ...Since Vars
        tooLowSince[i] = 0;
        lowSince[i] = 0;
        highSince[i] = 0;
        tooHighSince[i] = 0;
        if (!okSince[i]){
          okSince[i] = myTime;
        }
        break;
      }
    }

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