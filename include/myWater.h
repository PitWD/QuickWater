#include "quicklib.h"
#include <EEPROM.h>

#define EZO_MAX_PROBES 14
#define EZO_MAX_VALUES 2        // 3 for full HUM / 5 for full RGB...
#define EZO_1st_ADDRESS 32
#define EZO_LAST_ADDRESS 127

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

#define ezoHumT 0
#define ezoRTD 1
#define ezoPH 2
#define ezoEC 3
#define ezoORP 4
#define ezoHUM 5
#define ezoCO2 6
#define ezoDiO2 7
#define ezoFLOW 8
#define ezoRGB 9
#define ezoPRES 10

const char ezoStrType_0[] PROGMEM = "HumT";
const char ezoStrType_1[] PROGMEM = "RTD";
const char ezoStrType_2[] PROGMEM = "pH";
const char ezoStrType_3[] PROGMEM = "EC";
const char ezoStrType_4[] PROGMEM = "ORP";
const char ezoStrType_5[] PROGMEM = "HUM%";
const char ezoStrType_6[] PROGMEM = "CO2";
const char ezoStrType_7[] PROGMEM = "D.O.";
const char ezoStrType_8[] PROGMEM = "Flow";
const char ezoStrType_9[] PROGMEM = "RGB";
const char ezoStrType_10[] PROGMEM = "Pres";
PGM_P const ezoStrType[] PROGMEM = {
    ezoStrType_0,
    ezoStrType_1,
    ezoStrType_2,
    ezoStrType_3,
    ezoStrType_4,
    ezoStrType_5,
    ezoStrType_6,
    ezoStrType_7,
    ezoStrType_8,
    ezoStrType_9,
    ezoStrType_10
};

const char ezoStrLongType_0[] PROGMEM = "Air-Temp.";
const char ezoStrLongType_1[] PROGMEM = "H2O-Temp.";
// pH
// EC
const char ezoStrLongType_4[] PROGMEM = "Redox";
const char ezoStrLongType_5[] PROGMEM = "Humidity";
// CO2
const char ezoStrLongType_7[] PROGMEM = "H2O-O2";
PGM_P const ezoStrLongType[] PROGMEM = {
    ezoStrLongType_0,
    ezoStrLongType_1,
    ezoStrType_2,
    ezoStrType_3,
    ezoStrLongType_4,
    ezoStrLongType_5,
    ezoStrType_6,
    ezoStrLongType_7,   
};

//const char ezoStrUnit_0[] PROGMEM = "°C";
const char ezoStrUnit_1[] PROGMEM = "°C";
//const char ezoStrUnit_2[] PROGMEM = "pH";
const char ezoStrUnit_3[] PROGMEM = "µS";
const char ezoStrUnit_4[] PROGMEM = "mV";
const char ezoStrUnit_5[] PROGMEM = "r%";
const char ezoStrUnit_6[] PROGMEM = "ppm";
const char ezoStrUnit_7[] PROGMEM = "%";
PGM_P const ezoStrUnit[] PROGMEM = {
    ezoStrUnit_1,
    ezoStrUnit_1,
    ezoStrType_2,
    ezoStrUnit_3,
    ezoStrUnit_4,
    ezoStrUnit_5,
    ezoStrUnit_6,
    ezoStrUnit_7
};

// Waittime for readings...
const int ezoWait[11] PROGMEM = {0, 600, 900, 600, 900, 300, 900, 600, 300, 300, 900};

// Count of vals of probe
const byte ezoValCnt[11] PROGMEM = {0, 1, 1, 1, 1, 3, 2, 2, 2, 5, 1};

// if type has a calibration
const byte ezoHasCal[11] PROGMEM = {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};

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

long failSave[] = {25000L, 21000L, 6000L, 1250000L, 225000L, 50000L, 400000L, 99999};
#define failSave_TMP failSave[0]
#define failSave_RTD failSave[1]
#define failSave_pH failSave[2]
#define failSave_EC failSave[3]
#define failSave_ORP failSave[4]
#define failSave_HUM failSave[5]
#define failSave_CO2 failSave[6]
#define failSave_O2 failSave[7]

long avgVal[] = {25000L, 21000L, 6000L, 1250000L, 225000L, 50000L, 400000L, 99999};
#define avg_TMP avgVal[0]
#define avg_RTD avgVal[1]
#define avg_pH avgVal[2]
#define avg_EC avgVal[3]
#define avg_ORP avgVal[4]
#define avg_HUM avgVal[5]
#define avg_CO2 avgVal[6]
#define avg_O2 avgVal[7]

