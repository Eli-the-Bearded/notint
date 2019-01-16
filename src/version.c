
#include <stdio.h>
#include <stdlib.h>

const char *program_version = "notint version 1.4, a fork of tint 0.04";

void showversion (char *filename)
{
    puts (program_version);
    printf ("\n"
            "notint copyright 2018-2019 Eli the Bearded\n"
            "Forked from tint 0.04+nmu1 (c) Abraham vd Merwe\n");
    if (filename != NULL && *filename != '\0') {
        printf ("Using highscore file: %s\n", filename);
    }
    exit(0);
}
