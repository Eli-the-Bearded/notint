
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#define NEED_GAMETYPE

#include "basic.h"
#include "utils.h"
/* included in utils.: #include "typedefs.h" */
#include "io.h"
#include "config.h"
#include "engine.h"


static bool shownext;
static bool dottedlines;
static int shapecount[NUMSHAPES];
static int start_level = MINLEVEL - 1;
static int gamemode = GAME_TRADITIONAL;
static int quiet_scores = FALSE;
static char blockchar = ' ';
static char challchar = '+';

/*
 * Functions
 */

/* This function is responsible for increasing the score appropriately whenever
 * a block collides at the bottom of the screen (or the top of the heap).
 * In easy-tris mode, you also get points for rows cleared.
 * In zen mode, you only get points for rows cleared.
 * In challenge mode there are point penalties for non-challenge rows cleared,
 * and extra non-challenge mode blocks left over.
 */
static void score_function (engine_t *engine)
{
   int score;

   if (engine->game_mode == GAME_ZEN) {
      /* score is saved at a multiple real value */
      engine->score += SCOREVAL (engine->status.lastclear);
      return;
   }

   if (engine->game_mode == GAME_CHALLENGE) {
     	if (engine->status.challengeblocks == engine->status.challengeblocks_prev)
	   {
		/* penalty for clearing a line without any challenge blocks */
      		engine->score -= engine->level * 2 * SCOREVAL (engine->status.lastclear);
	   }
     	else if (0 == engine->status.challengeblocks)
	   {
		/* bonus for clearing all of the challenge blocks */
      		engine->score += SCOREVAL (engine->status.challengestart);
                /* and penalty for any other blocks remaining */
      		engine->score -= 10 * SCOREVAL (engine->status.nonchallengeblocks);
	   }
		
        engine->status.challengeblocks_prev = engine->status.challengeblocks;
   }

   score = SCOREVAL (engine->level * (engine->status.dropcount + 1));
   if (shownext) score /= SCORE_PENALTY;
   if (dottedlines) score /= SCORE_PENALTY;

   if(engine->game_mode == GAME_EASYTRIS) {
      score += engine->level * engine->status.lastclear;
   }
   engine->score += score;
}

/* Draw the board on the screen */
static void drawboard (board_t board)
{
   int x,y;
   int color, chall;
   out_setattr (ATTR_OFF);
   
   for (y = 1; y < NUMROWS - 1; y++) for (x = 0; x < NUMCOLS - 1; x++)
	 {
		out_gotoxy (XTOP + x * 2,YTOP + y);
                color = (board[x][y] & COLOR_MASK);
                chall = (board[x][y] & CHALLENGE_MASK);
		switch (color)
		  {
			 /* Wall */
		   case WALL:
			 out_setattr (ATTR_BOLD);
			 out_setcolor (COLOR_BLUE,COLOR_BLACK);
			 out_putch ('<');
			 out_putch ('>');
			 out_setattr (ATTR_OFF);
			 break;
			 /* Background */
		   case 0:
			 if (dottedlines)
			   {
				  out_setcolor (COLOR_BLUE,COLOR_BLACK);
				  out_putch ('.');
				  out_putch (' ');
			   }
			 else
			   {
				  out_setcolor (COLOR_BLACK,COLOR_BLACK);
				  out_putch (' ');
				  out_putch (' ');
			   }
			 break;
			 /* Block */
		   default:
			 out_setcolor (COLOR_BLACK,color);
			 if ( chall )
			   {
			 	out_putch (challchar);
			 	out_putch (challchar);
			   }
			 else
			   {
			 	out_putch (blockchar);
			 	out_putch (blockchar);
			   }
		  }
	 }
   out_setattr (ATTR_OFF);
}

