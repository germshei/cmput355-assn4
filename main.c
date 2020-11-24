#include <stdio.h>
#include <stdlib.h>

#include "ansi_colour.h"

int	main(int argc, char **argv)
{
	char *palette	= ANSI_STYLE(FG_BRIGHT RED WITH ITALIC WITH BLINK);
	char *text	= "Hello world!";

	text = colorize(text, palette);

	printf("%s\n", text);

	free(text);
}
