#include <Arduino.h>

#define RUNNING_TIMERS_CNT 12

struct TimerSTRUCT{
  uint16_t onTime;
  uint16_t offTime;
  uint16_t onTime2;
  uint16_t offTime2;
  uint16_t offset;
  union typeUNION{
    byte interval     :1;
    byte interrupted  :1;
    byte dayTimer     :1;
    byte invert       :1;
  }type;
  byte weekDays;
    // 0 = all
    // 2nd Bit = Sunday
    // 3rd Bit = Monday
    // 4th...
  union stateUNION{
    byte automatic    :1;
    byte permOff      :1;
    byte permOn       :1;
    byte tempOn       :1;
    byte tempOff      :1;
    byte lastVal      :1;
    byte hasChanged   :1;
  }state;
  uint32_t tempUntil;
  char name[17];
}runningTimers[RUNNING_TIMERS_CNT];



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
  unsigned int myYear = 2023;
  unsigned long myTime = 0;

// RTC-Temp
  long myRtcTemp = 0;

// Seconds between RTC sync's (0 disables sync
#define syncRTCinterval 86400L

char ByteToChar(byte valIN){
    // Keep Bit-Pattern
    if (valIN > 127){
        return (char)(valIN - 256);
    }
    return (char)valIN;
}

byte GetWeekDay(unsigned long serialTime){
  // 01.01.2023 (start of 'myTime') was a Sunday
  // Sunday is day 1

  byte r = 1;
  unsigned long hlp = serialTime % 604800;    // remove full weeks
  
  r += (byte)(hlp / 86400);                   // add full days
  if (hlp % 86400){
    r++;                                      // one more...    
  }
  
  return r;

}

byte IsLeapYear(uint16_t yearIN){
  if (!(yearIN % 4) && (yearIN % 100)){
    // Leap
    return 1;
  }
  else if(!(yearIN % 100) && (yearIN % 400)){
    // No Leap
    return 0;
  }
  else if(!(yearIN % 400)){
    // Leap
    return 1;
  }
  else{
    // No Leap
    return 0;
  }
}

byte GetDaysOfMonth(char monthIN, uint16_t yearIN){

  byte r = 31;

  // Regular months
  switch (monthIN){
  case 2:
    r = 28;
    if (IsLeapYear(yearIN)){
      r++;
    }
    break;
  case 4:
  case 6:
  case 9:
  case 11:
    r = 30;
  default:
    // r = 31;
    break;
  }

return r;

}

unsigned long SerializeTime(byte dayIN, byte monthIN, uint16_t yearIN, byte hourIN, byte minIN, byte secIN){
    // Do Serialized Time (Start from 01.01.2023)
    unsigned long serializedTime = 0;
    // Years
    for (uint16_t i = 2023; i < yearIN; i++){
      serializedTime += 31536000; // 365 * 86400
      if (IsLeapYear(i)){
        serializedTime += 86400;
      }
    }
    // Months
    for (int i = 1; i < monthIN; i++){
      serializedTime += GetDaysOfMonth(monthIN, yearIN) * 86400;
    }
    // Days
    serializedTime += (dayIN - 1) * 86400;
    // Hours
    serializedTime += hourIN * 3600;
    // Minutes
    serializedTime += minIN * 60;
    // Seconds
    return serializedTime + secIN;
}

void DeSerializeTime(unsigned long serializedIN, byte *dayIN, byte *monthIN, uint16_t *yearIN, byte *hourIN, byte *minIN, byte *secIN){


  unsigned long nextSeconds = 31535999L;
  *yearIN = 2023;

  // Year
  while (serializedIN > nextSeconds){
    // actual year is full
    
    *yearIN++;
    serializedIN -= nextSeconds;
    
    nextSeconds = 31535999L;
    if (IsLeapYear(*yearIN)){
      nextSeconds += 86400L;
    }
    
  }
  
  // Month
  nextSeconds = 2678400L;
  *monthIN = 1;
  while (serializedIN > nextSeconds){
    *monthIN++;
    serializedIN -= nextSeconds;
    nextSeconds = GetDaysOfMonth(*monthIN, *yearIN) * 86400L;
  }

  // Day
  *dayIN = (serializedIN / 86400) + 1;
  serializedIN = serializedIN % 86400;

  // Hour
  *hourIN = serializedIN / 3600;
  serializedIN = serializedIN % 3600;

  // Min
  *minIN = serializedIN / 60;

  // Sec
  *secIN = serializedIN % 60;

}

