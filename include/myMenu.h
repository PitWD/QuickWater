#ifndef MYMENU_H
#define MYMENU_H

#include "quicklib.h"
#include <EEPROM.h>
#include "myWater.h"

// my Eeprom - Variables
byte myBoot = 0;    // 0 = Terminal  /  1 = Slave
uint32_t mySpeed = 9600;
byte mySolarized = 0;
byte myAddress = 123;
byte myDefault = 0;
byte myCnt = 0;

void myToRom(){
  EEPROM.put(1000, myBoot);       // byte
  EEPROM.put(1001, mySolarized);  // byte
  EEPROM.put(1002, myAddress);    // byte
  EEPROM.put(1003, mySpeed);      // 4 byte
  EEPROM.put(1007, myDefault);    // byte
  EEPROM.put(1008, ezoCnt);       // byte
  // 1009 is next...
}
void myFromRom(){
  EEPROM.get(1000, myBoot);
  EEPROM.get(1001, mySolarized);
  EEPROM.get(1002, myAddress);
  EEPROM.get(1003, mySpeed);
  EEPROM.get(1007, myDefault);
  EEPROM.get(1008, myCnt);
  // 1009 is next...

  if (!IsSerialSpeedValid(mySpeed)){
    mySpeed = 9600;
  }
  if (!myAddress || myAddress > 254){
    myAddress = 123;
  }
  if (mySolarized){
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
  if (GetUserString(strDefault)){
    strcpy(strDefault, strHLP);
  }
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

  EscLocate(5, pos);
  PrintMenuKeyStd('A'); Serial.print(F("Reset"));
  EscLocate(18, pos);
  PrintMenuKeyStd('B'); Serial.print(F("Clear Calibration(s)"));
  EscLocate(46, pos++);
  PrintMenuKeyStd('C'); Serial.print(F("Delete Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('D'); Serial.print(F("(Auto-)Name = "));
  EscBold(1);
  Serial.print((char*)strDefault);
  EscLocate(42, pos++);
  PrintMenuKeyStd('E'); Serial.print(F("Do Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('F'); Serial.print(F("(1st) (Auto-)Address = "));
  EscBold(1);
  IntToIntStr(adrDefault, 3, '0');
  Serial.print((char*)strHLP);
  EscLocate(42, pos++);
  PrintMenuKeyStd('G'); Serial.print(F("Do Address(es)"));
    
  return pos;

}

void PrintAllMenu(){

  adrDefault = 33;
  DummyNameToStrDefault();

Start:

  int8_t pos = PrintMenuTop((char*)"- ALL Menu -");
  pos = PrintAllMenuOpt1(pos + 1);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('g', -1);

  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // Return
    break;
  case 'a':
    // Factory Reset for All
    EzoReset(0,2);
    break;
  case 'c':
    // Delete all names
    EzoSetName((char*)"", 0, 2, 0);
    break;
  case 'd':
    // Edit Auto Name
    EditAutoName();
    break;
  case 'e':
    // Set AutoNames
    EzoSetName(strDefault,0,2,1);
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
    EzoSetCal((char*)"Cal,clear", 0, 2);
    break;
  default:
    break;
  }

  if (pos > 0){
    goto Start;
  }
  
}

void PrintProbeLine(byte ezo, byte pos, byte bold){

    EscLocate(5, pos);
    PrintMenuKey((char)(ezo + 49), 0, '(', 0, 0, bold, !bold);
    // Name
    EscLocate(10, pos);
    Serial.print((char*)ezoProbe[ezo].name);
    EscLocate(26, pos);
    PrintSpacer(bold);
    // Value
    IntToFloatStr(ezoProbe[ezo].value[0], 5, 2, ' ');
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
int8_t PrintProbesOfType(byte ezo, byte all, int8_t pos){

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
      PrintProbeLine(i, pos, (i == ezo) || all);
    }    
  }
  EscFaint(0);
  pos++;
  PrintLine(pos++, 5, 63);
  
  return pos;
}

void PrintPointEqual(uint8_t pos){
  Serial.print (F("-Pt. "));
  EscLocate(29, pos);
  Serial.print (F("= "));
}

void PrintCalMenu(byte ezo, byte all){

  static byte ImInside = 0;

  long calLow = 0;      // Value for LowPoint
  long calMid = 0;      // Value for MidPoint
  long calHigh = 0;     // Value for HighPoint
  long calRes = 0;      // Value for Reset
  long calAvg = 0;      // Actual avg of actual ezoType

  long calTemp = avg_RTD;
  byte calCnt = 1;

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
      calRes = CAL_RTD_RES;
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
      calRes = CAL_EC_RES;
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
      calRes = CAL_PH_RES;
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
      calRes = CAL_ORP_RES;
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
      calRes = CAL_DiO2_RES;
      calAvg = avg_O2;
    }
    break;  
  default:
    break;
  }
  int8_t pos = PrintMenuTop(iicStr);
  ImInside = 1;

  pos = PrintProbesOfType(ezo, all, pos);
  pos++;
  //EscLocate(5, pos++);
  //PrintMenuKeyStd('A'); Serial.print(F("Clear Cal."));
  //pos = PrintShortLine(pos, 8);

  if (myMenu.a){
    // 1-Point Cal.
    EscLocate(5, pos);
    PrintMenuKeyStd('A'); Serial.print(F("Do 1-Pt. Cal."));
  }
  if (myMenu.b){
    // 3-Point Cal.
    EscLocate(26, pos);
    PrintMenuKeyStd('B'); Serial.print(F("Do 2-Pt. Cal."));
  }
  if (myMenu.c){
    // 3-Point Cal.
    EscLocate(47, pos);
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
  calCnt = 0;
  calTemp = 0;
  switch (pos){
  case 'a':
    if (myMenu.b){
      if (ezoProbe[ezo].type == ezoDiO2){
        // medium: air    cmd: "Cal"
      }
      else if (ezoProbe[ezo].type == ezoPH){
        // "Cal,mid,value"
      }
      else{
        // (ORP & RTD) "Cal,value"
      } 
    }
    break;
  case 'b':
    if (myMenu.c){
      // 2-Pt. Cal
      switch (ezoProbe[ezo].type){
      case ezoEC:
        // "Cal,dry" (air)
        // "Cal,value"
        break;
      case ezoDiO2:
        // medium: air        cmd:  "Cal"
        // medium: 0-liquid   cmd:  "Cal,0"        
        break;
      default:
        // just pH
        // "Cal,mid,value"
        // "Cal,low,value"
        break;
      }  
    }  
    break;
  case 'c':
    if (myMenu.d){
      // 3-Pt. Cal
      switch (ezoProbe[ezo].type){
      case ezoEC:
        // "Cal,dry" (air)
        // "Cal,low,value"
        // "Cal,high,value"
        break;
      default:
        // just pH
        // "Cal,mid,value"
        // "Cal,low,value"
        // "Cal,high,value"
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

  if (pos > 0){
    goto Start;
  }
  
  ImInside = 0;
}

void PrintProbeMenu(byte ezo){

  byte all = 0;

    DummyNameToStrDefault();
    adrDefault = 33;
    all = 0;

Start:

  int8_t pos = PrintMenuTop((char*)"- Probe(Type) Menu -");
  
  pos = PrintProbesOfType(ezo, all, pos);

  pos = PrintAllMenuOpt1(pos + 1);
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('H'); Serial.print(F("Calibration(s)..."));
  EscLocate(30, pos);
  PrintMenuKeyStd('I'); 
  if (!all){
    EscBold(1);
  }
  else{
    EscFaint(1);
  }
  Serial.print(F("Select Single"));
  EscLocate(51, pos++);
  PrintMenuKeyStd('J');
  if (all){
    EscBold(1);
  }
  else{
    EscFaint(1);
  }
  Serial.print(F("Select ALL"));
  EscBold(0);

  PrintMenuEnd(pos);

  pos = GetUserKey('j', 9);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // Return
    break;
  case 'a':
    EzoReset(ezo, all);
    break;
  case 'b':
    EzoSetCal((char*)"Cal,clear", ezo, all);
    break;
  case 'c':
    EzoSetName((char*)"", ezo, all, 0);
    break;
  case 'd':
    EditAutoName();
    break;
  case 'e':
    EzoSetName(strDefault, ezo, all, 1);
    break;
  case 'f':
    EditAutoAddress();
    break;
  case 'g':
    SetAutoAddress();
    break;
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
    PrintFloat(failSave[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('g' + i);
    PrintFloat(tooLow[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('m' + i);
    PrintFloat(low[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('s' + i);
    PrintFloat(high[i], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('A' + i);
    PrintFloat(tooHigh[i], 4, 2, ' ');
    PrintSpacer(0);

    EscLocate(3, pos++);
  }

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', -1);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'f'){
    // FailSave
    pos -= 'a';
    failSave[pos] = GetUserFloat(failSave[pos]);
    pos = 1;
  }
  else if (pos >= 'g' && pos <= 'l'){
    // tooLow
    pos -= 'g';
    tooLow[pos] = GetUserFloat(tooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'm' && pos <= 'r'){
    // Low
    pos -= 'm';
    low[pos] = GetUserFloat(low[pos]);
    pos = 1;
  }
  else if (pos >= 's' && pos <= 'x'){
    // High
    pos -= 's';
    high[pos] = GetUserFloat(high[pos]);
    pos = 1;
  }
  else if (pos >= 'A' && pos <= 'F'){
    // tooHigh
    pos -= 'A';
    tooHigh[pos] = GetUserFloat(tooHigh[pos]);
    pos = 1;
  }

  if (pos > 0){
    goto Start;
  }
  
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
    PrintSmallMenuKey('a' + i);
    PrintSerTime(delayTimes[i], 0, 1);
    PrintSmallSpacer();
    PrintSmallMenuKey('g' + i);
    PrintSerTime(actionTooLow[i], 0, 1);
    PrintSmallSpacer();
    PrintSmallMenuKey('m' + i);
    PrintSerTime(actionLow[i], 0, 1);
    PrintSmallSpacer();
    PrintSmallMenuKey('s' + i);
    PrintSerTime(actionHigh[i], 0, 1);
    PrintSmallSpacer();
    PrintSmallMenuKey('A' + i);
    PrintSerTime(actionTooHigh[i], 0, 1);
    PrintSmallSpacer();
    EscLocate(3, pos++);
  }

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', -1);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'f'){
    // FailSave
    pos -= 'a';
    delayTimes[pos] = GetUserTime(delayTimes[pos]);
    pos = 1;
  }
  else if (pos >= 'g' && pos <= 'l'){
    // tooLow
    pos -= 'g';
    actionTooLow[pos] = GetUserTime(actionTooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'm' && pos <= 'r'){
    // Low
    pos -= 'm';
    actionLow[pos] = GetUserTime(actionLow[pos]);
    pos = 1;
  }
  else if (pos >= 's' && pos <= 'x'){
    // High
    pos -= 's';
    actionHigh[pos] = GetUserTime(actionHigh[pos]);
    pos = 1;
  }
  else if (pos >= 'A' && pos <= 'F'){
    // tooHigh
    pos -= 'A';
    actionTooHigh[pos] = GetUserTime(actionTooHigh[pos]);
    pos = 1;
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
      PrintBoldFloat(ezoProbe[i].value[0] / divisor, lz, dp, ' ');
      avg += ezoProbe[i].value[0];
      PrintUnit(ezotype, 1, 0, 3);
    }
  }

  if (posAct){
    avgVal[ezotype] = avg / (long)posAct;
  }

  return posAct;

}
byte PrintWaterVals(byte pos){

  byte posMax = 0;
  byte posAct = 0;

  posMax = PrintWaterValsHlp(pos, 5, ezoRTD, 2, 2, 1); //, &avg_RTD);

  posAct = PrintWaterValsHlp(pos, 18, ezoEC, 4, 0, 1000); //, &avg_EC);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 30, ezoPH, 2, 2, 1); //, &avg_pH);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 42, ezoORP, 4, 2, 1); //, &avg_ORP);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 54, ezoDiO2, 3, 2, 1); //, &avg_O2);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 69, ezoLVL, 3, 2, 1); //, &avg_O2);
  if (posAct > posMax){
    posMax = posAct;
  }

  return pos + posMax;

}

byte PrintAVGs(byte pos){
    
  SetAvgColorEZO(ezoRTD);
  EscLocate(8, pos);
  PrintBoldFloat(avg_RTD, 2, 2, ' ');
  PrintUnit(ezoRTD, 0, 0, 3);

  //EscLocate(6, pos + 2);
  //PrintActionTimes(ezoRTD);


  SetAvgColorEZO(ezoEC);
  EscLocate(21, pos);
  PrintBoldInt(avg_EC / 1000, 4, ' ');
  PrintUnit(ezoEC, 0, 0, 3);
  
  //EscLocate(21, pos + 2);
  //PrintActionTimes(ezoEC);


  SetAvgColorEZO(ezoPH);
  EscLocate(33, pos);
  PrintBoldFloat(avg_pH, 2, 2, ' ');
  PrintUnit(ezoPH, 0, 0, 3);

  //EscLocate(35, pos + 2);
  //PrintActionTimes(ezoPH);


  SetAvgColorEZO(ezoORP);
  EscLocate(45, pos);
  PrintBoldFloat(avg_ORP, 4, 2, ' ');
  PrintUnit(ezoORP, 0,  0 , 3);

  //EscLocate(49, pos + 2);
  //PrintActionTimes(ezoORP);


  SetAvgColorEZO(ezoDiO2);
  EscLocate(59, pos);
  PrintBoldFloat(avg_O2, 3, 2, ' ');
  PrintUnit(ezoDiO2, 0, 0, 3);

  //EscLocate(63, pos + 2);
  //PrintActionTimes(ezoDiO2);

  SetAvgColorEZO(ezoLVL);
  EscLocate(72, pos++);
  PrintBoldFloat(avg_LVL, 3, 2, ' ');
  PrintUnit(ezoLVL, 0, 0, 3);

  return pos;

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
  PrintSpacer(0);

  //Serial.print(F(" | Temperature | Conductivity |     pH     |    Redox    |     O2     |"));
  pos = PrintLine(pos, 3, 77);
    
  
  //EscBold(1);
  EscBold(0);

  PrintErrorOK(0, 0, (char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLine(pos, 3, 77);
  //PrintLine(pos + 1, 6, 70);

  // Avg 
  pos = PrintAVGs(pos);

  EscBold(1);
  pos = PrintLine(pos, 3, 77);
  EscBold(0);

}

void PrintMainMenu(){

Start:

  int pos = PrintMenuTop((char*)"- QuickWater 1.01 -");
  
  uint32_t hlpTime = 0;

  pos = PrintProbesOfType(255, 1, pos);

  EscLocate(5, pos);
  PrintMenuKeyStd('A'); Serial.print(F("Boot"));
  EscLocate(16, pos);
  PrintMenuKeyStd('B'); Serial.print(F("Date"));
  EscLocate(27, pos);
  PrintMenuKeyStd('C'); Serial.print(F("Time"));
  EscLocate(38, pos);
  PrintMenuKeyStd('D'); Serial.print(F("Addr. = "));
  PrintBoldInt(myAddress, 3, '0');
  EscLocate(56, pos++);
  PrintMenuKeyStd('E'); Serial.print(F("Speed = "));
  EscBold(1);
  Serial.print(mySpeed);
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('F'); Serial.print(F("Boot4Terminal = "));
  if (myBoot){
    EscFaint(1);
    Serial.print(F("False"));
  }
  else{
    EscBold(1);
    Serial.print(F(" True"));
  }
  EscLocate(34, pos);

  PrintMenuKeyStd('G'); Serial.print(F("Boot4Slave = "));
  if (myBoot){
    EscBold(1);
    Serial.print(F(" True"));
  }
  else{
    EscFaint(1);
    Serial.print(F("False"));
  }
  EscLocate(59, pos++);

  PrintMenuKey('H', 0, 0, 0, 1, (mySolarized), (!mySolarized));
  Serial.print(F("Sol.Color"));
  EscBold(0);

  pos = PrintShortLine(pos++, 8);
//****************************************************
  EscLocate(5, pos);
  PrintMenuKeyStd('I'); Serial.print(F("All..."));
  EscLocate(17, pos);
  PrintMenuKey('J', 0, 0, 0, 1, (myDefault), (!myDefault)); Serial.print(F("SetDefault"));
  EscLocate(34, pos);
  PrintMenuKeyStd('K'); Serial.print(F("DelDef."));
  EscLocate(47, pos);
  PrintMenuKeyStd('L'); Serial.print(F("Values..."));
  EscLocate(62, pos);
  PrintMenuKeyStd('M'); Serial.print(F("Times..."));
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('m', ezoCnt - INTERNAL_LEVEL_CNT);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // EXIT
    break;
  case 'i':
    // All Menu
    PrintAllMenu();
    break;
  case 'a':
    // ReBoot
    EscCls();
    EscLocate(1,1);
    EzoScan();
    break;
  case 'b':
    // Date
    hlpTime = SerializeTime(1, 1, 2023, myHour, myMin, mySec);    // Time of now
    hlpTime += GetUserDate(myTime);
  case 'c':
    // Time
    if (pos == 'c'){
      hlpTime = SerializeTime(myDay, myMonth, myYear, 0, 0, 0);    // Midnight of today
      hlpTime += GetUserTime(myTime);
    }
    DeSerializeTime(hlpTime, &myDay, &myMonth, &myYear, &myHour, &myMin, &mySec);    
    RTC_SetDateTime();
    myTime = SerializeTime(myDay, myMonth, myYear, myHour, myMin, mySec);
    break;
  case 'd':
    // Address
    myAddress = GetUserInt(myAddress);
    if (!myAddress || myAddress > 254){
      // illegal address - reload from eeprom
      myFromRom();
    }
    else{
      // save to eeprom...
      myToRom();
    }
    break;
  case 'e':
    // Speed
    // Set Speed
    mySpeed = GetUserInt(mySpeed);
    if (IsSerialSpeedValid(mySpeed)){ 
      // valid - save to eeprom
      myToRom();
    }
    else{
      // illegal - reload from eeprom
      myFromRom();
    }
    break;
  case 'j':
    // Save as default
    myDefault = 1;
    myToRom();
    DefaultProbesToRom();
    break;
  case 'k':
    // Erase Defaults
    myDefault = 0;
    myToRom();
    break;
  case 'l':
    // Values
    PrintValuesMenu();
    break;
  case 'm':
    // Values
    PrintTimingsMenu();
    break;
  case 'f':
    // Boot for Terminal
    myBoot = 0;
    myToRom();
    break;
  case 'g':
    // Boot for Slave
    myBoot = 1;
    myToRom();
    break;
  case 'h':
    // Solarized
    mySolarized = !mySolarized;
    fgFaint = 90 + (mySolarized * 2);
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