/* Show the next piece on the screen */
static void drawnext (int shapenum,int x,int y)
{
   int i;
   block_t ofs[NUMSHAPES] =
	 { { 1,  0 }, { 1,  0 }, { 1, -1 }, { 2,  0 }, { 1, -1 }, { 1, -1 }, { 0, -1 } };
   out_setcolor (COLOR_BLACK,COLOR_BLACK);
   for (i = y - 2; i < y + 2; i++)
	 {
		out_gotoxy (x - 2,i);
		out_printf ("        ");
	 }
   out_setcolor (COLOR_BLACK,SHAPES[shapenum].color);
   for (i = 0; i < NUMBLOCKS; i++)
	 {
		out_gotoxy (x + SHAPES[shapenum].block[i].x * 2 + ofs[shapenum].x,
					y + SHAPES[shapenum].block[i].y + ofs[shapenum].y);
		out_putch (blockchar);
		out_putch (blockchar);
	 }
}

/* Draw the background */
static void drawbackground ()
{
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (4,YTOP + 7);   out_printf ("H E L P");
   out_gotoxy (1,YTOP + 9);   out_printf ("p: Pause");
   out_gotoxy (1,YTOP + 10);  out_printf ("j: Left");
   out_gotoxy (1,YTOP + 11);  out_printf ("l: Right");
   out_gotoxy (1,YTOP + 12);  out_printf ("k: Rotate");
   out_gotoxy (1,YTOP + 13);  out_printf ("s: Draw next");
   out_gotoxy (1,YTOP + 14);  out_printf ("d: Toggle lines");
   out_gotoxy (1,YTOP + 15);  out_printf ("a: Advance level");
   out_gotoxy (1,YTOP + 16);  out_printf ("q: Quit");
   out_gotoxy (2,YTOP + 17);  out_printf ("SPACE: Drop");
   out_gotoxy (3,YTOP + 19);  out_printf ("Next:");
}

static int getsum ()
{
   int i,sum = 0;
   for (i = 0; i < NUMSHAPES; i++) sum += shapecount[i];
   return (sum);
}

/* This show the current status of the game */
static void showstatus (engine_t *engine)
{
   static const int shapenum[NUMSHAPES] = { 4, 6, 5, 1, 0, 3, 2 };
   char tmp[MAXDIGITS + 1];
   int i,sum = getsum ();
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   if (engine->game_mode == GAME_ZEN) {
      out_gotoxy (1,YTOP + 1);   out_printf ("Your level is ");
      out_setattr (ATTR_BOLD);
      out_setcolor (COLOR_YELLOW,COLOR_BLACK);
      out_printf ("ZEN");
      out_setattr (ATTR_OFF);
      out_setcolor (COLOR_WHITE,COLOR_BLACK);
   } else {
      out_gotoxy (1,YTOP + 1);   out_printf ("Your level: %d",engine->level);
      out_gotoxy (1,YTOP + 2);   out_printf ("Full lines: %d",engine->status.droppedlines);
      if (engine->game_mode == GAME_CHALLENGE) {
         out_gotoxy (0,YTOP + 3);out_printf ("Blocks togo: %d",
	 					engine->status.challengeblocks);
      }
   }
   out_gotoxy (2,YTOP + 4);   out_printf ("Score");
   out_setattr (ATTR_BOLD);
   out_setcolor (COLOR_YELLOW,COLOR_BLACK);
   out_printf ("  %d",GETSCORE (engine->score));
   if (shownext) drawnext (engine->nextshape,3,YTOP + 22);
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 12,YTOP + 1);
   out_printf ("STATISTICS");
   out_setcolor (COLOR_BLACK,COLOR_MAGENTA);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 3);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 4);
   out_printf ("  ");
   out_setcolor (COLOR_MAGENTA,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 3);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[0]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 3);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_RED);
   out_gotoxy (out_width () - MAXDIGITS - 13,YTOP + 5);
   out_printf ("        ");
   out_setcolor (COLOR_RED,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 5);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[1]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 5);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_WHITE);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 7);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 13,YTOP + 8);
   out_printf ("  ");
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 7);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[2]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 7);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_GREEN);
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 9);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 11,YTOP + 10);
   out_printf ("    ");
   out_setcolor (COLOR_GREEN,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 9);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[3]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 9);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_CYAN);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 11);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 15,YTOP + 12);
   out_printf ("    ");
   out_setcolor (COLOR_CYAN,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 11);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[4]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 11);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_BLUE);
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 13);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 14);
   out_printf ("    ");
   out_setcolor (COLOR_BLUE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 13);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[5]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 13);
   out_printf ("%s",tmp);
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_BLACK,COLOR_YELLOW);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 15);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 15,YTOP + 16);
   out_printf ("  ");
   out_setcolor (COLOR_YELLOW,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 15);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[6]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 15);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 17);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch ('-');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 18);
   out_printf ("Sum          :");
   snprintf (tmp,MAXDIGITS + 1,"%d",sum);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 18);
   out_printf ("%s",tmp);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 20);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch (' ');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 20);
   out_printf ("Score ratio  :");
   snprintf (tmp,MAXDIGITS + 1,"%d",GETSCORE (engine->score) / sum);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 20);
   out_printf ("%s",tmp);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 21);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch (' ');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 21);

   switch (engine->game_mode)
     {
   	case GAME_TRADITIONAL:
	     out_printf ("Efficiency   :");
	     snprintf (tmp,MAXDIGITS + 1,"%d",engine->status.efficiency);
	     out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 21);
	     out_printf ("%s",tmp);
	     break;
   	case GAME_EASYTRIS:
	     out_printf ("Status-count : %1d-%2d",
			(engine->rand_status>>STATUS_SHIFT),
			(engine->rand_status % STATUS_MOD)
		);
	     break;
   	case GAME_CHALLENGE:
	     out_printf ("Challenge    : %3d", engine->status.challengeblocks);
	     out_printf ("Other blocks : %3d", engine->status.nonchallengeblocks);
	     break;
     }

}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

