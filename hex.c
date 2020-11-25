#include "hex.h"
#include "ansi_colour.h"
#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define HISTORY_LENGTH	20

#define BOARD_ROWS	11
#define BOARD_COLS	11

GameBoard		board;
Graph			*hex_graph;
static const char	*alphabet	= "abcdefghijklmnopqrstuvwxyz";

void	start_game()
{
	board.rows	= BOARD_ROWS;
	board.cols	= BOARD_COLS;
	board.turn	= -1;

	// if playing against computer:
	// board.computer_turn = 1 or 0
	// otherwise:
	board.computer_turn = 1;

	int contents_len	= board.rows * board.cols;
	board.contents		= calloc(contents_len, sizeof(char));

	board.history_len	= board.rows * board.cols + 1; // store max # possible moves + 1 for pie
	board.history		= calloc(board.history_len, sizeof(char));

	board.players[0]	= 'X';
	board.colours[0]	= ANSI_STYLE(FG RED);
	board.players[1]	= 'O';
	board.colours[1]	= ANSI_STYLE(FG_BRIGHT BLUE);

	board.empty_cell	= ' ';

	board.finished	= false;

	memset(board.contents, board.empty_cell, contents_len);

	hex_graph = construct_graph();

	while (!board.finished)
	{
		board.turn++;
		
		int	row	= -1;
		int	col	= -1;
		bool	valid_cell	= false;

		print_board();

		int player = board.turn % 2;

		compute_best_move(board.players[player]);

		if (player == board.computer_turn)
		{
			// do computer turn
			continue;
		}

		// Second player has the option to invoke pie rule
		if (board.turn == 1)
		{
			char	c;

			printf("Player %d, you have the option to invoke the pie rule.\n", player + 1);
			printf("Would you like to do so (y/N)? ");

			scanf("%c", &c);
			fflush(stdin);

			// advance to next player's turn
			if (c == 'y' || c == 'Y')
			{
				invoke_pie();
				Coordinate move_record = {-1, -1};
				board.history[board.turn] = move_record;
				continue;
			}

			// otherwise player 2 makes a normal move
		}

		// prompt player for a move
		do
		{
			printf("Player %d, make a move: ", player + 1);
			char	r;
			int	c;
			int	n	= scanf("%c%d", &r, &c);
			fflush(stdin);

			row	= r - 'a';
			col	= c - 1;

			valid_cell = validate_move(row, col) && n == 2;

			if (!valid_cell)
			{
				printf("A move must be in the form of a row followed by a column (%c1 - %c%d). "
					"A move can only be made in an empty cell.\n",
					alphabet[0], alphabet[board.rows - 1], board.cols);
			}

		}	while(!valid_cell);

		set_cell(board.players[board.turn % 2], row, col);
		Coordinate move_record = {row, col};
		board.history[board.turn] = move_record;
	}
}

bool	validate_move(int row, int col)
{
	bool invalid =	row >= board.rows ||
			row < 0 ||
			col >= board.cols || 
			col < 0 ;

	invalid |= cell_state(row, col) != board.empty_cell;

	return !invalid;

}

void	set_cell(char player, int row, int col)
{
	size_t	index	= row * board.cols + col;
	board.contents[index] = player;
}

void	invoke_pie()
{
	return;
}

