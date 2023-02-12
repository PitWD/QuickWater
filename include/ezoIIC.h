#include "myIIC.h"
#include "myTime.h"

#define EZO_MAX_PROBES 8
#define EZO_MAX_VALUES 3        // 5 just for full RGB...
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

byte myAddress = 123;

#define ezoRTD 1
#define ezoPH 2
#define ezoEC 3
#define ezoORP 4
#define ezoHUM 5
#define ezoCO2 6
#define ezoFLOW 7
#define ezoRGB 8
#define ezoDiO2 9
#define ezoPRES 10

const char ezoStrType_0[] PROGMEM = "N/A";
const char ezoStrType_1[] PROGMEM = "RTD";
const char ezoStrType_2[] PROGMEM = "pH";
const char ezoStrType_3[] PROGMEM = "EC";
const char ezoStrType_4[] PROGMEM = "ORP";
const char ezoStrType_5[] PROGMEM = "HUM";
const char ezoStrType_6[] PROGMEM = "CO2";
const char ezoStrType_7[] PROGMEM = "Flow";
const char ezoStrType_8[] PROGMEM = "RGB";
const char ezoStrType_9[] PROGMEM = "D.O.";
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

// Waittime for readings...
const int ezoWait[11] PROGMEM = {0, 600, 900, 600, 900, 300, 900, 300, 300, 600, 900};

// Count of vals of probe
const int ezoValCnt[11] PROGMEM = {0, 1, 1, 1, 1, 3, 2, 2, 5, 2, 1};

// if type has a calibration
const int ezoHasCal[11] PROGMEM = {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};

typedef struct ezoProbeSTRUCT{
    byte type;
    byte calibrated;
    byte error;            // 0=OK, 1=processing, 2=syntax, 3=IIC, unknown
    byte address;
    unsigned int version; 
    char name[17];
    long value[EZO_MAX_VALUES];
    //long valueLast[EZO_MAX_VALUES];
}ezoProbeSTRUCT;

ezoProbeSTRUCT ezoProbe[EZO_MAX_PROBES];

long failSave_HUM = 50000L;
long failSave_TMP = 25000L;
long failSave_CO2 = 400000L;
long failSave_RTD = 21000L;
long failSave_EC = 1250000L;
long failSave_pH = 6000L;
long failSave_ORP = 225000L;
long failSave_O2 = 99000L;

long avg_HUM = 50000L;
long avg_TMP = 25000L;
long avg_CO2 = 400000L;
long avg_RTD = 21000L;
long avg_EC = 1250000L;
long avg_pH = 6000L;
long avg_ORP = 225000L;
long avg_O2 = 99000L;

long tooLow_HUM = 40000L;
long tooLow_TMP = 16000L;
long tooLow_CO2 = 350000L;
long tooLow_RTD = 15000L;
long tooLow_EC = 1000000L;
long tooLow_pH = 5500L;
long tooLow_ORP = -750000L;
long tooLow_O2 = 50000L;

long low_HUM = 50000L;
long low_TMP = 19000L;
long low_CO2 = 400000L;
long low_RTD = 17000L;
long low_EC = 1250000L;
long low_pH = 6000L;
long low_ORP = -500000L;
long low_O2 = 66666L;

long high_HUM = 66666L;
long high_TMP = 25000L;
long high_CO2 = 1250000L;
long high_RTD = 20000L;
long high_EC = 1750000L;
long high_pH = 6800L;
long high_ORP = 500000L;
long high_O2 = 100001L;

long tooHigh_HUM = 75000L;
long tooHigh_TMP = 27000L;
long tooHigh_CO2 = 1500000L;
long tooHigh_RTD = 22000L;
long tooHigh_EC = 2000000L;
long tooHigh_pH = 7000L;
long tooHigh_ORP = 750000L;
long tooHigh_O2 = 100001L;




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