/*
 * Print one entry from the scores file
 */
static void print_scores(time_t curr, int mode, score_t *scores)
{
   char time_str_buf[TIME_STR_BUF], this;
   int i, offset, show_as;
   time_t when;

   if (quiet_scores)
	return;

   fprintf (stderr,"%s",scorecols);
   fprintf (stderr,"%s",scorebetw);
   i = 0;
   while ((i < NUMSCORES))
     {
        offset = i + NUMSCORES * mode;
   	when = scores[offset].timestamp;
        this = ((curr > 0) && (curr == when)) ? '*' : ' ';
	
	if (scores[offset].score < 1)
	   {
		snprintf(time_str_buf, TIME_STR_BUF, "Never");
		show_as = 0;
	   }
	else
	   {
		strftime(time_str_buf, TIME_STR_BUF, "%Y‐%m‐%d", localtime(&when));
		show_as = scores[offset].score;
	   }

	switch (scores[offset].trad_mode)
	   {
	      case GAME_EASYTRIS:
	      case GAME_TRADITIONAL:
	      case GAME_ZEN:
	      case GAME_CHALLENGE:
	      	fputs(gametype[scores[offset].trad_mode], stderr);
	      	break;

	      default:
	      	fputs(gametype[GAME_UNKNOWN], stderr);
	      	break;

	   }
	fprintf (stderr,"| %2d |%7d| %c | %-20s | %s\n",
		i + 1,show_as,this,scores[offset].name,time_str_buf);
        i++;

	if(scores[offset].score < 0) {
		break;
	}
     }
   fprintf (stderr,"%s",scorebetw);
}

static void getname (char *name)
{
   struct passwd *pw = getpwuid (geteuid ());
   char *noname = "(mystery player)";
   int okay = FALSE;

   if (!quiet_scores)
         {
	     fprintf (stderr,"Congratulations! You have a new high score.\n");

	     do
	        {
		 fprintf (stderr,"Enter your name [%s]: ",pw != NULL ? pw->pw_name : "");
		 fgets (name,NAMELEN - 1,stdin);
		 name[strlen (name) - 1] = '\0';

		 /* accept empty string or a string with no <escape> characters */
		 if ((strlen(name) == 0) || (NULL == strchr(name, 27))) { okay = TRUE; }
	        } while (!okay);
         }
    


   if (quiet_scores || !strlen (name))
       {
	   if(pw != NULL)
		 {
			strncpy (name,pw->pw_name,NAMELEN);
		 }
             else
		 {
			strncpy (name,noname,NAMELEN);
		 }
	   name[NAMELEN - 1] = '\0';
       }

   fprintf (stderr,"\n");
}

