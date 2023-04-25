#ifndef MYMENU_H
#define MYMENU_H

#include "quicklib.h"
#include <EEPROM.h>
#include "myWater.h"

// my Eeprom - Variables
/*
byte myBoot = 0;    // 0 = Terminal  /  1 = Slave
uint32_t mySpeed = 9600;
byte mySolarized = 0;
byte myAddress = 123;
byte myDefault = 0;
byte myCnt = 0;
byte myModel = 0;
byte myTemporary = 0;
*/
mySTRUCT my;

void myToRom(){
  /*
  EEPROM.put(1000, myBoot);       // byte
  EEPROM.put(1001, mySolarized);  // byte
  EEPROM.put(1002, myAddress);    // byte
  EEPROM.put(1003, mySpeed);      // 4 byte
  EEPROM.put(1007, myDefault);    // byte
  EEPROM.put(1008, ezoCnt);       // byte
  EEPROM.put(1009, myModel);      // byte
  EEPROM.put(1010, myTemporary);  // byte
  */
  EEPROM.put(997, my);
  // 1011 is next...
}
void myFromRom(){
  /*
  EEPROM.get(1000, myBoot);
  EEPROM.get(1001, mySolarized);
  EEPROM.get(1002, myAddress);
  EEPROM.get(1003, mySpeed);
  EEPROM.get(1007, myDefault);
  EEPROM.get(1008, myCnt);
  EEPROM.get(1009, myModel);
  EEPROM.get(1010, myTemporary);
  */
  // 1011 is next...
  EEPROM.get(997, my);
  if (!IsSerialSpeedValid(my.Speed)){
    my.Speed = 9600;
  }
  /*
  if (!myAddress || myAddress > 254){
    myAddress = 123;
  }
  */
  if (my.Setting > 1){
    my.Setting = 0;
    my.Default = 0;
  }
  if (my.Temporary > 3){
    my.Temporary = 0;
    my.Default = 0;
  }
  if (my.Cnt > EZO_MAX_PROBES - INTERNAL_LEVEL_CNT){
    my.Cnt = 0;
    my.Default = 0;
  }
  
  if (my.Solarized){
    fgFaint = 92;
  }
  else{
    fgFaint = 90;
  }
}

void DummyNameToStrDefault(void){
  strcpy_P(strDefault,(PGM_P)F( "-DummyName-"));
}
void EditAutoName(){
  GetUserString(strDefault);
  /*
  if (GetUserString(strDefault)){
    strcpy(strDefault, strHLP);
  }
  */
}
void EditAutoAddress(){
  adrDefault = GetUserInt(adrDefault);
  if (adrDefault > 127 - ezoCnt){
    adrDefault = 127 - ezoCnt;
  }
  else if (adrDefault < 32){
    adrDefault = 32;
  }
}
void SetAutoAddress(){
  EzoSetAddress(0, adrDefault, 2);
  // wait 4 reboots done
  delay(1000);
  // Scan new
  EzoScan();
}

