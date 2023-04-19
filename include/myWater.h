#include "quicklib.h"
#include <EEPROM.h>

#define EZO_MAX_PROBES 14
#define EZO_MAX_VALUES 2        // 3 for full HUM / 5 for full RGB...
#define EZO_1st_ADDRESS 32
#define EZO_LAST_ADDRESS 127

#define INTERNAL_LEVEL_CNT 1    // cnt of internal level sensors

// What is the actual Action
// 0 = all read
// 1 = write command(s)
// 2 = all written
// 3 = read answer(s)
static byte ezoAction = 0;

// Which is the actual ezo
static byte ezoAct = 0;

// How many ezo's we have
static byte ezoCnt = 0;

/*
#define ezoHumT 0
#define ezoRTD 1
#define ezoPH 2
#define ezoEC 3
#define ezoORP 4
#define ezoHUM 5
#define ezoCO2 6
#define ezoDiO2 7
*/
#define ezoRTD 0
#define ezoEC 1
#define ezoPH 2
#define ezoORP 3
#define ezoDiO2 4
#define ezoLVL 5

const char ezoStrType_0[] PROGMEM = "RTD";
const char ezoStrType_1[] PROGMEM = "EC";
const char ezoStrType_2[] PROGMEM = "pH";
const char ezoStrType_3[] PROGMEM = "ORP";
const char ezoStrType_4[] PROGMEM = "D.O.";
const char ezoStrType_5[] PROGMEM = "LVL";
PGM_P const ezoStrType[] PROGMEM = {
    ezoStrType_0,
    ezoStrType_1,
    ezoStrType_2,
    ezoStrType_3,
    ezoStrType_4,
    ezoStrType_5
};

const char ezoStrLongType_0[] PROGMEM = "Temp.";
// EC
// pH
const char ezoStrLongType_3[] PROGMEM = "Redox";
const char ezoStrLongType_4[] PROGMEM = "O2";
const char ezoStrLongType_5[] PROGMEM = "Level";
PGM_P const ezoStrLongType[] PROGMEM = {
    ezoStrLongType_0,
    ezoStrType_1,
    ezoStrType_2,
    ezoStrLongType_3,
    ezoStrLongType_4,
    ezoStrLongType_5
};

const char ezoStrUnit_0[] PROGMEM = "°C";
const char ezoStrUnit_1[] PROGMEM = "µS";
// "pH";
const char ezoStrUnit_3[] PROGMEM = "mV";
const char ezoStrUnit_4[] PROGMEM = "%";
PGM_P const ezoStrUnit[] PROGMEM = {
    ezoStrUnit_0,
    ezoStrUnit_1,
    ezoStrType_2,
    ezoStrUnit_3,
    ezoStrUnit_4,
    ezoStrUnit_4
};

// Waittime for readings...
const int ezoWait[] PROGMEM = {600, 600, 900, 900, 600, 0};

// Count of vals of probe
const byte ezoValCnt[] PROGMEM = {1, 1, 1, 1, 2, 1};

// if type has a calibration
const byte ezoHasCal[] PROGMEM = {1, 1, 1, 1, 0, 0};

typedef struct ezoProbeSTRUCT{
    byte type;
    byte calibrated;
    byte error;            // 0=OK, 1=processing, 2=syntax, 3=IIC, unknown
    byte address;
    uint16_t version; 
    char name[17];
    long value[EZO_MAX_VALUES];
    //long valueLast[EZO_MAX_VALUES];
}ezoProbeSTRUCT;
ezoProbeSTRUCT ezoProbe[EZO_MAX_PROBES];

// Delay & ActionTimes
/*
uint16_t delayTimes[] = {2700, 2400, 2400, 0, 0, 3600};
uint16_t actionTooLow[] = {900, 10, 6, 0, 0, 30};
uint16_t actionLow[] = {450, 5, 3, 0, 15, 15};
uint16_t actionHigh[] = {900, 30, 3, 0, 0, 0};
uint16_t actionTooHigh[] = {1800, 60, 6, 0, 0, 15};
*/
uint16_t delayTimes[] = {180, 120, 60, 0, 0, 10};
uint16_t actionTooLow[] = {30, 20, 10, 0, 0, 3};
uint16_t actionLow[] = {15, 10, 5, 0, 0, 1};
uint16_t actionHigh[] = {15, 10, 5, 0, 0, 1};
uint16_t actionTooHigh[] = {30, 20, 10, 0, 0, 3};