static void err1 ()
{
   fprintf (stderr,"Error creating %s\n",scorefile);
   exit (EXIT_FAILURE);
}

static void err2 ()
{
   fprintf (stderr,"Error writing to %s\n",scorefile);
   exit (EXIT_FAILURE);
}

void showplayerstats (engine_t *engine)
{
   fprintf (stderr,
			"\n\t   PLAYER STATISTICS\n\n\t"
			"Score       %11d\n",
			GETSCORE (engine->score));

   if(engine->game_mode != GAME_TRADITIONAL)
	{
		fprintf(stderr, "\n\n");
		return;
	}

   fprintf (stderr,     "\t"
			"Efficiency  %11d\n\t"
			"Score ratio %11d\n"
			"\n\n",
			engine->status.efficiency,GETSCORE (engine->score) / getsum ());
}

/*
 * Blank high score data structure.
 */
static void initscores (score_t *scores)
{
   int i, mode;
   mode = MODE_LOW - 1;

   for (i = 0; i < BIG_NUMSCORES; i++)
	 {
		if(0 == (i % NUMSCORES)) {
			mode ++;
		}
		strcpy (scores[i].name,"None");
		scores[i].score = -1;
   		scores[i].trad_mode = mode;
		scores[i].timestamp = 0;
	 }
}

/*
 * Create a new score file, if score at least 1
 */
static void createscores (int score)
{
   FILE *handle;
   int i,j, offset;
   score_t scores[BIG_NUMSCORES];
   char header[MAX_HEADER];
   if (score < 1) return;	/* No need saving this */

   /* first score for gamemode */
   offset = gamemode * NUMSCORES;

   initscores (scores);
   getname (scores[offset].name);
   scores[offset].score = score;
   scores[offset].trad_mode = gamemode;
   scores[offset].timestamp = time (NULL);

   if ((handle = fopen (scorefile,"w")) == NULL) err1 ();
   strcpy (header,SCORE_MAGIC_NUMBER);
   i = fwrite (header,strlen (SCORE_MAGIC_NUMBER),1,handle);
   if (i != 1) err2 ();
   for (i = 0; i < BIG_NUMSCORES; i++)
	 {
		j = fwrite (scores[i].name,strlen (scores[i].name) + 1,1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].trad_mode),sizeof (int),1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1) err2 ();
	 }
   fclose (handle);

   /* Just print the current mode's scores */
   print_scores(scores[0].timestamp, gamemode, scores);
}

static int cmpscores (const void *a,const void *b)
{
   int result, av, bv;
   time_t result_time, at, bt;

   av = (int) ((score_t *) a)->trad_mode;
   bv = (int) ((score_t *) b)->trad_mode;
   result = av - bv;

   /* (REVERSE) Sort by gamemmode first */
   /* a < b */
   if (result < 0) return -1;
   /* a > b */
   if (result > 0) return 1;
   /* a = b */

   /* Then by score */
   av = (int) ((score_t *) a)->score;
   bv = (int) ((score_t *) b)->score;
   result = av - bv;

   /* a < b */
   if (result < 0) return 1;
   /* a > b */
   if (result > 0) return -1;
   /* a = b */

   /* Then by timestamp (REVERSE again) */
   at = (int) ((score_t *) a)->timestamp;
   bt = (int) ((score_t *) b)->timestamp;
   result_time = at - bt;

   /* a is older */
   if (result_time < 0) return -1;
   /* b is older */
   if (result_time > 0) return 1;
   /* timestamps is equal */
   return 0;
}

/*
 * Try to save a score. Calls createscores() on read error of scorefile.
 * createscores() is no-op for score <= 0
 */
