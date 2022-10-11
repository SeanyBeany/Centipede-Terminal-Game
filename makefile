CC=gcc
CFLAGS=-Wall -I.
DEPS=myshell.h
OBJS=myshell.o
EXE=myshell
DEBUG=-g

myshell: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(DEPS) $(CFLAGS)

run:
	./$(EXE)

clean:
	rm -f $(OBJS)
	rm -f $(EXE)