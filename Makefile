CC=g++
CFLAGS=-g -W -Wall -Wextra -Wconversion -pedantic -std=c++11
LDLIBS=-lfreeimage -fopenmp
OBJ=raytracing.cpp
EXEC=raytracing.exe

all: clean $(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(LDLIBS)

clean:
	rm -rf $(EXEC)