// Counter for Low/High
uint32_t tooLowSince[6];
uint32_t lowSince[6];
uint32_t okSince[6];
uint32_t highSince[6];
uint32_t tooHighSince[6];
// Time of last action 
uint32_t lastAction[6];

long failSave[] = {21000L, 1250000L, 6000L, 225000L, 99999L, 66666L};
#define failSave_RTD failSave[0]
#define failSave_EC failSave[1]
#define failSave_pH failSave[2]
#define failSave_ORP failSave[3]
#define failSave_O2 failSave[4]
#define failSave_LVL failSave[5]

long avgVal[] = {21000L, 1250000L, 6000L, 225000L, 99999L, 66666L};
#define avg_RTD avgVal[0]
#define avg_EC avgVal[1]
#define avg_pH avgVal[2]
#define avg_ORP avgVal[3]
#define avg_O2 avgVal[4]
#define avg_LVL avgVal[5]

long tooLow[] = {15000L, 1000000L, 5500L, -750000L, 50000L, 24999L};
#define tooLow_RTD tooLow[0]
#define tooLow_EC tooLow[1]
#define tooLow_pH tooLow[2]
#define tooLow_ORP tooLow[3]
#define tooLow_O2 tooLow[4]
#define tooLow_LVL tooLow[5]

long low[] = {17000L, 1250000L, 5800L, -500000L, 66666L, 49999L};
#define low_RTD low[0]
#define low_EC low[1]
#define low_pH low[2]
#define low_ORP low[3]
#define low_O2 low[4]
#define low_LVL low[5]

long high[] = {20000L, 1750000L, 6800L, 500000L, 100001L, 74999L};
#define high_RTD high[0]
#define high_EC high[1]
#define high_pH high[2]
#define high_ORP high[3]
#define high_O2 high[4]
#define high_LVL high[5]

long tooHigh[] = {22000L, 2000000L, 7000L, 750000L, 100001L, 99999};
#define tooHigh_RTD tooHigh[0]
#define tooHigh_EC tooHigh[1]
#define tooHigh_pH tooHigh[2]
#define tooHigh_ORP tooHigh[3]
#define tooHigh_O2 tooHigh[4]
#define tooHigh_LVL tooHigh[5]


#define CAL_RTD_RES -1L         // Value for Reset
#define CAL_RTD_LOW 0L          // Value for LowPoint
#define CAL_RTD_MID 21000L      // Value for MidPoint
#define CAL_RTD_HIGH 100000L    // Value for HighPoint

#define CAL_EC_RES 0L
#define CAL_EC_LOW 84000L 
#define CAL_EC_MID 1413000L
#define CAL_EC_HIGH 1413000L

#define CAL_PH_RES -1L
#define CAL_PH_LOW 4000L 
#define CAL_PH_MID 7000L
#define CAL_PH_HIGH 10000L

#define CAL_ORP_RES -1L
#define CAL_ORP_LOW -1L 
#define CAL_ORP_MID 225000L
#define CAL_ORP_HIGH -1L

#define CAL_DiO2_RES 0L
#define CAL_DiO2_LOW 0L 
#define CAL_DiO2_MID 0L
#define CAL_DiO2_HIGH 0L

void DefaultProbesToRom(){
    // Save actual probe-constellation as Standard to Eeprom
    EEPROM.put(0, ezoProbe);
}
void DefaultProbesFromRom(){
    EEPROM.get(0, ezoProbe);
}

