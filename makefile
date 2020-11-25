hex: hex.o ansi.o
	clang -o hex hex.o ansi.o

hex.o: main.c ansi_colour.h
	clang -c main.c -o hex.o

ansi.o: ansi_colour.c ansi_colour.h
	clang -c ansi_colour.c -o ansi.o
