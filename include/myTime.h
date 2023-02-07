#include <Arduino.h>


// global RunTime Timing
  unsigned char myRunSec = 0;
  unsigned char myRunMin = 0;
  unsigned char myRunHour = 0;
  unsigned int myRunDay = 0;
  unsigned long myRunTime = 0;

// global RealTime Timing
  unsigned char mySec = 0;
  unsigned char myMin = 0;
  unsigned char myHour = 0;
  unsigned char myDay = 1;
  unsigned char myMonth = 1;
  unsigned int myYear = 1970;
  unsigned long myTime = 0;
  // Seconds between RTC sync's (0 disables sync
  #define syncRTCinterval 0

int IsLeapYear(){
  if (!(myYear % 4) && (myYear % 100)){
    // Leap
    return 1;
  }
  else if(!(myYear % 100) && (myYear % 400)){
    // No Leap
    return 0;
  }
  else if(!(myYear % 400)){
    // Leap
    return 1;
  }
  else{
    // No Leap
    return 0;
  }
}

void DoRealTime(){
  // need to get called every second...

  //Trigger for RTC sync
  static unsigned int triggerRTC = 0;

  // Overflow day (default 32 = more months with 31 days)
  char overflowDay = 32;

  mySec++;
  myTime++;
  
  if (mySec == 60){
    mySec = 0;
    myMin++;
  }
  if (myMin == 60){
    myMin = 0;
    myHour++;
  }
  if (myHour == 24){
    // Midnight Jump...
    myHour = 0;
    myDay++;
    
    if (myMonth == 2){
      // February
      if (myDay > 28){
        if (!IsLeapYear()){
          // It's not a leap-year
          myDay = 1;
          myMonth = 3;
        }        
      }
      else if(myDay > 29){
        // It's a leap-year
        myDay = 1;
        myMonth = 3;
      }
    }
    else{
      // Regular months
      switch (myMonth){
      case 4:
      case 6:
      case 9:
      case 11:
        // 30 days
        overflowDay = 31;
      default:
        if (myDay == overflowDay){
          myDay = 1;
          myMonth++;
        }
        break;
      }
    }
    if (myMonth == 13){
      myMonth = 1;
      myYear++;
    }    
  }
  if (syncRTCinterval){
    if (!triggerRTC){
      // sync with RTC

      // reset trigger
      triggerRTC = syncRTCinterval;
    }
    triggerRTC--;
  }
}

int DoTimer(){
  
  static unsigned long last_mS = 0;
  static unsigned long lost_mS = 0;
  unsigned long left_mS = 0;

  unsigned long sys_mS = millis();

  // Time left since last call
  if (last_mS > sys_mS){
    // millis() overflow
    left_mS = 4294967295UL - last_mS + sys_mS;
  }
  else{
    left_mS = sys_mS - last_mS;
  }

  // add lost mS from last call
  left_mS += lost_mS;
  
  // at least a second is over...
  if (left_mS >= 1000){

    // save actual time for next call
    last_mS = sys_mS;

    // get the lost time
    lost_mS = left_mS - 1000;

    // Calculate myRunTime
    myRunTime++;
    myRunSec++;
    if (myRunSec == 60){
      myRunSec = 0;
      myRunMin++;
    }
    if (myRunMin == 60){
      myRunMin = 0;
      myRunHour++;
    }
    if (myRunHour == 24){
      myRunHour = 0;
      myRunDay++;
    }
    
    // Calculate myRealTime
    DoRealTime();
    return 1;
  }
  
  return 0;
  
}