void	print_board()
{
	int	rows	= board.rows;
	int	cols	= board.cols;

	int	hex_width	= 4;

	int	height	= rows + cols + 1;
	int	width	= (rows + cols - 1) * (hex_width - 1) + 1;

	const char	*segment_line	= "__";
	const char	*segment_fore	= "/";
	const char	*segment_space	= "  ";
	const char	*segment_back	= "\\";
	const char	*sequence[]	= {segment_line, segment_fore, segment_space, segment_back};
	int		sequence_length	= 4;

	const char	*segment_space_occupied	= " %c";
	const char	*segment_line_half	= "_";

	int	sequence_start	= 0;
	int	sequence_end	= 1;

	size_t	line_length	= width
				* max(	strlen(board.colours[0]) + strlen(ANSI_CLEAR()) + 1,
					strlen(board.colours[1]) + strlen(ANSI_CLEAR()) + 1);

	char	*line_buffer	= calloc(line_length, sizeof(char));

	size_t	padding_length	= 80;
	char	*padding	= calloc(padding_length, sizeof(char));
	int	padding_amount	= 0;

	size_t	buffer_length	= 80;
	char	*buffer		= calloc(buffer_length, sizeof(char));


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

				if (player == board.empty_cell)
				{
					linep = stpcpy(linep, seg);
				}
				else
				{
					sprintf(buffer, segment_space_occupied, player);
					char *colour;
					char *fmt_seg;

					if (player == board.players[0])
					{
						colour = board.colours[0];
					}
					else if (player == board.players[1])
					{
						colour = board.colours[1];
					}

					fmt_seg	= colorize(buffer, colour);
					linep	= stpcpy(linep, fmt_seg);

					free(fmt_seg);
				}
			}
			else if ( (line == 0 || line == height - 1) && (seg == segment_line) )
			{
				char	*s1	= colorize(segment_line_half, board.colours[0]);
				char	*s2	= colorize(segment_line_half, board.colours[1]);

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

				fmt_seg	= colorize(buffer, board.colours[0]);
				linep	= stpcpy(linep, fmt_seg);

				free(fmt_seg);
			}
			else if ( (i - sequence_start < 2 && line > rows) ||
				(sequence_end - i - 1 < 2 && line <= rows) )
			{
				char *fmt_seg;

				strcpy(buffer, seg);

				fmt_seg	= colorize(buffer, board.colours[1]);
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
	size_t	size		= 4 * board.turn;
	char	*hist		= malloc(sizeof(char) * size);
	char	buffer[8];

	for (int i = 0; i < board.turn; i++)
	{
		Coordinate c = board.history[i];
		int row	= c.x;
		int col	= c.y;

		if (row == -1 && col == -1)
		{
			sprintf(buffer, "pie ");
		}
		else
		{
			char alpha_row	= alphabet[row];
			sprintf(buffer, "%c%d ", alpha_row, col);
		}

		if (strlen(hist) + strlen(buffer) + 1 > size)
		{
			size = size * 2;
			hist = realloc(hist, size);
		}

		strcat(hist, buffer);
	}

	char	*result = malloc((strlen(hist) + 1) * sizeof(char));
	strcpy(result, hist);

	return result;
}

Coordinate	coordinate_from_segment(int line, int segment)
{
	int rows = board.rows;
	int cols = board.cols;

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
	int index	= row * board.cols + col;
	char cell	= board.contents[index];

	return cell;
}

char	check_winner()
{
	return ' ';
}

int	count_neighbours(int row, int col)
{
	/*    1
	 * 0 \-1/-1
	 * -1/0 \_0
	 * 1 \_0/0
	 *  0/1 \ 1
	 *     1
	 */    

	int	rows	= board.rows,
		cols	= board.cols,
		n	= 6;

	if	(row == 0 && col == 0)	n -= 3;
	else if	(row == 0 || col == 0)	n -= 2;

	if	(row + 1 == rows && col + 1 == cols)	n -= 3;
	else if	(row + 1 == rows || col + 1 == cols)	n -= 2;

	printf("cell %c%2d has %d neighbours\n", alphabet[row], col, n);

	return	n;
}