byte ToBCD (byte val){
  return ((val / 10) << 4) + (val % 10);
}
byte FromBCD (byte bcd){
  return (10 * ((bcd & 0xf0) >> 4)) + (bcd & 0x0f);
}

void RTC_GetTemp(){
  IIcSetBytes(0x68, (char*)"\x11", 1);
  if (IIcGetBytes(0x68, 2) == 2){
    // Full Read success
    myRtcTemp = iicStr[0] * 1000;
    myRtcTemp += (iicStr[1] >> 6) * 250;
  }
}

long RTC_GetDateTime(){
  
  long r = 0;

  IIcSetBytes(0x68, (char*)"\0", 1);
  if (IIcGetBytes(0x68, 7) == 7){
    // Full Read Succeed
    mySec = FromBCD(iicStr[0]);
    myMin = FromBCD(iicStr[1]);
    myHour = FromBCD(iicStr[2]);
    myDay = FromBCD(iicStr[4]);
    myMonth = FromBCD(iicStr[5]);
    myYear = FromBCD(iicStr[6]) + 2000;

    if (myYear < 2023){
      myYear = 2023;
    }
    
    r = myTime;    // save for diff calculation   
    myTime = SerializeTime(myDay, myMonth, myYear, myHour, myMin, mySec);
    r -= myTime;

  }
    return r;   // positive = ??C is faster 
}

char RTC_SetDateTime(){
  
  char r = 0;

  strHLP[0] = 0;
  strHLP[1] = ByteToChar(ToBCD(mySec));
  strHLP[2] = ByteToChar(ToBCD(myMin));
  strHLP[3] = ByteToChar(ToBCD(myHour));
  r = IIcSetBytes(0x68, strHLP, 4);
    
  if (r > 0){
    strHLP[0] = 4;
    strHLP[1] = ByteToChar(ToBCD(myDay));
    strHLP[2] = ByteToChar(ToBCD(myMonth));
    strHLP[3] = ByteToChar(ToBCD((byte)(myYear - 2000)));
    r = IIcSetBytes(0x68, strHLP, 4);

  }

  return r;

}

