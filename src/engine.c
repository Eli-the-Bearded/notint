
/*
 * Copyright (c) Abraham vd Merwe <abz@blio.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "typedefs.h"
#include "utils.h"
#include "io.h"
#include "engine.h"

/*
 * Global variables
 */

const shapes_t SHAPES =
{
/*
 *      X.X       X.X           X.X
 *        X.X   X.X     X.X.X   X.X     X.X.X   X.X.X   X.X.X.X
 *                        X             X           X
 *
 * num    0       1       2       3       4       5       6
 * ASCII art from tetris-bsd, which uses same numbering. (This
 * is not the order shown in the tint statistics.)
 */     

   { COLOR_CYAN,    0, FALSE, { {  1,  0 }, {  0,  0 }, {  0, -1 }, { -1, -1 } } },
   { COLOR_GREEN,   1, FALSE, { {  1, -1 }, {  0, -1 }, {  0,  0 }, { -1,  0 } } },
   { COLOR_YELLOW,  2, FALSE, { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  0,  1 } } },
   { COLOR_BLUE,    3, FALSE, { { -1, -1 }, {  0, -1 }, { -1,  0 }, {  0,  0 } } },
   { COLOR_MAGENTA, 4, FALSE, { { -1,  1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } },
   { COLOR_WHITE,   5, FALSE, { {  1,  1 }, {  1,  0 }, {  0,  0 }, { -1,  0 } } },
   { COLOR_RED,     6, FALSE, { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  2,  0 } } }
};

static board_t blank_board;

/*
 * Functions
 */

/* This rotates a shape */
static void real_rotate (shape_t *shape,bool clockwise)
{
   int i,tmp;
   if (clockwise)
	 {
		for (i = 0; i < NUMBLOCKS; i++)
		  {
			 tmp = shape->block[i].x;
			 shape->block[i].x = -shape->block[i].y;
			 shape->block[i].y = tmp;
		  }
	 }
   else
	 {
		for (i = 0; i < NUMBLOCKS; i++)
		  {
			 tmp = shape->block[i].x;
			 shape->block[i].x = shape->block[i].y;
			 shape->block[i].y = -tmp;
		  }
	 }
}

/* Rotate shapes the way tetris likes it (= not mathematically correct) */
static void fake_rotate (shape_t *shape)
{
   switch (shape->type)
	 {
	  case 0:	/* Just rotate this one anti-clockwise and clockwise */
		if (shape->flipped) real_rotate (shape,TRUE); else real_rotate (shape,FALSE);
		shape->flipped = !shape->flipped;
		break;
	  case 1:	/* Just rotate these two clockwise and anti-clockwise */
	  case 6:
		if (shape->flipped) real_rotate (shape,FALSE); else real_rotate (shape,TRUE);
		shape->flipped = !shape->flipped;
		break;
	  case 2:	/* Rotate these three anti-clockwise */
	  case 4:
	  case 5:
		real_rotate (shape,FALSE);
		break;
	  case 3:	/* This one is not rotated at all */
		break;
	 }
}

/* Draw a shape on the board */
static void drawshape (board_t board,shape_t *shape,int x,int y)
{
   int i;
   for (i = 0; i < NUMBLOCKS; i++) board[x + shape->block[i].x][y + shape->block[i].y] = shape->color;
}

/* Erase a shape from the board */
static void eraseshape (board_t board,shape_t *shape,int x,int y)
{
   int i;
   for (i = 0; i < NUMBLOCKS; i++) board[x + shape->block[i].x][y + shape->block[i].y] = COLOR_BLACK;
}

/* Check if shape is allowed to be in this position */
static bool allowed (board_t board,shape_t *shape,int x,int y)
{
   int i,occupied = FALSE;
   for (i = 0; i < NUMBLOCKS; i++) if (board[x + shape->block[i].x][y + shape->block[i].y]) occupied = TRUE;
   return (!occupied);
}

/* Move the shape left if possible */
static bool shape_left (board_t board,shape_t *shape,int *x,int y)
{
   bool result = FALSE;
   eraseshape (board,shape,*x,y);
   if (allowed (board,shape,*x - 1,y))
	 {
		(*x)--;
		result = TRUE;
	 }
   drawshape (board,shape,*x,y);
   return result;
}

/* Move the shape right if possible */
static bool shape_right (board_t board,shape_t *shape,int *x,int y)
{
   bool result = FALSE;
   eraseshape (board,shape,*x,y);
   if (allowed (board,shape,*x + 1,y))
	 {
		(*x)++;
		result = TRUE;
	 }
   drawshape (board,shape,*x,y);
   return result;
}

