#
# Because my fingers type "make" more naturally than "gmake", a "make"
# compatible makefile, even if it does a little less (eg, no cross
# comples).
# 
localstatedir = $(HOME)
CFLAGS += -Wall
CPPFLAGS = -DSCOREFILE=\"$(localstatedir)/.$(PRG).scores\"
LDLIBS = -lcurses

OBJ = engine.o utils.o io.o tint.o
SRC = engine.c utils.c io.c tint.c
HEADERS = config.h engine.h io.h typedefs.h utils.h basic.h
PRG = notint


.PHONY: all clean distclean

all: $(PRG)

depends:
	rm -f depends
	set -e; for F in $(SRC); do $(CC) -MM $(CFLAGS) $(CPPFLAGS) $$F >> depends; done


$(PRG): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

tags: $(SRC) $(HEADERS)
	ctags $(SRC) $(HEADERS)

clean:
	rm -f depends *~ $(OBJ) $(PRG)

distclean: clean
	rm -f tags core


# created with "make depends && cat depends >> Makefile"
engine.o: engine.c typedefs.h utils.h io.h engine.h basic.h
utils.o: utils.c typedefs.h basic.h
io.o: io.c io.h
tint.o: tint.c typedefs.h utils.h io.h config.h engine.h basic.h
