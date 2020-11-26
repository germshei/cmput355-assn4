#include "utility.h"

#include <stdio.h>

int	max(int a, int b)
{
	return a > b ? a : b;
}

int	min(int a, int b)
{
	return a < b ? a : b;
}

void	flush_input()
{
	char	c;
	while	((c = getchar()) != EOF && c != '\n');
}
