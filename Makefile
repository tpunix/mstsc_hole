

CC = gcc
CFLAGS = -Wall -O3 -s -I.
LIBS =


all: ant.dll mstsc_hole.exe

ant.dll: ant.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LIBS)

mstsc_hole.exe: gogo.c gicon.res
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -Wl,--subsystem,windows

gicon.res: gicon.rc
	windres -i $^ --input-format=rc -o $@ -O coff

gicon.rc: p48.ico


clean:
	rm -f *.exe *.dll *.res

