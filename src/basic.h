#ifndef BASIC_H
#define BASIC_H

/* Number of shapes in the game */
#define NUMSHAPES	7

/* Number of blocks in each shape */
#define NUMBLOCKS	4

#define STATUS_GROUP    (30)
#define STATUS_MAX      (NUMSHAPES*STATUS_GROUP)

/* Number of rows and columns in board */
#define NUMROWS	23
#define NUMCOLS	13

/* Wall id - Arbitrary, but shouldn't have the same value as one of the colors */
#define WALL 16

#endif	/* #ifndef BASIC_H */
