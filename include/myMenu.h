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

byte myLastLine = 0;

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
  if (my.Setting > 2){
    my.Setting = 0;
    my.Default = 0;
  }
  if (my.Temporary > 3){
    // Manual (Prefill) Settings
    my.Temporary = 0;
    my.Default = 0;
  }
  if (my.Cnt > EZO_MAX_PROBES - INTERNAL_LEVEL_CNT){
    my.Cnt = 0;
    my.Default = 0;
  }
  
  fgFaint = my.Solarized;

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

byte PrintQuickWater(){
  return PrintMenuTop((char*)"- QuickWater 1.02 -");
}

byte PrintAllMenuOpt1(byte pos){

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("Reset     "));
  //PrintSpaces(5);
  PrintMenuKeyStd('b'); Serial.print(F("Clear Calibration(s)     "));
  //PrintSpaces(5);
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
      PrintMenuKey(ezo + '1', 0, '(', 0, 0, bold, !bold);
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
    EscBoldColor(0);
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
  PrintMenuKeyStdBoldFaint('h', my.Boot, !my.Boot);
  Serial.print(F("Boot As ModBUS Slave     "));
  EscFaint(0);
  //PrintSpaces(5);
  PrintMenuKeyStdBoldFaint('i', !my.Boot, my.Boot);
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

