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

int EzoIntToStr(long val, int lz, int dp, char lc){

    // dp = decimal places
    // lz = leading zero's
    // lc = leading char for zero
    // return = position of decimal point

    // int (scaled by 1000)
    ltoa(val, strHLP, 10);
    int len = strlen(strHLP);

    if (len < 4){
        // value is < 1 (1000)
        memmove(&strHLP[4 - len], &strHLP[0], len);
        memset(&strHLP[0], '0', 4 - len);
        len = 4;
    }
    
    // Set leading zero's
    lz -= (len - 3);
    if (lz > 0){
        // space for missing zeros
        memmove(&strHLP[lz], &strHLP[0], len);
        // set missing zeros
        memset(&strHLP[0], lc, lz);
        // correct len
        len += lz;        
    }

    // shift dp's to set decimal point
    memmove(&strHLP[len -2], &strHLP[len - 3], 3);
    // set decimal point
    strHLP[len - 3] = '.';
    len++;

    // Trailing zero's
    lz = dp + lz - len + 2;
    if (lz > 0){
        // missing trailing zero's
        memset(&strHLP[len], '0', lz);
        len += lz;
    }

    // Return final decimal point
    lz = len - 4;

    // calculate decimal places
    if (dp > 0){
        // cut the too much dp's
        len -= 3 - dp;
    }
    else if (dp == 0){
        // integer
        len = lz;
        lz = 0;
    }

    // set EndOfString
    strHLP[len] = 0;

    return lz;
}


void PrintHlpTime(unsigned char hourIN, unsigned char minIN, unsigned char secIN){
    EzoIntToStr((long)hourIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F(":"));
    EzoIntToStr((long)minIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F(":"));
    EzoIntToStr((long)secIN * 1000,2,0,'0');
    Serial.print(strHLP);
}

void PrintHlpDate(unsigned char dayIN, unsigned char monthIN, unsigned int yearIN){
    EzoIntToStr((long)dayIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F("."));
    EzoIntToStr((long)monthIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F("."));
    EzoIntToStr((long)yearIN * 1000,4,0,'0');
    Serial.print(strHLP);
}

void PrintTime(){
    PrintHlpTime(myHour, myMin, mySec);
}

void PrintRunTime(){
    EzoIntToStr((long)myRunDay * 1000,4,0,' ');
    Serial.print(strHLP);
    Serial.print(F("d"));
    Serial.print(F(" "));
    PrintHlpTime(myRunHour, myRunMin, myRunSec);
}

void PrintDateTime(){
    PrintHlpDate(myDay, myMonth, myYear);
    Serial.print(F(" "));
    PrintHlpTime(myHour, myMin, mySec);
}

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