byte GetAvgState(long avg, long tooLow, long low, long high, long tooHigh){
  if (avg < tooLow){
    return fgCyan;
  }
  else if (avg < low){
    return fgBlue;
  }
  else if (avg > tooHigh){
    return fgRed;
  }
  else if (avg > high){
    return fgYellow;
  }
  return fgGreen;
}
void SetAvgColor(long avg, long tooLow, long low, long high, long tooHigh){
  EscColor(GetAvgState(avg, tooLow, low, high, tooHigh));
}

// #define SetAvgColorEZO(avgVal, ezoType) SetAvgColor(avgVal, tooLow[ezoType], low[ezoType], high[ezoType], tooHigh[ezoType])
void SetAvgColorEZO(byte ezoType){
    // - 46 Flash (5x used)
    // +128 Ram
    SetAvgColor(avgVal[ezoType], tooLow[ezoType], low[ezoType], high[ezoType], tooHigh[ezoType]);
}

char EzoStartValues(byte ezo){
    return IIcSetStr(ezoProbe[ezo].address, (char*)"R", 0);
}

void EzoWaitValues(byte ezo){
    delay(Fi(ezoWait[ezoProbe[ezo].type]));
}

byte EzoGetValues(byte ezo){
    if (IIcGetAtlas(ezoProbe[ezo].address) > 0){
        ezoProbe[ezo].value[0] = StrTokFloatToInt(iicStr);
        for (byte i = 1; i < Fb(ezoValCnt[ezoProbe[ezo].type]); i++){
            ezoProbe[ezo].value[i] = StrTokFloatToInt(NULL);
        }
        return 1;        
    }
    else{
        return 0;
    }
}

byte EzoCheckOnSet(byte ezo, byte all, byte i){
    // Check, if Module on i is valid in a EzoSetXYZ() function...
    if (i == ezo || (all == 1 && ezoProbe[i].type == ezoProbe[ezo].type) || (all == 2)){
        return 1;
    }
    return 0;
}

