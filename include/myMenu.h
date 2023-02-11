#include "ezoIIC.h"

char myBoot = 0;    // 0 = Terminal  /  1 = Slave

int GetUserString(char *strIN){
  
  char c = 0;
  char timeOut = 60;
  int eos = 0;       // Pos of EndOfString

  EscBold(1);
  Serial.print(F(">> "));
  EscBold(0);
  EscColor(fgCyan);

  eos = strlen(strIN);
  if (eos){
    Serial.print(strIN);
    strcpy(strHLP, strIN);
  }
  
  strHLP[eos] = 0;

  while (c != 13){

    if (DoTimer()){
      timeOut--;
      if (!timeOut){
        strHLP[0] = 0;
        EscColor(0);
        return 0;
      }
    }

    if (Serial.available()){
      
      timeOut = 60;

      c = Serial.read();
      if (eos < IIC_HLP_LEN - 1){
        eos++;
      }
      else{
        EscCursorLeft(1);
      }

      switch (c){
      case 8:
      case 27:
      case 127:
        // DEL and Backspace
        if (eos > 1){
          eos -= 2;
          EscCursorLeft(1);
          Serial.print(F(" "));
          EscCursorLeft(1);
        }        
        break;
      case 10:
      case 13:
        break;
      default:
        // Print and save char
        Serial.print(c);
        strHLP[eos - 1] = c;
        break;
      }
    }
  }

  strHLP[eos - 1] = 0;
  EscColor(0);
  return 1;

}

long GetUserVal(long defVal, int type){
  // type:  0 = int as it is
  //        1 = float (*1000)
  if (type){
    // Is scaled float
    EzoIntToStr(defVal, 1, 3, ' ');
    strcpy(strHLP2, strHLP);
  }
  else{
    /* code */
    ltoa(defVal, strHLP2, 10);
  }
  if (GetUserString(strHLP2)){
    if (type){
      // Is scaled float
      strcpy(strHLP2, strHLP);
      defVal = StrToInt(strHLP2, 0);
    }
    else{
      /* code */
      defVal = atol(strHLP);
    }
  }
  return defVal;  
}

char GetUserKey(int maxChar, int ezoType){

  // ezoType = -1     No Numbers
  // ezoType = 0     1-9
  // ezoType = >0    Just probe-types

  int timeOut = 60;
  char charIN = 0;
  int r = -1;         // TimeOut

  while (timeOut){

    if (DoTimer()){
      // A Second is over...
      timeOut--;
    }
    if (Serial.available()){
      charIN = Serial.read();
      timeOut = 0;
      r = charIN;
      if (charIN > 48 && charIN - 48 <= ezoCnt){
        // Probe selected
        if (ezoType > 0){
          // Only probes of this type are allowed
          if (!(ezoProbe[charIN - 49].type == ezoType)){
            r = -1;
          }
        }
        else if (ezoType == 0){
          // OK
        }
        else{
          // no probes
          r = -1;
        }
      }
      else if (charIN > 96 && charIN < maxChar + 1){
        // a-(z) selected
      }
      else if (charIN == 13){
        // Enter - Exit - Back
        r = 0;
      }
      else{
        // Refresh
        r = -1;
      }
      if (r < 0){
        timeOut = 60;
      }
    } 
  }

  return r;

}

int PrintLine(int pos){
  EscLocate(5, pos++);
  Serial.print(F("---"));
  return pos;
}

int PrintLongLine(int pos){
  EscLocate(5, pos++);
  Serial.print(F(" ----------------------------------------------------------------------"));
  return pos;
}

int PrintBoldValue(long val, int lz, int dp, char lc){
  EscBold(1);
  int r = EzoIntToStr(val, lz, dp, lc);
  Serial.print(strHLP);
  EscBold(0);
  return r;
}

int PrintMenuTop(char *strIN){
  EscCls();
  EscLocate(1, 1);
  EscBold(1);
  Serial.print((char*)strIN);
  EscBold(0);
  return 2;
}

void PrintMenuEnd(int pos){
  Serial.println(F("\n"));
  Serial.print(F("    Select key, or Enter(for return)..."));
}

