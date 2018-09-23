#include <Adafruit_NeoPixel.h>

// Debug defines
#define DEBUG_MODE 0
#define VERBOSE 1
#define VERY_VERBOSE 0

#if DEBUG_MODE
#define DEBUG_DELAY 1000
#endif

#if DEBUG_MODE || VERBOSE
#define DEBUG_MAX_STRING_LEN 32
#define DEBUG_MSG_STR "!DEBUG! "
#define DEBUG_MATCH_STR "Codes match"
#define DEBUG_NOMATCH_STR "Codes don't match"
#endif

// Hardware defines
//55
#define NEOPIX_NUM 55
#define NEOPIX_PIN 13

// Serial defines
#define SERIAL_SCORE_UPDATE_STR "!UPDATE! "
#define SERIAL_SCORE_TEAM_DELI ", "        // Delimiter between teams
#define SERIAL_SCORE_POINT_DELI ": "       // Delimiter between team and points

// "Number of" defines
#define NUM_TEAMS 4
#define NUM_CODES 51
#define NUM_COLORS NUM_TEAMS
#define NUM_POINT_PINS 5
#define NUM_BLINK_TIMES_VALUES 2
#define NUM_SMALL_POINT_VALUES 5
#define CODE_ENTRY_PULSES 1

// Delay defines
#define CODE_SCAN_DELAY 200

#define COLOR_PULSE_DELAY 0
#define MID_COLOR_PULSE_DELAY 0

#define IDLE_PULSE_DELAY 10
#define MID_WIN_PULSE_DELAY 50

#define SMALL_POINTS_DELAY 300
#define SMALL_POINTS_BLINK_DURATION 100

// Length defines
#define CODE_LENGTH 6
#define LONG_CODE_LENGTH 7
#define MAX_COLOR_STRING 8

// Brightness defines
#define MIN_BRIGHTNESS 24
#define MAX_BRIGHTNESS 255
#define BRIGHTNESS_STEP 2

// Lose Animation defines
#define LOSE_MIN_BRIGHTNESS 0
#define LOSE_BRIGHTNESS_STEP 2

#define LOSE_PULSE_DELAY 1
#define LOSE_MID_PULSE_DELAY 1000

// Idle Animation defines
#define DO_IDLE 1
#define IDLE_ANIMATION_JUMP_DELAY 25
#define IDLE_TIMEOUT (10 * (unsigned long)1000) //10 = seconds

#define NULL_BYTE_ARRAY { '\0' };

// Enums
typedef enum TeamColors {
  RED,
  YELLOW,
  GREEN,
  BLUE //BLEW
} TeamColors;

// Structs
typedef struct PatternInputs {
  int times;
  int duration;
} PatternInputs;

#if DEBUG_MODE || VERBOSE
// Debug variabes
char debugString[DEBUG_MAX_STRING_LEN] = NULL_BYTE_ARRAY;
#endif

// NeoPixel Variables
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIX_NUM, NEOPIX_PIN, NEO_GRB + NEO_KHZ800);

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
int codePosition = 0;
int currentCode[CODE_LENGTH] = {0, 0, 0, 0, 0, 0};
const char colorStrings[NUM_COLORS][MAX_COLOR_STRING] =
{ "RED",
  "YELLOW",
  "GREEN",
  "BLEW"
};

const PatternInputs codeMatchPi   = {30, 30};
const PatternInputs codeNoMatchPi = {2,  1000};


// Color Variables
int currentColor; // Is also current team

// These MUST corispond to the order of enum TeamColors
int colorIntensities[NUM_COLORS] = {50, 50, 50, 50};
uint32_t colorValues[NUM_COLORS] =
{
  strip.Color(255, 0,   0),
  strip.Color(255, 255, 0),
  strip.Color(0,   255, 0),
  strip.Color(0,   0,   255)
};

