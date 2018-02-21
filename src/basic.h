#ifndef BASIC_H
#define BASIC_H

/* Number of shapes in the game */
#define NUMSHAPES	7

/* Number of blocks in each shape */
#define NUMBLOCKS	4

#define STATUS_GROUP    (20)			 /* needs to be < MOD */
#define STATUS_MAX      (NUMSHAPES*STATUS_GROUP) /* no longer used */
#define STATUS_SHIFT	(16)
#define STATUS_MOD	(65536)			 /* 2^(STATUS_SHIFT) */
#define PERCENT_RAND	(25)
#define TRAD_ADJUST	(5)			 /* high score bonus */

/* Number of rows and columns in board */
#define NUMROWS	23
#define NUMCOLS	13

/* Wall id - Arbitrary, but shouldn't have the same value as one of the colors */
#define WALL 16

/* Header for scorefile */
#define SCORE_HEADER	"notint scorefile"

/* Header for score title */
static const char scoretitle[] = "\n\t NOTINT HIGH SCORES\n\n"
		" Game Type    Rank   Score     Name                  When\n";

/* Length of a player's name */
#define NAMELEN 20

/* Number of scores allowed in highscore list */
#define NUMSCORES 10

/* How much space to use for a printable date */
#define TIME_STR_BUF  20

/* Maximum digits in a number (i.e. number of digits in score, */
/* number of blocks, etc. should not exceed this value */
#define MAXDIGITS 5

/* Number of levels in the game */
#define MINLEVEL	1
#define MAXLEVEL	9

/* The score is multiplied by this to avoid losing precision */
#define SCOREFACTOR 2


/*
 * Macros
 */

/* Upper left corner of board */
#define XTOP ((out_width () - NUMROWS - 3) >> 1)
#define YTOP ((out_height () - NUMCOLS - 9) >> 1)

/* This calculates the time allowed to move a shape, before it is moved a row down */
#define DELAY (1000000 / (level + 2))

/* This calculates the stored score value */
#define SCOREVAL(x) (SCOREFACTOR * (x))

/* This calculates the real (displayed) value of the score */
#define GETSCORE(score) ((score) / SCOREFACTOR)
#endif	/* #ifndef BASIC_H */
