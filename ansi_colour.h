#pragma once

#include "options.h"

#define	FG		"3"
#define	FG_BRIGHT	"9"
#define BG		"4"
#define BG_BRIGHT	"10"
#define WITH		";"
#define	PLAIN		""
#define NOT		"2"
#define BRIGHT		"1"
#define ITALIC		"3"
#define UNDERLINE	"4"
#define BLINK		"5"
#define STRIKETHROUGH	"9"
#define BLACK		"0"
#define RED		"1"
#define	GREEN		"2"
#define	YELLOW		"3"
#define	BLUE		"4"
#define	MAGENTA		"5"
#define	CYAN		"6"
#define	WHITE		"7"
#define	ANSI_ESCAPE	"\x1b"

#if	USE_COLOURS
	
	#define	ANSI_STYLE(x)	ANSI_ESCAPE "[" x "m"
	#define ANSI_CLEAR()	ANSI_STYLE(PLAIN)

#else
	
	#define	ANSI_STYLE(x)	""
	#define ANSI_CLEAR()	""

#endif

char	*colorize(const char *, const char *);