int PrintAllMenuOpt2(int address1st, int pos){

  EscLocate(5, pos++);

  EscFaint(1);
  Serial.print(F("(Auto)Name: "));
  EscFaint(0);
  Serial.print((char*)strDefault);
  EscFaint(1);
  Serial.print(F("   (1st)Address: "));
  EscFaint(0);
  Serial.print(address1st);

  return pos;

}

int PrintAllMenuOpt1(int pos){

  EscLocate(5, pos++);
  Serial.print(F("A): 'Factory' Reset"));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("B): Delete Name(s)"));
  EscLocate(5, pos++);
  Serial.print(F("C): Edit (Auto-)Name..."));
  EscLocate(5, pos++);
  Serial.print(F("D): Set (Auto-)Name(s)"));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("E): Edit (1st) Address..."));
  EscLocate(5, pos++);
  Serial.print(F("F): Set Address(es)"));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("G): Clear Calibration(s)"));
  
  return pos;

}

void PrintAllMenu(){

  static char ImInside = 0;
  static long address1st = 33;

  if (!ImInside){
    strcpy(strDefault, "-DummyName-");
    address1st = 33;
  }
  ImInside = 1;

Start:

  int pos = PrintMenuTop((char*)"                    - ALL Menu -");
  pos = PrintAllMenuOpt1(pos + 1);
  pos = PrintAllMenuOpt2(address1st, pos + 1);
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('g', -1);

  switch (pos){
  case 'a':
    // Factory Reset for All
    EzoReset(0,2);
    break;
  case 'b':
    // Delete all names
    EzoSetName((char*)"", 0, 2, 0);
    break;
  case 'c':
    // Edit Auto Name
    if (GetUserString(strDefault)){
      strcpy(strDefault, strHLP);
    }
    break;
  case 'd':
    // Set AutoNames
    EzoSetName(strHLP2,0,2,1);
    break;
  case 'e':
    // Edit 1st Address
    address1st = GetUserVal(address1st, 0);
    if (address1st > 127 - ezoCnt){
      address1st = 127 - ezoCnt;
    }
    else if (address1st < 32){
      address1st = 32;
    }
    break;
  case 'f':
    // Set Addresses
    EzoSetAddress(0, (int)address1st, 2);
    // wait 4 reboots done
    delay(1000);
    // Scan new
    EzoScan();
    // direct back to main
    pos = 0;
    break;
  case 'g':
    // Clear calibration
    EzoSetCal((char*)"Cal,clear", 0, 2);
    break;
  default:
    break;
  }

  if (pos > 0){
    goto Start;
  }
  
  ImInside = 0;

}

void PrintProbeLine(int ezo, int pos){
    EscLocate(5, pos);
    Serial.print(ezo + 1);
    Serial.print(F(".:"));
    EscLocate(9, pos);
    strcpy_P(strHLP,(PGM_P)pgm_read_word(&(ezoStrType[ezoProbe[ezo].type])));
    Serial.print(strHLP);
    EscLocate(15, pos);
    EzoIntToStr(ezoProbe[ezo].version, 2, 2, '0');
    Serial.print(strHLP);
    EscLocate(22, pos);
    Serial.print(ezoProbe[ezo].address);
    EscLocate(27, pos);
    Serial.print((char*)ezoProbe[ezo].name);
    EscLocate(45, pos);
    Serial.print(ezoProbe[ezo].calibrated);
    EscLocate(47, pos);
    EzoIntToStr(ezoProbe[ezo].value[0], 5,2, ' ');
    Serial.print(strHLP);
}

