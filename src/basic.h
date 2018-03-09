#ifndef BASIC_H
#define BASIC_H

/* Number of shapes in the game */
#define NUMSHAPES	7

/* Number of blocks in each shape */
#define NUMBLOCKS	4

#define STATUS_GROUP    (20)		 /* GROUP+MIN to be < MOD */
#define STATUS_MIN      (10)		 /* min number in group */
#define STATUS_SHIFT	(16)		 /* make one int be two values */
#define STATUS_MOD	(65536)		 /* 2^(STATUS_SHIFT) */
#define PERCENT_RAND	(15)		 /* % pieces that are lucky */
#define TRAD_ADJUST	(1)		 /* high score board bonus */

/* Number of rows and columns in board */
#define NUMROWS	23
#define NUMCOLS	13

/* Wall id - Arbitrary, but shouldn't have the same value as one of the colors
 * Colormasks:  01110000 background, 00000111 foreground
 * 16 would be  00010000 or red background, black foreground
 */
#define WALL 16

/* Headers for scorefile */
/* original tint */
#define SCORE_HEADER_TINT	"Tint 0.02b (c) Abraham vd Merwe - Scores"
/* first gen notint */
#define SCORE_HEADER		"notint scorefile"
/* current */
#define  SCORE_MAGIC_NUMBER	"<notint scorefile version=2>"

/* Longer than any "magic number" header in any recognized format,
 * but less than shortest legit score file.
 * len(SCORE_HEADER_TINT)  = 41
 * len(SCORE_HEADER)       = 17
 * len(SCORE_MAGIC_NUMBER) = 28
 */
#define MAX_HEADER	48

/* Header for score title */
static const char scoretitle[] = "\n\t NOTINT HIGH SCORES\n\n";
static const char scorecols[]  = " Game Type   |Rank| Score |New| Name                 | When\n";
static const char scorebetw[]  = "-------------+----+-------+---+----------------------+-----------\n";

/* Number of scores allowed in highscore list (per mode) */
#define NUMSCORES 10

/* NUMSCORES for each game mode */
#define BIG_NUMSCORES (3*NUMSCORES)

/* How much space to use for a printable date */
/* YYYY-MM-DD HH:MM:SS 
 *          1         2
 * 12345678901234567890
 */
#define TIME_STR_BUF  40

/* Maximum digits in a number (i.e. number of digits in score, */
/* number of blocks, etc. should not exceed this value */
#define MAXDIGITS 5

/* Number of levels in the game */
#define MINLEVEL	1
#define MAXLEVEL	9

/* for game_mode */
#define GAME_EASYTRIS		0
#define GAME_TRADITIONAL	1
#define GAME_ZEN		2
/* index to furture proof entry (so increase as game modes go up */
#define GAME_UNKNOWN		3

#ifdef NEED_GAMETYPE
/* Used in high score display, and shared between two different programs.
 * So ifdef'ed rather an a .c with just the one definition.
 * All entries should be same length.
 */
static char *gametype[] = {
	"- easy-tris -", 
	"-traditional-", 
	"- - -zen- - -",

	/* future proof */
	"***unknown***",
   };
#endif

/* lowest and highest game_mode values; used in scoreconvert */
#define MODE_LOW	0
#define MODE_HIGH	2

/* Zen game is perpetually at this level.
 * (unless explicitly changed.)
 */
#define GAME_ZEN_LEVEL		3

/* The score is multiplied by this to avoid losing precision.
 * It's SCORE_PENALTY ^ (number of possible penalties).
 */
#define SCORE_FACTOR 4
#define SCORE_PENALTY 2

/*
 * Macros
 */

/* Upper left corner of board */
#define XTOP ((out_width () - NUMROWS - 3) >> 1)
#define YTOP ((out_height () - NUMCOLS - 9) >> 1)

/* This calculates the time allowed to move a shape, before it is moved a row down */
#define DELAY (1000000 / (level + 2))

/* This calculates the stored score value */
#define SCOREVAL(x) (SCORE_FACTOR * (x))

/* This calculates the real (displayed / saved) value of the score */
#define GETSCORE(score) ((score) / SCORE_FACTOR)

#endif	/* #ifndef BASIC_H */