/* Rotate the shape if possible */
static bool shape_rotate (board_t board,shape_t *shape,int x,int y)
{
   bool result = FALSE;
   shape_t test;
   eraseshape (board,shape,x,y);
   memcpy (&test,shape,sizeof (shape_t));
   fake_rotate (&test);
   if (allowed (board,&test,x,y))
	 {
		memcpy (shape,&test,sizeof (shape_t));
		result = TRUE;
	 }
   drawshape (board,shape,x,y);
   return result;
}

/* Move the shape one row down if possible */
static bool shape_down (board_t board,shape_t *shape,int x,int *y)
{
   bool result = FALSE;
   eraseshape (board,shape,x,*y);
   if (allowed (board,shape,x,*y + 1))
	 {
		(*y)++;
		result = TRUE;
	 }
   drawshape (board,shape,x,*y);
   return result;
}

/* Check if shape can move down (= in the air) or not (= at the bottom */
/* of the board or on top of one of the resting shapes) */
static bool shape_bottom (board_t board,shape_t *shape,int x,int y)
{
   bool result = FALSE;
   eraseshape (board,shape,x,y);
   result = !allowed (board,shape,x,y + 1);
   drawshape (board,shape,x,y);
   return result;
}

/* Drop the shape until it comes to rest on the bottom of the board or */
/* on top of a resting shape */
static int shape_drop (board_t board,shape_t *shape,int x,int *y)
{
   int droppedlines = 0;
   eraseshape (board,shape,x,*y);
   while (allowed (board,shape,x,*y + 1))
	 {
		(*y)++;
		droppedlines++;
	 }
   drawshape (board,shape,x,*y);
   return droppedlines;
}

/* This removes all the rows on the board that is completely filled with blocks */
static int droplines (board_t board)
{
   int x,y,ny,status,droppedlines;
   board_t newboard;
   /* initialize new board */
   memcpy (newboard,blank_board,sizeof (board_t));
   /* ... */
   ny = NUMROWS - 3;
   droppedlines = 0;
   for (y = NUMROWS - 3; y > 0; y--)
	 {
		status = 0;
		for (x = 1; x < NUMCOLS - 2; x++) if (board[x][y]) status++;
		if (status < NUMCOLS - 3)
		  {
			 for (x = 1; x < NUMCOLS - 2; x++) newboard[x][ny] = board[x][y];
			 ny--;
		  }
		else droppedlines++;
	 }
   memcpy (board,newboard,sizeof (board_t));
   return droppedlines;
}

/*
 * This counts the blocks on the board after applying a bitmask
 * eg CHALLENGE_MASK finds only challenge blocks.
 */
static int countblocks (int mask, board_t board)
{
   int r, c, count = 0;
   /* row[0] empty (off top);
    * row[NUMROWS-1] and row[NUMROWS-2] bottom wall;
    * col[0] left wall; col[NUMCOLS-1], and col[NUMCOLS-2] right wall
    */
   for (r = 1; r < NUMROWS - 2; r++) {
	for (c = 1; c < NUMCOLS - 2; c++) {
		if (mask & board[c][r])
			count ++;
	}
   }
   return count;
}

/*
 * Initialize specified tetris engine
 */
void engine_init (engine_t *engine,void (*score_function)(engine_t *))
{
   int i;
   engine->score_function = score_function;
   /* intialize values */
   engine->curx = 5;
   engine->cury = 1;
   engine->curshape = rand_value (-1, NUMSHAPES);
   engine->nextshape = rand_value (-1, NUMSHAPES);
   engine->game_mode = GAME_TRADITIONAL;
   engine->score = 0;
   engine->rand_status = -1;
   engine->status.moves =
	engine->status.rotations =
	engine->status.dropcount =
	engine->status.efficiency =
	engine->status.droppedlines =
	engine->status.lastclear =
	engine->status.challengestart = 
	engine->status.challengeblocks =
	engine->status.challengeblocks_prev =
        engine->status.nonchallengeblocks =
		0;
   /* initialize shapes */
   memcpy (engine->shapes,SHAPES,sizeof (shapes_t));

   /* initialize board */
   memset (engine->board,0,sizeof (board_t));
   for (i = 0; i < NUMCOLS; i++) engine->board[i][NUMROWS - 1] = engine->board[i][NUMROWS - 2] = WALL;
   for (i = 0; i < NUMROWS; i++) engine->board[0][i] = engine->board[NUMCOLS - 1][i] = engine->board[NUMCOLS - 2][i] = WALL;
   
   /* and save a copy for resets */
   memcpy (blank_board,engine->board,sizeof (board_t));
/*
 * There's a double wall at the right and bottom:
 *
 *  ROW    0  1  2  3  4  5  6  7  8  9 10 11 12    COL
 *
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     0
 *        10 02 00 00 00 00 00 00 00 00 03 10 10     1
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     2
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     3
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     4
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     5
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     6
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     7
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     8
 *        10 00 00 00 00 00 00 00 00 00 00 10 10     9
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    10
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    11
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    12
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    13
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    14
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    15
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    16
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    17
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    18
 *        10 00 00 00 00 00 00 00 00 00 00 10 10    19
 *        10 04 00 00 00 00 00 00 00 00 05 10 10    20
 *        10 10 10 10 10 10 10 10 10 10 10 10 10    21
 *        10 10 10 10 10 10 10 10 10 10 10 10 10    22
 *
 * AND the top row (0) is off the screen.
 *
 *  engine->board[ 1][01]  = 2  left top corner
 *  engine->board[10][01]  = 3  right top corner
 *  engine->board[ 1][20]  = 4  left bottom corner
 *  engine->board[10][20]  = 5  right bottom corner
 */
}

