CC=gcc
DEBUG= -ggdb
OPT=	-O2

CFLAGS= -Wall $(DEBUG) $(OPT)
LFLAGS=

OBJ= ircBOT.o tools.o

all: $(OBJ) chomp_bot

ircBot.o:	ircBOT.c ircBOT.h tools.h
	$(CC) $(CFLAGS) -c ircBOT.c

tools.o:	tools.c tools.h
	$(CC) $(CFLAGS) -c tools.c

chomp_bot: chomp_bot.c 	$(OBJ)
	$(CC) $(CFLAGS) -o chomp_bot chomp_bot.c $(OBJ) $(LFLAGS)

clean:
	rm -f *.o
