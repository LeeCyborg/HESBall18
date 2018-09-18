
#include <Adafruit_NeoPixel.h>

// Debug defines
#define DEBUG_MODE 1

#if DEBUG_MODE
#define MAX_DEBUG_STRING_LEN 32
#define DEBUG_MATCH_STR "Codes match"
#define DEBUG_NOMATCH_STR "Codes dont match"
#define DEBUG_MSG_STR "!DEBUG! "
#endif

// Hardware defines
#define NEOPIX_PIN 13

// Serial defines
#define SERIAL_SCORE_UPDATE_STR "!UPDATE! "
#define SERIAL_SCORE_TEAM_DELI ", "        // Delimiter between teams
#define SERIAL_SCORE_POINT_DELI ": "       // Delimiter between team and points

// "Number of" defines
#define NUM_CODES 2
#define NUM_TEAMS 4
#define NUM_COLORS NUM_TEAMS
#define NUM_POINT_PINS 5
#define NUM_BLINK_TIMES_VALUES 2
#define NUM_SMALL_POINT_VALUES 4

// Delay defines
#define CODE_SCAN_DELAY 1000
#define SMALL_POINTS_DELAY 300
#define SMALL_POINTS_BLINK_DURATION 200

// Length defines
#define CODE_LENGTH 6
#define LONG_CODE_LENGTH 7

#define MAX_COLOR_STRING 8

#define NULL_BYTE_ARRAY { '\0' };

// Enums
typedef enum TeamColors {
  RED,
  YELLOW,
  GREEN,
  BLUE //BLEW
} TeamColors;

// Structs
typedef struct BlinkTimesStruct {
  int times;
  int duration;
} BlinkTimesStruct;

#if DEBUG_MODE
// Debug variabes
char debugString[MAX_DEBUG_STRING_LEN] = NULL_BYTE_ARRAY;
#endif

// NeoPixel Variables
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, NEOPIX_PIN, NEO_GRB + NEO_KHZ800);

// Hardware Variables

/* Pin assignments
    ===Colors===
    Red 2
    yellow 3
    green 4
    blue 5
*/
int colorPins[NUM_COLORS] = {2, 3, 4, 5};

/* Pin assignments
    ===Additions===
    25 points 6
    10 points 7
    5 points 8
    1 point 9
    - points 10
*/
int pointsPins[NUM_POINT_PINS] = {6, 7, 8, 9, 10};

/* Pin assignments
    Code Buttons
    ============
    22-27
*/
int codePins[CODE_LENGTH] = {22, 23, 24, 25, 26, 27};


// Code variables
int currentCode[CODE_LENGTH] = {0, 0, 0, 0, 0, 0};
int codePosition = 0;
const char colorStrings[NUM_COLORS][MAX_COLOR_STRING] =
  { "RED",
    "YELLOW",
    "GREEN",
    "BLEW"
  };

const BlinkTimesStruct codeMatchBts   = {30, 30};
const BlinkTimesStruct codeNoMatchBts = {2,  1000};


// Color Variables
int currentColor;

// These MUST corispond to the order of enum TeamColors
int colorIntensities[NUM_COLORS] = {50, 50, 50, 50};
uint32_t colorValues[NUM_COLORS] =
  {
    strip.Color(255, 0,   0),
    strip.Color(255, 255, 0),
    strip.Color(0,   255, 0),
    strip.Color(0,   0,   255)
  };

int blinkTimesIntensity[NUM_BLINK_TIMES_VALUES] = {50, 50};
uint32_t blinkTimesValues[NUM_BLINK_TIMES_VALUES] =
  {
    strip.Color(255, 255, 255),
    strip.Color(0, 0, 0)
  };

// Team Variables
int teamScores[NUM_TEAMS] = {0, 0, 0, 0}; // red >> yellow >> green >> blue

// Game Variables
int smallPointValues[NUM_SMALL_POINT_VALUES] = {1, 5, 10, 25};
int codes[NUM_CODES][LONG_CODE_LENGTH] = {
  {1, 1, 1, 1, 1, 1, 10},
  {2, 2, 2, 2, 2, 2, 10}
};



