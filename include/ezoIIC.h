#include "myIIC.h"

#define EZO_MAX_PROBES 8
#define EZO_MAX_VALUES 3        // 5 just for full RGB...
#define EZO_1st_ADDRESS 64
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

enum ezoType: int{
    ezoRTD = 1, ezoPH, ezoEC, ezoORP, ezoHUM, ezoCO2, ezoFLOW, ezoRGB, ezoDiO2
};

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
    ezoStrType_9
};

// Waittime for readings...
const int ezoWait[10] PROGMEM = {0, 600, 900, 600, 900, 300, 900, 300, 300, 600};

// Count of vals of probe
const int ezoValCnt[10] PROGMEM = {0, 1, 1, 1, 1, 3, 2, 2, 5, 2};

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

int EzoIntToStr(long val, int lz, int dp, char lc){

    // dp = decimal places
    // lz = leading zero's
    // lc = leading char for zero
    // return = position of decimal point

    // int (scaled by 1000)
    ltoa(val, strHLP, 10);
    int len = strlen(strHLP);

    if (len < 4){
        // value is < 1 (1000)
        memmove(&strHLP[4 - len], &strHLP[0], len);
        memset(&strHLP[0], '0', 4 - len);
        len = 4;
    }
    
    // Set leading zero's
    lz -= (len - 3);
    if (lz > 0){
        // space for missing zeros
        memmove(&strHLP[lz], &strHLP[0], len);
        // set missing zeros
        memset(&strHLP[0], lc, lz);
        // correct len
        len += lz;        
    }

    // shift dp's to set decimal point
    memmove(&strHLP[len -2], &strHLP[len - 3], 3);
    // set decimal point
    strHLP[len - 3] = '.';
    len++;

    // Trailing zero's
    lz = dp + lz - len + 2;
    if (lz > 0){
        // missing trailing zero's
        memset(&strHLP[len], '0', lz);
        len += lz;
    }

    // Return final decimal point
    lz = len - 4;

    // calculate decimal places
    if (dp > 0){
        // cut the too much dp's
        len -= 3 - dp;
    }
    else if (dp == 0){
        // integer
        len = lz;
        lz = 0;
    }

    // set EndOfString
    strHLP[len] = 0;

    return lz;
}

void EzoStartValues(int ezo){
    IIcSetStr(ezoProbe[ezo].address, (char*)"R");
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

void EzoScan(){
    // Scan for Ezo's

    int err;
    int recEzo;         // recognized EzoProbe[ezoCnt].module.version
    int verPos;         // 'pointer' on 1st char of version
    int hasCal;         // has a calibration
    
    int cntSetup;   // count of setup-lines to send
    char strSetup[6][9];

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
            err = IIcSetStr(i, (char*)"i");
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
                    cntSetup = 1;
                    verPos = 7;
                    hasCal = 1;
                    ezoProbe[ezoCnt].calibrated = 0;
                    ezoProbe[ezoCnt].name[0] = 0;
                    strcpy(strSetup[0],"L,1");  // Indicator LED

                    switch (iicStr[3]){
                    case 'R':
                        // RGB or RTD
                        switch (iicStr[4]){
                        case 'G':
                            // RGB
                            recEzo = ezoRGB;
                            hasCal = 0;
                            strcpy(strSetup[0],"iL,1");
                            strcpy(strSetup[1],"O,RGB,1");
                            strcpy(strSetup[2],"O,LUX,1");
                            strcpy(strSetup[3],"O,CIE,1");
                            cntSetup = 4;
                            break;
                        case 'T':
                            // RTD
                            recEzo = ezoRTD;
                            strcpy(strSetup[1],"S,c");  // Celsius (k = kelvin / f = fahrenheit)
                            cntSetup = 2;
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
                        strcpy(strSetup[1],"Frp,m");    // FlowRate/minute (s = second / h = hour)
                        strcpy(strSetup[2],"CF,0.001"); // Liter (1 would be ml)
                        strcpy(strSetup[3],"O,TV,1");   // Output Total Volume
                        strcpy(strSetup[4],"O,FR,1");   // Output Flow Rate
                        cntSetup = 5;
                        break;
                    case 'O':
                        // ORP
                        recEzo = ezoORP;
                        break;
                    case 'C':
                        // CO2
                        recEzo = ezoCO2;
                        strcpy(strSetup[1],"O,t,1");   // Output Internal Temp
                        cntSetup = 2;
                        break;
                    case 'H':
                        // HUM
                        recEzo = ezoHUM;
                        hasCal = 0;
                        strcpy(strSetup[1],"O,HUM,1"); // Humidity
                        strcpy(strSetup[2],"O,T,1");   // Temperature 
                        strcpy(strSetup[3],"O,Dew,1"); // Dewing point
                        cntSetup = 4;
                        break;    
                    case 'E':
                        // EC
                        recEzo = ezoEC;
                        verPos = 6;
                        strcpy(strSetup[1],"O,EC,1");
                        strcpy(strSetup[2],"O,TDS,0");
                        strcpy(strSetup[3],"O,S,0");
                        strcpy(strSetup[4],"O,SG,0");
                        strcpy(strSetup[5],"K,1.0");
                        cntSetup = 6;
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
                        strcpy(strSetup[1],"O,mg,1");
                        strcpy(strSetup[2],"O,%,1");
                        cntSetup = 3;
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
                        
                        // Write Setup
                        for (int i2 = 0; i2 < cntSetup; i2++){
                            IIcSetStr(i, strSetup[i2]);
                            delay(333);
                        }
                        // Extract Version
                        ezoProbe[ezoCnt].version = StrToInt(&iicStr[verPos], 0);
                        // Calibration
                        if (hasCal){
                            IIcSetStr(i,(char*)"Cal,?");
                            delay(333);
                            if (IIcGetAtlas(i) > 0){
                                ezoProbe[ezoCnt].calibrated = iicStr[5] - 48;
                            }                            
                        }
                        
                        // Name
                        IIcSetStr(i,(char*)"Name,?");
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
                        IIcSetStr(i, (char*)"Status");
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