void PrintPointEqual(uint8_t pos, int32_t value){
  Serial.print (F("-Pt. "));
  EscLocate(29, pos);
  Serial.print (F("= "));
  PrintBoldFloat(value, 4, 2, ' ');
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

  pos = PrintProbesOfType(ezo, all, pos++, 1);
//  pos++;

  EscLocate(5, pos++);
  if (myMenu.a){
    // 1-Point Cal.
    PrintMenuKeyStd('a'); Serial.print(F("Do 1-Pt. Cal.     "));
    //PrintSpaces(5);
  }
  if (myMenu.b){
    // 3-Point Cal.
    PrintMenuKeyStd('b'); Serial.print(F("Do 2-Pt. Cal.     "));
    //PrintSpaces(5);
  }
  if (myMenu.c){
    // 3-Point Cal.
    PrintMenuKeyStd('c'); Serial.print(F("Do 3-Pt. Cal."));
  }
  //pos++;
  pos = PrintShortLine(pos, 8);

  if (myMenu.d){
    // Set Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('d'); Serial.print(F("Set Single/Mid"));
    PrintPointEqual(pos++, calMid);
  }
  if (myMenu.e){
    // Use Avg as Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('e'); Serial.print(F("Set AVG As Mid"));
    PrintPointEqual(pos++, calAvg);
  }
  if (myMenu.f){
    // Set Low Point
    EscLocate(5, pos);
    PrintMenuKeyStd('f'); Serial.print(F("Set Low"));
    PrintPointEqual(pos++, calLow);
}
  if (myMenu.g){
    // Set High Point
    EscLocate(5, pos);
    PrintMenuKeyStd('g'); Serial.print(F("Set High"));
    PrintPointEqual(pos++, calHigh);
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
      calTemp = PrintValsForCal(ezo, all, calMid);
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
      switch (ezoProbe[ezo].type){
      case ezoEC:
        calTemp = PrintValsForCal(ezo, all, 0);
        break;
      default:
        calTemp = PrintValsForCal(ezo, all, calMid);
        break;
      }
      //calTemp = PrintValsForCal(ezo, all);
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
      switch (ezoProbe[ezo].type){
      case ezoEC:
        calTemp = PrintValsForCal(ezo, all, 0);
        break;
      default:
        calTemp = PrintValsForCal(ezo, all, calMid);
        break;
      }
      //calTemp = PrintValsForCal(ezo, all);
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
      calTemp = PrintValsForCal(ezo, all, calVal[i]);
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
  PrintMenuKeyStdBoldFaint('i', !all, all); 
  Serial.print(F("Select Single"));
  EscLocate(51, pos++);
  PrintMenuKeyStdBoldFaint('j', all, !all);
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
void PrintCenteredWithSpacer(char *strIN, byte centerLen){
  PrintSpacer(1);
  PrintCentered(strIN, centerLen);
}


void PrintLowToHigh(){
  Serial.print(F("tooLow   |    Low     |    High    |  tooHigh   |"));
}

int8_t PrintCopySettingTo(int8_t pos){
  EscLocate(5, pos++);
  EscKeyStyle(1);
  Serial.print(F("1-3):"));
  EscKeyStyle(0);
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

void PrintValuesMenuHlp(char key, byte i, uint32_t value){
  PrintSmallMenuKey(key + i);
  PrintFloat(value, 4, 2, ' ');
  PrintSpacer(0);
}
byte PrintValuesMenuChangeVal(int32_t *valIN){
  // THIS IS STRANGE
  //    Using this just two times has a lower flash use than
  //    if it's used more or less times as replacement for:
  //        pos -= 'a';
  //        setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
  //        pos = 1;
  // THIS IS STRANGE
  *valIN = GetUserFloat(*valIN);
  return 1;
}

byte IsKeyBetween(char key, char start, char stop){
  // THIS IS STRANGE
  //    Using this some times has a lower flash use than
  //    if it's used more or less times
  // THIS IS STRANGE
  return (key >= start && key <= stop);
}

//#define IsKeyBetween(key, start, stop) ((key >= start) && (key <= stop))

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

    PrintValuesMenuHlp('a', i, setting.FailSaveValue[i]);

    PrintValuesMenuHlp('g', i, setting.ValueTooLow[i]);

    PrintValuesMenuHlp('m', i, setting.ValueLow[i]);

    PrintValuesMenuHlp('s', i, setting.ValueHigh[i]);

    PrintValuesMenuHlp('A', i, setting.ValueTooHigh[i]);

    EscLocate(3, pos++);
  }

  pos = PrintCopySettingTo(pos);
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('x', 3);

  if (pos < 1){
    // Exit & TimeOut
  }
  
  else if (IsKeyBetween(pos, 'a', 'f')){
    // FailSave
    pos = PrintValuesMenuChangeVal(&setting.FailSaveValue[pos - 'a']);
    //pos -= 'a';
    //setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'g', 'l')){
    // tooLow
    pos = PrintValuesMenuChangeVal(&setting.ValueTooLow[pos - 'g']);
    //pos -= 'g';
    //setting.ValueTooLow[pos] = GetUserFloat(setting.ValueTooLow[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'm', 'r')){
    // Low
    pos = PrintValuesMenuChangeVal(&setting.ValueLow[pos - 'm']);
    //pos -= 'm';
    //setting.ValueLow[pos] = GetUserFloat(setting.ValueLow[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 's', 'x')){
    // High
    pos = PrintValuesMenuChangeVal(&setting.ValueHigh[pos - 's']);
    //pos -= 's';
    //setting.ValueHigh[pos] = GetUserFloat(setting.ValueHigh[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'A', 'F')){
    // tooHigh
    pos = PrintValuesMenuChangeVal(&setting.ValueTooHigh[pos - 'A']);
    //pos -= 'A';
    //setting.ValueTooHigh[pos] = GetUserFloat(setting.ValueTooHigh[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, '1', '3')){
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

byte CorrectType(byte i){
    // Correct type for the three times EC
    if (i == 2){
      return 1;
    }
    else if (i > 2){
      return i - 2;
    }
    return i;
    // type = (i == 2) ? 1 : ((i > 2) ? (type - 2) : type);
}

byte CorrectForRepeat(byte i){
  if (i < 2){
    // set repeat for temp & 1st EC high port
    return i + 8;
  }
  else if (i == 4){
    // set repeat for pH high port
    return 10;
  }
  else if (i == 7){
    // set repeat for level high port
    return 11;
  }
  return i;
}
byte CorrectFromRepeat(byte i){
  // set back i from high-port
  if (i == 11){
    return 7;
  }
  else if (i == 10){
    return 4;
  }
  else if (i > 7){
    return i - 8;
  }
  return i;
}

void RunManualSetting(byte port, byte style){

  struct runManualSTRUCT{
      uint16_t runTime;
      uint16_t offset;
      uint16_t onTime;
      uint16_t offTime;
      byte state;
      uint16_t inState;
      uint16_t repeats;
  }manualTiming[12];

  byte firstLine = 0;

  int8_t pos = PrintMenuTop((char*)"- RUN Manual -");
  pos++;

  EscLocate(19, pos++);
  Serial.print(F("|  Offset  |  OnTime  | OffTime  | ON's_2Do |  State   |"));
  //PrintCenteredWithSpacer((char*)"Offset", 8);
  //PrintCenteredWithSpacer((char*)"OnTime", 8);
  //PrintCenteredWithSpacer((char*)"OffTime", 8);
  //PrintCenteredWithSpacer((char*)"ON's_2Do", 8);
  //PrintCenteredWithSpacer((char*)"State", 8);
  //PrintSmallSpacer();
  EscBold(1);
  PrintLine(pos++, 7, 68);

  // Copy Low & High values to manualTiming array
  for (byte i = 0; i < 8; i++) {
    manualTiming[i].runTime = manual.Low[i];
    if (i < 4){
      manualTiming[i + 8].runTime = manual.High[i];
    }      
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
  //for (byte i = 0; i < 6; i++){
  for (byte i = 0; i < 8; i++){
    
    byte typeExist = 0;
    //byte type = i;

    // Correct type for the three times EC
    byte type = CorrectType(i);
    
    DoLowHigh:
    // Low-Ports
    /*
    Serial.print(i);
    Serial.print(F(":"));
    Serial.print(port);
    Serial.print(F(":"));
    Serial.println(manualTiming[i].runTime);
    */
    if ((i == port || port == 255) && manualTiming[i].runTime){

      // we're in Action and have a time...
      switch (style){
      case 0:
        // Distributed

        manualTiming[i].repeats = (maxTime / manualTiming[i].runTime);
        
        if (manualTiming[i].repeats > manualTiming[i].runTime){
          // we can't On/Off shorter than 1sec.
          manualTiming[i].repeats = manualTiming[i].runTime;
        }
        
        ReCalc:   // sucking integer resolution need this up & down
                  // to get total onTime exact but also as distributed as possible
        manualTiming[i].onTime = manualTiming[i].runTime / manualTiming[i].repeats;
        if ((manualTiming[i].onTime * manualTiming[i].repeats) < manualTiming[i].runTime){
          // onTime too short - raise onTime (suck - 1)
          manualTiming[i].onTime++;
        }
        if ((manualTiming[i].onTime * manualTiming[i].repeats) > manualTiming[i].runTime){
          // onTime too long - lower repeats (suck - 2)
          if (manualTiming[i].repeats){
            manualTiming[i].repeats--;
            goto ReCalc;
          }          
        }

        manualTiming[i].offTime = (maxTime - manualTiming[i].runTime) / manualTiming[i].repeats;               
        manualTiming[i].offset = (maxTime - ((manualTiming[i].onTime + manualTiming[i].offTime) * manualTiming[i].repeats)) / 2;
        break;
      case 1:
        // Centered
        manualTiming[i].repeats = 1;
        manualTiming[i].onTime = manualTiming[i].runTime;
        manualTiming[i].offTime = maxTime - manualTiming[i].runTime;
        manualTiming[i].offset = 0;
        break;
      default:
        break;
      }
      manualTiming[i].inState = manualTiming[i].offTime;
      if (!manualTiming[i].offset){
        manualTiming[i].offset = manualTiming[i].offTime / 2;
      }

      //if (i < 6){
      if (i < 8){
        // Low-Ports
        EscColor(fgBlue);
      }
      else{
        // High-Ports
        EscColor(fgYellow);
      }
      EscLocate(7, pos++);
      EscBold(1);
      PrintCentered(Fa(ezoStrLongType[type]), 11);
      EscColor(0);
      PrintSpacer(0);
      PrintSerTime(manualTiming[i].offset, 0, 1);
      PrintSpacer(0);
      PrintSerTime(manualTiming[i].onTime, 0, 1);
      PrintSpacer(0);
      PrintSerTime(manualTiming[i].offTime, 0, 1);
      PrintSpacer(1);
      if (!firstLine){
        // 1st Line
        EscSaveCursor();
        firstLine = 1;
      }
      typeExist = 1;
      
    }

    byte j = CorrectForRepeat(i);
    if (j != i){
      i = j;
      goto DoLowHigh;
    }
    i = CorrectFromRepeat(i);

    
    if ((typeExist && type != ezoEC) || (typeExist && i == 3)){
      // Print type-separator line
      EscBold(0);
      PrintLine(pos++, 7, 68);
    }
    
  }
  firstLine = 0;

  // Run times...
  while (maxTime){
    if (DoTimer()){
      // A second is gone...

      byte needRefresh = 0;

      maxTime--;

      PrintSerTime(maxTime, 0, 0); // Time left to strHLP2
      strcpy(&strHLP2[8], (char*)" left...");
      PrintErrorOK(0, strlen(strHLP2), strHLP2);

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
                needRefresh = 1;
              }
            }
            else{
              // port is OFF
              if (manualTiming[i].inState == manualTiming[i].offTime){
                // Start ON
                manualTiming[i].inState = 0;
                manualTiming[i].state = 1;
                manualTiming[i].repeats--;
                needRefresh = 1;
              }              
            }
            digitalWrite(i + 2, manualTiming[i].state);
            manualTiming[i].inState++;        
          }
        }
      }

      if (needRefresh){
        //for (byte i = 0; i < 6; i++){
        for (byte i = 0; i < 8; i++){

          byte typeExist = 0;
          //byte type = i;

          // Correct type for the three times EC
          byte type = CorrectType(i);

          DoLowHigh2:
          // Low-Ports
          if ((i == port || port == 255) && manualTiming[i].runTime){
            // we're in Action and have a time...

            typeExist = 1;

            EscRestoreCursor();
            if (firstLine){
              EscCursorDown(firstLine);
            }
            
            firstLine++;

            if (i < 8){
              // Low-Ports
              EscColor(fgBlue);
            }
            else{
              // High-Ports
              EscColor(fgYellow);
            }
            PrintBoldInt(manualTiming[i].repeats, 8, '0');
            EscColor(0);
            PrintSpacer(manualTiming[i].state);
            if (manualTiming[i].state){
              // ON
              EscColor(my.KeyColor);
              PrintCentered((char*)"ON", 8);
              EscColor(0);
            }
            else{
              // OFF
              PrintCentered((char*)"OFF", 8);
            }
            PrintSpacer(0);
          }

          byte j = CorrectForRepeat(i);
          if (j != i){
            i = j;
            goto DoLowHigh2;
          }
          i = CorrectFromRepeat(i);

          if ((typeExist && type != ezoEC) || (typeExist && i == 3)){
            // separator line
            firstLine++;
          }
                   
        }
        firstLine = 0;
      }
      
      PrintLoopTimes();    

    }
    if (Serial.available()){
      // STOP manual action...
      Serial.read();
      maxTime = 0;
    }
  }
  OffOutPorts();

}
void PrintManualMenuHlp1(char key, uint16_t value){
  PrintMenuKeySmallBoldFaint(key, 0, !value);
  PrintSerTime(value, 0, 1);
  PrintMenuKey(key - 32, 1, '(', 0, 0, !value, !value);
  PrintSpacer(1);
}
void PrintManualMenu(){

  //byte selectedSet = 0;

Start:

  int8_t pos = PrintMenuTop((char*)"- Manual Menu -") + 1;
  byte i = 0;
  
  EscLocate(5, pos++);
  PrintMenuKey(i + 'o', 0, '(', ' ', 0, 1, 0);
  EscColor(fgBlue);
  PrintCentered(manual.Name, 16);
  EscColor(0);
  PrintFlexSpacer(0, 6);
  Serial.print(F("LOW"));
  PrintFlexSpacer(6, 5);
  Serial.print(F("HIGH"));
  PrintFlexSpacer(6, 0);

  PrintLine(pos++, 5, 58);
  byte ecCnt = 0;

  for (i = 0; i < 6; i++){

    byte iOffset = 0;
    RedoEC:
    EscLocate(8, pos++);
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[i]), 17);
    PrintSpacer(1);
    
    //PrintSmallMenuKey('a' + i);
    //PrintMenuKeySmallBoldFaint(i + 'a' + ecCnt, 0, !manual.Low[i + ecCnt]);
    //PrintSerTime(manual.Low[i + ecCnt], 0, 1);
    //PrintMenuKey(i + ecCnt + 'A', 1, '(', 0, 0, !manual.Low[i + ecCnt], !manual.Low[i + ecCnt]);
    //PrintSpacer(1);
    PrintManualMenuHlp1('a' + i + ecCnt, manual.Low[i + ecCnt]);

    if (i == ezoEC && ecCnt && ecCnt < 3){
      // EC - three times...
      // 2nd & 3rd EC having no high-action ports
      ecCnt++;
      PrintFlexSpacer(15,0);
      if (ecCnt < 3){
        goto RedoEC;
      }    
      ecCnt--;
    }
    else{
      //PrintSmallMenuKey('g' + i);
      iOffset = i;
      if (i == ezoORP || i == ezoDiO2){
        // They do not have high-ports
        PrintFlexSpacer(15,0);
      }
      else{
        if (i == ezoLVL){
          iOffset = 3;
        }
        
        PrintMenuKeySmallBoldFaint(iOffset + 'i', 0, !manual.High[iOffset]);
        PrintSerTime(manual.High[iOffset], 0, 1);
        PrintMenuKey(iOffset + 'I', 1, '(', 0, 0, !manual.High[iOffset], !manual.High[iOffset]);
        PrintSpacer(0);
        //PrintManualMenuHlp1(iOffset + 'i', manual.High[iOffset], 0);
      }
            
      //EscLocate(8, pos++);

      if (i == ezoEC){
        // EC - three times...
        ecCnt++;
        goto RedoEC;
      }
    }

  }
  //pos--;
  PrintLine(pos++, 5, 58);
  pos++;
  EscLocate(5, pos++);
  EscKeyStyle(1);
  Serial.print(F("a-l):"));
  EscKeyStyle(0);
  Serial.print(F(" Edit   "));
  //PrintSpaces(3);
  EscKeyStyle(1);
  Serial.print(F("A-L):"));
  EscKeyStyle(0);
  Serial.print(F(" RunSingle   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('m');
  Serial.print(F("RunALL   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('n');
  Serial.print(F("RunAllCent."));
    
  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('o');
  Serial.print(F("EditName   "));
  //PrintSpaces(3);
  EscKeyStyle(1);
  Serial.print(F("1-4):"));
  EscKeyStyle(0);
  Serial.print(F(" SelectSet = "));
  EscColor(fgBlue);
  EscBold(1);
  Serial.print(my.Temporary + 1);
  //EscBold(0);
  EscColor(0);
  PrintSpaces(3);
  EscKeyStyle(1);
  Serial.print(F("5-8):"));
  EscKeyStyle(0);
  Serial.print(F(" CopyToSet [1-4]"));
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('o', 8);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (IsKeyBetween(pos, 'a', 'h')){
    // LowTime
    pos -= 'a';
    manual.Low[pos] = GetUserTime(manual.Low[pos]);
    pos = 1;
  }
  else if (IsKeyBetween(pos, 'i', 'l')){
    // HighTime
    pos -= 'i';
    manual.High[pos] = GetUserTime(manual.High[pos]);
    pos = 1;
  }
  else if (IsKeyBetween(pos, 'A', 'H')){
    // Run Single LowTime
    pos -= 'A';
    RunManualSetting(pos, 1);
    pos = 2;
  }
  else if (IsKeyBetween(pos, 'I', 'L')){
    // Run Single HighTime
    pos = pos - 'I' + 6;
    RunManualSetting(pos, 1);
    pos = 2;
  }
  else if (pos == 'm'){
    // Run Distributed
    RunManualSetting(255, 0);
    pos = 2;
  }
  else if (pos == 'n'){
    // Run Centered
    RunManualSetting(255, 1);
    pos = 2;
  }
  else if (IsKeyBetween(pos, '1', '4')){
    // Load Set
    my.Temporary = pos - '1';
    ManualTimesFromRom(my.Temporary);
    myToRom();
  }
  else if (IsKeyBetween(pos, '5', '8')){
    // Copy Set
    ManualTimesToRom(pos - '5');
    pos = 2;
  }
  else if (pos == 'o'){
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

void PrintTimingsMenuTime(char key, uint16_t timeIN, byte printSpacer){
  PrintMenuKeySmallBoldFaint(key, 0, !timeIN);
  PrintSerTime(timeIN, 0, 1);
  if (printSpacer){
    PrintSmallSpacer();
  }
}
byte PrintTimingsMenuChangeVal(uint16_t *valIN){
  // THIS IS STRANGE
  //    Using this just two times has a lower flash use than
  //    if it's used more or less times as replacement for:
  //        pos -= 'a';
  //        setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
  //        pos = 1;
  // THIS IS STRANGE
  *valIN = GetUserTime(*valIN);
  return 1;
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
  for (i = 0; i < 8; i++){

    //byte type = i;
    // Correct type for the three times EC
    byte type = CorrectType(i);
    
    EscBold(1);
    PrintCentered(Fa(ezoStrLongType[type]), 9);
    PrintSmallSpacer();
    
    //PrintSmallMenuKey('a' + i);
    if (type == ezoEC && i > ezoEC){
      // 2nd and 3rd EC
      PrintSpaces(11);
    }
    else{
      PrintTimingsMenuTime(type + 'a', setting.DelayTime[type], 0);
    }
    PrintSmallSpacer();
    
    PrintTimingsMenuTime(i + 'g', setting.TimeTooLow[i], 1);
    PrintTimingsMenuTime(i + 'o', setting.TimeLow[i], 1);
    
    // 0 & 1 as they are
    // 2 & 3 NOT
    // 4 = 2
    // 5 & 6 NOT
    // 7 = 3
    type = i;     // to set TimeHigh & TimeTooHigh index right
    switch (i){
    case 2:
    case 3:
      // 2nd & 3rd EC
    case 5:
    case 6:
      // Redox / O2
      type = 0;
      break;
    case 4:
      type = 2;
      break;
    case 7:
      type = 3;
    default:
      break;
    }    
    if (type || (!type && !i)){
      // Has High Times
      PrintTimingsMenuTime(type + 'w', setting.TimeHigh[type], 1);
      PrintTimingsMenuTime(type + 'A', setting.TimeTooHigh[type], 0);
    }
    else{
      // No HighTimes
      PrintFlexSpacer(11, 10);
    }
    
    PrintSmallSpacer();
    EscLocate(3, pos++);
  }

  pos = PrintCopySettingTo(pos);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('z', 3);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (IsKeyBetween(pos, 'a', 'f')){
  //else if (pos >= 'a' && pos <= 'f'){
    // FailSave
    pos = PrintTimingsMenuChangeVal(&setting.DelayTime[pos - 'a']);
    //pos -= 'a';
    //setting.DelayTime[pos] = GetUserTime(setting.DelayTime[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'g', 'n')){
    // tooLow
    pos = PrintTimingsMenuChangeVal(&setting.TimeTooLow[pos - 'g']);
    //pos -= 'g';
    //setting.TimeTooLow[pos] = GetUserTime(setting.TimeTooLow[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'o', 'v')){
    // Low
    pos = PrintTimingsMenuChangeVal(&setting.TimeLow[pos - 'o']);
    //pos -= 'o';
    //setting.TimeLow[pos] = GetUserTime(setting.TimeLow[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'w', 'z')){
    // High
    pos = PrintTimingsMenuChangeVal(&setting.TimeHigh[pos - 'w']);
    //pos -= 'w';
    //setting.TimeHigh[pos] = GetUserTime(setting.TimeHigh[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, 'A', 'D')){
    // tooHigh
    pos = PrintTimingsMenuChangeVal(&setting.TimeTooHigh[pos - 'A']);
    //pos -= 'A';
    //setting.TimeTooHigh[pos] = GetUserTime(setting.TimeTooHigh[pos]);
    //pos = 1;
  }
  else if (IsKeyBetween(pos, '1', '3')){
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
    if (ezotype == ezoORP){
      // Value of probe (eH/mV) is just the Redox-Potential
      // The Redox-Value (redox-capability) need the pH...
      // rH = eH /28.9 + (2 * pH)
      avgVal[ezoORP] = (avgVal[ezoORP] * 10) / (int32_t)289 + (2 * avgVal[ezoPH]);
    }    
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

void PrintPortStates(){

  // 8 low-ports / 4 high ports
  byte posOfPort[] = {8, 17, 22, 26, 34, 47, 60, 73, 10, 19, 36, 75};

  static byte firstRun = 0;
  static byte lastVal[12];
  byte isChanged = 0;

  // Check on Port_Change
  for (byte i = 0; i < 12; i++){
    byte val = digitalRead(i + 2);
    if (val != lastVal[i]){
      lastVal[i] = val;
      isChanged = 1;
    }
  }
  
  if (isChanged || !firstRun){
    for (byte i = 0; i < 8; i++){
      // Low-Ports
      EscLocate(posOfPort[i], myLastLine);
      if (lastVal[i]){
        EscBoldColor(fgBlue);
        Serial.print(F(">"));
        EscFaint(1);
      }
      else{
        EscFaint(1);
        Serial.print(F(">"));
        EscBoldColor(fgGreen);
      }
      Serial.print(F("~"));
    }
    for (byte i = 8; i < 12; i++){
      // High-Ports
      EscLocate(posOfPort[i], myLastLine);
      if (lastVal[i]){
        EscBoldColor(fgYellow);
        Serial.print(F("<"));        
        EscCursorLeft(2);
        EscFaint(1);
        Serial.print(F("~"));
      }
      else{
        EscFaint(1);
        Serial.print(F("<"));        
      }
    }
    firstRun = 1;
    EscBoldColor(0);
  }
}

void PrintAVGsHLP(byte type, byte posX, byte posY, byte preDot, byte printUnit){
  SetAvgColorEZO(type);
  EscLocate(posX, posY);
  PrintBoldFloat(avgVal[type], preDot, 2, ' ');
  if (printUnit){
    PrintUnit(type, 0, 0, 3);
  }
}
byte PrintAVGs(byte pos){

  PrintAVGsHLP(ezoRTD, 7, pos, 2, 1);  

  SetAvgColorEZO(ezoEC);
  EscLocate(20, pos);
  PrintBoldInt(avg_EC / 1000, 4, ' ');
  PrintUnit(ezoEC, 0, 0, 3);
  
  PrintAVGsHLP(ezoPH, 32, pos, 2, 1);  
  
  PrintAVGsHLP(ezoORP, 44, pos, 4, 0);
  EscColor(0);
  Serial.print(F("rH"));

  PrintAVGsHLP(ezoDiO2, 58, pos, 3, 1);

  PrintAVGsHLP(ezoLVL, 72, pos, 3, 1);

  return pos + 1;

}

void PrintLoopMenu(){

  EscCls();
  EscInverse(1);
  byte pos = PrintQuickWater();
  EscInverse(0);
  pos++;

  EscLocate(3, pos++);
  EscBold(1);
  Serial.print(F("|   Temp.    |    EC     |     pH     |    Redox    |    O2     |   Level  |"));

  pos = PrintLine(pos, 3, 76);
    
  EscBold(0);

  PrintErrorOK(0, 0, (char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLine(pos, 3, 76);
  //PrintLine(pos + 1, 6, 70);

  // Avg 
  pos = PrintAVGs(pos);

  //EscBold(1);
  pos = PrintLine(pos, 3, 76);
  
  EscBold(1);
  //pos = PrintLine(pos + 3, 3, 76);

  // we need this pos in loop()
  myLastLine = pos;

  PrintLine(pos + 1, 3, 76);

}

void PrintMainMenu(){

Start:

  int pos = PrintQuickWater();
  
  uint32_t hlpTime = 0;

  pos = PrintProbesOfType(255, 1, pos, 0);

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("ReScan   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('b'); Serial.print(F("ReBoot   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('c'); Serial.print(F("Date   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('d'); Serial.print(F("Time   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('e'); Serial.print(F("Addr. = "));
  PrintBoldInt(my.Address, 3, '0');

  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('f'); Serial.print(F("Speed = "));
  EscBold(1);
  Serial.print(my.Speed);
  EscBold(0);
  PrintSpaces(3);  
  PrintMenuKeyStd('g'); Serial.print(F("Dim"));
  EscFaint(1);
  Serial.print(F("Color   "));
  EscFaint(0);
  //PrintSpaces(3);
  PrintMenuKeyStd('h'); Serial.print(F("KeyColor   "));
  //PrintSpaces(3);
  PrintMenuKeyStdBoldFaint('i', (my.Default), (!my.Default)); Serial.print(F("AsDefault"));

  pos = PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('j');
  Serial.print(F("More...   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('k'); Serial.print(F("Values...   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('l'); Serial.print(F("Times...   "));
  //PrintSpaces(3);
  PrintMenuKeyStd('m'); Serial.print(F("Manual..."));

  PrintShortLine(pos++, 8);

  EscLocate(5, pos);
  PrintMenuKeyStd('n'); Serial.print(F("Setting-Name = "));
  EscBoldColor(fgBlue);
  Serial.print((char*)setting.Name);
  PrintSpaces(3);
  EscKeyStyle(1);
  Serial.print(F("o-q):"));
  EscKeyStyle(0);
  Serial.print(F(" Sel.Setting [1-3] = "));
  EscBoldColor(fgBlue);
  Serial.print(my.Setting + 1);
  EscBoldColor(0);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('q', ezoCnt - INTERNAL_LEVEL_CNT);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // EXIT
    break;
  case 'j':
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
  case 'i':
    // Use and save actual Scan as Default
    my.Default = !my.Default;
    myToRom();
    if (my.Default){
      DefaultProbesToRom();
    }  
    break;
  case 'k':
    // Values
    PrintValuesMenu();
    break;
  case 'l':
    // Values
    PrintTimingsMenu();
    break;
  case 'm':
    PrintManualMenu();
    break;
  case 'n':
    // Setting Name
    GetUserString(setting.Name);
    SettingsToRom(my.Setting);
    break;
  case 'o':
  case 'p':
  case 'q':
    // Select Setting
    my.Setting = pos - 'o';
    SettingsFromRom(my.Setting);
    myToRom();
    break;
  case 'g':
    // Solarized
    my.Solarized = EscGetNextColor(my.Solarized);
    fgFaint = my.Solarized;
    myToRom();
    break;
  case 'h':
    // KeyColor
    my.KeyColor = EscGetNextColor(my.KeyColor);
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