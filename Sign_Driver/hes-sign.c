/** Hand Eye Society Ball 2018 Team Score Sign Code
	* Edited and commented by: Jane Hacker (janehacker.com)
	* Originally included in the LEDscaper source code
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include "ledscape.h"


//#####
// Debug defines
//#####
#define VERBOSE 1
#define VERY_VERBOSE 0
#define BOARD_VERBOSE 0
#define BOARD_BLANK_LINE 40

//#####
// String defines
//#####
#define NULL_STRING "\0"

#define SCORE_STR_LEN 8
#define ARDUINO_LINE_OUT_MAX_LEN 128

// For the blank-out routines
#define BLANK_LEN 4
#define BLANK_CHAR 'X'

// Score inputs
#define CHECK_TRIGGER "!UPDATE!"
#define DEBUG_TRIGGER "!DEBUG! "
#define CHECK_TRIGGER_LEN 8
#define SCORE_SCAN_FORMAT "!UPDATE! RED: %i, YELLOW: %i, GREEN: %i, BLEW: %i,"

//#####
// Team defines
//#####
#define NUM_TEAMS 4
#define TEAM_NAME_RED "House Amiga"
#define TEAM_NAME_YELLOW "House Tudifore"
#define TEAM_NAME_GREEN "House Vivicus"
#define TEAM_NAME_BLUE "House Moonblossom"


//#####
// Screen positioning defines
//#####
#define COL_SHIFT_FIRST 120
#define COL_SHIFT_SECOND 350

//Right
#define COL_X_FIRST 10
#define COL_X_SECOND 200

//Down
#define ROW_Y_FIRST 12
#define ROW_Y_SECOND 40

//Debug line
#define DEBUG_X 0
#define DEBUG_Y 0

//#####
// Color values defines
//#####
#define HEX_RED 0x0000FF
#define HEX_YELLOW 0x00FFFF
#define HEX_GREEN 0x00FF00
#define HEX_BLUE 0xFF0000
#define HEX_BACKGROUND 0x000000
#define HEX_DEBUG 0xFFFFFF

//#####
// Structs
//#####
typedef struct TeamStruct {
	int x;
	int y;
	int color;
	int teamNameLength;
	char teamName[32];
} TeamStruct;


//#####
// Team Definitions
//#####

//First Row, First Col
TeamStruct teamRed =
{
	COL_X_FIRST,
	ROW_Y_FIRST,
	HEX_RED,
	11 + 1, //strlen(TEAM_NAME_RED),
	TEAM_NAME_RED
};

//First Row, Second Col
TeamStruct teamYellow =
{
	COL_X_SECOND,
	ROW_Y_FIRST,
	HEX_YELLOW,
	14 + 1, //strlen(TEAM_NAME_YELLOW),
	TEAM_NAME_YELLOW
};

//Second Row, First Col
TeamStruct teamGreen =
{
	COL_X_FIRST,
	ROW_Y_SECOND,
	HEX_GREEN,
	13 + 1, //strlen(TEAM_NAME_GREEN),
	TEAM_NAME_GREEN
};

//Second Row, Second Col
TeamStruct teamBlue =
{
	COL_X_SECOND,
	ROW_Y_SECOND,
	HEX_BLUE,
	17 + 1, //strlen(TEAM_NAME_BLUE),
	TEAM_NAME_BLUE
};


//#####
// External Source Variables
//#####

// Pull the fonts from another source compiled and linked with this one
extern const uint16_t font[][16];


//#####
// System Variables
//#####

// These are overwritten from the config if the type is matrix
int width = 256;
int height = 128;


//#####
// Application Specific Variables
//#####

// A string built to blankout the score areas
char blankoutStr[BLANK_LEN] = NULL_STRING;
char blankLineStr[BOARD_BLANK_LINE] = NULL_STRING;

// The length of the string that triggers the scanning of a score string
const int checkTriggerLen = CHECK_TRIGGER_LEN + 1; //strlen(CHECK_TRIGGER) + 1; // +1 for the null byte

//#####
// Function prototype
//#####
static int font_write(
	uint32_t * const buf,
	const uint32_t color,
	const int y0,
	const int x0,
	const char * s,
  const int doubleSize
);

void timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    printf("%s",asctime( localtime(&ltime) ) );
}

/*##########
 ## Main
 #########*/
