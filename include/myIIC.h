#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "myESC.h"

#define IIC_STR_LEN 34
#define IIC_HLP_LEN 17

// global IIC I/O buffer
char iicStr[IIC_STR_LEN];
char strHLP[IIC_HLP_LEN];
char strHLP2[IIC_HLP_LEN];
char strDefault[IIC_HLP_LEN];

char IICgETsTRING(byte address, byte atlasValidity){

    // returns
    //          >0 = successful read (count of chars)
    // IIC_STR_LEN = (string-len overflow)
    //           0 = nothing to read
    //          -1 = still processing (atlas)
    //          -2 = syntax error (atlas)
    //          -3 = IIC error
    //          -4 = unknown error (atlas)
  
    byte count = 0;
    int i2c_error;
    byte firstChar = 1;

    iicStr[0] = 0;

    // Request up to IIC_STR_LEN bytes
    Wire.requestFrom(address, (int)IIC_STR_LEN);

    while (Wire.available() && count < IIC_STR_LEN - 1){

        char c = Wire.read();

        if (firstChar && atlasValidity){
            // 1st char in a atlas answer indicates the answers "quality"
            // 255 = no data to send
            // 254 = still processing
            // 2 = syntax error
            // 1 = valid

            if (c == 1){
                // Dummy to prevent break
                count = 1;
            }
            else if (c == 255){
                count = 0;
            }
            else if (c == 254){
                count = -1;
            }
            else if (c == 2){
                count = -2;
            }
            else{
                count = -4;
            }

            if (count < 1){
                break;
            }
            count = 0;
            firstChar = 0; 
        }
        else{
            // regular chars

            iicStr[count] = c;

            if (c == 0){
                // regular End Of String
                break;
            }

            count++;

            iicStr[count] = 0;
        }    
    }

    i2c_error = Wire.endTransmission();  // Check for I2C communication errors

    if (i2c_error == 0 || count < 0){
        return count;
    }
    else{
        return -3;
    }
}
#define IIcGetStr(address) IICgETsTRING(address, 0)
#define IIcGetAtlas(address) IICgETsTRING(address, 1)

char IIcSetStr(int address, char *strIN, char term){

    char length = strlen(strIN);

    if (!length){
        return 0;
    }

    Wire.beginTransmission(address);

    for (int i = 0; i < length + term; i++){
        Wire.write(strIN[i]);
    }

    int i2c_error = Wire.endTransmission();

    if (i2c_error == 0){
        return length;
    }
    else{
        return -1;
    }
}