char EzoIntToStr(long val, char lz, byte dp, char lc){

    // dp = decimal places
    // lz = leading zero's
    // lc = leading char for zero
    // return = position of decimal point

    // int (scaled by 1000)
    ltoa(val, strHLP, 10);
    byte len = strlen(strHLP);

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
        memmove(&strHLP[(int)lz], &strHLP[0], len);
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

void PrintHlpTime(byte hourIN, byte minIN, byte secIN){
    EzoIntToStr((long)hourIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F(":"));
    EzoIntToStr((long)minIN * 1000,2,0,'0');
    Serial.print(strHLP);
    Serial.print(F(":"));
    EzoIntToStr((long)secIN * 1000,2,0,'0');
    Serial.print(strHLP);
}

void PrintHlpDate(byte dayIN, byte monthIN, unsigned int yearIN){
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

void DoRealTime(){
  // need to get called every second...

  //Trigger for RTC sync
  static unsigned long triggerRTC = 0;
  static long rtcSyncDiff = 0;

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
        if (!IsLeapYear(myYear)){
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
      rtcSyncDiff = RTC_GetDateTime();
      if (rtcSyncDiff){
        // Update stuff which has 'LastActionTime" dependencies...
      }
      // reset trigger
      triggerRTC = syncRTCinterval;
    }
    triggerRTC--;
  }
}

byte DoTimer(){
  
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

// TIMER

unsigned long CurrentIntervalPos(unsigned long timerIN, unsigned int onTime, unsigned int offTime, unsigned int offset){
  // Calculate the current position within the interval
  return (timerIN + (long)offset) % (long)(onTime + offTime);
}
#define LastInterval(timerIN, onTime, offTime, offset) CurrentIntervalPos(timerIN, onTime, offTime, offset)

byte IntervalTimer(unsigned long timerIN, unsigned int onTime, unsigned int offTime, unsigned int offset) {
    // Check if the current position is within the "on" interval
    if (CurrentIntervalPos(timerIN, onTime, offTime, offset) < onTime){
        return 1; // "on" interval
    }
    return 0;     // "off" interval
}

unsigned long NextInterval(unsigned long timerIN, unsigned int onTime, unsigned int offTime, unsigned int offset){

  return (long)(onTime + offTime) - CurrentIntervalPos(timerIN, onTime, offTime, offset);

}

byte InterruptedIntervalTimer(unsigned long timerIN, unsigned int onTime, unsigned int offTime, unsigned int offset, unsigned int onTime2, unsigned int offTime2){
  // Check if 1st interval is valid
  if (IntervalTimer(timerIN, onTime, offTime, offset)){
    // Check if 2nd interval is valid
    if (IntervalTimer(CurrentIntervalPos(timerIN, onTime, offTime, offset), onTime2, offTime2, 0)){
      return 1;
    }
  }
  return 0;
}

byte DayTimer (unsigned long timerIN, unsigned int onTime, unsigned int offTime){

  // ordinary 24h timer

  unsigned int onDuration;

  if (offTime < onTime){
    // Jump over Midnight
    onDuration = 86400 - onTime + offTime;
  }
  else{
    onDuration = offTime - onTime;
  }
  
  unsigned int offDuration = 86400 - onDuration;

  return IntervalTimer(timerIN, onDuration, offDuration, onTime);

}

// Returns the state of the bit at position 'bitToGet' in 'byteIN'
byte getBit(byte byteIN, byte bitToGet) {
    return (byteIN >> bitToGet) & 0x01;
}

// Sets the bit at position 'bitToSet' in 'byteIN' to 'setTo' 
uint8_t setBit(byte byteIN, byte bitToSet, byte setTo) {
    if (setTo) {
        byteIN |= (1 << bitToSet);
    } else {
        byteIN &= ~(1 << bitToSet);
    }
    return byteIN;
}

byte RunTimers(){
  // Returns true if one ore more states have changed

  byte r = 0; // Helper
  byte r2 = 0;

  for (byte i = 0; i < RUNNING_TIMERS_CNT; i++){
    if (runningTimers[i].type.dayTimer){
      // 24h DayTimer...
      r = DayTimer (myTime, runningTimers[i].onTime, runningTimers[i].offTime);
    }
    else if (runningTimers[i].type.interval){
      // Interval Timers
      if (runningTimers[i].type.interrupted){
        // Interrupted version
        r = InterruptedIntervalTimer(myTime, runningTimers[i].onTime, runningTimers[i].offTime, runningTimers[i].offset, runningTimers[i].onTime2, runningTimers[i].offTime2);
      }
      else{
        // Regular
        r = IntervalTimer(myTime, runningTimers[i].onTime, runningTimers[i].offTime, runningTimers[i].offset);
      }      
    }
    // Check on valid weekdays
    if (!getBit(runningTimers[i].weekDays, GetWeekDay(myTime)) && runningTimers[i].weekDays){
      // Day is not valid
      r = 0;
    }
    // Check on permanent & temporary state
    if (runningTimers[i].state.permOff){
      r = 0;
    }
    else if (runningTimers[i].state.permOn){
      r = 1;
    }
    else if (runningTimers[i].state.tempOff && myTime < runningTimers[i].tempUntil){
      r = 0;
    }
    else if (runningTimers[i].state.tempOn && myTime < runningTimers[i].tempUntil){
      r = 1;
    }
    else if (runningTimers[i].state.automatic){
      // r = r;
    }
    else{
      // Totally disabled
      r = 0;
    }
    // Clear Temp-Times and States if possible
    if (runningTimers[i].tempUntil && (myTime >= runningTimers[i].tempUntil)){
      runningTimers[i].tempUntil = 0;
      runningTimers[i].state.tempOn = 0;
      runningTimers[i].state.tempOff = 0;
    }
    // Invert if needed
    if (runningTimers[i].type.invert){
      r = !r;
    }
    // Check on change
    if (r != runningTimers[i].state.lastVal){
      // Is changed
      runningTimers[i].state.lastVal = r;
      runningTimers[i].state.hasChanged = 1;
      r2 = 1;
    }
    else{
      runningTimers[i].state.hasChanged = 0;
    }
  }
  return r2;
}