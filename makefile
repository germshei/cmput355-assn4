hex: hex.o ansi.o board.o utility.o
	clang -g -o hex hex.o ansi.o board.o utility.o;make clean

hex.o: hex.c hex.h utility.h board.h ansi_colour.h options.h
	clang -g -c -o hex.o hex.c

ansi.o: ansi_colour.h ansi_colour.c
	clang -g -c -o ansi.o ansi_colour.c

board.o: board.c board.h utility.h ansi_colour.h
	clang -g -c -o board.o board.c

utility.o: utility.c utility.h
	clang -g -c -o utility.o utility.c

clean:
	rm *.o
