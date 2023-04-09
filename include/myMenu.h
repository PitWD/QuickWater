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
  strcpy(strDefault, "-DummyName-");
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
    SetAvgColorEZO(ezoProbe[ezo].value[0], ezoProbe[ezo].type);
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

  struct myMenu{
    byte b:1;
    byte c:1;
    byte d:1;
    byte e:1;
    byte f:1;
    byte g:1;
    byte h:1;
  }myMenu;
  
  myMenu.b = 1;
  myMenu.c = 1;
  myMenu.d = 1;
  myMenu.e = 1;
  myMenu.f = 1;
  myMenu.g = 1;
  myMenu.h = 1;

  Start:

  strcpy(iicStr, "- Calibrate ");

  switch (ezoProbe[ezo].type){
  case ezoRTD:
    /* code */
    strcpy(&iicStr[12], "RTD -");
    if (!ImInside){
      myMenu.c = 0;
      myMenu.d = 0;
      myMenu.g = 0;
      myMenu.h = 0;
      calLow = CAL_RTD_LOW;
      calMid = CAL_RTD_MID;
      calHigh = CAL_RTD_HIGH;
      calRes = CAL_RTD_RES;
      calAvg = avg_RTD;
    }
    break;
  case ezoEC:
    /* code */
    strcpy(&iicStr[12], "EC -");
    if (!ImInside){
      myMenu.d = 0;
      myMenu.h = 0;
      calLow = CAL_EC_LOW;
      calMid = CAL_EC_MID;
      calHigh = CAL_EC_HIGH;
      calRes = CAL_EC_RES;
      calAvg = avg_EC;
    }
    break;
  case ezoPH:
    /* code */
    strcpy(&iicStr[12], "pH -");
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
    strcpy(&iicStr[12], "ORP -");
    if (!ImInside){
      myMenu.c = 0;
      myMenu.d = 0;
      myMenu.g = 0;
      myMenu.h = 0;
      calLow = CAL_ORP_LOW;
      calMid = CAL_ORP_MID;
      calHigh = CAL_ORP_HIGH;
      calRes = CAL_ORP_RES;
      calAvg = avg_ORP;
    }
    break;
  case ezoDiO2:
    /* code */
    strcpy(&iicStr[12], "O2 -");
    if (!ImInside){
      myMenu.d = 0;
      myMenu.e = 0;
      myMenu.f = 0;
      myMenu.g = 0;
      myMenu.h = 0;
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
  EscLocate(5, pos++);
  PrintMenuKeyStd('A'); Serial.print(F("Clear Cal."));
  pos = PrintShortLine(pos, 8);

  if (myMenu.b){
    // 1-Point Cal.
    EscLocate(5, pos);
    PrintMenuKeyStd('B'); Serial.print(F("Do 1-Pt. Cal."));
  }
  if (myMenu.c){
    // 3-Point Cal.
    EscLocate(26, pos);
    PrintMenuKeyStd('C'); Serial.print(F("Do 2-Pt. Cal."));
  }
  if (myMenu.d){
    // 3-Point Cal.
    EscLocate(47, pos);
    PrintMenuKeyStd('D'); Serial.print(F("Do 3-Pt. Cal."));
  }
  pos++;
  pos = PrintShortLine(pos, 8);

  if (myMenu.e){
    // Set Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('E'); Serial.print(F("Set Single/Mid"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calMid, 4, 2, ' ');
  }
  if (myMenu.f){
    // Use Avg as Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('F'); Serial.print(F("Set AVG As Mid"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calAvg, 4, 2, ' ');
  }
  if (myMenu.g){
    // Set Low Point
    EscLocate(5, pos);
    PrintMenuKeyStd('G'); Serial.print(F("Set Low"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calLow, 4, 2, ' ');
}
  if (myMenu.h){
    // Set High Point
    EscLocate(5, pos);
    PrintMenuKeyStd('H'); Serial.print(F("Set High"));
    PrintPointEqual(pos++);
    PrintBoldFloat(calHigh, 4, 2, ' ');
  }
  
  /*
  switch (ezoProbe[ezo].type){
  case ezoRTD:
    pos = PrintCal_B(0, pos);
    pos = PrintCal_C(1, pos); myMenu.c = 0;
    pos = PrintCal_D(1, pos); myMenu.d = 0;
    pos = PrintCal_E(0, pos, calMid);
    pos = PrintCal_F(0, pos, avg_RTD);
    pos = PrintCal_G(1, pos, calLow); myMenu.g = 0;
    pos = PrintCal_H(1, pos, calHigh); myMenu.h = 0;
    break;
  case ezoEC:
    pos = PrintCal_B(0, pos);
    pos = PrintCal_C(0, pos);
    pos = PrintCal_D(1, pos); myMenu.d = 0;
    pos = PrintCal_E(0, pos, calMid);
    pos = PrintCal_F(0, pos, avg_EC);
    pos = PrintCal_G(0, pos, calLow);
    pos = PrintCal_H(1, pos, calHigh); myMenu.h = 0;
    break;
  case ezoPH:
    pos = PrintCal_B(0, pos);
    pos = PrintCal_C(0, pos);
    pos = PrintCal_D(0, pos);
    pos = PrintCal_E(0, pos, calMid);
    pos = PrintCal_F(0, pos, avg_pH);
    pos = PrintCal_G(0, pos, calLow);
    pos = PrintCal_H(0, pos, calHigh);
    break;
  case ezoORP:
    pos = PrintCal_B(0, pos);
    pos = PrintCal_C(1, pos); myMenu.c = 0;
    pos = PrintCal_D(1, pos); myMenu.d = 0;
    pos = PrintCal_E(0, pos, calMid);
    pos = PrintCal_F(0, pos, avg_ORP);
    pos = PrintCal_G(1, pos, calLow); myMenu.g = 0;
    pos = PrintCal_H(1, pos, calHigh); myMenu.h = 0;
    break;
  case ezoDiO2:
    pos = PrintCal_B(0, pos);
    pos = PrintCal_C(0, pos);
    pos = PrintCal_D(1, pos); myMenu.d = 0;
    pos = PrintCal_E(1, pos, calMid); myMenu.e = 0;
    pos = PrintCal_F(1, pos, avg_O2); myMenu.f = 0;
    pos = PrintCal_G(1, pos, calLow); myMenu.g = 0;
    pos = PrintCal_H(1, pos, calHigh); myMenu.h = 0;
    break;
  default:
    break;
  }
  */

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('h', 0);
  switch (pos){
  case 'a':
    break;
  case 'b':
    if (myMenu.b){
      /* code */
    }  
    break;
  case 'c':
    if (myMenu.c){
      /* code */
    }  
    break;
  case 'd':
    if (myMenu.d){
      /* code */
    }  
    break;
  case 'e':
    if (myMenu.e){
      /* code */
    }  
    break;
  case 'f':
    if (myMenu.f){
      /* code */
    }  
    break;
  case 'g':
    if (myMenu.g){
      /* code */
    }  
    break;
  case 'h':
    if (myMenu.h){
      /* code */
    }  
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
  // Order to EzoType
                // 0  1  2  3  4  5  6  7
  byte offset[] = {0, 5, 6, 1, 2, 3, 4, 7};
  
  EscLocate(12, pos++);
  PrintFlexSpacer(0, 1);
  Serial.print(F("FailSafe"));
  PrintFlexSpacer(1, 2);
  PrintLowToHigh();

  PrintLine(pos++, 3, 76);
  EscLocate(3, pos++);
  for (i = 0; i < 8; i++){
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[offset[i]]), 9);
    PrintSpacer(0);
    PrintSmallMenuKey('a' + i);
    PrintFloat(failSave[offset[i]], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('i' + i);
    PrintFloat(tooLow[offset[i]], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('q' + i);
    PrintFloat(low[offset[i]], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('I' + i);
    PrintFloat(high[offset[i]], 4, 2, ' ');
    PrintSpacer(0);
    PrintSmallMenuKey('Q' + i);
    PrintFloat(tooHigh[offset[i]], 4, 2, ' ');
    PrintSpacer(0);
    if (i == 2){
      pos = PrintShortLine(pos, 6);
    }   
    EscLocate(3, pos++);
  }

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', -1);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'h'){
    // FailSave
    pos = offset[pos - 'a'];
    failSave[pos] = GetUserFloat(failSave[pos]);
    pos = 1;
  }
  else if (pos >= 'i' && pos <= 'p'){
    // tooLow
    pos = offset[pos - 'i'];
    tooLow[pos] = GetUserFloat(tooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'q' && pos <= 'x'){
    // Low
    pos = offset[pos - 'q'];
    low[pos] = GetUserFloat(low[pos]);
    pos = 1;
  }
  else if (pos >= 'I' && pos <= 'P'){
    // High
    pos = offset[pos - 'I'];
    high[pos] = GetUserFloat(high[pos]);
    pos = 1;
  }
  else if (pos >= 'Q' && pos <= 'X'){
    // tooHigh
    pos = offset[pos - 'Q'];
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
  // Order to EzoType
                // 0  1  2  3  4  5  6  7
  byte offset[] = {0, 5, 6, 1, 2, 3, 4, 7};
  
  EscLocate(12, pos++);
  PrintSpacer(1);
  Serial.print(F("DelayTimes"));
  PrintFlexSpacer(0, 2);
  PrintLowToHigh();

  PrintLine(pos++, 3, 76);
  EscLocate(3, pos++);
  for (i = 0; i < 8; i++){
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[offset[i]]), 9);
    PrintSmallSpacer();
    PrintSmallMenuKey('a' + i);
    PrintSerTime(delayTimes[offset[i]], 0);
    PrintSmallSpacer();
    PrintSmallMenuKey('i' + i);
    PrintSerTime(actionTooLow[offset[i]], 0);
    PrintSmallSpacer();
    PrintSmallMenuKey('q' + i);
    PrintSerTime(actionLow[offset[i]], 0);
    PrintSmallSpacer();
    PrintSmallMenuKey('I' + i);
    PrintSerTime(actionHigh[offset[i]], 0);
    PrintSmallSpacer();
    PrintSmallMenuKey('Q' + i);
    PrintSerTime(actionTooHigh[offset[i]], 0);
    PrintSmallSpacer();
    if (i == 2){
      pos = PrintShortLine(pos, 6);
    }   
    EscLocate(3, pos++);
  }

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', -1);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (pos >= 'a' && pos <= 'h'){
    // FailSave
    pos = offset[pos - 'a'];
    delayTimes[pos] = GetUserTime(delayTimes[pos]);
    pos = 1;
  }
  else if (pos >= 'i' && pos <= 'p'){
    // tooLow
    pos = offset[pos - 'i'];
    actionTooLow[pos] = GetUserTime(actionTooLow[pos]);
    pos = 1;
  }
  else if (pos >= 'q' && pos <= 'x'){
    // Low
    pos = offset[pos - 'q'];
    actionLow[pos] = GetUserTime(actionLow[pos]);
    pos = 1;
  }
  else if (pos >= 'I' && pos <= 'P'){
    // High
    pos = offset[pos - 'I'];
    actionHigh[pos] = GetUserTime(actionHigh[pos]);
    pos = 1;
  }
  else if (pos >= 'Q' && pos <= 'X'){
    // tooHigh
    pos = offset[pos - 'Q'];
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

byte PrintWaterValsHlp(byte pos, byte posX, byte ezotype, byte lz, byte dp, int divisor, long *avgExt){

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
    *avgExt = avg / (long)posAct;
  }

  return posAct;

}

byte PrintWaterVals(byte pos){

  byte posMax = 0;
  byte posAct = 0;

  posMax = PrintWaterValsHlp(pos, 8, ezoRTD, 2, 2, 1, &avg_RTD);

  posAct = PrintWaterValsHlp(pos, 23, ezoEC, 4, 0, 1000, &avg_EC);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 37, ezoPH, 2, 2, 1, &avg_pH);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 49, ezoORP, 4, 2, 1, &avg_ORP);
  if (posAct > posMax){
    posMax = posAct;
  }

  posAct = PrintWaterValsHlp(pos, 63, ezoDiO2, 3, 2, 1000, &avg_O2);
  if (posAct > posMax){
    posMax = posAct;
  }

  return pos + posMax;

}

byte PrintAVGs(byte pos){
  
  SetAvgColorEZO(avg_RTD, ezoRTD);
  EscLocate(11, pos);
  PrintBoldFloat(avg_RTD,2,2,' ');
  PrintUnit(ezoRTD, 0, 0, 3);

  SetAvgColorEZO(avg_EC, ezoEC);
  EscLocate(26, pos);
  PrintBoldInt(avg_EC / 1000, 4, ' ');
  PrintUnit(ezoEC, 0, 0, 3);
  
  SetAvgColorEZO(avg_pH, ezoPH);
  EscLocate(40, pos);
  PrintBoldFloat(avg_pH,2,2,' ');
  PrintUnit(ezoPH, 0, 0, 3);
  
  SetAvgColorEZO(avg_ORP, ezoORP);
  EscLocate(52, pos);
  PrintBoldFloat(avg_ORP,4,2,' ');
  PrintUnit(ezoORP, 0,  0 , 3);
  
  SetAvgColorEZO(avg_O2, ezoDiO2);
  EscLocate(66, pos++);
  PrintBoldFloat(avg_O2,3,2,' ');
  PrintUnit(ezoDiO2, 0, 0, 3);
  
  return pos;

}

void PrintCenteredWithSpacer(char *strIN, byte centerLen){
  PrintSpacer(1);
  PrintCentered(strIN, centerLen);
}
void PrintLoopMenu(){

  /*
  byte type[] = {1, 3, 2, 4, 7};
  byte cnt[] = {11, 12, 10, 11, 10};
  */

  EscCls();
  EscInverse(1);
  byte pos = PrintMenuTop((char*)"- QuickWater 1.01 -");
  EscInverse(0);
  pos++;

  EscLocate(5, pos++);

  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoRTD]),11);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoEC]),12);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoPH]),10);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoORP]),11);
  PrintCenteredWithSpacer(FaStrange(ezoStrLongType[ezoDiO2]),10);
  PrintSpacer(0);

  //EscBold(1);
  //Serial.print(F(" | Temperature | Conductivity |     pH     |    Redox    |     O2     |"));
  pos = PrintLine(pos, 6, 70);
  EscBold(0);

  PrintErrorOK(0, -1 ,(char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLine(pos, 6, 70);

  // Avg 
  pos = PrintAVGs(pos);

  EscBold(1);
  pos = PrintLine(pos, 6, 70);
  EscBold(0);

}

void PrintMainMenu(){

Start:

  int pos = PrintMenuTop((char*)"- QuickWater 1.01 -");
  
  uint32_t hlpTime = 0;

/*
  PrintLine(pos, 5, 63);
  for (int i = 0; i < ezoCnt; i++){
    pos++;
    PrintProbeLine(i, pos, 1);
  }
  pos++;
  PrintLine(pos++, 5, 63);
*/

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

  pos = GetUserKey('m', ezoCnt);
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
    DeSerializeTime(hlpTime + GetUserDate(myTime), &myDay, &myMonth, &myYear, &myHour, &myMin, &mySec);
    RTC_SetDateTime();
    myTime = SerializeTime(myDay, myMonth, myYear, myHour, myMin, mySec);
    break;
  case 'c':
    // Time
    hlpTime = SerializeTime(myDay, myMonth, myYear, 0, 0, 0);    // Midnight of today
    DeSerializeTime(hlpTime + GetUserTime(myTime), &myDay, &myMonth, &myYear, &myHour, &myMin, &mySec);
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