static void savescores (int score)
{
   FILE *handle;
   int i,j,ch;
   score_t scores[BIG_NUMSCORES];
   int expect_scores;
   char header[MAX_HEADER];
   time_t tmp = 0;

   if ((handle = fopen (scorefile,"r")) == NULL)
	 {
		createscores (score);
		if(score < 0)
			printf("NO SCOREFILE TO PRINT\n");
		return;
	 }

   /* SCORE_MAGIC_NUMBER and SCORE_MAGIC_NUMBER_1 have same length by design */
   i = fread (header,strlen (SCORE_MAGIC_NUMBER),1,handle);
   if (i != 1)
	 {
		createscores (score);
		if(score < 0)
			printf("CANNOT READ SCOREFILE\n");
		return;
	 }
   if (strncmp (SCORE_MAGIC_NUMBER_1,header,strlen (SCORE_MAGIC_NUMBER_1)) == 0)
	 {
		/* this version has trad, easy, and zen but no challenge */
	 	expect_scores = BIG_NUMSCORES - NUMSCORES;
		if(score < 0)
			printf("OLDER SCOREFILE; MISSING SOME SCORES\n");
	 }
   else if (strncmp (SCORE_MAGIC_NUMBER,header,strlen (SCORE_MAGIC_NUMBER)) == 0)
	 {
	 	expect_scores = BIG_NUMSCORES;
	 }
   else 
	 {
		if(score < 0)
			printf("INCOMPATIBLE SCOREFILE\n");
		return;
	 }

   if (BIG_NUMSCORES != expect_scores)
		initscores (scores);

   for (i = 0; i < expect_scores; i++)
	 {
		j = 0;
		while ((ch = fgetc (handle)) != '\0')
		  {
			 if ((ch == EOF) || (j >= NAMELEN - 2))
			   {
				  createscores (score);
				  return;
			   }
			 scores[i].name[j++] = (char) ch;
		  }
		scores[i].name[j] = '\0';
		j = fread (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1)
		  {
			 createscores (score);
			 return;
		  }
		j = fread (&(scores[i].trad_mode),sizeof (int),1,handle);
		if (j != 1)
		  {
			 createscores (score);
			 return;
		  }
		j = fread (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1)
		  {
			 createscores (score);
			 return;
		  }
	 }
   fclose (handle);

   /* don't even consider writing the score file for -s mode */
   if (score < 0)
     {
	   /* print each of the game mode score lists */
	   fprintf(stderr,"%s",scoretitle);
           print_scores(tmp,GAME_EASYTRIS,scores);
           print_scores(tmp,GAME_TRADITIONAL,scores);
           print_scores(tmp,GAME_ZEN,scores);
           print_scores(tmp,GAME_CHALLENGE,scores);
     }
   else
     {
	   /* last score for gamemode */
	   int offset = (gamemode + 1) * NUMSCORES -1;
	   if (score > scores[offset].score)
		 {
			getname (scores[offset].name);
			scores[offset].score = score;
			scores[offset].trad_mode = gamemode;
			scores[offset].timestamp = tmp = time (NULL);
		 }
            else if (!quiet_scores)
		 {
			fprintf (stderr, "Sorry, not a high score worthy effort.\n\n");
		 }

             

	   qsort (scores,BIG_NUMSCORES,sizeof (score_t),cmpscores);
	   if ((handle = fopen (scorefile,"w")) == NULL) err2 ();
	   strcpy (header,SCORE_MAGIC_NUMBER);
	   i = fwrite (header,strlen (SCORE_MAGIC_NUMBER),1,handle);
	   if (i != 1) err2 ();
	   for (i = 0; i < BIG_NUMSCORES; i++)
		 {
			j = fwrite (scores[i].name,strlen (scores[i].name) + 1,1,handle);
			if (j != 1) err2 ();
			j = fwrite (&(scores[i].score),sizeof (int),1,handle);
			if (j != 1) err2 ();
			j = fwrite (&(scores[i].trad_mode),sizeof (int),1,handle);
			if (j != 1) err2 ();
			j = fwrite (&(scores[i].timestamp),sizeof (time_t),1,handle);
			if (j != 1) err2 ();
		 }
	   fclose (handle);

	   /* just print this mode's score list */
           print_scores(tmp,gamemode,scores);
     }

}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

