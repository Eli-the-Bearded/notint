/*
 * Tries to convert an older score file format to a newer.
 * Likely not of general interest.
 *
 * Does NOT attempt to identify / fix issues like
 *     size of int changing
 *     size of time_t changing
 *     endian-ness changes
 *
 * DOES try to apply sanity checks to the data to not accept
 * the input if any of those things have happened.
 *
 * Usage:
 *	scoreconvert INFILE OUTFILE
 *
 *      Review output (good scroll back is a must) then confirm
 *      or decline the change.
 *
 * Both files can be the same, but the original will be lost
 * afterwards. 
 *
 * March 2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define NEED_GAMETYPE

#include "basic.h"
#include "typedefs.h"

/* Unix timestamps of (low) 1990-01-01 and (high) 2100-01-01.
 * Outside that range considered "insane"
 */
#define SCORE_DATE_LOW	 631170000UL
#define SCORE_DATE_HIGH	4102462800UL

/* score file format */
#define PRE_NOTINT	1
#define FIRST_NOTINT	2

score_t scores[BIG_NUMSCORES],
        new_scores[BIG_NUMSCORES];

/*
 * Initializes the data structures.
 */
void initscores (void)
{
   int i, mode;

   mode = MODE_LOW - 1;
   for (i = 0; i < BIG_NUMSCORES; i++)
	 {
		strcpy (new_scores[i].name,"None");
		strcpy (    scores[i].name,"None");
		scores[i].score     = new_scores[i].score = -1;
		scores[i].timestamp = new_scores[i].timestamp = 0;

		if(0 == (i % NUMSCORES)) {
			mode ++;
		}
   		scores[i].trad_mode = GAME_TRADITIONAL;
   		new_scores[i].trad_mode = mode;
	 }
}

/*
 * Reads in, and applies sanity checks, a scorefile.
 * Returns zero on error and number of entries (should be 10)
 * on success.
 */
int readscores (char *scorefile)
{
   FILE *handle;
   int i,j,ch;
   int conf_mode;
   char header[MAX_HEADER];
   char time_str_buf[TIME_STR_BUF];

   if ((handle = fopen (scorefile,"r")) == NULL)
	 {
		printf("Could not open scorefile\n");
		return(0);
	 }
   i = fread (header,strlen (SCORE_HEADER),1,handle);
   if (i != 1)
	 {
		printf("Could not read scorefile\n");
		return(0);
	 }
   if (strncmp (SCORE_HEADER,header,strlen (SCORE_HEADER)) == 0)
	 {
		conf_mode = FIRST_NOTINT;
	 }
   else 
	 {
		fseek(handle,0,SEEK_SET);
		i = fread (header,strlen (SCORE_HEADER_TINT),1,handle);
		if (i != 1)
			 {
				printf("Could not read scorefile\n");
				return(0);
			 }
		if (strncmp (SCORE_HEADER_TINT,header,strlen (SCORE_HEADER_TINT)) == 0)
			 {
			     conf_mode = PRE_NOTINT;
			 }
		else
			 {
				printf("Unknown scorefile format\n");
				return(0);
			 }
	 }

   for (i = 0; i < NUMSCORES; i++)
	 {
		j = 0;
		while ((ch = fgetc (handle)) != '\0')
		  {
			 if ((ch == EOF) || (j >= NAMELEN - 2))
			   {
				  scores[i].name[j] = '\0';
				  printf("Read error, name on score %d (position %d)\n", i, j);
				  return(0);
			   }
			 scores[i].name[j++] = (char) ch;
		  }
		scores[i].name[j] = '\0';
                printf("rank %d\t%s\n", i, scores[i].name);

		j = fread (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1)
		  {
			 printf("Read error, score value on score %d\n", i);
			 return(0);
		  }
                printf("score\t%d\n", scores[i].score);

                if(conf_mode == PRE_NOTINT)
		  {
			 scores[i].trad_mode = GAME_TRADITIONAL;
		  }
                else
		  {
			 j = fread (&(scores[i].trad_mode),sizeof (int),1,handle);
			 if (j != 1) 
			   {
				  printf("Read error, game mode on score %d\n", i);
				  return(0);
			   }
			 if ((scores[i].trad_mode < MODE_LOW) || (scores[i].trad_mode > MODE_HIGH))
			   {
				  if (scores[i].score > 0)
				    {
					  printf("Mode out of whack for score %d\n", i);
					  return(0);
				    } else {
					  scores[i].trad_mode = GAME_TRADITIONAL;
				    }
			   }
		  }
                printf("mode\t%d\n", scores[i].trad_mode);

		j = fread (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1)
		  {
			 printf("Read error, game date %d\n", i);
			 return(0);
		  }
		if ((scores[i].timestamp < SCORE_DATE_LOW) || (scores[i].timestamp > SCORE_DATE_HIGH))
		  {
			 if (scores[i].score > 0)
			    {
				 printf("Read error, insane date on score %d\n", i);
				 printf("time_t probably different on source system\n");
				 return(0);
			    } else {
				 scores[i].timestamp = SCORE_DATE_LOW + 1;
			    }
		  }
                memset(time_str_buf,0,TIME_STR_BUF);
		strftime(time_str_buf, TIME_STR_BUF, "%Y‐%m‐%d %H:%M:%S", localtime(&scores[i].timestamp));
                printf("set\t%s\n\n", time_str_buf);
	 }

   fclose (handle);
   return(NUMSCORES);
}