/*
 * Post-options engine tweak.
 */
void engine_tweak (int level, int mode, engine_t *engine)
{
     engine->level = level;
     engine->game_mode = mode;
     engine->start_time = time(NULL);
     engine->pause_start = engine->pause_end = engine->accumulated_pause =
	 (time_t)0;

     if (engine->game_mode == GAME_CHALLENGE) 
        {
	    engine_chalset (engine);
	    return;
        }

     if (engine->game_mode != GAME_EASYTRIS) 
	return;

     /* 
      * In early versions, setting the level would determine first
      * shape. Now setting the level merely blocks a particular
      * shape from being first.
      */
     engine->rand_status = ((level - 1) % NUMSHAPES) << STATUS_SHIFT;
     engine->rand_status = update_rs(engine->rand_status);

      /*
       * Pick first two pieces according to the easy-tris rules,
       * replacing the engine_init choices.
      */
     engine->curshape = rand_value(engine->rand_status, NUMSHAPES);
     engine->rand_status = update_rs(engine->rand_status);
     engine->nextshape = rand_value(engine->rand_status, NUMSHAPES);
     engine->rand_status = update_rs(engine->rand_status);
}

/*
 * Set up a game board and update status for a level appropriate
 * challenge.
 */
void engine_chalset (engine_t *engine)
{
   int r, c;	/* row and column */
   int h,i,j,k;   /* misc use */

   switch (engine->level)
	{
	   case 1:
	   case 10:
	         /* Level one and ten are both left hand flush side triangles
		  * of column striped blocks. Level one is solid, level
		  * ten is full of holes.
		  */
	         j = (engine->level == 1) ? 0: 1;
		 k = (engine->level == 1) ? 1: 2;

	         for (c = 1; c < 8; c++)
		    {
		      for (r = 12 + c; r < 21; r++)
		         {
			      i = ((j == (r % k)) || (j == (c % k)));
			      if (i) engine->board[c][r] = (CHALLENGE_MASK | c);
		         }
		    }
	         break;

	         /* Stripes with wide (j=3) or narrow (j=2) spaces.
		  * Lower h is taller stripes.
		  * Colors are sequential, but missing pieces help hide that.
		  */
	   case 3:
	   case 4:
	   case 5:
	   case 6:
	         /* gcc optimizer doesn't like setting these on the case N:
		  * lines and then falling through, so a second test of
		  * game level is needed.
		  */
                 if      (engine->level == 3) { h = 0; j = 3; }
		 else if (engine->level == 4) { h = 2; j = 3; }
		 else if (engine->level == 5) { h = 1; j = 2; }
		 else                         { h = 3; j = 2; }
	         for (c = 1; c < 11; c++)
		    {
		      for (r = 18 - h; r < 21; r++)
		      {
		        k = (r + c) % 7 + 1;  /* color */
			if(0 == (c % j)) engine->board[c][r] = (CHALLENGE_MASK | k);
		      }
		    }

	         break;
	         /* Level seven and eight are both right hand side triangles,
		  * not flush, and checkerboarded. The level eight version is
		  * taller. Colors are striped by row.
		  */
	   case -7: /* don't use */
	   case -8: /* kinda dumb */
	         j = 8 - engine->level; /* remainder */
	         for (h = 2; h < 9; i++)
		    {
		      c = 10 - h;
		      for (r = 12 + h; r < 21; r++)
		         {
			      k = 1 + (r % 7); /* color */
			      if (j == (c+r) %2) engine->board[c][r] = (CHALLENGE_MASK | k);
		         }
		    }
	         break;
		 
	   default: /* more interesting than older level 7 / 8 anyway */
	       /* Level nine and up (no level cap) are just random blocks.
	        * Higher levels have more rows of blocks, with fewer in them.
		* Colors are in horizontal runs.
		*/
	       h = 25 - engine->level; /* height adjust */
	       if (h < 6) { h = 6; }   /* 6 from top to 16 from top */
	       if (h > 20) { h = 18; }
	       k = 25 + h * 3;   /* rand threshold */


	       for (r = h; r < 21; r++)
		  {
		    i = 0;                      /* block count in row */
		    j = 1 + rand_value(-1, 7);  /* current color */
		    for (c = 1; c < 11; c++)
		       {
			    if (k > rand_value (-1, 99)) {
			        i ++; 
				if (i < 6) {
				   engine->board[c][r] = (CHALLENGE_MASK | j);

		                   if (i > 3) { j = 1 + rand_value(-1, 7); }  /* new color */
			        } else {
				   /* ensure at least one blank */
				   i = 0;
				}
			    }
		       }
		  }
	}

   engine->status.challengestart =
	engine->status.challengeblocks =
	engine->status.challengeblocks_prev =
		countblocks (CHALLENGE_MASK, engine->board);
}

