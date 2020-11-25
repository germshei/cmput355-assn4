#pragma once

struct	coordinate
{
	int x, y;
};

typedef struct coordinate Coordinate;

void	start_game();
void	set_cell(char player, int row, int col);	// player places a piece at x, y
void	invoke_pi();					// invokes pi, moves the board accordingly
void	print_board();					// print the current state of the game
char	*serialize_game();				// convert the list of moves into a string
static char	cell_state(int row, int col);
static Coordinate	coordinate_from_segment(int line, int segment);