/*
 * Copies old scores into new score ordering.
 */
void shufflescores (void) {
	int old, trad, easy, zen, use;

	easy = NUMSCORES * GAME_EASYTRIS;	/*  0 ..  9 */
	trad = NUMSCORES * GAME_TRADITIONAL;	/* 10 .. 19 */
	zen  = NUMSCORES * GAME_ZEN;		/* 20 .. 29 */

	for (old = 0, use = 0; old < NUMSCORES; old++)
	   {
		switch(scores[old].trad_mode)
		  {
			case GAME_EASYTRIS:
				use = easy;
				easy ++;
				break;
			case GAME_TRADITIONAL:
				use = trad;
				trad ++;
				break;
			case GAME_ZEN:
				use = zen;
				zen ++;
				break;
		  }

		
		strncpy(new_scores[use].name, scores[old].name, NAMELEN);
		new_scores[use].score       = scores[old].score;
		new_scores[use].timestamp   = scores[old].timestamp;

		printf("old slot %d -> new slot %d\n", old, use);
	   }
	
}

/*
 * Displays the post-shuffle score data
 */
void shownewscores (void) {
	int i, show_as;
	for (i = 0; i < BIG_NUMSCORES; i++)
	  {
		show_as = 1 + (i % NUMSCORES);
		printf("%2d   %-20s  %6d    %s\n",
			show_as, new_scores[i].name, new_scores[i].score,
			gametype[new_scores[i].trad_mode]);
	  }
}

#define ERROR_OUT()  {printf("write failed\n"); return(0);}

/*
 * Asks for a confirmation and then writes the new format file.
 * Returns 0 on error, and number of entries on success.
 */
int savenewscores (char* scorefile) {
	int i, rv;
	FILE *handle;
        char header[MAX_HEADER];
	char confirm[NAMELEN];


        fprintf (stderr,"Confirm that conversion looks good (y/n): ");
        fgets (confirm,NAMELEN - 1,stdin);
	if ((confirm[0] != 'y') && (confirm[0] != 'Y')) {
		fprintf (stderr,"NOT CONFIRMED\n");
		return(0);
	}

	if ((handle = fopen (scorefile,"w")) == NULL) {
		printf("Open for write error to %s\n", scorefile);
		return(0);
	}
        strcpy (header,SCORE_MAGIC_NUMBER);
        rv = fwrite (header,strlen (SCORE_MAGIC_NUMBER),1,handle);
	if (rv != 1) ERROR_OUT ();
	for (i = 0; i < BIG_NUMSCORES; i++)
	  {
		rv = fwrite (new_scores[i].name,strlen (new_scores[i].name) + 1,1,handle);
		if (rv != 1) ERROR_OUT ();
		rv = fwrite (&(new_scores[i].score),sizeof (int),1,handle);
		if (rv != 1) ERROR_OUT ();
		rv = fwrite (&(new_scores[i].trad_mode),sizeof (int),1,handle);
		if (rv != 1) ERROR_OUT ();
		rv = fwrite (&(new_scores[i].timestamp),sizeof (time_t),1,handle);
		if (rv != 1) ERROR_OUT ();

	  }

        fclose (handle);
	return(i);
}

int main(int argc, char**argv)
{
    if (argc != 3) {
	printf("%s: usage scoreconvert OLDFILE NEWFILE\n", argv[0]);
	return(1);
    }
    initscores();
    if (NUMSCORES != readscores(argv[1])) {
	printf("%s: score read issue\n", argv[0]);
	return(1);
    }
    shufflescores();
    shownewscores();

    if (BIG_NUMSCORES != savenewscores(argv[2])) {
	printf("%s: score write issue\n", argv[0]);
	return(1);
    }
    printf("Converted %s to %s\n", argv[1], argv[2]);
    return(0);
}
