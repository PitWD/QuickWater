#include "myMenu.h"
#include <EEPROM.h>

#define WIRE Wire

// Seconds between RTC sync's (0 disables sync)
#define syncRTCinterval 0

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  Wire.setClock(31000L);
  Wire.begin();

  EzoScan();

  if (ezoCnt){
    // at least 1 ezo got found
    Serial.println(F(""));
    PrintMainMenu();
  }

  int pos1st = 0;
  int posMax = 0;
  int posAct = 0;

  int pos = PrintMenuTop((char*)"                                 - QuickWater 1.00 -");
  pos++;

  EscLocate(5, pos++);
  EscBold(1);
  Serial.print(F(" | Temperature  | Conductivity |      pH      |     Redox    |      O2      |"));
  EscLocate(5, pos++);
  Serial.print(F(" ----------------------------------------------------------------------------"));
  EscBold(0);

  pos1st = pos;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoRTD){
      posAct++;
      EscLocate(8, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],2,2,' ');
      Serial.print(F(" °C"));
    }
  }
  posMax = posAct;

  pos = pos1st;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoEC){
      posAct++;
      EscLocate(24, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],4,0,' ');
      Serial.print(F(" µS"));
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }

  pos = pos1st;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoPH){
      posAct++;
      EscLocate(38, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],2,2,' ');
      Serial.print(F(" pH"));
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }

  pos = pos1st;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoORP){
      posAct++;
      EscLocate(53, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],4,2,' ');
      Serial.print(F(" mV"));
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }

  pos = pos1st;
  posAct = 0;
  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezoDiO2){
      posAct++;
      EscLocate(68, pos++);
      Serial.print(i + 1);
      Serial.print(F(": "));
      PrintBoldValue((long)ezoProbe[i].value[0],3,2,' ');
      Serial.print(F(" r%"));
    }
  }
  if (posAct > posMax){
    posMax = posAct;
  }

  pos = pos1st + posMax;

  // Avg 
  EscLocate(5, pos++);
  Serial.print(F(" ----------------------------------------------------------------------------"));
  
  SetAvgColor(avg_RTD, tooLow_RTD, low_RTD, high_RTD, tooHigh_RTD);
  EscLocate(11, pos);
  PrintBoldValue(avg_RTD,2,2,' ');
  EscColor(0);
  Serial.print(F(" °C"));

  SetAvgColor(avg_EC, tooLow_EC, low_EC, high_EC, tooHigh_EC);
  EscLocate(27, pos);
  PrintBoldValue(avg_EC,4,0,' ');
  EscColor(0);
  Serial.print(F(" µS"));

  SetAvgColor(avg_pH, tooLow_pH, low_pH, high_pH, tooHigh_pH);
  EscLocate(41, pos);
  PrintBoldValue(avg_pH,2,2,' ');
  EscColor(0);
  Serial.print(F(" pH"));

  SetAvgColor(avg_ORP, tooLow_ORP, low_ORP, high_ORP, tooHigh_ORP);
  EscLocate(56, pos);
  PrintBoldValue(avg_ORP,4,2,' ');
  EscColor(0);
  Serial.print(F(" mV"));

  SetAvgColor(avg_O2, tooLow_O2, low_O2, high_O2, tooHigh_O2);
  EscLocate(71, pos++);
  PrintBoldValue(avg_O2,3,2,' ');
  EscColor(0);
  Serial.print(F(" r%"));

  EscBold(1);
  EscLocate(5, pos++);
  Serial.print(F(" ----------------------------------------------------------------------------"));
  EscBold(0);

}

void loop() {
// put your main code here, to run repeatedly:



  if (DoTimer()){
  // A Second is over...

  }

  if (Serial.available()){
    Serial.read();
    PrintMainMenu();
  }


}