void PrintCal_Faint(){
    EscFaint(1);
    Serial.print(F("NA:"));
}
void PrintCal_Value(long val, int faint){
  EscFaint(0);
  if (faint){
    Serial.print(F("NA"));
  }
  else{
    PrintBoldValue(val, 4, 2, ' ');
  }
}
int PrintCal_B(int faint, int pos){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("B):"));
  }
  Serial.print(F(" Do 1-Point Cal..."));
  EscFaint(0);
  
  return pos;

}
int PrintCal_C(int faint, int pos){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("C):"));
  }
  Serial.print(F(" Do 2-Point Cal..."));
  EscFaint(0);

  return pos;
  
}
int PrintCal_D(int faint, int pos){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("D):"));
  }
  Serial.print(F(" Do 3-Point Cal..."));
  EscFaint(0);
  pos = PrintLine(pos);
  
  return pos;
}
int PrintCal_E(int faint, int pos, long val){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("E):"));
  }
  Serial.print(F(" Set Single/Mid Point    = "));
  PrintCal_Value(val, faint);  

  return pos;
  
}
int PrintCal_F(int faint, int pos, long val){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("F):"));
  }
  Serial.print(F(" Avg As Single/Mid Point = "));
  PrintCal_Value(val, faint);  

  pos = PrintLine(pos);
  
  return pos;

}
int PrintCal_G(int faint, int pos, long val){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("G):"));
  }
  Serial.print(F(" Set Low Point           = "));
  PrintCal_Value(val, faint);  
  
  return pos;

}
int PrintCal_H(int faint, int pos, long val){

  EscLocate(5, pos++);
  if (faint){
    PrintCal_Faint();
  }
  else{
    Serial.print(F("H):"));
  }
  Serial.print(F(" Set High Point          = "));
  PrintCal_Value(val, faint);  
  
  return pos;

}

