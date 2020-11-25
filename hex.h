#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct	coordinate
{
	int x, y;
};

typedef struct coordinate Coordinate;

struct board
{
	int	rows, cols;
	int	turn;
	int	computer_turn;

	char	*contents;

	Coordinate	*history;
	size_t		history_len;

	char	players[2];
	char	*colours[2];
	char	empty_cell;

	bool	finished;
};

struct	hex_node
{
	struct hex_node	**neighbours;
	size_t	length;
	int	row,
		col;
	char	player;
};

typedef struct hex_node	HexNode;

struct	graph
{
	struct hex_node	*nodes,
			*top,
			*bottom,
			*left,
			*right;

	size_t		length;
};

typedef struct graph	Graph;

void	start_game();
bool	validate_move(int row, int col);
void	set_cell(char player, int row, int col);	// player places a piece at x, y
void	invoke_pie();					// invokes pi, moves the board accordingly
void	print_board();					// print the current state of the game
char	*serialize_game();				// convert the list of moves into a string
char	cell_state(int row, int col);
Graph	*construct_graph();
char	determine_winner(Graph g);
Coordinate	compute_best_move(char player);
Coordinate	coordinate_from_segment(int line, int segment);
