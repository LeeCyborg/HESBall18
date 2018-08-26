void setup() {
  /* Pin assignments
   *  ===Colors===
   *  Red 2
   *  yellow 3
   *  green 4
   *  blue 5
   *  ===Additions===
   *  25 points 6
   *  10 points 7
   *  5 points 8
   *  1 point 9
   *  - points 10
   *  ============
   *  22-27 code buttons
   */
  int pinAssignments[18];
  for (int i = 2; i < 18; i++) {
    pinMode(i, INPUT_PULLUP);
  }

int codes[][6] =   { {1,2,3,4,5,6},
                     {6,5,4,3,2,1} };
}

void loop() {

}
bool checkIt(int code[]){
  return true;
}