long tooLow[] = {16000L, 15000L, 5500L, 1000000L, -750000L, 40000L, 350000L, 50000L};
#define tooLow_TMP tooLow[0]    // 2nd Val ezoHUM
#define tooLow_RTD tooLow[1]
#define tooLow_pH tooLow[2]
#define tooLow_EC tooLow[3]
#define tooLow_ORP tooLow[4]
#define tooLow_HUM tooLow[5]
#define tooLow_CO2 tooLow[6]
#define tooLow_O2 tooLow[7]

long low[] = {19000L, 17000L, 6000L, 1250000L, -500000L, 50000L, 400000L, 66666L};
#define low_TMP low[0]      // 2nd Val ezoHUM
#define low_RTD low[1]
#define low_pH low[2]
#define low_EC low[3]
#define low_ORP low[4]
#define low_HUM low[5]
#define low_CO2 low[6]
#define low_O2 low[7]

long high[] = {25000L, 20000L, 6800L, 1750000L, 500000L, 66666L, 1250000L, 100001L};
#define high_TMP high[0]    // 2nd Val ezoHUM
#define high_RTD high[1]
#define high_pH high[2]
#define high_EC high[3]
#define high_ORP high[4]
#define high_HUM high[5]
#define high_CO2 high[6]
#define high_O2 high[7]

long tooHigh[] = {27000L, 22000L, 7000L, 2000000L, 750000L, 75000L, 1500000L, 100001L};
#define tooHigh_TMP tooHigh[0]    // 2nd Val ezoHUM
#define tooHigh_RTD tooHigh[1]
#define tooHigh_pH tooHigh[2]
#define tooHigh_EC tooHigh[3]
#define tooHigh_ORP tooHigh[4]
#define tooHigh_HUM tooHigh[5]
#define tooHigh_CO2 tooHigh[6]
#define tooHigh_O2 tooHigh[7]


#define CAL_RTD_RES -1L         // Value for Reset
#define CAL_RTD_LOW 0L          // Value for LowPoint
#define CAL_RTD_MID 21000L      // Value for MidPoint
#define CAL_RTD_HIGH 100000L    // Value for HighPoint

#define CAL_EC_RES 0L
#define CAL_EC_LOW 84000L 
#define CAL_EC_MID 1413000L
#define CAL_EC_HIGH -1L

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

void SetAvgColor(long avg, long tooLow, long low, long high, long tooHigh){
  byte color = fgGreen;
  if (avg < tooLow){
    color = fgCyan;
  }
  else if (avg < low){
    color = fgBlue;
  }
  else if (avg > tooHigh){
    color = fgRed;
  }
  else if (avg > high){
    color = fgYellow;
  }
  EscColor(color);
}