uint32_t lowColorValues[NUM_COLORS] =
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
int smallPointValues[NUM_SMALL_POINT_VALUES] = {1, 5, 10, 25, -10};
int codes[NUM_CODES][LONG_CODE_LENGTH] = {

  //First Hour - 20 Codes (15?)

  { 1, 4, 3, 3, 2, 2, 25 }, //1
  { 1, 6, 2, 4, 3, 6, 25 }, //2
  { 4, 2, 3, 5, 4, 3, 25 }, //3
  { 1, 5, 4, 4, 6, 5, 25 }, //4
  { 6, 5, 3, 4, 1, 2, 25 }, //5

  { 3, 2, 1, 4, 3, 3, 25 }, //6
  { 4, 3, 2, 5, 1, 2, 25 }, //7
  { 1, 6, 5, 2, 1, 4, 25 }, //8
  { 6, 4, 2, 6, 5, 2, 25 }, //9
  { 2, 3, 4, 6, 3, 1, 25 }, //10

  { 4, 5, 1, 4, 3, 4, 25 }, //11
  { 2, 1, 3, 2, 6, 1, 25 }, //12
  { 3, 4, 3, 1, 2, 4, 25 }, //13
  { 1, 1, 3, 2, 3, 1, 25 }, //14
  { 2, 1, 4, 4, 3, 3, 25 }, //15

  //Second Hour - 24 Codes (17?)

  { 1, 2, 1, 4, 5, 1, 25 }, //16
  { 2, 3, 1, 5, 6, 1, 25 }, //17
  { 3, 1, 4, 1, 2, 4, 25 }, //18
  { 6, 1, 4, 5, 4, 3, 25 }, //19
  { 5, 3, 3, 2, 2, 4, 25 }, //20
  { 2, 3, 6, 5, 1, 5, 25 }, //21

  { 5, 3, 1, 6, 1, 1, 25 }, //22
  { 2, 5, 6, 6, 6, 1, 25 }, //23
  { 1, 6, 2, 3, 3, 4, 25 }, //24
  { 1, 2, 3, 5, 3, 3, 25 }, //25
  { 5, 4, 1, 2, 2, 1, 25 }, //26
  { 4, 6, 3, 6, 1, 2, 25 }, //27

  { 5, 2, 1, 2, 5, 3, 25 }, //28
  { 3, 2, 5, 1, 5, 6, 25 }, //29
  { 1, 2, 6, 1, 6, 1, 25 }, //30
  { 4, 4, 4, 6, 5, 3, 25 }, //31
  { 6, 1, 5, 3, 1, 4, 25 }, //32
  { 1, 4, 2, 2, 5, 1, 25 }, //33

  //Third Hour - 24 Codes (17?)

  { 2, 3, 5, 2, 5, 5, 25 }, //34
  { 4, 6, 1, 2, 3, 2, 25 }, //35
  { 1, 2, 2, 1, 3, 1, 25 }, //36
  { 3, 1, 5, 3, 1, 6, 25 }, //37
  { 2, 1, 5, 1, 1, 4, 25 }, //38
  { 4, 5, 1, 3, 6, 6, 25 }, //39

  { 5, 2, 3, 5, 2, 3, 25 }, //40
  { 4, 2, 1, 4, 3, 5, 25 }, //41
  { 5, 3, 1, 6, 1, 5, 25 }, //42
  { 1, 3, 3, 2, 1, 4, 25 }, //43
  { 1, 2, 5, 3, 3, 4, 25 }, //44
  { 4, 2, 6, 1, 1, 1, 25 }, //45

  { 4, 1, 5, 5, 1, 2, 25 }, //46
  { 1, 2, 4, 5, 5, 3, 25 }, //47
  { 4, 6, 1, 2, 4, 4, 25 }, //48
  { 2, 2, 5, 4, 1, 6, 25 }, //49
  { 2, 6, 1, 2, 2, 4, 25 }, //50
  { 1, 1, 3, 4, 1, 5, 25 }, //51
};

int wasInIdle = 0;

#if DO_IDLE
unsigned long startIdleTime = 0;
#endif

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
  Serial.println("Arduino READY!");

  // Start NeoPixels
  strip.begin();
  strip.show();

  // Initial display of score
  displayPoints();

  resetIdleTimer();
}


/*###########
  ## Main Loop
  ## Main program loop
  #
  # Takes: Void
  # Returns: Void
  ###########*/
void loop() {
#if DO_IDLE
  checkIdleTimer();
#endif

#if DEMUG_MODE
  debug();
  //Serial.println(array_cmp(testArray, codes[0]));
  //Serial.println(sizeof(codes));
#endif

  // Detect which team color is selected
  colorScan();

  // Detect which code buttons are pressed
  codeScan();

  // Award small amounts of points for button presses
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
#if DO_IDLE
      // Reset the idle timer
      resetIdleTimer();
#endif

      // Set the color
      setColor(i);

      resetCode();

      // End the loop
      break;
    }
  }
}

#if DO_IDLE
void checkIdleTimer() {
  if (startIdleTime + IDLE_TIMEOUT < millis()) {
    idleAnimation();
    wasInIdle = 1;
  }
}

void resetIdleTimer() {
  startIdleTime = millis();
  setColor(currentColor);

  if (wasInIdle) {
    wasInIdle = 0;
    //resetCode();
  }
}
#endif

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
  colorWipe(colorValues[color]);

