

CC = gcc
CFLAGS = -Wall -O3 -s -I.
LIBS =


all: ant.dll mstsc_hole.exe

ant.dll: ant.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LIBS)

mstsc_hole.exe: gogo.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.exe *.dll