Graph	*construct_graph()
{
	int	rows	= board.rows,
		cols	= board.cols;

	size_t	length	= board.rows * board.cols;
	Graph	*graph	= malloc(sizeof(Graph));

	graph->nodes	= malloc(sizeof(Node) * length);
	graph->size	= length;

	graph->top.neighbours	= malloc(sizeof(Node *) * cols);
	graph->bottom.neighbours= malloc(sizeof(Node *) * cols);
	graph->left.neighbours	= malloc(sizeof(Node *) * rows);
	graph->right.neighbours	= malloc(sizeof(Node *) * rows);

	graph->top.size		= cols;
	graph->bottom.size	= cols;
	graph->left.size	= rows;
	graph->right.size	= rows;
	graph->top.length	= 0;
	graph->bottom.length	= 0;
	graph->left.length	= 0;
	graph->right.length	= 0;

	graph->top.row		= -1;
	graph->top.col		= 0;
	graph->bottom.row	= rows;
	graph->bottom.col	= 0;
	graph->left.row		= 0;
	graph->left.col		= -1;
	graph->right.row	= 0;
	graph->right.col	= cols;

	for (int i = 0; i < length; i++)
	{ 
		int	col	= i % rows,
			row	= (i - col) / rows;
		Node	*node	= &(graph->nodes[i]);

		node->row	= row;
		node->col	= col;
		node->length	= 0;

		int	n	= count_neighbours(row, col);

		if (	(row == 0 && col == 0)		||	// 0, 0	
			(row + 1 == rows && col == 0)	||	// n, 0
			(row == 0 && col + 1 == cols)	||	// 0, m
			(row + 1 == rows && col + 1 == cols) )	// n, m
		{
			// corner cells connect to their neighbours plus two sides
			node->size		= n + 2;
		}
		else if	(row == 0 || col == 0 || row + 1 == rows || col + 1 == cols)
		{
			// edge cells connect to their neighbours and one side
			node->size		= n + 1;
		}
		else
		{
			// regular cells connec to all 6 neighbours
			node->size		= n;
		}

		printf("Allocating space for %d nodes for cell %c%2d\n", node->size, alphabet[node->row], node->col);
		node->neighbours = malloc(sizeof(Node *) * node->size);
	}

	for (int i = 0; i < graph->size; i++)
	{
		int	col	= i % rows,
			row	= (i - col) / rows;
		Node	*node	= &(graph->nodes[i]);

		if (row == 0)
		{
			set_neighbouring(node, &graph->left);
		}
		else
		{
			int n_row = row - 1;
			int n_col = col;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

		if (col == 0)
		{
			set_neighbouring(node, &graph->top);
		}
		else
		{
			int n_row = row;
			int n_col = col - 1;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

		if (row + 1 == rows)
		{
			set_neighbouring(node, &graph->right);
		}
		else
		{
			int n_row = row + 1;
			int n_col = col;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

		if (col + 1 == cols)
		{
			set_neighbouring(node, &graph->bottom);
		}
		else
		{
			int n_row = row;
			int n_col = col + 1;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

		if (row + 1 != rows && col + 1 != cols)
		{
			int n_row = row + 1;
			int n_col = col + 1;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

		if (row - 1 >= 0 && col - 1 >= 0)
		{
			int n_row = row - 1;
			int n_col = col - 1;
			set_neighbouring(node, &graph->nodes[n_row * rows + n_col]);
		}

	}

	return graph;
}

void	set_neighbouring(Node *a, Node *b)
{
	if (a->length == a->size || b->length == b->size)
	{
		// node already has max # of connections
		return;
	}

	for (int i = 0; i < a->size; i++)
	{
		if (a->neighbours[i] == b)	return;	// A & B already connected
	}

	for (int i = 0; i < b->size; i++)
	{
		if (b->neighbours[i] == a)	return; // shouldn't be necesary if the above check passed
	}

	a->neighbours[a->length] = b;
	b->neighbours[b->length] = a;

	a->length++;
	b->length++;
}

int	alphabeta(char *s, int d, int a, int b, int turn, int max_turn)
{
	if (d == 0 || check_winner(s))
	{
		printf("reached leaf / limit\n");
		return alphabeta_heuristic(s, max_turn);
	}

	if (turn % 2 == max_turn)
	{
		int v = -INFINITY;
		printf("max step, turn %d, max_turn %d, a %d, b %d\n", turn, max_turn, a, b);
		for (int i = 0; i < (board.rows * board.cols); i++)
		{
			if (s[i] == board.empty_cell)
			{
				s[i]	= board.players[max_turn];
				v	= max(v, alphabeta(s, d - 1, a, b, turn + 1, max_turn));
				s[i]	= board.empty_cell;

				a = max(a, v);
				if (a >= b) break;
			}
		}
		printf("done max step\n");
		return v;
	}
	else
	{
		int v = +INFINITY;
		printf("min step, turn %d, max_turn %d, a %d, b %d\n", turn, max_turn, a, b);
		for (int i = 0; i < (board.rows * board.cols); i++)
		{
			if (s[i] == board.empty_cell)
			{
				s[i]	= board.players[(max_turn + 1) % 2];
				v	= min(v, alphabeta(s, d - 1, a, b, turn + 1, max_turn));
				s[i]	= board.empty_cell;

				b	= min(b, v);
				if (a >= b) break;
			}
		}
		printf("done max step\n");
		return v;
	}
}

int	alphabeta_heuristic(char *state, int max_turn)
{
	char	player		= board.players[max_turn],
		opponent	= board.players[(max_turn + 1) % 2];
	//return	shortest_path(state, opponent) - shortest_path(state, player);
	return	djikstra(state, opponent) - djikstra(state, player);
}

int	shortest_path(char *state, char player)
{
	printf("Calculating shortest path for %c\n", player);
	Node	start, 
		*node,
		*child,
		dest;
	// djikstra shortest path
	if (player == board.players[0])
	{
		start	= hex_graph->left;
		dest	= hex_graph->right;
	}
	else
	{
		start	= hex_graph->top;
		dest	= hex_graph->bottom;
	}

	start.dist = 0;
	start.prev = NULL;

	bool *visited = malloc(sizeof(bool) * hex_graph->size);
	int	*dist;
	memset(visited, false, sizeof(bool) * hex_graph->size);

	for (int i = 0; i < hex_graph->size; i++)
	{
		node = &(hex_graph->nodes[i]);
		node->prev = NULL;
		node->dist = INFINITY;
	}

	for (int i = 0; i < hex_graph->size; i++)
	{
		if (visited[i])	continue;

		node = &(hex_graph->nodes[i]);
		for (int j = 0; j < node->size; j++)
		{
			child			= node->neighbours[j];
			int	child_index	= child->row * board.cols + child->col;
			if (visited[child_index])	continue;

			char	cell_value	= cell_state(child->row, child->col);

			if (cell_value != board.empty_cell && cell_value != player)
			{
				child->dist		= INFINITY;
				visited[child_index]	= true;
				continue;
			}

			int n;

			if (cell_value == player)
			{
				n = 0;
			}
			else
			{
				n = 1;
			}

			n += node->dist;

			if (n < child->dist)
			{
				child->dist = n;
				child->prev = node;
			}
		}
	}

	free(visited);
	printf("Found shortest path: %d\n", dest.dist);
	return dest.dist;
}

int	djikstra(char *state, char player)
{
	Node	*node,
		*dest;

	if (player == board.players[0])
	{
		node = &(hex_graph->left);
		dest = &(hex_graph->right);
	}
	else
	{
		node = &(hex_graph->top);
		dest = &(hex_graph->bottom);
	}

	hex_graph->left.dist	= INFINITY;
	hex_graph->right.dist	= INFINITY;
	hex_graph->top.dist		= INFINITY;
	hex_graph->bottom.dist	= INFINITY;

	hex_graph->left.prev	= NULL;
	hex_graph->right.prev	= NULL;
	hex_graph->top.prev		= NULL;
	hex_graph->bottom.prev	= NULL;

	node->dist		= 0;

	int	size		= board.rows * board.cols;
	bool	*visited	= malloc(sizeof(bool) * size);
	//int	*distances	= malloc(sizeof(int) * size);

	memset(visited, false, sizeof(bool) * size);

	for (int i = 0; i < size; i++)
	{
		hex_graph->nodes[i].dist = INFINITY;
		hex_graph->nodes[i].prev = NULL;
	}

	for (int i = 0; i < size; i++)
	{
		if (node->row >= 0 && node->col >= 0)
		{
			int	node_index	= node->row * board.cols + node->col;
			visited[node_index]	= true;
		}

		for (int j = 0; j < node->length; j++)
		{
			Node	*child		= node->neighbours[j];
			int	child_index	= child->row * board.cols + child->col;

			// child is either a side (top, bottom, etc..) or an unvisited node
			if ( (child->row < 0 || child->col < 0) || !visited[child_index] )
			{
				int weight;
				if (	(child->row < 0 || child->col < 0) ||
					state[child_index] == player)
				{
					weight = 0;
				}
				else if (state[child_index] == board.empty_cell)
				{
					weight = 1;
				}
				else
				{
					continue;
				}

				if (node->dist + weight < child->dist)
				{
					child->dist = node->dist + weight;
					child->prev = node;
				}
			}
		}
	}

	free(visited);

	if (player == board.players[0])
	{
		printf("Shortest path to right for %c is %d\n", player, dest->dist);
	}
	else
	{
		printf("Shortest path to bottom for %c is %d\n", player, dest->dist);
	}

	return dest->dist;
}

Coordinate	compute_best_move(char player)
{
	printf("Computing best move for %c\n", player);
	size_t	length		= board.rows * board.cols;
	char	*current_state	= malloc(length * sizeof(char));

	memcpy(current_state, board.contents, length * sizeof(char));

	Coordinate	c	= {-1, -1};
	int		v	= -INFINITY;

	int		turn	= player == board.players[0] ? 0 : 1;

	for (int i = 0; i < length; i++)
	{
		if (current_state[i] != board.empty_cell)	continue;
		current_state[i]	= player;
		int n			= alphabeta(current_state, 10, -INFINITY, INFINITY, turn, turn);
		current_state[i]	= board.empty_cell;

		if (n > v)
		{
			c.x = i / board.rows;
			c.y = i % board.rows;
			v = n;
		}
	}

	free(current_state);

	printf("For player %c best move is %c%2d\n", player, alphabet[c.x], c.y + 1);

	return c;
}