void setup() {
  // Initilize input pins mode
  for (int i = 0; i < NUM_COLORS; i++) {
    pinMode(colorPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUM_POINT_PINS; i++) {
    pinMode(pointsPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < CODE_LENGTH; i++) {
    pinMode(codePins[i], INPUT_PULLUP);
  }

  // Start serial and set bitrate
  Serial.begin(9600);

  // Start NeoPixels
  strip.begin();
  strip.show();
}

/*###########
## Main Loop
## Main program loop
#
# Takes: Void
# Returns: Void
###########*/
void loop() {
#if DEMUG_MODE
    debug();
    //Serial.println(array_cmp(testArray, codes[0]));
    //Serial.println(sizeof(codes));
#endif

  // Detect which team color is selected
  colorScan();

  // Detect which code buttons are pressed
  codeScan();

  //
  smolPoints();
}


#if DEMUG_MODE
/*###########
## Debug
## Prints debug information to serial
#
# Called by: loop
#
# Takes: Void
# Returns: Void
###########*/
void debug() {
  Serial.print("Current Color is: ");
  Serial.println(currentColor);
}
#endif


/*###########
## Color Scanner
## Iterates through the team color inputs and returns upon a press
#
# Called by: loop
#
# Takes: Void
# Returns: Void
###########*/
void colorScan() {
  // Go through the colors
  for (int i = 0; i < NUM_COLORS; i++) {
    // If the pin is being pulled down by a button push
    if (LOW == digitalRead(colorPins[i])) {
      // Set the color
      setColor(i);

      // End the loop
      break;
    }
  }
}

/*###########
## Set Color
## Records the current color, and displays it physically (lights)
#
# Called by: colorScan, blinkTimes
#
# Takes: const int color: the color to set too
# Returns: Void
###########*/
void setColor(const int color) {
  // Record the color
  currentColor = color;

  //Run colorWipe in the new color
  colorWipe(colorValues[color], colorIntensities[color]);

#if DEBUG_MODE
  // Print the color to the serial console
  Serial.print(DEBUG_MSG_STR);
  Serial.println(colorStrings[color]);
#endif

}


/*###########
## Code Scan
## Detect which code buttons are pressed
#
# Called by: loop
#
# Takes: void
# Returns: Void
###########*/
void codeScan() {
  // Iterate through the code pins
  for (int i = 0; i < CODE_LENGTH; i++) {
    // If the pin is being pulled down by a button push
    if (LOW == digitalRead(codePins[i])) {
      // Store this code position's input
      currentCode[codePosition] = i + 1; // + 1 to use 1 index vs 0 index

      // Advance to the next code position for the next code
      codePosition++;

      // Blink an arbitrary number of times
      blinkTimes(3, 30);

      //TODO: This shouldn't be here...
      if (codePosition == CODE_LENGTH) {
        checkIt();
        resetCode();
      }

      delay(CODE_SCAN_DELAY);

#if DEMUG_MODE
      Serial.print("You Pressed:" );
      Serial.println(inputPins[i]);
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
#endif
    }
  }
}


/*###########
## Blink Times
## Blinks a number of times with a delay between, then resets to team color
#
# Called by: codeScan, array_cmp, smolPoints
# Takes: int times: The number of times to "blink"
#        int duration: The ammount of time to wait between "blinks"
#
# Returns: Void
###########*/
void blinkTimes(int times, int duration) {
  // "Blink" number of times
  for (int i = 0; i < times; i++) {
    // Each value in a blink
    for(int j = 0; j < NUM_BLINK_TIMES_VALUES; j++) {
      // Perform the NeoPixel display
      colorWipe(blinkTimesValues[j], blinkTimesIntensity[j]);

      // Wait amount of time between "blinks"
      delay(duration);
    }
  }

  // Reset back to the team color
  setColor(currentColor);
}

/*###########
## Blink Times (Overloaded)
###########*/
void blinkTimes(BlinkTimesStruct *bts) {
  blinkTimes(bts->times, bts->duration);
}

/*###########
## CheckIt
## Compares all of the good codes to the inputted code
#
# Called by: codeScan
#
# Takes: void
# Returns: Void
###########*/
void checkIt() {
  int i;
  bool isCodeCorrect = false;
  BlinkTimesStruct *codeBts = (BlinkTimesStruct*)&codeNoMatchBts;

  // Go through all of the codes
  for (i = 0; i < NUM_CODES; i++) {
    // Compare the code we are checking against the inputed code
    if ((isCodeCorrect = array_cmp(currentCode, codes[i]))) {
      // If it did match, finish searching
      break;
    }
  }

  if (isCodeCorrect) {
#if DEBUG_MODE
    strcpy(debugString, DEBUG_MATCH_STR);
#endif

    // Have the points added to the current team
    addPoints(codes[i][6]);

    codeBts = (BlinkTimesStruct*)&codeMatchBts;
  }
  else {
#if DEBUG_MODE
    strcpy(debugString, DEBUG_NOMATCH_STR);
#endif
  }

#if DEBUG_MODE
  Serial.print(DEBUG_MSG_STR);
  Serial.println(debugString);
#endif

  blinkTimes(codeBts);

}


/*###########
## resetCode
## Resets the input code buffer and resets the code buffer index
#
# Called by: codeScan
#
# Takes: void
# Returns: Void
###########*/
void resetCode() {
  // Go through all of the digets of the code buffer
  for (int i = 0; i < CODE_LENGTH; i++) {
    // Reset the element to 0
    currentCode[i] = 0;
  }

  // Reset to code possition to 0
  codePosition = 0;
}


/*###########
## array_cmp
##
#
# Called by: CheckIt
#
# Takes: int a[6]
#        int b[6]
# Returns: bool
###########*/
bool array_cmp(int a[CODE_LENGTH], int b[CODE_LENGTH]) {
  for (int n = 0; n < CODE_LENGTH; n++) {

#if DEBUG_MODE
    Serial.print(DEBUG_MSG_STR);
    Serial.print("Array I Made value:");
    Serial.println(a[n]);

    Serial.print(DEBUG_MSG_STR);
    Serial.print("Array I Stored value:");
    Serial.println(b[n]);
#endif

    if (a[n] != b[n]) {
      // This array cannot be a match, bail early
      return false;
    }
  }

  // The array matched
  return true;
}


/*###########
## addPoints
##
#
# Called by: array_cmp
#
# Takes: int points: The number of points to add to the team
# Returns: Void
###########*/
void addPoints (int points) {
  // Add on points points to the teams score element
  teamScores[currentColor] += points;

  // Display all teams points
  displayPoints();
}


/*###########
## Display Points
## Print all the teams points to the serial console
#
# !UPDATE! RED: 1, YELLOW: 2, GREEN: 3, BLEW: 4/n
#
# Called by: addPoints, smolPoints
#
# Takes: void
# Returns: Void
###########*/
void displayPoints() {
  for (int i = 0; i < NUM_TEAMS; i++) {
    Serial.print(SERIAL_SCORE_UPDATE_STR);
    Serial.print(colorStrings[i]);
    Serial.print(SERIAL_SCORE_POINT_DELI);
    Serial.print(teamScores[i]);
    Serial.print(SERIAL_SCORE_TEAM_DELI);
  }

  // Send newline to clear buffer
  Serial.println();
}


/*###########
## Smol Points
##
#
# Called by: loop
#
# Takes: void
# Returns: Void
###########*/
void smolPoints() {
  for (int i = 0; i < NUM_POINT_PINS; i++) {
    if (LOW == digitalRead(pointsPins[i])) {

#if DEBUG_MODE
      Serial.print(DEBUG_MSG_STR);
      Serial.print("Small points index: ");
      Serial.println(i);
#endif

      if (i > 0) {
        int addSmallPoints = smallPointValues[i - 1];

        addPoints(addSmallPoints);
        blinkTimes(addSmallPoints, SMALL_POINTS_BLINK_DURATION);
      }

      delay(SMALL_POINTS_DELAY);
    }
  }

  displayPoints();
}


/*###########
## Color Wipe
##
# Takes: void
# Returns: Void
###########*/
void colorWipe(uint32_t c, uint8_t wait) {
  //TODO: Reimpliment wait usage, or remove wait parameter from all calls
  wait = wait; //Just because everything calls this function with a wait parameter, but it isn't used

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}