static void showhelp ()
{
   fprintf (stderr,"USAGE: notint [-h|-s|-v]\n");
   fprintf (stderr,"or   : notint [-c|-e|-t|-z] [-l level] [-n] [-d] [-b char]\n");

   fprintf (stderr,"Non-game play flags (show and exit)\n");
   fprintf (stderr,"  -h           Show this help message\n");
   fprintf (stderr,"  -s           Show high scores\n");
   fprintf (stderr,"  -v           Show game version\n");

   fprintf (stderr,"Game mode\n");
   fprintf (stderr,"  -c           Play the challenge version\n");
   fprintf (stderr,"  -e           Play the easytris version\n");
   fprintf (stderr,"  -t           Play the traditional version\n");
   fprintf (stderr,"  -z           Play the zen version\n");

   fprintf (stderr,"Game options\n");
   fprintf (stderr,"  -b <char>    Use this character to draw blocks instead of spaces\n");
   fprintf (stderr,"  -d           Draw vertical dotted lines\n");
   fprintf (stderr,"  -l <level>   Specify the starting level (%d-%d)\n",MINLEVEL,MAXLEVEL);
   fprintf (stderr,"  -n           Draw next shape\n");

   exit (EXIT_FAILURE);
}

void showversion(/* in version.c */);

static void parse_options (int argc,char *argv[])
{
   int i = 1;
   while (i < argc)
	 {
		/* Help? */
		if (strcmp (argv[i],"-h") == 0)
		  showhelp ();
		/* High scores? */
		else if (strcmp (argv[i],"-v") == 0)
		 {
		  showversion ();
		 }
		else if (strcmp (argv[i],"-s") == 0)
		 {
		  savescores (-1);
		  exit(EXIT_SUCCESS);
		 }
		/* Challenge? */
		else if (strcmp (argv[i],"-c") == 0)
		 {
		  gamemode = GAME_CHALLENGE;
		 }
		/* Easytris? */
		else if (strcmp (argv[i],"-e") == 0)
		 {
		  gamemode = GAME_EASYTRIS;
		 }
		/* Zen? */
		else if (strcmp (argv[i],"-z") == 0)
		 {
		  gamemode = GAME_ZEN;
		 }
		/* Traditional Tetris? */
		else if (strcmp (argv[i],"-t") == 0)
		 {
		  gamemode = GAME_TRADITIONAL;
		 }
		/* Level? */
		else if (strcmp (argv[i],"-l") == 0)
		  {
			 i++;
			 if (i >= argc || !str2int (&start_level,argv[i])) showhelp ();
			 /* no upper level for challenge mode */
			 if ((start_level < MINLEVEL) || ((gamemode != GAME_CHALLENGE) && (start_level > MAXLEVEL)))
			   {
				  fprintf (stderr,"You must specify a level between %d and %d\n",MINLEVEL,MAXLEVEL);
				  exit (EXIT_FAILURE);
			   }
		  }
		/* Show next? */
		else if (strcmp (argv[i],"-n") == 0)
		  shownext = TRUE;
		else if(strcmp(argv[i],"-d")==0)
		  dottedlines = TRUE;
		else if(strcmp(argv[i], "-b")==0)
		  {
		    i++;
		    if (i >= argc || strlen(argv[i]) < 1) showhelp();
		    if( challchar == argv[i][0] )
		    		challchar = blockchar;
		    blockchar = argv[i][0];
		  }
		else
		  {
			 fprintf (stderr,"Invalid option -- %s\n",argv[i]);
			 showhelp ();
		  }
		i++;
	 }

   if ((gamemode == GAME_EASYTRIS) || (gamemode == GAME_ZEN))
      {
	  shownext = TRUE;
      }
}

static void choose_level ()
{
   char buf[NAMELEN];

   if (gamemode == GAME_ZEN) {
      start_level = GAME_ZEN_LEVEL;
      return;
   }

   fprintf (stderr,"Choose a level to start [%d-%d]: ",MINLEVEL,MAXLEVEL);
   fgets (buf,NAMELEN - 1,stdin);
   buf[strlen (buf) - 1] = '\0';
   /* sssh, not telling anyone, but no cap on level in challenge mode */
   if (!str2int (&start_level,buf) || start_level < MINLEVEL || ((gamemode != GAME_CHALLENGE) && start_level > MAXLEVEL)) {
      start_level = 1 + rand_value(-1, 8);
      fprintf (stderr,"Okay, picked level %d\n",start_level);
      sleep(1);
   }
}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