void EzoSetName(char *strIN, byte ezo, byte all, byte autoName){
    
    byte cnt[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    byte len = 0;


    for (int i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){

        if (EzoCheckOnSet(ezo,all, i)){

            strcpy_P(iicStr, (PGM_P)F("Name,"));

            if (autoName){
                cnt[ezoProbe[i].type]++;
                // Type
                strcpy(&iicStr[5], Fa(ezoStrType[ezoProbe[i].type]));
            }
            
            strcpy(&iicStr[strlen(iicStr)], strIN);
            
            if (autoName){
                len = strlen(iicStr);
                iicStr[len] = cnt[ezoProbe[i].type] + 48;
                iicStr[len + 1] = 0;
            }
            
            if (IIcSetStr(ezoProbe[i].address, iicStr, 1) > 0){
                strcpy(ezoProbe[i].name, &iicStr[5]);
            }               
        }
        
        // No clue why - but a second 0 is needed... otherwise module go kind of crazy - sometimes...
        Wire.beginTransmission(ezoProbe[i].address);
        Wire.write(0);
        Wire.endTransmission(ezoProbe[i].address);
    }
}

void EzoReset(byte ezo, byte all){
    // 'Factory' Reset
    // All = 1 = all of type ezo
    // All = 2 = all types

    byte cntSetup;           // count of setup-lines to send
    char strSetup[6][9];

    for (int i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){

        if (EzoCheckOnSet(ezo,all, i)){
        
            strcpy_P(strSetup[0],(PGM_P)F("L,1"));  // Indicator LED
            cntSetup = 1;

            switch (ezoProbe[i].type){
            case ezoRTD:
                strcpy_P(strSetup[1],(PGM_P)F("S,c"));  // Celsius (k = kelvin / f = fahrenheit)
                cntSetup = 2;
                break;
            case ezoEC:
                strcpy_P(strSetup[1],(PGM_P)F("O,EC,1"));
                strcpy_P(strSetup[2],(PGM_P)F("O,TDS,0"));
                strcpy_P(strSetup[3],(PGM_P)F("O,S,0"));
                strcpy_P(strSetup[4],(PGM_P)F("O,SG,0"));
                strcpy_P(strSetup[5],(PGM_P)F("K,1.0"));
                cntSetup = 6;
                break;
            case ezoPH:
                break;
            case ezoORP:
                break;
            case ezoDiO2:
                strcpy_P(strSetup[1],(PGM_P)F("O,mg,1"));
                strcpy_P(strSetup[2],(PGM_P)F("O,%,1"));
                cntSetup = 3;
                break;
            default:
                break;
            }
            // Write Setup
            for (int i2 = 0; i2 < cntSetup; i2++){
                IIcSetStr(ezoProbe[i].address, strSetup[i2], 0);
                delay(300);
            }
            delay(300);
        }
    }   
}

void EzoSetCal(char *strCmd, byte ezo, byte all){
    for (int i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){
        if (EzoCheckOnSet(ezo,all, i)){
            if (Fb(ezoHasCal[ezoProbe[ezo].type])){
                // Has set-able calibration
                IIcSetStr(ezoProbe[i].address, strCmd, 0);
                ezoProbe[i].calibrated = 0;
            }
        }
    }
}

void EzoSetAddress(byte ezo, byte addrNew, byte all){
    for (int i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){
        if (EzoCheckOnSet(ezo,all, i)){
            strcpy_P(strHLP, (PGM_P)F("I2C,"));
            itoa(addrNew, strHLP2, 10);
            strcpy(&strHLP[4], strHLP2);
            IIcSetStr(ezoProbe[i].address, strHLP, 0);
            addrNew++;
            delay(300);
        }
    }
}

int32_t GetRtdAvgForCal(){

    long avgTemp = 0;
    byte avgCnt = 0;

    for (byte i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){
        if (ezoProbe[i].type == ezoRTD){
            // RTD
            avgCnt++;
            EzoStartValues(i);
            EzoWaitValues(i);
            EzoGetValues(i);
            avgTemp += ezoProbe[i].value[0];
        }
    }
    if (avgCnt){
        // Real RTD exist
        return avgTemp / avgCnt;
    }
    else{
        return avg_RTD;
    }
}

int32_t PrintValsForCal(byte ezo, byte all){
    
    long avgTemp = 25;
    byte pos = 1;
    byte startPos = 1;
    byte timeOut = 120;

START:

    startPos = 0;
    if (ezoProbe[ezo].type == ezoPH || ezoProbe[ezo].type == ezoEC){
        // Need on temperature to do cal right
        avgTemp = GetRtdAvgForCal();
        avg_RTD = avgTemp;
        Serial.println();
        PrintFloat(avgTemp, 3, 2, ' ');
        EscFaint(1);
        Serial.print(F("°C"));
        EscFaint(0);
        EscCursorRight(3);
        pos += 11;
        startPos = 11;
    }   // else no need on temp

    // Read Vals of all to calibrate probes
    for (int i = 0; i < ezoCnt - INTERNAL_LEVEL_CNT; i++){
        if (EzoCheckOnSet(ezo,all, i)){
            if (Fb(ezoHasCal[ezoProbe[ezo].type])){
                // Has set-able calibration
                EzoStartValues(i);
                EzoWaitValues(i);
                EzoGetValues(i);
                PrintInt(i + 1, 2, '0');
                Serial.print(F(": "));
                PrintBoldFloat(ezoProbe[i].value[0], 4, 2, ' ');
                PrintSpacer(0);
                pos += 14;
                if (pos > 66){
                    Serial.println("");
                    EscCursorRight(startPos);
                    pos = startPos + 1;
                }
            }
        }
        if (DoTimer()){
            timeOut--;
        }
        if (Serial.available() || !timeOut){
            // Accept
            if (Serial.read() != 27){
                // valid return
                return avgTemp;
            }
            return 0;
        }
        goto START;
    }
}

int8_t EzoDoNext(){

    int8_t err = 1;
    char errInfo[] = "'?'";
    byte errCnt = 0;

    switch (ezoAction){
    case 0:
      // Set Avg-RTD to EC & pH probes
      IntToFloatStr(avg_RTD,2,2,'0');
      strcpy(&iicStr[2], strHLP);
      iicStr[0] = 'T'; iicStr[1] = ',';

      if (ezoProbe[ezoAct].type == ezoPH || ezoProbe[ezoAct].type == ezoEC){
        errCnt = 0;
        err = -1;
        while (err < 0){
          err = IIcSetStr(ezoProbe[ezoAct].address, iicStr, 0);
          if (err < 0){
            errCnt++;
            if (errCnt > 3){
              // Fatal for this Probe
              errInfo[1] = 'T';
              errCnt = ezoAct;
              break;
            }
            delay(333);
          }
          else{
              // errCnt = ezoAct;
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
            errInfo[1] = 'R';
            errCnt = ezoAct;
            break;
          }
          delay(333);
        }
        else{
          // errCnt = ezoAct;
        }          
      }
    
      break;
    
    case 2:
      // Get Data
      err = EzoGetValues(ezoAct);

      if (err == 0){
        // Immediately Fatal for this Probe
        errInfo[1] = 'D';
        errCnt = ezoAct;
        err = - 1;
      }
      else{
        // errCnt = ezoAct;
      }          
      break;

    default:
      ezoAction = 0;
      ezoAct = 0;
      return 0;
      break;
    }

    if (ezoAction == 2){
        // Actions done for this Module
        ezoAct++;
        ezoAction = 0;
        if (ezoAct == ezoCnt - INTERNAL_LEVEL_CNT){
            // All Modules Done
            

            // Analyze internal Level-Ports
                // tooLow
                ezoAct = 0;     // temporary use for comparison 
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 0;
            if (!digitalRead(14)){
                // Low
                ezoAct += 25;
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 25;
            }
            if (!digitalRead(15)){
                // OK
                ezoAct += 25;
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 50;
            }
            if (!digitalRead(16)){
                // High
                ezoAct += 25;
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 75L;
            }
            if (!digitalRead(17)){
                // toHigh
                ezoAct += 25;
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 100;
            }
            if (ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] != ezoAct){
                // One or some level-switches are wrong/faulty
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] = 66666;
            }     
            else{
                ezoProbe[ezoCnt - INTERNAL_LEVEL_CNT].value[0] *= 1000;
            }
            
            ezoAct = 0;
            return 1;
        }
    }
    else{
      // Next Module  
      ezoAction++;
    }

    if (err < 0){
        PrintErrorOK(-1, strlen(errInfo), errInfo);
        return -1;
    }
    else{
        // PrintErrorOK(1, errCnt, errInfo);
        return 0;
    }

}

