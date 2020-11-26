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

#define INF		((int)INFINITY)

GameBoard		board;
static const char	*alphabet	= "abcdefghijklmnopqrstuvwxyz";

const Coordinate	TOP	= {INF, -INF};
const Coordinate	BOTTOM	= {INF, +INF};
const Coordinate	LEFT	= {-INF, INF};
const Coordinate	RIGHT	= {+INF, INF};

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

	memset(board.contents, board.empty_cell, contents_len * sizeof(char));

	while (!board.finished)
	{
		board.turn++;
		
		int	row	= -1;
		int	col	= -1;
		bool	valid_cell	= false;

		print_board();

		int player = board.turn % 2;


		if (player == board.computer_turn)
		{
			// do computer turn
			Coordinate move = compute_best_move(board.players[player]);

			printf("Computer move: %c%2d\n", alphabet[move.x], move.y + 1);

			set_cell(board.players[board.turn % 2], move.x, move.y);
			board.history[board.turn] = move;
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
			int	d	= djikstra(board.contents, board.players[player]);
			printf("Player %d has %d moves to go\n", player + 1, d);
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

char	check_winner(char *state)
{
	printf("Checking winner\n");
	if (djikstra(state, board.players[0]) == 0) return board.players[0];
	if (djikstra(state, board.players[1]) == 0) return board.players[1];

	printf("No winner yet!\n");

	return 0;
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

	//printf("cell %c%2d has %d neighbours\n", alphabet[row], col, n);

	return	n;
}

/*
int	*get_neighbours(int index)
{
	int col = index % board.cols;
}
*/

Coordinate	compute_best_move(char player)
{
	size_t	size		= board.rows * board.cols;
	char	*temp_state	= malloc(sizeof(char) * size);
	char	opponent	= player == board.players[0] ? board.players[1] : board.players[0];

	memcpy(temp_state, board.contents, size * sizeof(char));

	Coordinate	best_move	= {0, 0};
	int		v		= -INF;

	for (int i = 0; i < size; i++)
	{
		// skip occupied cells
		if (temp_state[i] != board.empty_cell) continue;

		int col = i % board.cols;
		int row = (i - col) / board.cols;
		// check aB value of this move
		//printf("Checking value of move %c%d for %c\n", alphabet[row], col + 1, player);
		temp_state[i] = player;

		int s = alphabeta(temp_state, 5, -INF, INF, player, opponent);

		if (s > v)
		{
			int col = i % board.cols;
			int row = (i - col) / board.cols;

			printf("aB for %c%d = %d\n", alphabet[row], col + 1, s);

			v = s;
			best_move.x = row;
			best_move.y = col;
		}
		
		temp_state[i] = board.empty_cell;
	}

	return best_move;
}

int	alphabeta(char *state, int depth, int alpha, int beta, char maxp, char player)
{
	/*
	printf("alphabeta d=%d, a=%d, b=%d, maxp=%d, p=%d\n",
		depth,
		alpha,
		beta,
		maxp,
		player);
		*/

	if (depth == 0 || check_winner(state) != 0)
	{
		//printf(">>>terminal -- getting heuristic\n");
		return alphabeta_heuristic(state, maxp);
	}

	char	minp	= maxp == board.players[0] ? board.players[1] : board.players[0];
	int	cells	= board.rows * board.cols;

	//printf("There are %d cells\n", cells);

	if (player == maxp)	// maximizing state
	{
		//printf(">>>>maxp\n");
		int v = -INF;

		for (int anythingotherthani = 0; anythingotherthani < cells; anythingotherthani++)
		{
			//printf("FUCK FUCK FUCK\n");
			if (state[anythingotherthani] == board.empty_cell)
			{
				int col = anythingotherthani % board.cols;
				int row = (anythingotherthani - col) / board.cols;

				/*
				printf(">>>>checking maxp move %c%d\n",
					alphabet[row],
					col + 1);
					*/

				state[anythingotherthani]	= maxp;
				v		= max(v, alphabeta(state, depth - 1, alpha, beta, maxp, minp));
				alpha		= max(v, alpha);
				state[anythingotherthani]	= board.empty_cell;
			}

			if (beta <= alpha)
			{
				break;
			}
		}

		return v;
	}
	else
	{
		//printf(">>>>minp\n");
		int v = INF;

		for (int i = 0; i < cells; i++)
		{
			if (state[i] == board.empty_cell)
			{
				int col = i % board.cols;
				int row = (i - col) / board.cols;

				/*
				printf(">>>>checking minp move %c%d\n",
					alphabet[row],
					col);
					*/
				state[i]	= minp;
				v		= min(v, alphabeta(state, depth - 1, alpha, beta, maxp, maxp));
				beta		= min(v, beta);
				state[i]	= board.empty_cell;
			}
			else
			{
				//printf(">>>>ignoring cell %c\n", state[i]);
			}

			if (beta <= alpha)
			{
				break;
			}
		}

		return v;
	}
}

int	alphabeta_heuristic(char *state, char player)
{
	char opponent = player == board.players[0] ? board.players[1] : board.players[0];
	//return djikstra(state, opponent) - djikstra(state, player);
	int	moves_left_player	= djikstra(state, player);
	int	moves_left_opponent	= djikstra(state, opponent);

	printf("Heuristic: %d moves left for player, %d moves left for opponent\n", moves_left_player, moves_left_opponent);

	return moves_left_opponent - moves_left_player;
}

int	djikstra(char *state, char player)
{
	size_t	size		= board.rows * board.cols + 2;
	int	*dist		= malloc(sizeof(int) * size);
	bool	*visited	= malloc(sizeof(bool) * size);
	Coordinate *nodes	= malloc(sizeof(Coordinate) * size);

	memset(dist, INF, size * sizeof(int));
	memset(visited, false, size * sizeof(bool));

	//nodes[0]	= TOP;
	//nodes[1]	= BOTTOM;
	//nodes[2]	= LEFT;
	//nodes[3]	= RIGHT;

	int	dest_index = 1;

	if (player == board.players[0])
	{
		//dist[2]		= 0;	// distance to left is 0
		//dest_index	= 3;	// destination is right

		/*
		printf("DJIKSTRA destination right, initial dist: %d\n",
			dist[dest_index]);
			*/
		nodes[0]	= LEFT;
		nodes[1]	= RIGHT;

		dist[0]		= 0;
	}
	else
	{
		//dist[0]		= 0;	// distance to top is 0
		//dest_index	= 1;	// destination is bottom

		/*
		printf("DJIKSTRA destination bottom, initial dist: %d\n",
			dist[dest_index]);
			*/
		nodes[0]	= TOP;
		nodes[1]	= BOTTOM;

		dist[0]		= 0;
	}

	for (int i = 2; i < size; i++)
	{
		int	j	= i - 2;
		int	col	= j % board.cols;
		int	row	= (j - col) / board.cols;
		nodes[i].x	= row;
		nodes[i].y	= col;
	}

	for (int i = 0; i < size; i++)
	{
		// select node with minimum distance
		int node_index	= 0;
		int node_dist	= INF;

		for (int j = 0; j < size; j++)
		{
			if (dist[j] < node_dist && !visited[j])
			{
				node_index	= j;
				node_dist	= dist[j];
			}
		}

		visited[node_index] = true;

		//if (node_dist == INF) continue;	// ignore nodes with infinite distance-- must be opponents
		int row = nodes[node_index].x;
		int col = nodes[node_index].y;
		if (node_dist == INF)
		{
			/*
			printf("Skipping node %d (%d) %d,%d / %c%d\n",
			node_index,
			node_index - 4,
			row,
			col,
			node_index < 4 ? 'A' + node_index : alphabet[row], col + 1);
			*/
			continue;
		}

		/*
		printf("Evaluating node %d (%d) %d,%d / %c%d\n",
		node_index,
		node_index - 4,
		row,
		col,
		node_index < 4 ? 'A' + node_index : alphabet[row], col + 1);
		*/

		// find all children of this node
		for (int j = 0; j < size; j++)
		{
			// node is a neighbour and hasn't already been processed
			if (neighbouring(nodes[node_index], nodes[j]) && !visited[j] && j != node_index)
			{
				if (j < 2)	// node is one of top, bottom, left, right
				{
					if (node_index < 2)
					{
						printf("ERROR connecting two edges!\n");
					}
					if (dist[j] > dist[node_index])
					{
						// cost of visiting edge is 0
						dist[j] = dist[node_index];
					}
				}
				else if (state[j - 2] == board.empty_cell)
				{
					if (dist[j] > dist[node_index] + 1)
					{
						// cost of visiting empty node is 1
						dist[j] = dist[node_index] + 1;
					}
				}
				else if (state[j - 2] == player)
				{
					if (dist[j] > dist[node_index])
					{
						// cost of visting owned cell is 0
						dist[j] = dist[node_index];
					}
				}
			}
		}
	}

	//printf("For player %c, shortest path is %d\n", player, dist[dest_index]);

	if (dist[dest_index] == INF || dist[dest_index] == -INF) printf("Could not find a path!\n");
	return dist[dest_index];
}

bool	neighbouring(Coordinate a, Coordinate b)
{
	if (a.y == TOP.y)
	{
		return b.x == 0;
	}
	else if (b.y == TOP.y)
	{
		return a.x == 0;
	}

	if (a.y == BOTTOM.y)
	{
		return b.x + 1 == board.rows;
	}
	else if (b.y == BOTTOM.y)
	{
		return a.x + 1 == board.rows;
	}

	if (a.x == LEFT.x)
	{
		return b.y == 0;
	}
	else if (b.x == LEFT.x)
	{
		return a.y == 1;
	}

	if (a.x == RIGHT.x)
	{
		return b.y + 1 == board.cols;
	}
	else if (b.x == RIGHT.x)
	{
		return a.y + 1 == board.cols;
	}

	if (	a.x == b.x && 
		(a.y == b.y + 1 || a.y == b.y - 1))
	{
		return true;
	}

	if (	a.y == b.y &&
		(a.x == b.x + 1 || a.x == b.x - 1))
	{
		return true;
	}

	if (	(a.x - 1 == b.x && a.y - 1 == b.y) ||
		(a.x + 1 == b.x && a.y + 1 == b.y))
	{
		return true;
	}

	return false;
}
