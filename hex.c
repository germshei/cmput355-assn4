#include "hex.h"
#include "ansi_colour.h"
#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HISTORY_LENGTH	20

#define BOARD_ROWS	11
#define BOARD_COLS	11

struct board
{
	int	rows, cols;
	int	turn;

	char	*contents;

	char	*history;
	size_t	history_len;

	char	players[2];
	char	*colours[2];
	char	empty_cell;
};

static struct board	game_state;

void	start_game()
{
	game_state.rows	= BOARD_ROWS;
	game_state.cols	= BOARD_COLS;
	game_state.turn	= 0;

	int contents_len	= game_state.rows * game_state.cols;
	game_state.contents	= calloc(contents_len, sizeof(char));

	game_state.history_len	= HISTORY_LENGTH;
	game_state.history	= calloc(game_state.history_len, sizeof(char));

	game_state.players[0]	= 'X';
	game_state.colours[0]	= ANSI_STYLE(FG RED);
	game_state.players[1]	= 'O';
	game_state.colours[1]	= ANSI_STYLE(FG_BRIGHT BLUE);

	game_state.empty_cell	= ' ';

	memset(game_state.contents, game_state.empty_cell, contents_len);
	
	//TEMPORARY
	//set_cell(game_state.players[0], 8, 5);
	//set_cell(game_state.players[1], 4, 9);
	//game_state.turn = 2;
	//XXXXXXXXX
}

void	set_cell(char player, int row, int col)
{
	size_t	index	= row * game_state.rows + col;
	game_state.contents[index] = player;
}

void	invoke_pi()
{
	return;
}

void	print_board()
{
	int	rows	= game_state.rows;
	int	cols	= game_state.cols;

	int	hex_width	= 4;

	int	height	= rows + cols + 1;
	int	width	= (rows + cols - 1) * (hex_width - 1) + 1;

	const char	*segment_line	= "__";
	const char	*segment_fore	= "/";
	const char	*segment_space	= "  ";
	const char	*segment_back	= "\\";
	const char	*sequence[]	= {segment_line, segment_fore, segment_space, segment_back};
	int		sequence_length	= 4;

	const char	*segment_space_occupied	= " %s";
	const char	*segment_line_half	= "_";

	int	sequence_start	= 0;
	int	sequence_end	= 1;

	size_t	line_length	= width
				* max(	strlen(game_state.colours[0]) + strlen(ANSI_CLEAR()) + 1,
					strlen(game_state.colours[1]) + strlen(ANSI_CLEAR()) + 1);

	char	*line_buffer	= calloc(line_length, sizeof(char));

	size_t	padding_length	= 80;
	char	*padding	= calloc(padding_length, sizeof(char));
	int	padding_amount	= 0;

	size_t	buffer_length	= 80;
	char	*buffer		= calloc(buffer_length, sizeof(char));

	char	*alphabet	= "abcdefghijklmnopqrstuvwxyz";

	if (!line_buffer || !padding || !buffer)
	{
		printf("Could not allocate enough memory!\n");
		return;
	}

	for (int line = 0; line < height;)
	{
		char	*linep	= line_buffer;
		memset(line_buffer, 0, sizeof(char) * line_length);
		memset(padding, ' ', sizeof(char) * padding_length);

		if (line < rows)
		{
			padding_amount = 1 + (rows - line - 1) * 3;
		}
		else if (line - rows < 2)
		{
			padding_amount = 0;
		}
		else
		{
			padding_amount = (line - rows - 1) * 3;
		}

		if (line < rows)
		{
			padding[padding_amount - 1] = alphabet[line];
		}

		padding[padding_amount] = '\0';

		linep	= stpcpy(linep, padding);

		// add segments to the line
		for (int i = sequence_start; i < sequence_end; i++)
		{
			int		s	= i % sequence_length;
			const char	*seg	= sequence[s];

			// format the 'space' character to show cell ownership
			if (seg == segment_space)
			{
				Coordinate	c	= coordinate_from_segment(line, i - sequence_start);
				char		player	= cell_state(c.x, c.y);

				if (player == game_state.empty_cell)
				{
					linep = stpcpy(linep, seg);
				}
				else
				{
					sprintf(buffer, segment_space_occupied, player);
					char *colour;
					char *fmt_seg;

					if (player == game_state.players[0])
					{
						colour = game_state.colours[0];
					}
					else if (player == game_state.players[1])
					{
						colour = game_state.colours[1];
					}

					fmt_seg	= colorize(buffer, colour);
					linep	= stpcpy(linep, seg);

					free(fmt_seg);
				}
			}
			else if ( (line == 0 || line == height - 1) && (seg == segment_line) )
			{
				char	*s1	= colorize(segment_line_half, game_state.colours[0]);
				char	*s2	= colorize(segment_line_half, game_state.colours[1]);

				if (line == height - 1)
				{
					char *t = s1;
					s1	= s2;
					s2	= t;
				}

				linep	= stpcpy(linep, s1);
				linep	= stpcpy(linep, s2);

				free(s1);
				free(s2);
			}
			else if ( (i - sequence_start < 2 && line <= rows) ||
				(sequence_end - i - 1 < 2 && line > rows) )
			{
				char *fmt_seg;

				strcpy(buffer, seg);

				fmt_seg	= colorize(buffer, game_state.colours[0]);
				linep	= stpcpy(linep, fmt_seg);

				free(fmt_seg);
			}
			else if ( (i - sequence_start < 2 && line > rows) ||
				(sequence_end - i - 1 < 2 && line <= rows) )
			{
				char *fmt_seg;

				strcpy(buffer, seg);

				fmt_seg	= colorize(buffer, game_state.colours[1]);
				linep	= stpcpy(linep, fmt_seg);

				free(fmt_seg);
			}
			else
			{
				linep = stpcpy(linep, seg);
			}
		}

		if (line < rows)
		{
			sprintf(buffer, "%d", line + 1);
			linep = stpcpy(linep, buffer);
		}

		printf("%s\n", line_buffer);

		line++;

		if (line < rows && line < cols)
		{
			sequence_end += 4;
		}
		else if (line - cols < 2)
		{
			sequence_end += 3 - line + cols;
		}

		if (line >= rows && line - rows < 2)
		{
			sequence_start += line - rows + 1;
		}
		else if (line >= rows)
		{
			sequence_start += 4;
		}
	}

	free(line_buffer);
	free(padding);
	free(buffer);
}

char	*serialize_game()
{
	return game_state.history;
}

Coordinate	coordinate_from_segment(int line, int segment)
{
	int rows = game_state.rows;
	int cols = game_state.cols;

	int row, col, shift;

	if (line < rows)
	{
		shift	= (segment - 2) / 4;
		row	= line - shift;
		col	= shift;
	}
	else if (line > rows)
	{
		shift	= (segment - 3) / 4;
		row	= rows - shift;
		col	= line - rows + shift;
	}
	else
	{
		shift	= (segment - 1) / 4;
		row	= line - shift;
		col	= shift;
	}

	row--;

	Coordinate c = {row, col};

	return c;
}

char	cell_state(int row, int col)
{
	int index	= row * game_state.rows + col;
	char cell	= game_state.contents[index];

	return cell;
}