byte PrintAllMenuOpt1(byte pos){

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("Reset"));
  PrintSpaces(5);
  PrintMenuKeyStd('b'); Serial.print(F("Clear Calibration(s)"));
  PrintSpaces(5);
  PrintMenuKeyStd('c'); Serial.print(F("Delete Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('d'); Serial.print(F("(Auto-)Name = "));
  EscBold(1);
  Serial.print((char*)strDefault);
  EscLocate(42, pos++);
  PrintMenuKeyStd('e'); Serial.print(F("Do (Auto-)Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos++);
  PrintMenuKeyStd('f'); Serial.print(F("(1st) (Auto-)Address = "));
  EscBold(1);
  IntToIntStr(adrDefault, 3, '0');
  Serial.print((char*)strHLP);
  PrintSpaces(5);
  PrintMenuKeyStd('g'); Serial.print(F("Do (Auto-)Address(es)"));
    
  return pos;

}

byte SwitchAllAndProbeMenu(int8_t pos, byte ezo, byte all){
  
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // Return
    break;
  case 'a':
    // Factory Reset for All
    EzoReset(ezo, all);
    break;
  case 'c':
    // Delete all names
    EzoSetName((char*)"", ezo, all, 0);
    break;
  case 'd':
    // Edit Auto Name
    EditAutoName();
    break;
  case 'e':
    // Set AutoNames
    EzoSetName(strDefault, ezo, all, 1);
    break;
  case 'f':
    // Edit 1st Address
    EditAutoAddress();
    break;
  case 'g':
    // Set Addresses
    SetAutoAddress();
    // direct back to main
    pos = 0;
    break;
  case 'b':
    // Clear calibration
    EzoSetCal((char*)"clear", ezo, all, 0, 5);
    break;
  default:
    return 0;
    break;
  }
  return 1;
}

void PrintProbeLine(byte ezo, byte pos, byte bold, byte noKey){

    EscLocate(5, pos);
    if (!noKey){
      // Print Menu-Key
      PrintMenuKey((char)(ezo + 49), 0, '(', 0, 0, bold, !bold);
      EscCursorLeft(1);
      //EscLocate(10, pos);
    }
    if (bold){
      EscBold(1);
    }
    else{
      EscFaint(1);
    }
    
    EscCursorRight(3);
    // Name
    Serial.print((char*)ezoProbe[ezo].name);
    EscLocate(26, pos);
    PrintSpacer(bold);
    // Value
    IntToFloatStr(ezoValue[ezo][0], 5, 2, ' ');
    SetAvgColorEZO(ezoProbe[ezo].type);
    if (!bold){
      EscFaint(1);
    }
    Serial.print(strHLP);
    EscBold(0);
    EscColor(0);
    EscFaint(1);
    Print1Space();
    Serial.print((char*)Fa(ezoStrUnit[ezoProbe[ezo].type]));
    //PrintFa(Fa(ezoStrUnit[ezoProbe[ezo].type]));
    EscLocate(41, pos);
    PrintSpacer(0);
    // Type
    Serial.print((char*)Fa(ezoStrType[ezoProbe[ezo].type]));
    //PrintFa(Fa(ezoStrType[ezoProbe[ezo].type]));
    EscLocate(48, pos);
    PrintSpacer(0);
    // Version
    IntToFloatStr(ezoProbe[ezo].version, 2, 2, '0');
    Serial.print(strHLP);
    EscLocate(56, pos);
    PrintSpacer(0);
    // Address
    IntToIntStr(ezoProbe[ezo].address, 3, ' ');
    Serial.print(strHLP);
    EscLocate(62, pos);
    PrintSpacer(ezoProbe[ezo].calibrated);
    // Calibrated
    Serial.print(ezoProbe[ezo].calibrated);
    EscBold(0);
    PrintSpacer(0);

}
int8_t PrintProbesOfType(byte ezo, byte all, int8_t pos, byte noKey){

  byte veryAll = 0;
  if (ezo == 255){
    // Print all types
    veryAll = 1;
    ezo = 0;
  }
  
  PrintLine(pos, 5, 63);

  for (int i = 0; i < ezoCnt; i++){  
    if ((ezoProbe[i].type == ezoProbe[ezo].type) || veryAll) {
      // Right Probe Type
      pos++;
      PrintProbeLine(i, pos, (i == ezo) || all, noKey);
    }    
  }
  EscFaint(0);
  pos++;
  PrintLine(pos++, 5, 63);
  
  return pos;
}

void PrintAllMenu(){

  adrDefault = 33;
  DummyNameToStrDefault();

Start:

  int8_t pos = PrintMenuTop((char*)"- ALL Menu -");
  pos = PrintProbesOfType(255, 1, pos, 1);
  pos = PrintAllMenuOpt1(pos);
  
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyBoldFaint('h', my.Boot, !my.Boot);
  Serial.print(F("Boot As ModBUS Slave"));
  EscFaint(0);
  PrintSpaces(5);
  PrintMenuKeyBoldFaint('i', !my.Boot, my.Boot);
  Serial.print(F("Boot For Terminal Use"));
  EscFaint(0);
  
  PrintMenuEnd(pos + 2);

  pos = GetUserKey('i', -1);

  if (!SwitchAllAndProbeMenu(pos, 0, 2)){
    switch (pos){
    case 'h':
      // Boot As Slave
      my.Boot = 1;
      myToRom();
      break;
    case 'i':
      // Boot For Terminal
      my.Boot = 0;
      myToRom();
      break;
    default:
      break;
    }
  }
  
  if (pos > 0){
    goto Start;
  }
  
}

void PrintPointEqual(uint8_t pos){
  Serial.print (F("-Pt. "));
  EscLocate(29, pos);
  Serial.print (F("= "));
}

void PrintCalMenu(byte ezo, byte all){

  byte ImInside = 0;

  int32_t calLow = 0;      // Value for LowPoint
  int32_t calMid = 0;      // Value for MidPoint
  int32_t calHigh = 0;     // Value for HighPoint
  // long calRes = 0;      // Value for Reset
  int32_t calAvg = 0;      // Actual avg of actual ezoType

  int32_t calTemp = avg_RTD;
  int32_t calVal[3];
  byte calCnt = 1;
  byte calAction[3];

  struct myMenu{
    byte a:1;
    byte b:1;
    byte c:1;
    byte d:1;
    byte e:1;
    byte f:1;
    byte g:1;
  }myMenu;
  
  myMenu.a = 1;
  myMenu.b = 1;
  myMenu.c = 1;
  myMenu.d = 1;
  myMenu.e = 1;
  myMenu.f = 1;
  myMenu.g = 1;


  Start:

  strcpy_P(iicStr,(PGM_P)F( "- Calibrate "));

  switch (ezoProbe[ezo].type){
  case ezoRTD:
    /* code */
    strcpy_P(&iicStr[12], (PGM_P)F("RTD -"));
    if (!ImInside){
      myMenu.b = 0;
      myMenu.c = 0;
      myMenu.f = 0;
      myMenu.g = 0;
      calLow = CAL_RTD_LOW;
      calMid = CAL_RTD_MID;
      calHigh = CAL_RTD_HIGH;
      calAvg = avg_RTD;
    }
    break;
  case ezoEC:
    /* code */
    strcpy_P(&iicStr[12], (PGM_P)F("EC -"));
    if (!ImInside){
      myMenu.a = 0;
      calLow = CAL_EC_LOW;
      calMid = CAL_EC_MID;
      calHigh = CAL_EC_HIGH;
      calAvg = avg_EC;
    }
    break;
  case ezoPH:
    /* code */
    strcpy_P(&iicStr[12], (PGM_P)F("pH -"));
    if (!ImInside){
      calLow = CAL_PH_LOW;
      calMid = CAL_PH_MID;
      calHigh = CAL_PH_HIGH;
      calAvg = avg_pH;
    }
    break;
  case ezoORP:
    /* code */
    strcpy_P(&iicStr[12], (PGM_P)F("ORP -"));
    if (!ImInside){
      myMenu.b = 0;
      myMenu.c = 0;
      myMenu.f = 0;
      myMenu.g = 0;
      calLow = CAL_ORP_LOW;
      calMid = CAL_ORP_MID;
      calHigh = CAL_ORP_HIGH;
      calAvg = avg_ORP;
    }
    break;
  case ezoDiO2:
    /* code */
    strcpy_P(&iicStr[12], (PGM_P)F("O2 -"));
    if (!ImInside){
      myMenu.c = 0;
      myMenu.d = 0;
      myMenu.e = 0;
      myMenu.f = 0;
      myMenu.g = 0;
      calLow = CAL_DiO2_LOW;
      calMid = CAL_DiO2_MID;
      calHigh = CAL_DiO2_HIGH;
      calAvg = avg_O2;
    }
    break;  
  default:
    break;
  }
  
  int8_t pos = PrintMenuTop(iicStr);
  
  if (!ImInside){
    EzoReset(ezo, all);
    if (ezoProbe[ezo].type == ezoPH || ezoProbe[ezo].type == ezoEC){
      // Set int. temp to 25°C
      EzoSetCalTemp(ezo, all);
    }
  }  
  ImInside = 1;

  pos = PrintProbesOfType(ezo, all, pos, 1);
  pos++;

  EscLocate(5, pos);
  if (myMenu.a){
    // 1-Point Cal.
    PrintMenuKeyStd('A'); Serial.print(F("Do 1-Pt. Cal."));
    PrintSpaces(5);
  }
  if (myMenu.b){
    // 3-Point Cal.
    PrintMenuKeyStd('B'); Serial.print(F("Do 2-Pt. Cal."));
    PrintSpaces(5);
  }
  if (myMenu.c){
    // 3-Point Cal.
    PrintMenuKeyStd('C'); Serial.print(F("Do 3-Pt. Cal."));
  }
  pos++;
  pos = PrintShortLine(pos, 8);

  if (myMenu.d){
    // Set Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('D'); Serial.print(F("Set Single/Mid"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calMid, 4, 2, ' ');
  }
  if (myMenu.e){
    // Use Avg as Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('E'); Serial.print(F("Set AVG As Mid"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calAvg, 4, 2, ' ');
  }
  if (myMenu.f){
    // Set Low Point
    EscLocate(5, pos);
    PrintMenuKeyStd('F'); Serial.print(F("Set Low"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calLow, 4, 2, ' ');
}
  if (myMenu.g){
    // Set High Point
    EscLocate(5, pos);
    PrintMenuKeyStd('G'); Serial.print(F("Set High"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calHigh, 4, 2, ' ');
  }
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('g', 0);
  
  calCnt = 1;
  calTemp = 0;
  calAction[0] = 0;
  calAction[1] = 0;
  calAction[2] = 0;
  calVal[0] = 0;
  calVal[1] = 0;
  calVal[2] = 0;

  switch (pos){
  case 'a':
    // 1-Pt. Cal
    if (myMenu.a){
      calTemp = PrintValsForCal(ezo, all);
      if (calTemp){
        if (ezoProbe[ezo].type == ezoDiO2){
          // medium: air    cmd: "Cal"
          // calAction & calVal are right
        }
        else if (ezoProbe[ezo].type == ezoPH){
          // "Cal,mid,value"
          calAction[0] = 2;
          calVal[0] = CompensatePH(calMid, calTemp);
        }
        else{
          // RTD & ORP "Cal,value"
          calAction[0] = 4;
          calVal[0] = calMid;
        } 
      }      
    }
    break;
  case 'b':
    // 2-Pt. Cal
    calCnt = 2;
    if (myMenu.b){
      calTemp = PrintValsForCal(ezo, all);
      if (calTemp){
        switch (ezoProbe[ezo].type){
        case ezoEC:
          // "Cal,dry" (air)
          // "Cal,value"
          calAction[0] = 6;
          calAction[1] = 4;
          calVal[1] = calMid;//compensateEC(calMid, calTemp);   // !!! MODIFY !!!
          break;
        case ezoDiO2:
          // medium: air        cmd:  "Cal"
          // medium: 0-liquid   cmd:  "Cal,0"     
          calAction[1] = 4;   
          break;
        default:
          // just pH
          // "Cal,mid,value"
          // "Cal,low,value"
          calAction[0] = 2;
          calVal[0] = CompensatePH(calMid, calTemp);   // !!! MODIFY !!!
          calAction[1] = 2;
          calVal[1] = calLow;
          break;
        }  
      }      
    }  
    break;
  case 'c':
    // 3-Pt. Cal
    calCnt = 3;
    if (myMenu.c){
      calTemp = PrintValsForCal(ezo, all);
      switch (ezoProbe[ezo].type){
      case ezoEC:
        // "Cal,dry" (air)
        // "Cal,low,value"
        // "Cal,high,value"
        calAction[0] = 6;
        calAction[1] = 4;
        calVal[1] = CompensateEC(calLow, calTemp);   // !!! MODIFY !!!
        calAction[2] = 4;
        calVal[2] = calHigh;
        break;
      default:
        // just pH
        // "Cal,mid,value"
        // "Cal,low,value"
        // "Cal,high,value"
        calAction[0] = 2;
        calVal[0] = CompensatePH(calMid, calTemp);   // !!! MODIFY !!!
        calAction[1] = 2;
        calVal[1] = calLow;
        calAction[2] = 2;
        calVal[2] = calHigh;
        break;
      }  
    }  
    break;
  case 'd':
    calMid = GetUserFloat(calMid);
    break;
  case 'e':
    calMid = calAvg;
    break;
  case 'f':
    calLow = GetUserFloat(calLow);
    break;
  case 'g':
    calHigh = GetUserFloat(calHigh);
    break;
  default:
    break;
  }

  if (calTemp){
    EzoSetCal((char*)"", ezo, all, calVal[0], calAction[0]);
    for (byte i = 1; i < calCnt; i++){
      calTemp = PrintValsForCal(ezo, all);
      if (calTemp){
        if (ezoProbe[ezo].type == ezoPH){
          // pH Temp - Compensation
          calVal[i] = CompensatePH(calVal[i], calTemp);
        }
        else if (ezoProbe[ezo].type == ezoEC){
          // EC Temp - Compensation
          calVal[i] = CompensateEC(calVal[i], calTemp);
        }
        EzoSetCal((char*)"", ezo, all, calVal[i], calAction[i]);
      }
      else{
        //ESC
        i = calCnt;
      }
    } 
    calCnt = ezoCnt;
    EzoScan();
    if (ezoCnt < calCnt){
      // not all modules recognized
      // sometimes needed...
      EzoScan();
    }  
    if (my.Default){
      DefaultProbesToRom();
    }
  }

  if (pos > 0){
    goto Start;
  }
  
}

void PrintProbeMenu(byte ezo){

  byte all = 0;

    DummyNameToStrDefault();
    adrDefault = 33;

Start:

  int8_t pos = PrintMenuTop((char*)"- Probe(Type) Menu -");
  
  pos = PrintProbesOfType(ezo, all, pos, 0);

  pos = PrintAllMenuOpt1(pos + 1);
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('h'); Serial.print(F("Calibration(s)..."));
  EscLocate(30, pos);
  PrintMenuKeyBoldFaint('i', !all, all); 
  Serial.print(F("Select Single"));
  EscLocate(51, pos++);
  PrintMenuKeyBoldFaint('j', all, !all);
  Serial.print(F("Select ALL"));
  EscBold(0);

  PrintMenuEnd(pos);

  pos = GetUserKey('j', 9);

  if (!SwitchAllAndProbeMenu(pos, ezo, all)){
    switch (pos){
    case 'h':
      PrintCalMenu(ezo, all);
      break;
    case 'i':
      // Single
      all = 0;
      break;
    case 'j':
      // All
      all = 1;
      break;
    default:
      // Select another...
      ezo = pos - 49;
      break;
    }
  }
   
  if (pos > 0){
    goto Start;
  }
  
}

void PrintFlexSpacer(byte leading, byte trailing){
  PrintSpaces(leading);
  PrintSpacer(1);
  PrintSpaces(trailing);
}
void PrintSmallSpacer(){
  PrintSpacer(0);
  EscCursorLeft(1);
}
void PrintSmallMenuKey(char key){
  PrintMenuKey(key, 0, 0, ' ', 0, 0, 0);
}

void PrintLowToHigh(){
  Serial.print(F("tooLow"));
  PrintFlexSpacer(2, 3);  
  Serial.print(F("Low"));
  PrintFlexSpacer(4,3);
  Serial.print(F("High"));
  PrintFlexSpacer(3, 1);
  Serial.print(F("tooHigh"));
  PrintFlexSpacer(2,0);
}

int8_t PrintCopySettingTo(int8_t pos){
  EscLocate(5, pos++);
  EscColor(my.KeyColor);
  EscBold(1);
  EscUnder(1);
  Serial.print(F("1-3):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" Copy FULL SETTING "));
  EscColor(fgBlue);
  Serial.print((char*)setting.Name);
  Serial.print(F(" ("));
  Serial.print(my.Setting + 1);
  Serial.print(F(")"));
  EscColor(0);
  Serial.print(F(" to Setting-No. [1-3]"));
  return pos;
}
void PrintValuesMenu(){

Start:

  int8_t pos = PrintMenuTop((char*)"- Set Values -") + 1;
  byte i = 0;
  
  EscLocate(12, pos++);
  PrintFlexSpacer(0, 1);
  Serial.print(F("FailSafe"));
  PrintFlexSpacer(1, 2);
  PrintLowToHigh();

  PrintLine(pos++, 3, 76);
  EscLocate(3, pos++);
  for (i = 0; i < 6; i++){
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[i]), 9);
    PrintSpacer(0);
    PrintSmallMenuKey('a' + i);
    PrintFloat(setting.FailSaveValue[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('g' + i);
    PrintFloat(setting.ValueTooLow[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('m' + i);
    PrintFloat(setting.ValueLow[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('s' + i);
    PrintFloat(setting.ValueHigh[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('A' + i);
    PrintFloat(setting.ValueTooHigh[i], 4, 2, ' ');
    PrintSpacer(0);

    EscLocate(3, pos++);
  }

  pos = PrintCopySettingTo(pos);
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', 3);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'f'){
    // FailSave
    pos -= 'a';
    setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
    pos = 1;
  }
  else if (pos >= 'g' && pos <= 'l'){
    // tooLow
    pos -= 'g';
    setting.ValueTooLow[pos] = GetUserFloat(setting.ValueTooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'm' && pos <= 'r'){
    // Low
    pos -= 'm';
    setting.ValueLow[pos] = GetUserFloat(setting.ValueLow[pos]);
    pos = 1;
  }
  else if (pos >= 's' && pos <= 'x'){
    // High
    pos -= 's';
    setting.ValueHigh[pos] = GetUserFloat(setting.ValueHigh[pos]);
    pos = 1;
  }
  else if (pos >= 'A' && pos <= 'F'){
    // tooHigh
    pos -= 'A';
    setting.ValueTooHigh[pos] = GetUserFloat(setting.ValueTooHigh[pos]);
    pos = 1;
  }
  else if (pos >= '1' && pos <= '3'){
    SettingsToRom(pos - '1'); 
    pos = 2;   
  }

  if (pos == 1){
    SettingsToRom(my.Setting);
  }
  

  if (pos > 0){
    goto Start;
  }
  
}

void RunManualSetting(byte port){

  struct runManualSTRUCT{
      uint16_t runTime;
      uint16_t offset;
      uint16_t onTime;
      uint16_t offTime;
      byte state;
      uint16_t inState;
  }manualTiming[12];

  Serial.println(F("\n"));
  EscSaveCursor();

  // Copy Low & High values to manualTiming array
  for (byte i = 0; i < 6; i++) {
      manualTiming[i].runTime = manual.Low[i];
      manualTiming[i+6].runTime = i+6;
  }

  // Search longest time
  uint16_t maxTime = 0;
  for (byte i = 0; i < 12; i++){
    if ((i == port || port == 255) && manualTiming[i].runTime){
      // Port is in action...
      if (manualTiming[i].runTime > maxTime){
        maxTime = manualTiming[i].runTime;
      }
    }
    manualTiming[i].state = 0;
  }
  
  // Calc offset / onTime / offTime
  for (byte i = 0; i < 12; i++){
    if ((i == port || port == 255) && manualTiming[i].runTime){
      // we're in Action and have a time...
      uint16_t repeats = maxTime / manualTiming[i].runTime;
      if (repeats > manualTiming[i].runTime){
        // we can't On/Off shorter than 1sec.
        repeats = manualTiming[i].runTime;
      }
      manualTiming[i].onTime = manualTiming[i].runTime / repeats;
      manualTiming[i].offTime = (maxTime - manualTiming[i].runTime) / repeats;
      manualTiming[i].offset = (maxTime - ((manualTiming[i].onTime + manualTiming[i].offTime) * repeats)) / 2;
      manualTiming[i].inState = manualTiming[i].offTime;
    }
  }
  
  // Run times...
  while (maxTime){
    if (DoTimer()){
      // A second is gone...
      for (byte i = 0; i < 12; i++){
        if ((i == port || port == 255) && manualTiming[i].runTime){
          // port is valid & timing exist
          if (manualTiming[i].offset){
            // Start-Offset not reached
            manualTiming[i].offset--;
          }
          else{
            // Offset reached
            if (manualTiming[i].state){
              // port is ON
              if (manualTiming[i].inState == manualTiming[i].onTime){
                // Start OFF
                manualTiming[i].inState = 0;
                manualTiming[i].state = 0;
              }
            }
            else{
              // port is OFF
              if (manualTiming[i].inState == manualTiming[i].offTime){
                // Start ON
                manualTiming[i].inState = 0;
                manualTiming[i].state = 1;
              }              
            }
            digitalWrite(i + 2, manualTiming[i].state);
            manualTiming[i].inState++;        
          }
        }
      }
      maxTime--;
    }
    if (Serial.available()){
      // STOP manual action...
      Serial.read();
      maxTime = 0;
    }
  }
  OffOutPorts();
  
  /*
  // Sort the manualTiming array in descending order
  for (int i = 0; i < 11; i++) {
      for (int j = i+1; j < 12; j++) {
          if (manualTiming[j].runTime > manualTiming[i].runTime) {
              uint16_t temp_times = manualTiming[i].runTime;
              byte temp_port = manualTiming[i].onPort;
              manualTiming[i].runTime = manualTiming[j].runTime;
              manualTiming[i].onPort = manualTiming[j].onPort;
              manualTiming[j].runTime = temp_times;
              manualTiming[j].onPort = temp_port;
          }
      }
  }
  */


}
void PrintManualMenu(){

  //byte selectedSet = 0;

Start:

  int8_t pos = PrintMenuTop((char*)"- Manual Menu -") + 1;
  byte i = 0;
  
  EscLocate(5, pos++);
  PrintMenuKey(i + 'm', 0, '(', ' ', 0, 1, 0);
  EscColor(fgBlue);
  PrintCentered(manual.Name, 16);
  EscColor(0);
  PrintFlexSpacer(0, 6);
  Serial.print(F("LOW"));
  PrintFlexSpacer(6, 5);
  Serial.print(F("HIGH"));
  PrintFlexSpacer(6, 0);

  PrintLine(pos++, 5, 58);
  EscLocate(8, pos++);
  for (i = 0; i < 6; i++){
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[i]), 17);
    PrintSpacer(1);
    //PrintSmallMenuKey('a' + i);
    PrintMenuKey(i +'a', 0, 0, ' ', 0, 0, !manual.Low[i]);
    PrintSerTime(manual.Low[i], 0, 1);
    PrintMenuKey(i + 'A', 1, '(', 0, 0, 0, 0);
    PrintSpacer(1);
    //PrintSmallMenuKey('g' + i);
    PrintMenuKey(i + 'g', 0, 0, ' ', 0, 0, !manual.High[i]);
    PrintSerTime(manual.High[i], 0, 1);
    PrintMenuKey(i + 'G', 1, '(', 0, 0, 0, 0);
    PrintSpacer(0);
    EscLocate(8, pos++);
  }
  pos--;
  PrintLine(pos++, 5, 58);
  pos++;
  EscLocate(5, pos++);
  EscColor(my.KeyColor);
  EscBold(1);
  EscUnder(1);
  Serial.print(F("a-l):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" EditTimes"));
  PrintSpaces(3);
  EscColor(my.KeyColor);
  EscBold(1);
  EscUnder(1);
  Serial.print(F("A-L):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" RunSingle"));
  PrintSpaces(3);
  PrintMenuKeyStd('0');
  Serial.print(F("RunAll"));
  PrintSpaces(3);
  PrintMenuKeyStd('m');
  Serial.print(F("EditName"));
  
  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  EscColor(my.KeyColor);
  EscBold(1);
  EscUnder(1);
  Serial.print(F("1-4):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" Select Manual-Set = "));
  EscColor(fgBlue);
  EscBold(1);
  Serial.print(my.Temporary + 1);
  //EscBold(0);
  EscColor(0);
  PrintSpaces(5);
  //EscBold(1);
  EscColor(my.KeyColor);
  EscUnder(1);
  Serial.print(F("5-8):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" Copy to Manual [1-4]"));
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('m', 8);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'f'){
    // LowTime
    pos -= 'a';
    manual.Low[pos] = GetUserTime(manual.Low[pos]);
    pos = 1;
  }
  else if (pos >= 'g' && pos <= 'l'){
    // HighTime
    pos -= 'g';
    manual.High[pos] = GetUserTime(manual.High[pos]);
    pos = 1;
  }
  else if (pos >= 'A' && pos <= 'F'){
    // Run Single LowTime
    pos -= 'A';
    RunManualSetting(pos);
    pos = 2;
  }
  else if (pos >= 'G' && pos <= 'L'){
    // Run Single HighTime
    pos -= 'G' + 6;
    RunManualSetting(pos);
    pos = 2;
  }
  else if (pos == '0'){
    // Run Together
    RunManualSetting(255);
    pos = 2;
  }
  else if (pos >= '1' && pos <= '4'){
    // Load Set
    my.Temporary = pos - '1';
    ManualTimesFromRom(my.Temporary);
    myToRom();
  }
  else if (pos >= '5' && pos <= '8'){
    // Copy Set
    ManualTimesToRom(pos - '5');
    pos = 2;
  }
  else if (pos == 'm'){
    // Edit Name
    GetUserString(manual.Name);
    // strcpy(manual.Name, strHLP);
    pos = 1;
  }
  
  
  if (pos == 1){
    ManualTimesToRom(my.Temporary);
  }
  
  if (pos > 0){
    goto Start;
  }

  OffOutPorts();
  
}

void PrintTimingsMenu(){

Start:

  int8_t pos = PrintMenuTop((char*)"- Set Timings -") + 1;
  byte i = 0;
  
  EscLocate(12, pos++);
  PrintSpacer(1);
  Serial.print(F("DelayTimes"));
  PrintFlexSpacer(0, 2);
  PrintLowToHigh();

  PrintLine(pos++, 3, 76);
  EscLocate(3, pos++);
  for (i = 0; i < 6; i++){
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[i]), 9);
    PrintSmallSpacer();
    //PrintSmallMenuKey('a' + i);
    PrintMenuKey(i + 'a', 0, 0, ' ', 0, 0, !setting.DelayTime[i]);
    PrintSerTime(setting.DelayTime[i], 0, 1);
    PrintSmallSpacer();
    //PrintSmallMenuKey('g' + i);
    PrintMenuKey(i + 'g', 0, 0, ' ', 0, 0, !setting.DelayTime[i]);
    PrintSerTime(setting.TimeTooLow[i], 0, 1);
    PrintSmallSpacer();
    //PrintSmallMenuKey('m' + i);
    PrintMenuKey(i + 'm', 0, 0, ' ', 0, 0, !setting.TimeLow[i]);
    PrintSerTime(setting.TimeLow[i], 0, 1);
    PrintSmallSpacer();
    //PrintSmallMenuKey('s' + i);
    PrintMenuKey(i + 's', 0, 0, ' ', 0, 0, !setting.TimeHigh[i]);
    PrintSerTime(setting.TimeHigh[i], 0, 1);
    PrintSmallSpacer();
    //PrintSmallMenuKey('A' + i);
    PrintMenuKey(i + 'A', 0, 0, ' ', 0, 0, !setting.TimeTooHigh[i]);
    PrintSerTime(setting.TimeTooHigh[i], 0, 1);
    PrintSmallSpacer();
    EscLocate(3, pos++);
  }

  pos = PrintCopySettingTo(pos);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', 3);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'f'){
    // FailSave
    pos -= 'a';
    setting.DelayTime[pos] = GetUserTime(setting.DelayTime[pos]);
    pos = 1;
  }
  else if (pos >= 'g' && pos <= 'l'){
    // tooLow
    pos -= 'g';
    setting.TimeTooLow[pos] = GetUserTime(setting.TimeTooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'm' && pos <= 'r'){
    // Low
    pos -= 'm';
    setting.TimeLow[pos] = GetUserTime(setting.TimeLow[pos]);
    pos = 1;
  }
  else if (pos >= 's' && pos <= 'x'){
    // High
    pos -= 's';
    setting.TimeHigh[pos] = GetUserTime(setting.TimeHigh[pos]);
    pos = 1;
  }
  else if (pos >= 'A' && pos <= 'F'){
    // tooHigh
    pos -= 'A';
    setting.TimeTooHigh[pos] = GetUserTime(setting.TimeTooHigh[pos]);
    pos = 1;
  }
  else if (pos >= '1' && pos <= '3'){
    SettingsToRom(pos - '1'); 
    pos = 2;   
  }
  

  if (pos == 1){
    SettingsToRom(my.Setting);
  }
  
  if (pos > 0){
    goto Start;
  }
  
}

void PrintUnit (byte ezotype, byte faint, byte leadingSpaces, byte trailingSpaces){
  // Prints Unit of ezoType
  // Adds optional leading and trailing spaces
  // Sets and resets faint
  PrintSpaces(leadingSpaces);
  if (faint){
    EscFaint(1);
  }
  else{
    EscColor(0);
  }
  
  Serial.print((char*)Fa(ezoStrUnit[ezotype]));
  //PrintFa(Fa(ezoStrUnit[ezotype]));
  PrintSpaces(trailingSpaces);
  EscFaint(0);
}

byte PrintWaterValsHlp(byte pos, byte posX, byte ezotype, byte lz, byte dp, int divisor){ //, long *avgExt){

  byte posAct = 0;
  long avg = 0;

  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezotype){
      posAct++;
      EscLocate(posX, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldFloat(ezoValue[i][0] / divisor, lz, dp, ' ');
      avg += ezoValue[i][0];
      PrintUnit(ezotype, 1, 0, 3);
    }
  }

  if (posAct){
    avgVal[ezotype] = avg / (long)posAct;
  }

  return posAct;

}

byte GetPosMax(byte posAct, byte posMax){
  if (posAct > posMax){
    return posAct;
  }
  return posMax;
}
byte PrintWaterVals(byte pos){

  byte posMax = 0;
  byte posAct = 0;

  posMax = PrintWaterValsHlp(pos, 4, ezoRTD, 2, 2, 1); //, &avg_RTD);

  posAct = PrintWaterValsHlp(pos, 17, ezoEC, 4, 0, 1000); //, &avg_EC);
  posMax = GetPosMax(posAct, posMax);

  posAct = PrintWaterValsHlp(pos, 29, ezoPH, 2, 2, 1); //, &avg_pH);
  posMax = GetPosMax(posAct, posMax);

  posAct = PrintWaterValsHlp(pos, 42, ezoORP, 4, 2, 1); //, &avg_ORP);
  posMax = GetPosMax(posAct, posMax);

  posAct = PrintWaterValsHlp(pos, 54, ezoDiO2, 3, 2, 1); //, &avg_O2);
  posMax = GetPosMax(posAct, posMax);

  posAct = PrintWaterValsHlp(pos, 69, ezoLVL, 3, 2, 1); //, &avg_O2);
  posMax = GetPosMax(posAct, posMax);

  return pos + posMax;

}

void PrintActionTimes(byte ezoType){
  // Check which tooLow to tooHigh is actually valid
  
  int32_t timeToUse = 0;
  int32_t timeToAction = 0;

  byte colorState = GetAvgState(avgVal[ezoType], setting.ValueTooLow[ezoType], setting.ValueLow[ezoType], setting.ValueHigh[ezoType], setting.ValueTooHigh[ezoType]);
  
  switch (colorState){
  case fgCyan:
    // tooLow
    timeToUse = tooLowSince[ezoType];
  case fgBlue:
    // Low
    if (colorState == fgCyan && lowSince[ezoType] && lowSince[ezoType] < tooLowSince[ezoType]){
      timeToUse = lowSince[ezoType];
      colorState = fgCyan;
    }
    else if (colorState == fgBlue){
      timeToUse = lowSince[ezoType];
    }  
    break;
  case fgRed:
    // tooHigh
    timeToUse = tooHighSince[ezoType];
  case fgYellow:
    // High
    if (colorState == fgRed && highSince[ezoType] && highSince[ezoType] < tooHighSince[ezoType]){
      timeToUse = highSince[ezoType];
      colorState = fgYellow;
    }
    else if (colorState == fgYellow){
      timeToUse = highSince[ezoType];
    }  
    break;
  default:
    // OK
    timeToUse = okSince[ezoType];
    break;
  } 
  EscColor(colorState);
  EscFaint(1);

  timeToUse = (myTime - timeToUse);            // time since state became true
  if (colorState != fgGreen){
    timeToAction = setting.DelayTime[ezoType] - timeToUse;     // time until action begins
  }
  
  EscSaveCursor();
  PrintSerTime(timeToUse, 1, 1);
  EscRestoreCursor();
  EscCursorDown(1);
  EscFaint(0);
  if (timeToAction >= 0){
    PrintSerTime(timeToAction, 1, 1);
  }
  else{
    Serial.print(F("  -On Action-"));
  }
  
  EscRestoreCursor();
  EscCursorDown(2);
  EscFaint(1);
  timeToUse = 0;
  if (lastAction[ezoType]){
    timeToUse = myTime - lastAction[ezoType];
  }
  PrintSerTime(timeToUse, 1, 1);
  EscFaint(0);
  EscColor(0);
}

byte PrintAVGs(byte pos){
    
  SetAvgColorEZO(ezoRTD);
  EscLocate(7, pos);
  PrintBoldFloat(avg_RTD, 2, 2, ' ');
  PrintUnit(ezoRTD, 0, 0, 3);

  EscLocate(1, pos + 2);
  PrintActionTimes(ezoRTD);


  SetAvgColorEZO(ezoEC);
  EscLocate(20, pos);
  PrintBoldInt(avg_EC / 1000, 4, ' ');
  PrintUnit(ezoEC, 0, 0, 3);
  
  EscLocate(14, pos + 2);
  PrintActionTimes(ezoEC);


  SetAvgColorEZO(ezoPH);
  EscLocate(32, pos);
  PrintBoldFloat(avg_pH, 2, 2, ' ');
  PrintUnit(ezoPH, 0, 0, 3);

  EscLocate(27, pos + 2);
  PrintActionTimes(ezoPH);


  SetAvgColorEZO(ezoORP);
  EscLocate(44, pos);
  PrintBoldFloat(avg_ORP, 4, 2, ' ');
  PrintUnit(ezoORP, 0,  0 , 3);

  EscLocate(40, pos + 2);
  PrintActionTimes(ezoORP);


  SetAvgColorEZO(ezoDiO2);
  EscLocate(58, pos);
  PrintBoldFloat(avg_O2, 3, 2, ' ');
  PrintUnit(ezoDiO2, 0, 0, 3);

  EscLocate(53, pos + 2);
  PrintActionTimes(ezoDiO2);

  SetAvgColorEZO(ezoLVL);
  EscLocate(72, pos);
  PrintBoldFloat(avg_LVL, 3, 2, ' ');
  PrintUnit(ezoLVL, 0, 0, 3);

  EscLocate(66, pos + 2);
  PrintActionTimes(ezoLVL);

  return pos + 1;

}

void PrintCenteredWithSpacer(char *strIN, byte centerLen){
  PrintSpacer(1);
  PrintCentered(strIN, centerLen);
}

void PrintLoopMenu(){

  EscCls();
  EscInverse(1);
  byte pos = PrintMenuTop((char*)"- QuickWater 1.01 -");
  EscInverse(0);
  pos++;

  EscLocate(2, pos++);

  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoRTD]),10);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoEC]),9);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoPH]),10);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoORP]),11);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoDiO2]),9);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoLVL]),9);
  EscCursorLeft(1);
  PrintSmallSpacer();
  //Serial.print(F(" | Temperature | Conductivity |     pH     |    Redox    |     O2     |"));
  pos = PrintLine(pos, 3, 76);
    
  //EscBold(1);
  EscBold(0);

  PrintErrorOK(0, 0, (char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLine(pos, 3, 76);
  //PrintLine(pos + 1, 6, 70);

  // Avg 
  pos = PrintAVGs(pos);

  EscBold(1);
  pos = PrintLine(pos, 3, 76);
  EscBold(0);

}

void PrintMainMenu(){

Start:

  int pos = PrintMenuTop((char*)"- QuickWater 1.01 -");
  
  uint32_t hlpTime = 0;

  pos = PrintProbesOfType(255, 1, pos, 0);

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("ReScan"));
  PrintSpaces(3);
  PrintMenuKeyStd('b'); Serial.print(F("ReBoot"));
  PrintSpaces(3);
  PrintMenuKeyStd('c'); Serial.print(F("Date"));
  PrintSpaces(3);
  PrintMenuKeyStd('d'); Serial.print(F("Time"));
  PrintSpaces(3);
  PrintMenuKeyStd('e'); Serial.print(F("Addr. = "));
  PrintBoldInt(my.Address, 3, '0');

  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('f'); Serial.print(F("Speed = "));
  EscBold(1);
  Serial.print(my.Speed);
  PrintSpaces(3);  
  PrintMenuKeyBoldFaint('g', (my.Solarized), (!my.Solarized)); Serial.print(F("FaintHack"));
  EscBold(0);
  PrintSpaces(3);
  PrintMenuKeyBoldFaint('h', (my.Default), (!my.Default)); Serial.print(F("IsDefault"));
  PrintSpaces(3);
  EscBold(1);
  Serial.print(F("+/-): "));
  EscColor(my.KeyColor);
  EscUnder(1);
  Serial.print(F("KeyColor"));
  EscColor(0);
  EscUnder(0);

  pos = PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('i');
  Serial.print(F("More..."));
  PrintSpaces(3);
  PrintMenuKeyStd('j'); Serial.print(F("Values..."));
  PrintSpaces(3);
  PrintMenuKeyStd('k'); Serial.print(F("Times..."));
  PrintSpaces(3);
  PrintMenuKeyStd('l'); Serial.print(F("Manual..."));

  PrintShortLine(pos++, 8);

  EscLocate(5, pos);
  PrintMenuKeyStd('m'); Serial.print(F("Setting-Name = "));
  EscBold(1);
  EscColor(fgBlue);
  Serial.print((char*)setting.Name);
  PrintSpaces(3);
  EscColor(my.KeyColor);
  EscUnder(1);
  Serial.print(F("n-p):"));
  EscColor(0);
  EscBold(0);
  EscUnder(0);
  Serial.print(F(" Sel.Setting [1-3] = "));
  EscBold(1);
  EscColor(fgBlue);
  Serial.print(my.Setting + 1);
  EscColor(0);
  EscBold(0);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('p', ezoCnt - INTERNAL_LEVEL_CNT);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // EXIT
    break;
  case 'i':
    // All/More Menu
    PrintAllMenu();
    break;
  case 'a':
    // ReScan
    EscCls();
    EscLocate(1,1);
    EzoScan();
    break;
  case 'b':
    // ReBoot
    break;
  case 'c':
    // Date
    hlpTime = SerializeTime(1, 1, 2023, myHour, myMin, mySec);    // Time of now
    hlpTime += GetUserDate(myTime);
  case 'd':
    // Time
    if (pos == 'c'){
      hlpTime = SerializeTime(myDay, myMonth, myYear, 0, 0, 0);    // Midnight of today
      hlpTime += GetUserTime(myTime);
    }
    DeSerializeTime(hlpTime, &myDay, &myMonth, &myYear, &myHour, &myMin, &mySec);    
    //RTC_SetDateTime();
    myTime = SerializeTime(myDay, myMonth, myYear, myHour, myMin, mySec);
    break;
  case 'e':
    // Slave Address
    my.Address = GetUserInt(my.Address);
    if (my.Address > 254){
      // illegal address - reload from eeprom
      myFromRom();
    }
    else{
      // save to eeprom...
      myToRom();
    }
    break;
  case 'f':
    // Speed
    // Set Speed
    my.Speed = GetUserInt(my.Speed);
    if (IsSerialSpeedValid(my.Speed)){ 
      // valid - save to eeprom
      myToRom();
    }
    else{
      // illegal - reload from eeprom
      myFromRom();
    }
    break;
  case 'h':
    // Use and save actual Scan as Default
    my.Default = !my.Default;
    myToRom();
    if (my.Default){
      DefaultProbesToRom();
    }  
    break;
  case 'j':
    // Values
    PrintValuesMenu();
    break;
  case 'k':
    // Values
    PrintTimingsMenu();
    break;
  case 'l':
    PrintManualMenu();
    break;
  case 'm':
    // Setting Name
    GetUserString(setting.Name);
    SettingsToRom(my.Setting);
    break;
  case 'n':
  case 'o':
  case 'p':
    // Select Setting
    my.Setting = pos - 'n';
    SettingsFromRom(my.Setting);
    myToRom();
    break;
  case 'g':
    // Solarized
    my.Solarized = !my.Solarized;
    fgFaint = 90 + (my.Solarized * 2);
    myToRom();
    break;
  case '+':
    if (my.KeyColor == bgWhiteB){
      my.KeyColor = 0;
    }
    else if (my.KeyColor == fgWhiteB){
      my.KeyColor = 100;
    }
    else if (my.KeyColor == bgWhite){
      my.KeyColor = 90;
    }
    else if (my.KeyColor == fgWhite){
      my.KeyColor = 40;
    }
    else if (my.KeyColor == 0){
      my.KeyColor = 30;
    }
    else{
      my.KeyColor++;
    }
    myToRom();
    break;    
  case '-':
    if (my.KeyColor == fgBlack){
      my.KeyColor = 0;
    }
    else if (my.KeyColor == bgBlack){
      my.KeyColor = 37;
    }
    else if (my.KeyColor == fgBlackB){
      my.KeyColor = 47;
    }
    else if (my.KeyColor == bgBlackB){
      my.KeyColor = 97;
    }
    else if (my.KeyColor == 0){
      my.KeyColor = 107;
    }
    else{
      my.KeyColor--;
    }
    myToRom();
    break;
  default:
    // Single Probe/Type
    PrintProbeMenu(pos - 49);
    break;
  }
  if (pos > 0){
    goto Start;
  }
  PrintLoopMenu();
}

#endif