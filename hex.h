#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct	coordinate
{
	int x, y;
};

typedef struct coordinate	Coordinate;

struct	state
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

typedef struct state	GameBoard;

struct	node
{
	struct node	**neighbours,
			*prev;
	size_t		length,
			size;
	int		row,
			col,
			dist;
};

typedef struct node	Node;

struct	graph
{
	struct node	*nodes;
	struct node	top,
			bottom,
			left,
			right;

	size_t		size;
};

typedef struct graph	Graph;

void	start_game();
bool	validate_move(int row, int col);
void	set_cell(char player, int row, int col);	// player places a piece at x, y
void	invoke_pie();					// invokes pi, moves the board accordingly
void	print_board();					// print the current state of the game
char	*serialize_game();				// convert the list of moves into a string
char	cell_state(int row, int col);
char	check_winner(char * state);
int	count_neighbours(int row, int col);
Graph	*construct_graph();
void	set_neighbouring(Node *a, Node *b);
int	alphabeta(char *state, int depth, int alpha, int beta, int turn, int maximizing_turn);
int	alphabeta_heuristic(char *state, int maximizing_turn);
int	shortest_path(char *state, char player);
int	djikstra(char *state, char player);
Coordinate	compute_best_move(char player);
Coordinate	coordinate_from_segment(int line, int segment);
