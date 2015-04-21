# cs335 lab1
# to compile your project, type make and press enter
CFLAGS=-I ./include
LIB = ./libggfonts32.so
LFLAGS = $(LIB) -lrt -lX11 -lGL -pthread -lm

all: lab1

lab1: lab1.cpp
	g++ $(CFLAGS) lab1.cpp log.c -Wall -Wextra $(LFLAGS) -o lab1

clean:
	rm -f lab1
	rm -f *.o