/*
 * Perform the given action on the specified tetris engine
 */
void engine_move (engine_t *engine,action_t action)
{
   switch (action)
	 {
		/* move shape to the left if possible */
	  case ACTION_LEFT:
		if (shape_left (engine->board,&engine->shapes[engine->curshape],&engine->curx,engine->cury)) engine->status.moves++;
		break;
		/* rotate shape if possible */
	  case ACTION_ROTATE:
		if (shape_rotate (engine->board,&engine->shapes[engine->curshape],engine->curx,engine->cury)) engine->status.rotations++;
		break;
		/* move shape to the right if possible */
	  case ACTION_RIGHT:
		if (shape_right (engine->board,&engine->shapes[engine->curshape],&engine->curx,engine->cury)) engine->status.moves++;
		break;
		/* drop shape to the bottom */
	  case ACTION_DROP:
		engine->status.dropcount += shape_drop (engine->board,&engine->shapes[engine->curshape],engine->curx,&engine->cury);
	 }
}

/*
 * Evaluate the status of the specified tetris engine. In challenge mode, might
 * reset the level and challenge.
 *
 * OUTPUT:
 *   1 = shape moved down one line
 *   0 = shape at bottom, next one released
 *  -1 = game over (board full)
 */
int engine_evaluate (engine_t *engine)
{
   int need_reset = FALSE;

   if (shape_bottom (engine->board,&engine->shapes[engine->curshape],engine->curx,engine->cury))
	 {
		/* collect data to increase score */
		engine->status.lastclear = droplines (engine->board);
		
		/* count blocks only if we actually cleared something */
                if ((engine->game_mode == GAME_CHALLENGE) &&
		    (engine->status.lastclear > 0))
                    {
			engine->status.challengeblocks = countblocks (CHALLENGE_MASK, engine->board);
			engine->status.nonchallengeblocks = countblocks ( COLOR_MASK, engine->board) - engine->status.challengeblocks; 
			if(engine->status.challengeblocks < 1)
			    {
				/* level may effect score, just collect data now, then
				 * score, then up level and reset
				 */
				need_reset = TRUE;
				sleep(1);
			    }
		    }

		engine->score_function (engine);

		if (need_reset)
		    {
			memcpy (engine->board,blank_board,sizeof (board_t));
			engine->level ++;
	                engine_chalset (engine);
		    }
		
                if ((engine->game_mode == GAME_CHALLENGE) &&
		    (engine->status.lastclear == 0))
		    {
			engine->status.nonchallengeblocks += NUMBLOCKS;
		    }

		/* update status information */
		engine->status.droppedlines += engine->status.lastclear;
                engine->status.lastclear = 0;
		engine->curx -= 5;
		engine->curx = abs (engine->curx);
		engine->status.rotations = 4 - engine->status.rotations;
		engine->status.rotations = engine->status.rotations > 0 ? 0 : engine->status.rotations;
		engine->status.efficiency += engine->status.dropcount + engine->status.rotations + (engine->curx - engine->status.moves);
		engine->status.efficiency >>= 1;
		engine->status.dropcount = engine->status.rotations = engine->status.moves = 0;
		/* intialize values */
		if(engine->game_mode == GAME_EASYTRIS) {
			/* go wild */
			engine->curx = 4+rand_value(-1,5);
		} else {
			engine->curx = 5;
		}
		engine->cury = 1;
		engine->curshape = engine->nextshape;
		engine->nextshape = rand_value (engine->rand_status, NUMSHAPES);
		engine->rand_status = update_rs(engine->rand_status);
		/* initialize shapes */
		memcpy (engine->shapes,SHAPES,sizeof (shapes_t));
		/* return games status */
		return allowed (engine->board,&engine->shapes[engine->curshape],engine->curx,engine->cury) ? 0 : -1;
	 }
   shape_down (engine->board,&engine->shapes[engine->curshape],engine->curx,&engine->cury);
   return 1;
}

