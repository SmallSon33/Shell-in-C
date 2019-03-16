CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow \
-pedantic-errors -fstack-protector-all
PROGS = d8sh

all: $(PROGS)

d8sh: executor.o lexer.o parser.tab.o d8sh.o
	$(CC) parser.tab.o executor.o lexer.o -lreadline d8sh.o -o d8sh

# object files
lexer.o: lexer.c parser.tab.h lexer.h
	$(CC) $(CFLAGS) -c lexer.c
parser.tab.o: command.h parser.tab.c parser.tab.h executor.h
	$(CC) $(CFLAGS) -c parser.tab.c
executor.o: command.h executor.h executor.c
	$(CC) $(CFLAGS) -c executor.c
d8sh.o: executor.h lexer.h d8sh.c
	$(CC) $(CFLAGS) -c d8sh.c

clean:
	rm -f *.o $(PROGS) a.out
