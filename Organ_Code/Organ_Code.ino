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
#include <Adafruit_NeoPixel.h>
#define PIN 13
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

int debugMode = true;
int pinAssignments[18] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 22, 23, 24, 25, 26, 27};
int currentCode[6];
int index = 0;
int currentColor;
int pointAmounts[4] = {1, 5, 10, 25};
int currentPoints[4] = {0, 0, 0, 0}; // red >> yellow >> green >> blue
int codes[2][7] = {
  {1, 1, 1, 1, 1, 1, 10},
  {2, 2, 2, 2, 2, 2, 10}
};
void setup() {
  for (int i = 0; i < 15; i++) {
    pinMode(pinAssignments[i], INPUT_PULLUP);
  }
  Serial.begin(9600);
  strip.begin();
  strip.show();
}

void loop() {
  if (debugMode) {
    debug();
  }
  //Serial.println(array_cmp(testArray, codes[0]));
  //Serial.println(sizeof(codes));
  colorChecker();
  code();
  smolPoints();

}
void code() {
  for (int i = 8; i < 14; i++) {
    if (digitalRead(pinAssignments[i]) == LOW) {
      currentCode[index] = int(coderDecoder(pinAssignments[i]));
      index++;
      blinkTimes(3, 30);
      if (index == 6) {
        checkIt(currentCode);
        resetCode();
      }
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
  for (int i; i < 2; i++) {
    array_cmp(currentCode, codes[i]);
  }
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
    colorWipe(strip.Color(255, 0, 0), 50); // Red

  }
  if (color == 1) {
    Serial.println("YELLOW");
    currentColor = 1;
    colorWipe(strip.Color(255, 255, 0), 50); // Red

  }
  if (color == 2) {
    Serial.println("GREEN");
    currentColor = 2;
    colorWipe(strip.Color(0, 255, 0), 50); // Red

  }
  if (color == 3) {
    Serial.println("BLEW");
    currentColor = 3;
    colorWipe(strip.Color(0, 0, 255), 50); // Red

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
boolean array_cmp(int a[6], int b[6]) {
  int n;
  for (n = 0; n < 6; n++) {
    Serial.print("Array I Made value:");
    Serial.println(a[n]);
    Serial.print("Array I Stored value:");
    Serial.println(b[n]);
    if (a[n] != b[n]) {
      Serial.println("Codes dont match");
      blinkTimes(2, 1000);
      return false;
    }
  }
  addPoints(b[6]);
  displayPoints();
  Serial.println("Codes match");
  blinkTimes(30, 30);
  return true;
}

void addPoints (int points) {
  currentPoints[currentColor] += points;
}

void displayPoints() {
  for (int i = 0; i < 4   ; i++) {
    Serial.print("This is the points for team #");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(currentPoints[i]);
  }
}

void smolPoints() {
  for (int i = 3; i < 8; i++) {
    if (digitalRead(pinAssignments[i]) == LOW) {
      Serial.println(i);
      if (i == 7) {
        addPoints(1);
        blinkTimes(1, 200);
      }
      if (i == 4) {
        addPoints(5);
        blinkTimes(5, 200);
      }
      if (i == 5) {
        addPoints(10);
        blinkTimes(10, 200);
      }
      if (i == 6) {
        addPoints(25);
        blinkTimes(25, 200);
      }
      delay(300);
    }
  }
  displayPoints();
}
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}


void blinkTimes(int times, int duration) {
  for (int i = 0; i < times; i++) {
    colorWipe(strip.Color(255, 255, 255), 50); // Red
    delay(duration);
    colorWipe(strip.Color(0, 0, 0), 50); // Red
    delay(duration);
  }
  lightsColorMaker(currentColor);
}