void PrintErrorOK(int err, int ezo, char *strIN){

  // Err: 0 = info, -1 = err, 1 = OK
  // Err: 0 = black, -1 = red, 1 = green

  int len = strlen(strIN) + 48;

  EscColor(bgBlueB);
  EscLocate(1,24);

  if (err == -1){
    EscColor(fgRed);
  }
  else if(err == 1){
    EscColor(fgBlackB);
  }
  else{
    EscColor(fgBlack);
    EscBold(1);
  }
  
  Serial.print(F("    "));
  Serial.print(strIN);
  EscBold(0);
  
  if (ezo > -1){
    Serial.print(F(" - on: "));
    EscColor(fgBlack);
    EzoIntToStr((long)ezoProbe[ezo].address * 1000,3,0,'0');
    Serial.print(strHLP);
    len += strlen(strHLP);
  }
  else{
    EscColor(fgBlack);
    len -= 7;
  }
    
  Serial.print(F(" @ "));
  PrintRunTime();

  len = 80 - len;
  for (int i = 0; i < len; i++){
    Serial.print(F(" "));
  }
  
  PrintDateTime();
  EscColor(0);

}

void SetAvgColor(long avg, long tooLow, long low, long high, long tooHigh){
  if (avg < tooLow){
    EscColor(fgBlue);
  }
  else if (avg < low){
    EscColor(fgGreen);
  }
  else if (avg > high){
    EscColor(fgRedB);
  }
  else if (avg > tooHigh){
    EscColor(fgRed);
  }
  else{
    EscColor(fgCyan);
  }
}


long exp10(int e){
  long x = 1;
  for (int i = 0; i < e; i++) {
    x = x * 10;
  }
  return x;
}

long StrToInt(char *strIN, int next){

    // "1.234" , 1000  ==> 1234
    // mul == 0 search next

    long r = 0;

    static char *nextVal = NULL;
    static char *actVal = NULL;

    long preDot = 0;
    long afterDot = 0;
    
    if (next){
        // Next Val
        actVal = nextVal;
    }
    else{
        // New Val
        actVal = strIN;
    }

    preDot = atol(actVal) * 1000;

    // decimal dot
    char *dot = strchr(actVal, '.');
    
    // (probably) next number
    nextVal = strchr(actVal, ',');
    
    if (dot){
        if (nextVal == NULL){
            nextVal = strchr(actVal, '\0');
        }
        // count of missing digits after dot
        r = 3 - (long)(nextVal - dot);
        r = exp10(r + 1);
        for (nextVal -= 1; nextVal > dot; nextVal--){
            afterDot += r * (nextVal[0] - 48);
            r *= 10;
        }
    }
    if (nextVal){
        nextVal++;
    }

    if (preDot >= 0){
        r = preDot + afterDot;
    }
    else{
        r = preDot - afterDot;
    }

    return r;
}

int EzoStartValues(int ezo){
    return IIcSetStr(ezoProbe[ezo].address, (char*)"R", 0);
}

void EzoWaitValues(int ezo){
    delay((int)pgm_read_word(&(ezoWait[ezoProbe[ezo].type])));
}

int EzoGetValues(int ezo){
    if (IIcGetAtlas((int)ezoProbe[ezo].address) > 0){
        ezoProbe[ezo].value[0] = StrToInt(iicStr, 0);
        for (int i = 1; i < (int)pgm_read_word(&(ezoValCnt[ezoProbe[ezo].type])); i++){
            ezoProbe[ezo].value[i] = StrToInt(iicStr, 1);
        }
        return 1;        
    }
    else{
        return 0;
    }
}

