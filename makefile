/**********************************************************************
  Module: makefile
  Author: Shawn Whalen

  Purpose: makefile includes all the make commands

**********************************************************************/
CC = gcc

BASEFLAGS = -Wall -pthread -std=c99 -D_XOPEN_SOURCE=700
NODEBUG_FLAGS = -dNDEBUG 
DEBUG_FLAGS = -g

LDLIBS = -lcurses -pthread

OBJS = main.o console.o centipede.o

EXE = centipede

debug: CFLAGS = $(BASEFLAGS) $(DEBUG_FLAGS)
debug: $(EXE)

release: CFLAGS = $(BASEFLAGS) $(NODEBUG_FLAGS) 
release: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

console.o: console.c console.h
	$(CC) $(CFLAGS) -c console.c

centipede.o: centipede.c
	$(CC) $(CFLAGS) -c centipede.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(OBJS)
	rm -f *~
	rm -f $(EXE)
	rm -f $(EXE)_d

run:
	./$(EXE)

run_debug:
	./$(EXE)_d