// #define SetAvgColorEZO(avgVal, ezoType) SetAvgColor(avgVal, tooLow[ezoType], low[ezoType], high[ezoType], tooHigh[ezoType])
void SetAvgColorEZO(long avgVal, byte ezoType){
    // - 46 Flash (5x used)
    // +128 Ram
    SetAvgColor(avgVal, tooLow[ezoType], low[ezoType], high[ezoType], tooHigh[ezoType]);
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


    for (int i = 0; i < ezoCnt; i++){

        if (EzoCheckOnSet(ezo,all, i)){

            strcpy(iicStr,"Name,");

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

    for (int i = 0; i < ezoCnt; i++){

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
            case ezoHUM:
                strcpy_P(strSetup[1],(PGM_P)F("O,HUM,1")); // Humidity
                strcpy_P(strSetup[2],(PGM_P)F("O,T,1"));   // Temperature 
                strcpy_P(strSetup[3],(PGM_P)F("O,Dew,1")); // Dewing point
                strcpy_P(strSetup[4],(PGM_P)F("Alarm,en,0"));
                cntSetup = 5;
                break;
            case ezoFLOW:
                strcpy_P(strSetup[1],(PGM_P)F("Frp,m"));    // FlowRate/minute (s = second / h = hour)
                strcpy_P(strSetup[2],(PGM_P)F("CF,0.001")); // Liter (1 would be ml)
                strcpy_P(strSetup[3],(PGM_P)F("O,TV,1"));   // Output Total Volume
                strcpy_P(strSetup[4],(PGM_P)F("O,FR,1"));   // Output Flow Rate
                cntSetup = 5;
                break;
            case ezoPRES:
                strcpy_P(strSetup[1],(PGM_P)F("Dec,3"));
                strcpy_P(strSetup[2],(PGM_P)F("U,bar"));
                strcpy_P(strSetup[3],(PGM_P)F("Alarm,en,0"));
                cntSetup = 4;
                break;
            case ezoRGB:
                strcpy_P(strSetup[0],(PGM_P)F("iL,1"));
                strcpy_P(strSetup[1],(PGM_P)F("O,RGB,1"));
                strcpy_P(strSetup[2],(PGM_P)F("O,LUX,1"));
                strcpy_P(strSetup[3],(PGM_P)F("O,CIE,1"));
                strcpy_P(strSetup[4],(PGM_P)F("L,33,T"));
                cntSetup = 5;
                break;
            case ezoCO2:
                strcpy_P(strSetup[1],(PGM_P)F("O,t,1"));   // Output Internal Temp
                cntSetup = 2;
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
    for (int i = 0; i < ezoCnt; i++){
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
    for (int i = 0; i < ezoCnt; i++){
        if (EzoCheckOnSet(ezo,all, i)){
            strcpy(strHLP, "I2C,");
            itoa(addrNew, strHLP2, 10);
            strcpy(&strHLP[4], strHLP2);
            IIcSetStr(ezoProbe[i].address, strHLP, 0);
            addrNew++;
            delay(300);
        }
    }
}

char EzoDoNext(){

    char err = 1;
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
        if (ezoAct == ezoCnt){
            // All Modules Read
            ezoAct = 0;
            return 1;
        }
    }
    else{
      // Next Module  
      ezoAction++;
    }

    if (err < 0){
        PrintErrorOK(-1, errCnt, errInfo);
        return -1;
    }
    else{
        // PrintErrorOK(1, errCnt, errInfo);
        return 0;
    }

}

void EzoScan(){
    // Scan for Ezo's

    int err;
    int recEzo;         // recognized EzoProbe[ezoCnt].module.version
    int verPos;         // 'pointer' on 1st char of version
    int hasCal;         // has a calibration
    
    ezoCnt = 0;
    Serial.println("");

    for (int i = EZO_1st_ADDRESS; i < EZO_LAST_ADDRESS + 1 && ezoCnt < EZO_MAX_PROBES; i++){
        
        //Exclude known stuff
        if (!(i > 79 && i < 88) && !(i == 104)){        

            Wire.beginTransmission(i);
            err = Wire.endTransmission();
            if (!err){
                Serial.print(F("Slave @: "));
                Serial.print(i);
                Serial.print(F(" : "));
                // Slave found... looking for EZO-ID
                err = IIcSetStr(i, (char*)"i", 0);
                Serial.print(err);
                Serial.print(F(" : "));
                delay(333);
                err = IIcGetAtlas(i);
                Serial.print(err);
                Serial.print(F(" : "));
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
                    Serial.print(F(" : "));
                    if (iicStr[0] == '?' && iicStr[1] == 'I'){
                        // It's an ezo...

                        recEzo = 0;
                        verPos = 7;
                        hasCal = 1;
                        ezoProbe[ezoCnt].calibrated = 0;
                        ezoProbe[ezoCnt].name[0] = 0;

                        switch (iicStr[3]){
                        case 'R':
                            // RGB or RTD
                            switch (iicStr[4]){
                            case 'G':
                                // RGB
                                recEzo = ezoRGB;
                                hasCal = 0;
                                break;
                            case 'T':
                                // RTD
                                recEzo = ezoRTD;
                                break;
                            default:
                                // LATE ERROR
                                break;
                            }
                            break;
                        case 'F':
                            // FLO(W)
                            recEzo = ezoFLOW;
                            hasCal = 0;
                            break;
                        case 'O':
                            // ORP
                            recEzo = ezoORP;
                            break;
                        case 'C':
                            // CO2
                            recEzo = ezoCO2;
                            hasCal = 0;
                            break;
                        case 'H':
                            // HUM
                            recEzo = ezoHUM;
                            hasCal = 0;
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
                        case 'P':
                            // Embedded Pressure
                            recEzo = ezoPRES;
                            break;
                        default:
                            // LATE ERROR
                            break;
                        }
                        Serial.println(recEzo);
                        if (recEzo){
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
                                    Serial.print(F("PoweredOff"));
                                    break;
                                case 'S':
                                    // software reset
                                    Serial.print(F("SoftReset"));
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
                                Serial.println(F("ERROR"));
                            }
                            
                            // Value(s)
                            Serial.print(F("Value(s): "));
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
                                Serial.println(F("ERROR"));
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
}