void EzoSetName(char *strIN, int ezo, int all, int autoName){
    
    char cnt[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int len = 0;

    for (int i = 0; i < ezoCnt; i++){

        if ((i == ezo) || (all == 1 && (ezoProbe[i].type == ezoProbe[ezo].type)) || (all == 2)){

            strcpy(iicStr,"Name,");

            if (autoName){
                cnt[ezoProbe[i].type]++;
                // Type
                strcpy_P(strHLP,(PGM_P)pgm_read_word(&(ezoStrType[ezoProbe[i].type])));
                strcpy(&iicStr[5], strHLP);
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

void EzoReset(int ezo, int all){
    // 'Factory' Reset
    // All = 1 = all of type ezo
    // All = 2 = all types

    int cntSetup;           // count of setup-lines to send
    char strSetup[6][9];

    for (int i = 0; i < ezoCnt; i++){

        if (i == ezo || (all == 1 && ezoProbe[i].type == ezoProbe[ezo].type) || (all == 2)){
        
            strcpy(strSetup[0],"L,1");  // Indicator LED
            cntSetup = 1;

            switch (ezoProbe[i].type){
            case ezoRTD:
                strcpy(strSetup[1],"S,c");  // Celsius (k = kelvin / f = fahrenheit)
                cntSetup = 2;
                break;
            case ezoEC:
                strcpy(strSetup[1],"O,EC,1");
                strcpy(strSetup[2],"O,TDS,0");
                strcpy(strSetup[3],"O,S,0");
                strcpy(strSetup[4],"O,SG,0");
                strcpy(strSetup[5],"K,1.0");
                cntSetup = 6;
                break;
            case ezoPH:
                break;
            case ezoORP:
                break;
            case ezoDiO2:
                strcpy(strSetup[1],"O,mg,1");
                strcpy(strSetup[2],"O,%,1");
                cntSetup = 3;
                break;
            case ezoHUM:
                strcpy(strSetup[1],"O,HUM,1"); // Humidity
                strcpy(strSetup[2],"O,T,1");   // Temperature 
                strcpy(strSetup[3],"O,Dew,1"); // Dewing point
                strcpy(strSetup[4],"Alarm,en,0");
                cntSetup = 5;
                break;
            case ezoFLOW:
                strcpy(strSetup[1],"Frp,m");    // FlowRate/minute (s = second / h = hour)
                strcpy(strSetup[2],"CF,0.001"); // Liter (1 would be ml)
                strcpy(strSetup[3],"O,TV,1");   // Output Total Volume
                strcpy(strSetup[4],"O,FR,1");   // Output Flow Rate
                cntSetup = 5;
                break;
            case ezoPRES:
                strcpy(strSetup[1],"Dec,3");
                strcpy(strSetup[2],"U,bar");
                strcpy(strSetup[3],"Alarm,en,0");
                cntSetup = 4;
                break;
            case ezoRGB:
                strcpy(strSetup[0],"iL,1");
                strcpy(strSetup[1],"O,RGB,1");
                strcpy(strSetup[2],"O,LUX,1");
                strcpy(strSetup[3],"O,CIE,1");
                strcpy(strSetup[4],"L,33,T");
                cntSetup = 5;
                break;
            case ezoCO2:
                strcpy(strSetup[1],"O,t,1");   // Output Internal Temp
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

void EzoSetCal(char *strCmd, int ezo, int all){
    for (int i = 0; i < ezoCnt; i++){
        if (i == ezo|| (all == 1 && ezoProbe[i].type == ezoProbe[ezo].type) || (all == 2)){
            if ((int)pgm_read_word(&(ezoHasCal[ezoProbe[ezo].type]))){
                // Has set-able calibration
                IIcSetStr(ezoProbe[i].address, strCmd, 0);
                ezoProbe[i].calibrated = 0;
            }
        }
    }
}

void EzoSetAddress(int ezo, int addrNew, int all){
    for (int i = 0; i < ezoCnt; i++){
        if (i == ezo|| (all == 1 && ezoProbe[i].type == ezoProbe[ezo].type) || (all == 2)){
            strcpy(strHLP, "I2C,");
            itoa(addrNew, strHLP2, 10);
            strcpy(&strHLP[4], strHLP2);
            IIcSetStr(ezoProbe[i].address, strHLP, 0);
            addrNew++;
            delay(300);
        }
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
        Wire.beginTransmission(i);
        err = Wire.endTransmission();
        if (!err){
            Serial.print("Slave @: ");
            Serial.print(i);
            Serial.print(" : ");
            // Slave found... looking for EZO-ID
            err = IIcSetStr(i, (char*)"i", 0);
            Serial.print(err);
            Serial.print(" : ");
            delay(333);
            err = IIcGetAtlas(i);
            Serial.print(err);
            Serial.print(" : ");
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
                Serial.print(" : ");
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
                        ezoProbe[ezoCnt].version = StrToInt(&iicStr[verPos], 0);
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

                        // Output (in RAM)
                        Serial.print("Found: ");
                        strcpy_P(strHLP,(PGM_P)pgm_read_word(&(ezoStrType[recEzo])));
                        Serial.print(strHLP);
                        Serial.print(" @: ");
                        Serial.println(i);

                        Serial.print("         Name: ");
                        Serial.print((char*)ezoProbe[ezoCnt].name);
                        Serial.println("");
                        Serial.print("      Version: ");
                        Serial.println(ezoProbe[ezoCnt].version);

                        Serial.print("  Calibration: ");
                        Serial.println(ezoProbe[ezoCnt].calibrated);

                        // Output (in Module)
                        // Status
                        IIcSetStr(i, (char*)"Status", 0);
                        Serial.print("        State: ");
                        delay(300);
                        if (IIcGetAtlas(i) > 0){
                            switch (iicStr[8]){
                            case 'P':
                                // powered off
                                Serial.print("'powered off'");
                                break;
                            case 'S':
                                // software reset
                                Serial.print("'software reset'");
                                break;
                            case 'B':
                                // brown out
                                Serial.print("'brown out'");
                                break;
                            case 'W':
                                // watchdog
                                Serial.print("'watchdog'");
                                break;
                            case 'U':
                                // Unknown
                            default:
                                Serial.print("'unknown'");
                                break;
                            }             
                            Serial.print(" @ ");
                            Serial.print(&iicStr[10]);
                            Serial.println(" Volt");
                        }
                        else{
                            Serial.println("ERROR");
                        }
                        
                        // Value(s)
                        Serial.print("     Value(s): ");
                        EzoStartValues(ezoCnt);
                        EzoWaitValues(ezoCnt);
                        if (EzoGetValues(ezoCnt)){
                            Serial.print(ezoProbe[ezoCnt].value[0]);
                            for (int i2 = 1; i2 < (int)pgm_read_word(&(ezoValCnt[recEzo])); i2++){
                                Serial.print(" , ");
                                Serial.print(ezoProbe[ezoCnt].value[i2]);
                            }          
                            Serial.println("");
                        }
                        else{
                            Serial.println("ERROR");
                        }
                        Serial.println("");
                        
                        // done
                        ezoCnt++;
                    }
                } 
                break;
            }
        }
    }
}

int EzoDoNext(){

    int err = 1;
    int errCnt = 0;

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
          err = IIcSetStr(ezoProbe[ezoAct].address, iicStr, 0);
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
              PrintErrorOK(1,ezoAct,(char*)"'T'");
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
          PrintErrorOK(1,ezoAct,(char*)"'R'");
        }          
      }
    
      break;
    
    case 2:
      // Get Data
      err = EzoGetValues(ezoAct);

      if (err == 0){
        // Immediately Fatal for this Probe
        PrintErrorOK(-1,ezoAct,(char*)"Data");
        err = - 1;
      }
      else{
        PrintErrorOK(1,ezoAct,(char*)"Data");
      }          
      break;

    default:
      ezoAction = 0;
      ezoAct = 0;
      return 0;
      break;
    }

    if (ezoAct == ezoCnt - 1){
        // Modules done in actual step
        ezoAct = 0;
        ezoAction++;
        if (ezoAction == 3){
            // All read
            return 1;
        }
    }
    else{
      // Next Module  
      ezoAct++;
    }

    if (err < 0){
        return -1;
    }
    else{
        return 0;
    }
    

}