void PrintColon(){
    Serial.print(F(" : "));
}
void EzoScan(){
    // Scan for Ezo's

    int err;
    int recEzo;         // recognized EzoProbe[ezoCnt].module.version
    int verPos;         // 'pointer' on 1st char of version
    int hasCal;         // has a calibration
    
    ezoCnt = 0;
    Serial.println("");

    for (int i = EZO_1st_ADDRESS; i < EZO_LAST_ADDRESS + 1 && ezoCnt < EZO_MAX_PROBES - INTERNAL_LEVEL_CNT; i++){
        
        //Exclude known stuff
        if (!(i > 79 && i < 88) && !(i == 104)){        

            Wire.beginTransmission(i);
            err = Wire.endTransmission();
            if (!err){
                Serial.print(F("Slave @: "));
                Serial.print(i);
                PrintColon();
                // Slave found... looking for EZO-ID
                err = IIcSetStr(i, (char*)"i", 0);
                Serial.print(err);
                PrintColon();
                delay(333);
                err = IIcGetAtlas(i);
                Serial.print(err);
                PrintColon();
                switch (err){
                case 0:
                    // nothing received
                    break;
                case -1:
                case -2:
                case -4:
                    // ezo errors
                    break;
                case -3:
                    // IIC error
                    break;
                default:
                    // something received
                    Serial.print(iicStr);
                    PrintColon();
                    if (iicStr[0] == '?' && iicStr[1] == 'I'){
                        // It's an ezo...

                        recEzo = -1;
                        verPos = 7;
                        hasCal = 1;
                        ezoProbe[ezoCnt].calibrated = 0;
                        ezoProbe[ezoCnt].name[0] = 0;

                        switch (iicStr[3]){
                        case 'R':
                            // RGB or RTD
                            switch (iicStr[4]){
                            case 'T':
                                // RTD
                                recEzo = ezoRTD;
                                break;
                            default:
                                // LATE ERROR
                                break;
                            }
                            break;
                        case 'O':
                            // ORP
                            recEzo = ezoORP;
                            break;
                        case 'E':
                            // EC
                            recEzo = ezoEC;
                            verPos = 6;
                            break;           
                        case 'p':
                            // pH
                            recEzo = ezoPH;
                            verPos = 6;
                            break;
                        case 'D':
                            // dissolved oxygen
                            recEzo = ezoDiO2;
                            verPos = 8;
                            break;
                        default:
                            // LATE ERROR or not supported...
                            break;
                        }
                        Serial.println(recEzo);
                        if (recEzo > -1){
                            // Valid Probe found
                            
                            // Save address and type
                            ezoProbe[ezoCnt].address = i;
                            ezoProbe[ezoCnt].type = recEzo;
                            
                            // Extract Version
                            ezoProbe[ezoCnt].version = StrTokFloatToInt(iicStr);
                            // Calibration
                            if (hasCal){
                                IIcSetStr(i,(char*)"Cal,?", 0);
                                delay(333);
                                if (IIcGetAtlas(i) > 0){
                                    ezoProbe[ezoCnt].calibrated = iicStr[5] - 48;
                                }                            
                            }
                            
                            // Name
                            IIcSetStr(i,(char*)"Name,?", 0);
                            delay(333);
                            if (IIcGetAtlas(i) > 0){
                                strcpy(ezoProbe[ezoCnt].name, &iicStr[6]);
                            }
// ********************
                            // Output (only stored in Module)
                            // Status
                            IIcSetStr(i, (char*)"Status", 0);
                            Serial.print(F("State: "));
                            delay(300);
                            if (IIcGetAtlas(i) > 0){
                                switch (iicStr[8]){
                                case 'P':
                                    // powered off
                                    Serial.print(F("PwrOff"));
                                    break;
                                case 'S':
                                    // software reset
                                    Serial.print(F("Reset"));
                                    break;
                                case 'B':
                                    // brown out
                                    Serial.print(F("BrownOut"));
                                    break;
                                case 'W':
                                    // watchdog
                                    Serial.print(F("Watchdog"));
                                    break;
                                case 'U':
                                    // Unknown
                                default:
                                    Serial.print(F("N/A"));
                                    break;
                                }             
                                Serial.print(F(" @ "));
                                Serial.print(&iicStr[10]);
                                Serial.println(F(" V"));
                            }
                            else{
                                Serial.println(F("ERR"));
                            }
                            
                            // Value(s)
                            Serial.print(F("Val's: "));
                            EzoStartValues(ezoCnt);
                            EzoWaitValues(ezoCnt);
                            if (EzoGetValues(ezoCnt)){
                                Serial.print(ezoProbe[ezoCnt].value[0]);
                                for (byte i2 = 1; i2 < Fb(ezoValCnt[recEzo]); i2++){
                                    Serial.print(F(" , "));
                                    Serial.print(ezoProbe[ezoCnt].value[i2]);
                                }          
                                Serial.println(F(""));
                            }
                            else{
                                Serial.println(F("ERR"));
                            }
                            Serial.println(F(""));
                            
                            // done
                            ezoCnt++;
                        }
                    } 
                    break;
                }
            }
        }
    }

    // Add internal 4-step level
    ezoProbe[ezoCnt].value[0] = 66666;
    ezoProbe[ezoCnt].address = 0;
    strcpy_P(ezoProbe[ezoCnt].name, (PGM_P)F("Int. Level - 1"));
    ezoProbe[ezoCnt].type = ezoLVL;
    ezoCnt++;

}

