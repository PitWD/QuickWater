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

}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    int err = 1;
    int errCnt = 0;

    // Runtime
    EscLocate(67,1);
    EscInverse(1);
    PrintRunTime();
    EscInverse(0);    
    
    // Realtime
    EscColor(bgBlueB);
    EscColor(fgBlack);
    EscLocate(62,24);
    PrintDateTime();
    EscColor(0);

    //Read EZO's
    switch (ezoAction){
    case 0:
      // Set Avg-RTD to EC & pH probes
      EzoIntToStr(avg_RTD,2,2,'0');
      strcpy(&iicStr[2], strHLP);
      iicStr[0] = 'T'; iicStr[1] = ',';

      if (ezoProbe[ezoAct].type == ezoPH || ezoProbe[ezoAct].type == ezoEC){
        errCnt = 0;
        err = -1;
        while (err < 0){
          err = IIcSetStr(ezoProbe[ezoAct].address, iicStr);
          if (err < 0){
            errCnt++;
            if (errCnt > 3){
              // Fatal for this Probe
              PrintErrorOK(-1,ezoAct,(char*)"'T'");
              break;
            }
            delay(333);
          }
          else{
              // PrintErrorOK(1,ezoAct,(char*)"'T'");
          }          
        }
      }

      break;
    
    case 1:
      // Ask for Data
      errCnt = 0;
      err = -1;
      while (err < 0){
        err = EzoStartValues(ezoAct);
        if (err < 0){
          errCnt++;
          if (errCnt > 3){
            // Fatal for this Probe
            PrintErrorOK(-1,ezoAct,(char*)"'R'");
            break;
          }
          delay(333);
        }
        else{
          // PrintErrorOK(1,ezoAct,(char*)"'R'");
        }          
      }
    
      break;

    case 2:
      // Get Data
      err = EzoGetValues(ezoAct);

      if (err == 0){
        // Immediately Fatal for this Probe
        PrintErrorOK(-1,ezoAct,(char*)"Data");
      }
      else{
        // PrintErrorOK(1,ezoAct,(char*)"Data");
      }          
      break;
    default:
      ezoAction = 0;
      break;
    }

    if (ezoAct == ezoCnt -1){
      ezoAction++;
      ezoAct = 0;
    }
    else{
      ezoAct++;
    }

  }

  if (Serial.available()){
    Serial.read();
    PrintMainMenu();
  }


}