void PrintCalMenu(int ezo, int all){

  static int ImInside = 0;

  long calLow = 0;      // Value for LowPoint
  long calMid = 0;      // Value for MidPoint
  long calHigh = 0;     // Value for HighPoint
  long calRes = 0;      // Value for Reset
  
  union myMenu
  {
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

  switch (ezoProbe[ezo].type){
  case ezoRTD:
    /* code */
    strcpy(iicStr, "          - Calibrate RTD (°C) -");
    if (!ImInside){
      calLow = CAL_RTD_LOW;
      calMid = CAL_RTD_MID;
      calHigh = CAL_RTD_HIGH;
      calRes = CAL_RTD_RES;
    }
    break;
  case ezoEC:
    /* code */
    strcpy(iicStr, "          - Calibrate EC  (µS) -");
    if (!ImInside){
      calLow = CAL_EC_LOW;
      calMid = CAL_EC_MID;
      calHigh = CAL_EC_HIGH;
      calRes = CAL_EC_RES;
    }
    break;
  case ezoPH:
    /* code */
    strcpy(iicStr, "          - Calibrate pH  (pH) -");
    if (!ImInside){
      calLow = CAL_PH_LOW;
      calMid = CAL_PH_MID;
      calHigh = CAL_PH_HIGH;
      calRes = CAL_PH_RES;
    }
    break;
  case ezoORP:
    /* code */
    strcpy(iicStr, "          - Calibrate ORP (mV) -");
    if (!ImInside){
      calLow = CAL_ORP_LOW;
      calMid = CAL_ORP_MID;
      calHigh = CAL_ORP_HIGH;
      calRes = CAL_ORP_RES;
    }
    break;
  case ezoDiO2:
    /* code */
    strcpy(iicStr, "          - Calibrate O2  (r%) -");
    if (!ImInside){
      calLow = CAL_DiO2_LOW;
      calMid = CAL_DiO2_MID;
      calHigh = CAL_DiO2_HIGH;
      calRes = CAL_DiO2_RES;
    }
    break;  
  default:
    break;
  }
  int pos = PrintMenuTop(iicStr);
  ImInside = 1;

  for (int i = 0; i < ezoCnt; i++){  
    if (ezoProbe[i].type == ezoProbe[ezo].type) {
      // Right Probe Type
      if (i == ezo || all){
        // Selected
        EscFaint(0);
      }
      else{
        // Not selected
        EscFaint(1);
      } 
      pos++;
      PrintProbeLine(i, pos);
    }    
  }
  EscFaint(0);

  pos = PrintLine(pos + 1);

  EscLocate(5, pos++);
  Serial.print(F("A): Clear Calibration"));
  pos = PrintLine(pos);

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

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('h', ezoProbe[ezo].type);
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

void PrintProbeMenu(int ezo){


  static char ImInside = 0;
  static char address1st = 33;
  static char all = 0;

  if (!ImInside){
    strcpy(strDefault, "-DummyName-");
    address1st = 33;
    all = 0;
  }
  ImInside = 1;

Start:

  int pos = PrintMenuTop((char*)"               - Probe(Type) Menu -");

  for (int i = 0; i < ezoCnt; i++){  
    if (ezoProbe[i].type == ezoProbe[ezo].type) {
      // Right Probe Type
      if (i == ezo || all){
        // Selected
        EscFaint(0);
      }
      else{
        // Not selected
        EscFaint(1);
      } 
      pos++;
      PrintProbeLine(i, pos);
    }    
  }
  EscFaint(0);

  pos = PrintLine(pos + 1);

  pos = PrintAllMenuOpt1(pos);
  EscLocate(5, pos++);
  Serial.print(F("H): Calibration(s)..."));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("I): Select Single"));
  EscLocate(5, pos++);
  Serial.print(F("J): Select ALL"));

  pos = PrintAllMenuOpt2(address1st, pos + 1);
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('j', ezoProbe[ezo].type);
  switch (pos){
  case 'a':
    break;
  case 'b':
    break;
  case 'c':
    break;
  case 'd':
    break;
  case 'e':
    break;
  case 'f':
    break;
  case 'g':
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
  
  ImInside = 0;

}

void PrintValuesMenu(){

Start:

  int pos = PrintMenuTop((char*)"              - Set FailSafe Values -") + 1;

  EscLocate(5, pos++);
  Serial.print(F("A): Humidity %rH = "));
  PrintBoldValue(failSave_HUM, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("B): Air-Temp °C  = "));
  PrintBoldValue(failSave_TMP, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("C):  Air-CO  ppm = "));
  PrintBoldValue(failSave_CO2, 4, 2, ' ');
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("D):   RTD    °C  = "));
  PrintBoldValue(failSave_RTD, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("E):   EC     µS  = "));
  PrintBoldValue(failSave_EC, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("F):   pH     pH  = "));
  PrintBoldValue(failSave_pH, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("G):   ORP    mV  = "));
  PrintBoldValue(failSave_ORP, 4, 2, ' ');
  EscLocate(5, pos++);
  Serial.print(F("H):  H2O-O2  r%  = "));
  PrintBoldValue(failSave_O2, 4, 2, ' ');

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('h', -1);

  switch (pos){
  case 0:
    // Exit
  case -1:
    // Timeout
    break;
  case 'a':
    // Humidity
    break;
  case 'b':
    // Air-Temp
    break;
  case 'c':
    // CO2
    break;
  case 'd':
    // RTD
    break;
  case 'e':
    // EC
    break;
  case 'f':
    // pH
    break;
  case 'g':
    // ORP
    break;
  case 'h':
    // H2=-O2"
    break;
  default:
    break;
  }

  if (pos > 0){
    goto Start;
  }
  
}

int PrintWaterVals(int pos){

  int pos1st = 0;
  int posMax = 0;
  int posAct = 0;

  long avg = 0;

  pos1st = pos;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoRTD){
      posAct++;
      EscLocate(7, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],2,2,' ');
      avg += ezoProbe[i].value[0];
      EscFaint(1);
      Serial.print(F("°C"));
      EscFaint(0);
    }
  }
  posMax = posAct;
  if (posAct){
    avg_RTD = avg / (long)posAct;
  }
  

  pos = pos1st;
  posAct = 0;
  avg = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoEC){
      posAct++;
      EscLocate(24, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],4,0,' ');
      avg += ezoProbe[i].value[0];
      EscFaint(1);
      Serial.print(F("µS"));
      EscFaint(0);
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }
  if (posAct){
    avg_EC = avg / (long)posAct;
  }

  pos = pos1st;
  posAct = 0;
  avg = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoPH){
      posAct++;
      EscLocate(37, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],2,2,' ');
      avg += ezoProbe[i].value[0];
      EscFaint(1);
      Serial.print(F("pH"));
      EscFaint(0);
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }
  if (posAct){
    avg_pH = avg / (long)posAct;
  }

  pos = pos1st;
  posAct = 0;
  avg = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoORP){
      posAct++;
      EscLocate(48, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],4,2,' ');
      avg += ezoProbe[i].value[0];
      EscFaint(1);
      Serial.print(F("mV"));
      EscFaint(0);
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }
  if (posAct){
    avg_ORP = avg / (long)posAct;
  }

  pos = pos1st;
  posAct = 0;
  avg = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoDiO2){
      posAct++;
      EscLocate(63, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],3,2,' ');
      avg += ezoProbe[i].value[0];
      EscFaint(1);
      Serial.print(F("r%"));
      EscFaint(0);
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }
  if (posAct){
    avg_O2 = avg / (long)posAct;
  }

  return pos1st + posMax;

}