int main (int argc,char *argv[])
{
   bool finished;
   int ch;
   engine_t engine;
   /* Initialize */
   rand_init ();				/* must be called before engine_init () */
   engine_init (&engine,score_function);	/* must be called before using engine.curshape */
   finished = shownext = FALSE;
   memset (shapecount,0,NUMSHAPES * sizeof (int));
   shapecount[engine.curshape]++;
   parse_options (argc,argv);			/* must be called after initializing variables */
   if (start_level < MINLEVEL) choose_level ();
   engine_tweak (start_level, gamemode, &engine);	/* must be called after level selected */
   io_init ();
   drawbackground ();
   in_timeout (DELAY);
   /* Main loop */
   do
	 {
		/* draw shape */
		showstatus (&engine);
		drawboard (engine.board);
		out_refresh ();
		/* Check if user pressed a key */
		if ((ch = in_getch ()) != ERR)
		  {
			 switch (ch)
			   {
				case 'j':
				case KEY_LEFT:
				  engine_move (&engine,ACTION_LEFT);
				  break;
				case 'k':
				case '\n':
				  engine_move (&engine,ACTION_ROTATE);
				  break;
				case 'l':
				case KEY_RIGHT:
				  engine_move (&engine,ACTION_RIGHT);
				  break;
				case ' ':
				case KEY_DOWN:
				  engine_move (&engine,ACTION_DROP);
				  break;
				  /* show next piece */
				case 's':
				  shownext = TRUE;
				  break;
				  /* toggle dotted lines */
				case 'd':
				  dottedlines = !dottedlines;
				  break;
				  /* next level */
				case 'a':
				case KEY_UP:
				  if (engine.level < MAXLEVEL)
					{
					   engine.level++;
					   in_timeout (DELAY);
					}
				  else if (engine.game_mode == GAME_ZEN)
					{
                                           /* wrap around on zen */
					   engine.level = MINLEVEL;
					   in_timeout (DELAY);
					}
				  else out_beep ();
				  break;
				  /* quit */
				case 'q':
				case 'Q':
				  finished = TRUE;
				  break;
				  /* pause */
				case 'p':
				  out_setcolor (COLOR_WHITE,COLOR_BLACK);
				  out_gotoxy ((out_width () - 34) / 2,out_height () - 2);
				  out_printf ("Paused - Press any key to continue");
				  while ((ch = in_getch ()) == ERR) ;	/* Wait for a key to be pressed */
				  in_flush ();							/* Clear keyboard buffer */
				  out_gotoxy ((out_width () - 34) / 2,out_height () - 2);
				  out_printf ("                                  ");
				  break;
				  /* unknown keypress */
				default:
				  out_beep ();
			   }
			 in_flush ();
		  }
		else
		  {
			 switch (engine_evaluate (&engine))
			   {
				  /* game over (board full) */
				case -1:
				  if ((engine.level < MAXLEVEL) && ((engine.status.droppedlines / 10) > engine.level)) engine.level++;
				  finished = TRUE;
				  break;
				  /* shape at bottom, next one released */
				case 0:
				  if ((engine.level < MAXLEVEL) &&
				      ((engine.status.droppedlines / 10) > engine.level) &&
				      (engine.game_mode != GAME_CHALLENGE) &&
				      (engine.game_mode != GAME_ZEN))
					{
					   engine.level++;
					   in_timeout (DELAY);
					}
				  shapecount[engine.curshape]++;
				  break;
				  /* shape moved down one line */
				case 1:
				  break;
			   }
		  }
	 }
   while (!finished);
   /* Restore console settings and exit */
   io_close ();
   /* Don't bother the player if he want's to quit */
   if (ch != 'q' && ch != 'Q')
	showplayerstats (&engine);
   else
	quiet_scores = TRUE;

   savescores (GETSCORE (engine.score));
   exit (EXIT_SUCCESS);
}