#if DEBUG_MODE || VERBOSE
  // Print the color to the serial console
  Serial.print(DEBUG_MSG_STR);
  Serial.print("Team: ");
  Serial.println(colorStrings[color]);
#if DEBUG_MODE
  delay(DEBUG_DELAY);
#endif
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
#if DO_IDLE
      // Reset the idle timer
      resetIdleTimer();
#endif

      // Store this code position's input
      currentCode[codePosition] = i + 1; // + 1 to use 1 index vs 0 index

      // Advance to the next code position for the next code
      codePosition++;

#if DEBUG_MODE || VERBOSE
      Serial.print(DEBUG_MSG_STR);
      Serial.print("Pressed: " );
      Serial.print(codePins[i]);

      Serial.print(", Code: ");
      Serial.print(currentCode[0]);
      Serial.print(",");
      Serial.print(currentCode[1]);
      Serial.print(",");
      Serial.print(currentCode[2]);
      Serial.print(",");
      Serial.print(currentCode[3]);
      Serial.print(",");
      Serial.print(currentCode[4]);
      Serial.print(",");
      Serial.println(currentCode[5]);
#if DEBUG_MODE
      delay(DEBUG_DELAY);
#endif
#endif

      // Blink an arbitrary number of times
      colorPulse(CODE_ENTRY_PULSES);

      if (codePosition == CODE_LENGTH) {
        checkIt();
        resetCode();
      }

      delay(CODE_SCAN_DELAY);
    }
  }
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

  // Go through all of the codes
  for (i = 0; i < NUM_CODES; i++) {
    // Compare the code we are checking against the inputed code
    if ((isCodeCorrect = array_cmp(currentCode, codes[i]))) {
      // If it did match, finish searching
      break;
    }
  }

  // The code was correct!
  if (isCodeCorrect) {
#if DEBUG_MODE || VERBOSE
    // Match found message
    strcpy(debugString, DEBUG_MATCH_STR);
#endif

    // Have the points added to the current team
    addPoints(codes[i][6]);

    winAnimation();
  }
  // The code was INcorrect!
  else {
#if DEBUG_MODE || VERBOSE
    // Match not found message
    strcpy(debugString, DEBUG_NOMATCH_STR);
#endif

    loseAnimation();
  }

#if DEBUG_MODE || VERBOSE
  Serial.print(DEBUG_MSG_STR);
  Serial.println(debugString);
#if DEBUG_MODE
  delay(DEBUG_DELAY);
#endif
#endif
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
  ## Array Cmp
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

#if DEBUG_MODE && VERY_VERBOSE
    Serial.print(DEBUG_MSG_STR);
    Serial.print("Input Array:");
    Serial.print(a[n]);

    Serial.print(":Stored Array:");
    Serial.println(b[n]);
    delay(DEBUG_DELAY);
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
  ## Add Points
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
  Serial.print(SERIAL_SCORE_UPDATE_STR);

  for (int i = 0; i < NUM_TEAMS; i++) {
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
#if DO_IDLE
      // Reset the idle timer
      resetIdleTimer();
#endif

#if DEBUG_MODE || VERBOSE
      Serial.print(DEBUG_MSG_STR);
      Serial.print("Small points index: ");
      Serial.println(i);
#if DEBUG_MODE
      delay(DEBUG_DELAY);
#endif
#endif

      int addSmallPoints = smallPointValues[i];

      addPoints(addSmallPoints);

      colorPulse(abs(addSmallPoints));

      displayPoints();

      delay(SMALL_POINTS_DELAY);
    }
  }
}


/*###########
  ## Color Wipe
  ##
  # Takes: void
  # Returns: Void
  ###########*/
void colorWipe(uint32_t toColor) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, toColor);
    strip.show();
  }
}


/*###########
  ## Color Pulse
  ##
  # Takes: void
  # Returns: Void
  ###########*/
void colorPulse(int times) {
  for (int i = 0; i < times; i += BRIGHTNESS_STEP) {
    for (int i = MAX_BRIGHTNESS; i < MIN_BRIGHTNESS; i--) {
      strip.setBrightness(i);
      strip.show();
      delay(COLOR_PULSE_DELAY);
    }

    delay(MID_COLOR_PULSE_DELAY);

    for (int i = MIN_BRIGHTNESS; i < MAX_BRIGHTNESS; i += BRIGHTNESS_STEP) {
      strip.setBrightness(i);
      strip.show();
      delay(COLOR_PULSE_DELAY);
    }

    delay(MID_COLOR_PULSE_DELAY);
  }
}


/*###########
  ## Color Pulse (Overloaded)
  ###########*/
