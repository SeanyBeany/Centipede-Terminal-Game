CC = gcc

BASEFLAGS = -Wall -pthread -std=c99
NODEBUG_FLAGS = -dNDEBUG 
DEBUG_FLAGS = -g

LDLIBS = -lcurses -pthread

OBJS = console.o centipede.o

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

clean:
	rm -f $(OBJS)
	rm -f *~
	rm -f $(EXE)
	rm -f $(EXE)_d

run:
	./$(EXE)

run_debug:
	./$(EXE)_d
