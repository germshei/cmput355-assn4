#pragma once

#include <stdbool.h>
#include <stdlib.h>

// represents a position on the board
struct	location
{
	int row, col;
};

typedef struct location	Location;

struct	board
{
	int	rows, cols;
	int	turn;
	int	computer_player;

	char	*state;
	size_t	size;

	Location	*history;
	size_t	history_size,
		history_length;
	
	char	players[2];
	char	*colours[2];
	char	empty_cell;
};

typedef struct board	GameBoard;

struct	node
{
	char		state;
	struct node	**neighbours;
	size_t		count,
			size;
	int		dist;
	bool		visited;
};

typedef struct node	Node;

struct	graph
{
	struct node	*nodes,
			*top,
			*bottom,
			*left,
			*right;
	size_t		size;
};

typedef struct graph	Graph;

GameBoard	board;

void	set_cell(int row, int col, char state);
char	get_cell(int row, int col);
char	*compute_pie();
void	print_board();
void	record_turn(int row, int col);
char	*serialize_history();
Graph	*graph_from_state(char *state);
int	index_from_location(int row, int col);
void	free_board();
Location	location_from_index(int index);
static void	set_neighbouring(Node *a, Node *b);
static Graph	*construct_graph();
static Location	location_from_segment(int line, int segment);