int PrintAVGs(int pos){
  
  SetAvgColor(avg_RTD, tooLow_RTD, low_RTD, high_RTD, tooHigh_RTD);
  EscLocate(10, pos);
  PrintBoldValue(avg_RTD,2,2,' ');
  EscColor(0);
  Serial.print(F("°C"));

  SetAvgColor(avg_EC, tooLow_EC, low_EC, high_EC, tooHigh_EC);
  EscLocate(27, pos);
  PrintBoldValue(avg_EC,4,0,' ');
  EscColor(0);
  Serial.print(F("µS"));

  SetAvgColor(avg_pH, tooLow_pH, low_pH, high_pH, tooHigh_pH);
  EscLocate(40, pos);
  PrintBoldValue(avg_pH,2,2,' ');
  EscColor(0);
  Serial.print(F("pH"));

  SetAvgColor(avg_ORP, tooLow_ORP, low_ORP, high_ORP, tooHigh_ORP);
  EscLocate(51, pos);
  PrintBoldValue(avg_ORP,4,2,' ');
  EscColor(0);
  Serial.print(F("mV"));

  SetAvgColor(avg_O2, tooLow_O2, low_O2, high_O2, tooHigh_O2);
  EscLocate(66, pos++);
  PrintBoldValue(avg_O2,3,2,' ');
  EscColor(0);
  Serial.print(F("r%"));

  return pos;

}

void PrintLoopMenu(){

  EscCls();
  EscCursorVisible(0);
  EscInverse(1);
  int pos = PrintMenuTop((char*)"                                - QuickWater 1.00 -                             ");
  EscInverse(0);
  pos++;

  EscLocate(5, pos++);
  EscBold(1);
  Serial.print(F(" | Temperature | Conductivity |     pH     |    Redox    |     O2     |"));
  pos = PrintLongLine(pos);
  EscBold(0);

  PrintErrorOK(0,-1,(char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLongLine(pos);

  // Avg 
  pos = PrintAVGs(pos);

  EscBold(1);
  pos = PrintLongLine(pos);
  EscBold(0);

}

void PrintMainMenu(){

Start:

  int pos = PrintMenuTop((char*)"          - Main Menu QuickWater 1.00 -");
  
  for (int i = 0; i < ezoCnt; i++){
    pos++;
    PrintProbeLine(i, pos);
  }

  pos = PrintLine(pos + 1);

  EscLocate(5, pos++);
  Serial.print(F("A):  Select All..."));
  EscLocate(5, pos++);
  Serial.print(F("B):  ReBoot"));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("C):  Save State As Default"));
  EscLocate(5, pos++);
  Serial.print(F("D):  Erase Default State"));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("E):  Set FailSafe Values..."));
  pos = PrintLine(pos);
  EscLocate(5, pos++);
  Serial.print(F("F):  Boot for Terminal = "));
  if (myBoot){
    EscFaint(1);
    Serial.print(F("False"));
  }
  else{
    EscBold(1);
    Serial.print(F("True"));
  }
  EscBold(0);
  EscLocate(5, pos++);
  Serial.print(F("G):  Boot as Slave     = "));
  if (myBoot){
    EscBold(1);
    Serial.print(F("True"));
  }
  else{
    EscFaint(1);
    Serial.print(F("False"));
  }
  EscBold(0);
  EscLocate(5, pos++);
  Serial.print(F("H):  Set Slave Address = "));
  PrintBoldValue((long)myAddress * 1000, 3, 0, '0');
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('h', 0);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // EXIT
    break;
  case 'a':
    // All Menu
    PrintAllMenu();
    break;
  case 'b':
    // ReBoot
    EscCls();
    EscLocate(1,1);
    EzoScan();
    break;
  case 'c':
    // Save as default
    break;
  case 'd':
    // Erase Defaults
    break;
  case 'e':
    // FailSafe Values
    PrintValuesMenu();
    break;
  case 'f':
    // Boot for Terminal
    myBoot = 0;
    break;
  case 'g':
    // Boot for Slave
    myBoot = 1;
    break;
  case 'h':
    // Slave Address
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