int main (int argc, char ** argv) {
	//TODO: Nicely catch signals and break from the loop for a proper exit

	// Storage for LEDscape configuration values
	ledscape_config_t * config = &ledscape_matrix_default;

  // If there was an argument to the binary use, it as a config filename
	if (argc > 1) {
		// Load the config file
		config = ledscape_config(argv[1]);

		// If it didn't work, bail
		if (!config)
			return EXIT_FAILURE;
	}

	// If the config type is for a matrix, pull the width and height from it
	if (config->config_type.type == LEDSCAPE_TYPE_MATRIX) {
		width = config->matrix_config.framebuf_width;
		height = config->matrix_config.framebuf_height;
	}

	// What we push the pixel buffer to get the library to push it to HW
	ledscape_t * const ledBuffer = ledscape_init(config, 0);

#if VERBOSE
	// Let the world know the LEDscape configuration initilization is done
	timestamp();
	printf("LEDscape configuration initilization done\n");
	fflush(stdout);
#endif

	// Make a pixel buffer we can muck around with and push to the library
	uint32_t * const pixelBuffer = calloc(width * height, 4);

	// Fill the blankout string with the blankout character
	memset(&blankoutStr, (int)BLANK_CHAR, BLANK_LEN);
	memset(&blankLineStr, (int)BLANK_CHAR, BOARD_BLANK_LINE);

	// Make storage for the four teams scores, initilize them
	int teamScores[NUM_TEAMS] = { 0, 0, 0, 0 };

	// Make storage for the stringified team scores,
	char teamScoreStrings[NUM_TEAMS][SCORE_STR_LEN] =
	{
		NULL_STRING,
		NULL_STRING,
		NULL_STRING,
		NULL_STRING
	};

	// A string buffer for lines from STDIN/Arduino piped in
	char arduinoOutStr[ARDUINO_LINE_OUT_MAX_LEN] = NULL_STRING;

	// Another string buffer for a chunk of the string above
	// to look for a format to scan
	char checkStr[CHECK_TRIGGER_LEN + 1] = NULL_STRING;

	// Our faithful foreverloop
	while (1) {
		// Push null byte to front of strings
		arduinoOutStr[0] = '\0';
		checkStr[0]      = '\0';

		// Grab the next line from stdin
		//(The Arduino being piped in)
		gets(arduinoOutStr);

		// Print the string that came in to STDOUT so it can be logged
		// (piped to log)
#if VERBOSE
		timestamp();
		printf("ardOut: %s\n", arduinoOutStr);
		fflush(stdout);
#endif

		// Copy the first X lettes of the string
		// (to test against the string we will read from)
		strncpy(checkStr, arduinoOutStr, checkTriggerLen - 1); // -1 nix the null byte


#if VERY_VERBOSE
		// Print the buffer we are checking against for debugging
		timestamp();
		printf("Check string: %s\n", checkStr);
		fflush(stdout);
#endif

		// Compare the passed in buffer to the check trigger
		if (0 == strcmp(checkStr, CHECK_TRIGGER)) {

#if VERY_VERBOSE
			// Let us know the trigger was caught and we are parsing it
			timestamp();
			printf("Doing sscanf\n");
			fflush(stdout);
#endif

			// Pull out the score values with the score's scan format
			sscanf(
				arduinoOutStr,
				SCORE_SCAN_FORMAT,
				&(teamScores[0]),
				&(teamScores[1]),
				&(teamScores[2]),
				&(teamScores[3])
			);
		}
#if BOARD_VERBOSE
		else if (0 == strcmp(checkStr, DEBUG_TRIGGER)) {
			// Print debug info to top-left of screen in white
			font_write(pixelBuffer, HEX_BACKGROUND, DEBUG_Y, DEBUG_X, blankLineStr, 0);
			font_write(pixelBuffer, HEX_DEBUG, DEBUG_Y, DEBUG_X, arduinoOutStr, 0);
		}
#endif

#if VERY_VERBOSE
		// Debug out for what we scanned in
		timestamp();
		printf(
			"%i %i %i %i\n",
			teamScores[0],
			teamScores[1],
			teamScores[2],
			teamScores[3]
		);
		fflush(stdout);
#endif

		// Teams
		// Write teams to pixel buffer
	//font_write(pixelBuffer, 0xBBGGRR,         vert, 				horz, 			  string,      				 isBig)
		font_write(pixelBuffer, teamRed.color,    teamRed.y,    teamRed.x,    teamRed.teamName,    0);  //RED
		font_write(pixelBuffer, teamYellow.color, teamYellow.y, teamYellow.x, teamYellow.teamName, 0);  //YELLOW
		font_write(pixelBuffer, teamGreen.color,  teamGreen.y,  teamGreen.x,  teamGreen.teamName,  0);  //GREEN
		font_write(pixelBuffer, teamBlue.color,   teamBlue.y,   teamBlue.x,   teamBlue.teamName,   0);  //BLUE

		// Stringify the scores
		for(int i = 0; i < NUM_TEAMS; i++) {
			sprintf(teamScoreStrings[i], "%i", teamScores[i]);
		}

		//TODO: Make more effecient by only updating scores that change

		// Blank-out
		// Clears out the space where and updated score will be printed
	//font_write(pixelBuffer, 0xBBGGRR,       vert, 				horz, 						string,      isBig)
		font_write(pixelBuffer, HEX_BACKGROUND, teamRed.y,    COL_SHIFT_FIRST,  blankoutStr, 0);  //RED
		font_write(pixelBuffer, HEX_BACKGROUND, teamYellow.y, COL_SHIFT_SECOND, blankoutStr, 0);  //YELLOW
		font_write(pixelBuffer, HEX_BACKGROUND, teamGreen.y,  COL_SHIFT_FIRST,  blankoutStr, 0);  //GREEN
		font_write(pixelBuffer, HEX_BACKGROUND, teamBlue.y,   COL_SHIFT_SECOND, blankoutStr, 0);  //BLUE

		// Scores
		// Write scores to pixel buffer
  //font_write(pixelBuffer, 0xBBGGRR,         vert, 				horz, 						string,      				 isBig)
		font_write(pixelBuffer, teamRed.color,    teamRed.y,    COL_SHIFT_FIRST,  teamScoreStrings[0], 0);  //RED
		font_write(pixelBuffer, teamYellow.color, teamYellow.y, COL_SHIFT_SECOND, teamScoreStrings[1], 0);  //YELLOW
		font_write(pixelBuffer, teamGreen.color,  teamGreen.y,  COL_SHIFT_FIRST,  teamScoreStrings[2], 0);  //GREEN
		font_write(pixelBuffer, teamBlue.color,   teamBlue.y,   COL_SHIFT_SECOND, teamScoreStrings[3], 0);  //BLUE

		// Have the library deal with pushing our pixel buffer to the HW
		ledscape_draw(ledBuffer, pixelBuffer);

		// wait for the previous frame to finish;
		ledscape_wait(ledBuffer);
	}

	// Nicely close the HW buffer
	ledscape_close(ledBuffer);

	// We did good
	return EXIT_SUCCESS;
}


