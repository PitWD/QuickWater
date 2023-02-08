#include "ezoIIC.h"
#include "myTime.h"


void EscLocate(int x, int y){
  Serial.print("\x1B[");
  Serial.print(y);
  Serial.print(";");
  Serial.print(x);
  Serial.print("H");
}
void EscCls(){
  Serial.print("\x1B[2J");
}
void EscBold(int set){
  if (set){
    Serial.print("\x1B[1m");
  }
  else{
    Serial.print("\x1B[22m");
  } 
}
void EscFaint(int set){
  if (set){
    Serial.print("\x1B[2m");
  }
  else{
    EscBold(0);
  } 
}

char GetUserKey(int maxChar, int ezoType){

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

int PrintMenuTop(char *strIN){
  EscCls();
  EscLocate(1, 1);
  EscBold(1);
  Serial.print((char*)strIN);
  EscBold(0);
  return 2;
}

void PrintMenuEnd(int pos){
  Serial.println("\n");
  Serial.print("    Select key, or Enter(for return)...");
}

int PrintAllMenuOpt2(int address1st, int pos){

  EscLocate(5, pos++);

  EscFaint(1);
  Serial.print("(Auto)Name: ");
  EscFaint(0);
  Serial.print((char*)strHLP2);
  EscFaint(1);
  Serial.print("   (1st)Address: ");
  EscFaint(0);
  Serial.print(address1st);

  return pos;

}

int PrintAllMenuOpt1(int pos){

  EscLocate(5, pos++);
  Serial.print("A): 'Factory' Reset");
  EscLocate(5, pos++);
  Serial.print("B): Delete Name(s)");
  EscLocate(5, pos++);
  Serial.print("C): Set (Auto-)Name...");
  EscLocate(5, pos++);
  Serial.print("D): Write (Auto-)Name(s)");
  EscLocate(5, pos++);
  Serial.print("E): Set (1st) Address...");
  EscLocate(5, pos++);
  Serial.print("F): Write Address(es)");
  EscLocate(5, pos++);
  Serial.print("G): Clear Calibration(s)");
  
  return pos;

}

void PrintAllMenu(){

  static char ImInside = 0;
  static char address1st = 33;

  if (!ImInside){
    strcpy(strHLP2, "-DummyName-");
    address1st = 33;
  }
  ImInside = 1;

  int pos = PrintMenuTop((char*)"                    - ALL Menu -");
  pos = PrintAllMenuOpt1(pos + 1);
  pos = PrintAllMenuOpt2(address1st, pos + 1);
  PrintMenuEnd(pos + 1);

  switch (GetUserKey('g', -1)){
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
  
  default:
    break;
  }

  ImInside = 0;

}

void PrintProbeLine(int ezo, int pos){
    EscLocate(5, pos);
    Serial.print(ezo + 1);
    Serial.print(".:");
    EscLocate(10, pos);
    strcpy_P(strHLP,(PGM_P)pgm_read_word(&(ezoStrType[ezoProbe[ezo].type])));
    Serial.print(strHLP);
    EscLocate(16, pos);
    EzoIntToStr(ezoProbe[ezo].version);
    Serial.print(strHLP);
    EscLocate(23, pos);
    Serial.print(ezoProbe[ezo].address);
    EscLocate(28, pos);
    Serial.print((char*)ezoProbe[ezo].name);
    EscLocate(46, pos);
    Serial.print(ezoProbe[ezo].calibrated);
}

void PrintProbeMenu(int ezo){

  static char ImInside = 0;
  static char address1st = 33;
  static char all = 0;

  if (!ImInside){
    strcpy(strHLP2, "-DummyName-");
    address1st = 33;
    all = 0;
  }
  ImInside = 1;

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

  EscLocate(5, pos++ + 1);
  Serial.print("---");

  pos = PrintAllMenuOpt1(pos + 1);
  EscLocate(5, pos++);
  Serial.print("H): Calibration(s)...");
  EscLocate(5, pos++);
  Serial.print("I): Select Single");
  EscLocate(5, pos++);
  Serial.print("J): Select ALL");

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
    PrintProbeMenu(ezo);
  }
  
  ImInside = 0;

}

void PrintValuesMenu(){

  int pos = PrintMenuTop((char*)"              - Set FailSafe Values -");

  EscLocate(5, pos);
  Serial.print("A): Humidity %rH = ");
  EscLocate(22, pos++);
  Serial.print("B): Air-Tem  °C  = ");
  EscLocate(22, pos++);
  Serial.print("C): Air-CO   ppm = ");
  
  EscLocate(5, pos);
  Serial.print("D):  RTD     °C  = ");
  EscLocate(22, pos++);
  Serial.print("E):  EC      µS  = ");
  EscLocate(5, pos++);
  Serial.print("F):  pH      pH  = ");
  EscLocate(22, pos++);
  Serial.print("G):  ORP     mV  = ");
  EscLocate(22, pos++);
  Serial.print("H): H2O-O2   r%  = ");

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
    PrintValuesMenu();
  }
  
}

void PrintMainMenu(){

  int pos = PrintMenuTop((char*)"          - Main Menu QuickWater 1.00 -");
  
  for (int i = 0; i < ezoCnt; i++){
    pos++;
    PrintProbeLine(i, pos);
  }

  EscLocate(5, pos + 1);
  Serial.print("---");
  pos += 2;

  EscLocate(5, pos++);
  Serial.print("A):  Select All...");
  EscLocate(5, pos++);
  Serial.print("B):  ReBoot");
  EscLocate(5, pos++);
  Serial.print("C):  Save State As Default");
  EscLocate(5, pos++);
  Serial.print("D):  Erase Default State");
  EscLocate(5, pos++);
  Serial.print("E):  Set FailSafe Values...");
  EscLocate(5, pos++);
  Serial.print("F):  Set Boot Values...");
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('f', 0);
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
    // Boot Values
    break;
  default:
    // Single Probe/Type
    PrintProbeMenu(pos - 49);
    break;
  }
  if (pos > 0){
    PrintMainMenu();
  }
  EscCls();
}
