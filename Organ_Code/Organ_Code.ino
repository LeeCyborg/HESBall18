/* Pin assignments
    ===Colors===
    Red 2
    yellow 3
    green 4
    blue 5
    ===Additions===
    25 points 6
    10 points 7
    5 points 8
    1 point 9
    - points 10
    ============
    22-27 code buttons
*/
int debugMode = true;
int pinAssignments[18] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 22, 23, 24, 25, 26, 27};
int currentCode[6];
int testArray[6] = {1, 1, 1, 1, 1, 1};
int index = 0;;
int currentColor;
int currentPoints[4] = {0, 0, 0, 0}; // red >> yellow >> green >> blue
int codes[2][6] = {
  {1,1,1,1,1,1}
};
void setup() {
  for (int i = 0; i < sizeof(pinAssignments); i++) {
    pinMode(pinAssignments[i], INPUT_PULLUP);
  }
  Serial.begin(9600);

}

void loop() {
  if (debugMode) {
    debug();
  }
  colorChecker();
  code();

}
void code() {
  for (int i = 8; i < 14; i++) {
    if (digitalRead(pinAssignments[i]) == LOW) {
        //Serial.println(array_cmp(currentCode, codes[0]));
      if (index == 6) {
        checkIt(currentCode);
        resetCode();
      }
      currentCode[index] = int(coderDecoder(pinAssignments[i]));
      index++;
      delay(1000);
      if (debugMode) {
        Serial.print("You Pressed:" );
        Serial.println(pinAssignments[i]);
        Serial.print("The Code is:");
        Serial.print(currentCode[0]);
        Serial.print(" , ");
        Serial.print(currentCode[1]);
        Serial.print(" , ");
        Serial.print(currentCode[2]);
        Serial.print(" , ");
        Serial.print(currentCode[3]);
        Serial.print(" , ");
        Serial.print(currentCode[4]);
        Serial.print(" , ");
        Serial.println(currentCode[5]);
      }
    }
  }
}
bool checkIt(int code) {
  array_cmp(testArray, codes[0]);
}
void colorChecker() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(pinAssignments[i]) == LOW) {
      lightsColorMaker(i);
      return;
    }
  }
}
void lightsColorMaker(int color) {
  if (color == 0) {
    Serial.println("RED");
    currentColor = 0;
  }
  if (color == 1) {
    Serial.println("YELLOW");
    currentColor = 1;
  }
  if (color == 2) {
    Serial.println("GREEN");
    currentColor = 2;
  }
  if (color == 3) {
    Serial.println("BLEW");
    currentColor = 3;
  }
}
void resetCode() {
  for (int i = 0; i < 6; i++) {
    currentCode[i] = 0;
  }
  index = 0;
}

int coderDecoder(int whichPin) {
  if (whichPin == 27) {
    return 6;
  }
  if (whichPin == 26) {
    return 5;
  }
  if (whichPin == 25) {
    return 4;
  }
  if (whichPin == 24) {
    return 3;
  }
  if (whichPin == 23) {
    return 2;
  }
  if (whichPin == 22) {
    return 1;
  }
}
void debug() {
  //Serial.print("Current Color is: ");
  //Serial.println(currentColor);


}
boolean array_cmp(int a[6], int b[6]){
  int n;
  for (n = 0; n < 6; n++) {
    Serial.print("Array I Made value:");
    Serial.println(a[n]); 
    Serial.print("Array I Stored value:");
    Serial.println(b[n]);
    if (a[n] != b[n]) {
       Serial.println("Codes dont match");
      return false;
    }
  }
  Serial.println("Codes match");
  return true;
}

