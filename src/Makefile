#
# Because my fingers type "make" more naturally than "gmake", a "make"
# compatible makefile, even if it does a little less (eg, no cross
# comples).
# 
localstatedir = $(DESTDIR)/var/games
SCORE_TEMPLATE = $(PRG).scores
CFLAGS += -Wall
CPPFLAGS = # -DSCOREFILE=\"$(localstatedir)/$(SCORE_TEMPLATE)\"
LDLIBS = -lcurses

OBJ = engine.o utils.o io.o tint.o version.o
SRC = engine.c utils.c io.c tint.c version.c
HEADERS = config.h engine.h io.h typedefs.h utils.h basic.h version.h
PRG = notint


.PHONY: all clean distclean

all: $(PRG) $(SCORE_TEMPLATE)

depends:
	rm -f depends
	set -e; for F in $(SRC); do $(CC) -MM $(CFLAGS) $(CPPFLAGS) $$F >> depends; done


$(PRG): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

$(SCORE_TEMPLATE): scoreconvert
	echo y | ./scoreconvert /dev/null $(SCORE_TEMPLATE)

# scorecovert is a second-class program, not built by default,
# nor included in the tags file.
scorecovert: scorecovert.c typedefs.h basic.h
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)
	
tags: $(SRC) $(HEADERS)
	ctags $(SRC) $(HEADERS)

clean:
	rm -f depends *~ $(OBJ) $(PRG) scorecovert

distclean: clean
	rm -f tags core


# created with "make depends && cat depends >> Makefile"
engine.o: engine.c typedefs.h utils.h io.h engine.h basic.h
utils.o: utils.c typedefs.h basic.h
io.o: io.c io.h
tint.o: tint.c typedefs.h utils.h io.h config.h engine.h basic.h version.h
