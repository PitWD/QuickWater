#define fgBlack 30
#define fgRed 31
#define fgGreen 32
#define fgYellow 33
#define fgBlue 34
#define fgMagenta 35
#define fgCyan 36
#define fgWhite 37

#define bgBlack 40
#define bgRed 41
#define bgGreen 42
#define bgYellow 43
#define bgBlue 44
#define bgMagenta 45
#define bgCyan 46
#define bgWhite 47

#define fgBlackB 90
#define fgRedB 91
#define fgGreenB 92
#define fgYellowB 93
#define fgBlueB 94
#define fgMagentaB 95
#define fgCyanB 96
#define fgWhiteB 97

#define bgBlackB 100
#define bgRedB 101
#define bgGreenB 102
#define bgYellowB 103
#define bgBlueB 104
#define bgMagentaB 105
#define bgCyanB 106
#define bgWhiteB 107

void EscLocate(int x, int y){
  Serial.print(F("\x1B["));
  Serial.print(y);
  Serial.print(F(";"));
  Serial.print(x);
  Serial.print(F("H"));
}
void EscCls(){
  Serial.print(F("\x1B[2J"));
}
void EscBold(int set){
  if (set){
    Serial.print(F("\x1B[1m"));
  }
  else{
    Serial.print(F("\x1B[22m"));
  } 
}
void EscFaint(int set){
  if (set){
    Serial.print(F("\x1B[2m"));
  }
  else{
    EscBold(0);
  } 
}
void EscColor(int color){
  if (!color){
    color = 39;
  }
  Serial.print(F("\x1B["));
  Serial.print(color);
  Serial.print(F("m"));
  if (color == 39){
    EscColor(49);
  }
  
}
void EscInverse(int set){
 	if (set) {
		// Set
		Serial.print(F("\x1B[7m"));
	}
	else {
		// Reset
		Serial.print(F("\x1B[27m"));
	}
}
void EscCursorVisible(int set){
	Serial.print(F("\x1B?25"));
	if (set){
		// visible
		Serial.print(F("h"));
	}
	else{
		// invisible
		Serial.print(F("l"));
	}	
}
void EscCursorLeft(int cnt){
    Serial.print(F("\x1B["));
    Serial.print(cnt);
    Serial.print(F("D"));
}