void colorPulse(PatternInputs *pi) {
  colorPulse(pi->times);
}


/*###########
  ## Win Animation
  ## The NeoPixel animation that plays a valid code has been entered
  ## modified theater chase,
  ## pass through with only value of current color 0-3 r y g b
  #
  # Called by: CheckIt
  #
  # Takes: void
  # Returns: Void
  ###########*/
void winAnimation() {
  uint32_t currColor = colorValues[currentColor];

  uint32_t winColors[4] =
  {
    strip.Color(255, 0,   0),
    strip.Color(255, 255, 0),
    strip.Color(0,   255, 0),
    strip.Color(0,   0,   255)
  };

  uint32_t winColor = winColors[currentColor];

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      for (uint16_t k = 0; k < strip.numPixels(); k += 3) {
        strip.setPixelColor(k + j, winColor);
        strip.show();
      }

      delay(MID_WIN_PULSE_DELAY);

      for (uint16_t l = 0; l < strip.numPixels(); l += 3) {
        strip.setPixelColor(l + j, 0);
        strip.show();
      }
    }
  }

  delay(50);

  // Reset to team color
  for (uint16_t m = 0; m < strip.numPixels(); m++) {
    strip.setPixelColor(m, currColor);
  }

  strip.show();
}


/*###########
  ## Lose Animation
  ## The NeoPixel animation that plays an invalid code has been entered
  #
  # Called by: CheckIt
  #
  # Takes: void
  # Returns: Void
  ###########*/
void loseAnimation() {
  for (int i = MAX_BRIGHTNESS; i > LOSE_MIN_BRIGHTNESS; i -= LOSE_BRIGHTNESS_STEP) {
    strip.setBrightness(i);
    strip.show();

    delay(LOSE_PULSE_DELAY);
  }

  delay(LOSE_MID_PULSE_DELAY);

  for (int i = MIN_BRIGHTNESS; i < MAX_BRIGHTNESS; i += LOSE_BRIGHTNESS_STEP) {
    strip.setBrightness(i);
    strip.show();

    delay(LOSE_PULSE_DELAY);
  }
}


/*###########
  ## Idle Animation
  ## The NeoPixel animation that plays while idle
  #
  # Called by: Setup, loop
  #
  # Takes: void
  # Returns: Void
  ###########*/

void idleAnimation() {
  static float sPos = 0.0;
  static int sX = 0;
  static int sI = 0;

  unsigned long idleTimer = millis();

  // reset sI
  if (sI >= NEOPIX_NUM) {
    sI = 0;
  }

  // reset sX
  if (sX >= NEOPIX_NUM) {
    sX = 0;
    sPos = 0.0;
  }

  for (int x = sX; x < NEOPIX_NUM; x++, sX++) {
    sPos += 0.1;

    for (int i = sI; i < NEOPIX_NUM; i++, sI++) {
      float wave = sin(i + sPos) * 127 + 200;

      switch (currentColor) {
        case 0:
          strip.setPixelColor(
              i,
              (int)random(wave),
              (int)random(wave) / 20,
              0
            ); // reds
        break;

        case 1:
          strip.setPixelColor(
              i,
              (int)random(wave),
              (int)random(wave),
              0
            ); /// yellows
        break;

        case 2:
          strip.setPixelColor(
            i,
            0,
            (int)random(wave),
            (int)random(wave) / 20
          ); // greens
        break;

        case 3:
          strip.setPixelColor(
              i,
              (int)random(wave) / 20,
              0,
              (int)random(wave)
            ); // blue
        break;
      };

      strip.show();

      delay(IDLE_PULSE_DELAY);

      if (idleTimer + IDLE_ANIMATION_JUMP_DELAY > millis()) {
        break;
      }
    }
  }
}

void idleAnimation2() {
  float pos = 0.0;
  uint16_t numPix = strip.numPixels();

  for (uint16_t x = 0; x < numPix; x++) {
    pos += 0.1;

    for (uint16_t i = 0; i < numPix; i++) {
      float wave = sin(i + pos) * 127 + 200;

      switch (currentColor) {
        case 0:
          strip.setPixelColor(i, (int)random(wave), (int)random(wave) / 20, 0); // reds
        break;

        case 1:
          strip.setPixelColor(i, (int)random(wave), (int)random(wave), 0);      // yellows
        break;

        case 2:
          strip.setPixelColor(i, 0, (int)random(wave), (int)random(wave) / 20); // greens
        break;

        case 3:
          strip.setPixelColor(i, (int)random(wave) / 20, 0, (int)random(wave)); // blue
        break;
      };

      strip.show();

      delay(10);
    }
  }
}
