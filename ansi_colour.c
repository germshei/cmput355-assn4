#include "ansi_colour.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *format	= "%s%s%s";
static const char *clear	= ANSI_CLEAR();

char	*colorize(const char* input, const char* palette)
{
#if	USE_COLOURS
	size_t	buffer_len	= strlen(input) + strlen(palette) + strlen(clear) + 1;
	char	*buffer		= calloc(buffer_len, sizeof(char));

	sprintf(buffer, format, palette, input, clear);
#else
	size_t	buffer_len	= strlen(input) + 1;
	char	*buffer		= calloc(buffer_len, sizeof(char));
	strcpy(buffer, input);
#endif

	return buffer;
}
