
#include <stdio.h>
#include <stdlib.h>

const char *program_version = "notint version 1.1, a fork of tint 0.04";

void showversion () {
    puts (program_version);
    printf ("\n"
            "notint copyright 2018 Eli the Bearded\n"
            "tint 0.04+nmu1 (c) Abraham vd Merwe\n");
    exit(0);
}