/*##########
 ## Font Write
 #########*/
static int font_write(
	uint32_t * const buf,
	const uint32_t color,
	const int y0,
	const int x0,
	const char * s,
        const int doubleSize
)
{
	int x = x0;
	int y = y0;

	while (1)
	{
		char c = *s++;
		if (!c)
			break;

		if (c == '\n')
		{
			x = x0;
			y += 16 * width;
			continue;
		}

		const uint16_t * ch = font[(uint8_t) c];
		int max_width = 0;

		if (c == ' ' || c == '.')
			max_width = 3;
		else
		for (int h = 0 ; h < 16 ; h++)
		{
			int width = 0;
			//uint16_t row = ch[h] >> 2;
			uint16_t row = ch[h] >> 1;
			while (row)
			{
				row >>= 1;
				width++;
			}

			if (width > max_width)
				max_width = width;
		}

		// add space after the character
		max_width++;

		if (doubleSize) {
			// Double it
			max_width *= 2;
		}

		for (int h = 0 ; h < 16 ; h++)
		{
			//uint16_t row = ch[h] >> 2;
			uint16_t row = ch[h] >> 1;
			for (int j = 0 ; j < max_width ; j++) //, row >>= 1)
			{
				if(!doubleSize || j % 2) row >>=1;

				uint32_t pixel_color = (row & 1) ? color : 0;
				int ox = x + j;
/*
				if (x + j >= width || x + j < 0)
					continue;
*/
				if (ox >= width)
					continue;

				// wrap in x
				if (ox < 0)
					ox += width;

				if (y + h >= height || y + h < 0)
					continue;

				if (doubleSize) {
					for(int jah = 0; jah < 2; jah++) {
						int characterY = (h * 2) + jah;
						int lineY = (y + characterY) * width;

						uint8_t* pix = (uint8_t*) &buf[lineY + ox];

			       			pix[0] = pixel_color >> 16;
			       			pix[1] = pixel_color >>  8;
			       			pix[2] = pixel_color >>  0;
					}
				} else {
					uint8_t * pix = (uint8_t*) &buf[(y+h)*width + ox];
                                	pix[0] = pixel_color >> 16;
                                	pix[1] = pixel_color >>  8;
                                	pix[2] = pixel_color >>  0;
				}
			}
		}

		x += max_width;
	}

	return x;
}
