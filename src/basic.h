#ifndef BASIC_H
#define BASIC_H

/* Number of shapes in the game */
#define NUMSHAPES	7

/* Name the shapes for convience; letters kinda resemble the shape */
#define SHAPE_Z    0		/* the cyan shape */
#define SHAPE_S    1		/* the green shape */
#define SHAPE_T    2		/* the yellow shape */
#define SHAPE_O    3		/* the blue shape */
#define SHAPE_L    4		/* the magenta shape */
#define SHAPE_J    5		/* the white shape */
#define SHAPE_I    6		/* the red shape */
#define NO_SHAPE   -9		/* must be negative for rand_value() */

/* Number of blocks in each shape */
#define NUMBLOCKS	4

/* Used by easytris mode */
#define STATUS_GROUP    (20)		 /* GROUP+MIN to be < MOD */
#define STATUS_MIN      (10)		 /* min number in group */
#define STATUS_SHIFT	(16)		 /* make one int be two values */
#define STATUS_MOD	(65536)		 /* 2^(STATUS_SHIFT) */
#define PERCENT_RAND	(15)		 /* % pieces that are lucky */
#define TRAD_ADJUST	(1)		 /* high score board bonus */

/* Number of rows and columns in board */
/* includes rows and columns that are out of play */
#define NUMROWS	23
#define NUMCOLS	13

/* Wall id - Arbitrary, but shouldn't have the same value as one of the colors
 * Colormasks:  01110000 background, 00000111 foreground
 * 16 would be  00010000 or red background, black foreground
 */
#define WALL 16

/* 
 * Challenge mode blocks are displayed with a different character, but 
 * the same colors; "color" is the value saved in the game board array
 */
#define COLOR_MASK 	0x77
#define CHALLENGE_MASK	0x80

/* Headers for scorefile */
/* original tint */
#define SCORE_HEADER_TINT	"Tint 0.02b (c) Abraham vd Merwe - Scores"
/* first gen notint */
#define SCORE_HEADER		"notint scorefile"
/* current */

/* trad, easy, and zen */
#define  SCORE_MAGIC_NUMBER_1	"<notint scorefile version=2>"
/* trad, easy, zen, and challenge */
#define  SCORE_MAGIC_NUMBER_2	"<notint scorefile version=3>"

/* trad, easy, zen, challenge, and speed */
#define  SCORE_MAGIC_NUMBER	"<notint scorefile version=4>"

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

/* BIG_NUMSCORES defined after GAME_ definitions */

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
#define GAME_CHALLENGE		3
#define GAME_SPEED		4

/* number of modes with high score lists */
#define GAME_MODE_COUNT		5
/* index to future proof entry (so increase as game modes go up) */
#define GAME_UNKNOWN		5

/* lowest and highest game_mode values; high only used in scoreconvert */
#define MODE_LOW		0
#define MODE_HIGH		4

/* NUMSCORES for each game mode */
#define BIG_NUMSCORES (GAME_MODE_COUNT*NUMSCORES)

#ifdef NEED_GAMETYPE
/* Used in high score display, and shared between two different programs.
 * So ifdef'ed rather than a .c with just the one definition.
 * All entries should be same length.
 */
static char *gametype[] = {
	"- easy-tris -", 	/* 0 GAME_EASYTRIS	*/
	"-traditional-", 	/* 1 GAME_TRADITIONAL	*/
	"- - -zen- - -",	/* 2 GAME_ZEN		*/
	"- challenge -",	/* 3 GAME_CHALLENGE	*/
	"- speed run -",	/* 4 GAME_SPEED		*/

	/* future proof */
	"***unknown***",
   };
#endif

/* Zen game is perpetually at this level.
 * (unless explicitly changed.)
 */
#define GAME_ZEN_LEVEL		3

/* How many rounds to display the "this is special" message */
#define SHOW_SPECIAL_ROUNDS     5

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
#define DELAY (1000000 / (engine.level + 2))
#define CHALLENGE_DELAY (1000000 / (3))

/* This calculates the stored score value */
#define SCOREVAL(x) (SCORE_FACTOR * (x))

/* This calculates the real (displayed / saved) value of the score */
#define GETSCORE(score) ((score) / SCORE_FACTOR)

#endif	/* #ifndef BASIC_H */
