
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
#include <time.h>
#include <limits.h>

#include "typedefs.h"
#include "basic.h"

/*
 * Initialize random number generator
 */
void rand_init ()
{
   srandom (time (NULL));
}

/*
 * With status < 0: Generate a random number within range
 * Otherwise use status to pick a mostly determinate value.
 */
int rand_value (int status, int range)
{
   if(status < 0) {
     return (random() % range);
   } else {
     int rc = status % range;
     int lucky = random() % 100;
     if(lucky < PERCENT_RAND) {
       rc = (random () % range);
     }
     return(rc);
   }
}

/*
 * Pick a new value for rand_status
 */
int update_rs(int old)
{
  int use_next;
  if(old < 0) {
    return(old);
  }

  use_next = random();
  if ((old % NUMSHAPES) == (use_next % NUMSHAPES)) {
     int lucky = random() % 100;
     /* this lucky checks the opposite way from rand_value() lucky */
     if(lucky > PERCENT_RAND) {
       /* pick a new random without checking status mod 7 */
       use_next = random();
     }
    
  }
  return(use_next);
}

/*
 * Convert an str to long. Returns TRUE if successful,
 * FALSE otherwise.
 */
bool str2int (int *i,const char *str)
{
   char *endptr;
   long tmp;	/* fix for 64-bit systems where long > int */
   tmp = strtol (str,&endptr,0);
   if (*str == '\0' || *endptr != '\0' || tmp <= INT_MIN || tmp >= INT_MAX) return FALSE;
   *i = (int)tmp;
   return TRUE;
}

/*
 * If env variable name has a value, return that, otherwise return default
 */
char *getenv_with_default (const char *name,char *def)
{
     char *env = getenv(name);
     if (env == NULL || *env == '\0') return def;
     return env;
}

