#include "board.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "utility.h"
#include "ansi_colour.h"

GameBoard	board;
const char	*alphabet	= "abcdefghijklmnopqrstuvwxyz";

static	Graph	*graph = NULL;

/*
 * Set the cell at [row][col] to player
 */
void	set_cell(int row, int col, char player)
{
	//size_t	index	= row * board.cols + col;
	int	index		= index_from_location(row, col);
	board.state[index]	= player;
}

char	get_cell(int row, int col)
{
	int	index	= index_from_location(row, col);
	char	cell	= board.state[index];

	return cell;
}

char	*compute_pie()
{
	//size_t	size	= board.rows * board.cols;
	size_t	size	= board.size;
	char	*nstate	= malloc(sizeof(char) * size);
	char	*state	= board.state;

	memset(nstate, board.empty_cell, sizeof(char) * size);

	for (int i = 0; i < size; i++)
	{
		if (state[size - i - 1] == board.players[0])
		{
			nstate[i] = board.players[1];
			break;
		}
	}

	return nstate;

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
				Location	l	= location_from_segment(line, i - sequence_start);
				char		player	= get_cell(l.row, l.col);

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
				(sequence_end - i - 1 < 2 && line > cols) )
			{
				char *fmt_seg;

				strcpy(buffer, seg);

				fmt_seg	= colorize(buffer, board.colours[0]);
				linep	= stpcpy(linep, fmt_seg);

				free(fmt_seg);
			}
			else if ( (i - sequence_start < 2 && line > rows) ||
				(sequence_end - i - 1 < 2 && line <= cols) )
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

		if (line < rows && line < cols)
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

void	record_turn(int row, int col)
{
	Location move = {row, col};

	board.history[board.history_length] = move;
	board.history_length++;
}

char	*serialize_history()
{
	size_t	move_size	= 4,
		size		= board.history_length * move_size,
		length		= 0;
	char	*history	= calloc(size, sizeof(char)),
		*buffer		= calloc(size, sizeof(char)),
		*result;

	for (int i = 0; i < board.history_length; i++)
	{
		char	row	= board.history[i].row + alphabet[0];
		int	col	= board.history[i].col + 1;

		if (board.history[i].row < 0 || board.history[i].col < 0)
		{
			sprintf(buffer, "pie ");
		}
		else
		{
			sprintf(buffer, "%c%d ", row, col);
		}

		size_t	bufflen	= strlen(buffer);

		if (length + bufflen > size)
		{
			size	+=	board.turn * move_size;
			history	=	realloc(history, sizeof(char) * size);
		}

		strcpy(&(history[length]), buffer);
		length		+=	strlen(buffer);
	}

	char	*endp		= &(history[size - 1]);

	while (*endp == ' ' || *endp == '\0')
	{
		*endp	= '\0';
		endp--;
	}

	size_t	min_size	= strlen(history) + 1;
	result			= malloc(sizeof(char) * min_size);

	strcpy(result, history);

	free(history);
	free(buffer);

	return result;
}

Graph	*graph_from_state(char *state)
{
	if (graph == NULL)
	{
		graph = construct_graph();
	}

	for (int i = 0; i < graph->size; i++)
	{
		Node	*n	= &(graph->nodes[i]);

		n->dist		= INT_MAX;
		n->visited	= false;
		
		if ( !(i < 4) )
		{
			n->state = state[i - 4];
		}
	}

	return graph;

}

int	index_from_location(int row, int col)
{
	return row * board.cols + col;
}

Location	location_from_index(int index)
{
	int	col	= index % board.cols,
		row	= (index - col) / board.cols;

	Location l	= {row, col};
	return l;
}

Graph	*construct_graph()
{
	size_t	size	= board.size + 4;
	Graph	*graph	= malloc(sizeof(Graph));
	graph->nodes	= malloc(sizeof(Node) * size);
	graph->size	= size;

	graph->top	= &(graph->nodes[0]);
	graph->bottom	= &(graph->nodes[1]);
	graph->left	= &(graph->nodes[2]);
	graph->right	= &(graph->nodes[3]);

	// set size and state of each 'board' node
	for (int i = 4; i < graph->size; i++)
	{
		Node *n		= &(graph->nodes[i]);
		n->size		= 6;
		n->state	= board.empty_cell;
		//n->state	= state[i - 4];
	}

	// set size and state of each 'border' node
	graph->top->size	= board.cols;
	graph->top->state	= board.players[1];
	graph->bottom->size	= board.cols;
	graph->bottom->state	= board.players[1];
	graph->left->size	= board.rows;
	graph->left->state	= board.players[0];
	graph->right->size	= board.rows;
	graph->right->state	= board.players[0];

	// allocate neighbours list for each node
	for (int i = 0; i < graph->size; i++)
	{
		Node	*n	= &(graph->nodes[i]);
		n->neighbours	= malloc(sizeof(Node *) * n->size);
		n->count	= 0;
		n->dist		= INT_MAX;
		n->visited	= false;
	}


	// set all nodes in the first or last column to neighbour the top/bottom
	// edge
	for (int col = 0; col < board.cols; col++)
	{
		int	row0	= 0,
			rown	= board.rows - 1;

		//int index_0	= row0 * board.cols + col + 4;
		//int index_n	= rown * board.cols + col + 4;
		
		int	index_0	= index_from_location(row0, col) + 4,
			index_n	= index_from_location(rown, col) + 4;

		set_neighbouring(graph->top, &(graph->nodes[index_0]));
		set_neighbouring(graph->bottom, &(graph->nodes[index_n]));
	}

	// set all nodes in the first or last row to neighbour the left/right
	// edge
	for (int row = 0; row < board.rows; row++)
	{
		int	col0	= 0,
			coln	= board.cols - 1;

		//int index_0	= row * board.cols + col0 + 4;
		//int index_n	= row * board.cols + coln + 4;
		
		int	index_0	= index_from_location(row, col0) + 4,
			index_n	= index_from_location(row, coln) + 4;

		set_neighbouring(graph->left, &(graph->nodes[index_0]));
		set_neighbouring(graph->right, &(graph->nodes[index_n]));
	}

	// for all non-border nodes set their neighbours
	for (int i = 4; i < graph->size; i++)
	{
		Node	*n	= &(graph->nodes[i]),
			*nbr;

		// node coordinates
		//int	col	= (i - 4) % board.cols,
			//row	= (i - col - 4) / board.cols,
			//nbr_i;

		int	col,
			row,
			nbr_i;

		Location l	= location_from_index(i - 4);
		row		= l.row;
		col		= l.col;

		// upper-right neighbour
		if (row > 0)
		{
			//nbr_i	= (row - 1) * board.cols + col + 4;
			nbr_i	= index_from_location(row - 1, col) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}

		// upper-left neighbour
		if (col > 0)
		{
			//nbr_i	= row * board.cols + (col - 1) + 4;
			nbr_i	= index_from_location(row, col - 1) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}

		// bottom-left neighbour
		if (row + 1 < board.rows)
		{
			//nbr_i	= (row + 1) * board.cols + col + 4;
			nbr_i	= index_from_location(row + 1, col) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}

		// bottom-right neighbour
		if (col + 1 < board.cols)
		{
			//nbr_i	= row * board.cols + (col + 1) + 4;
			nbr_i	= index_from_location(row, col + 1) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}

		// top neighbour
		if (row > 0 && col > 0)
		{
			//nbr_i	= (row - 1) * board.cols + (col - 1) + 4;
			nbr_i	= index_from_location(row - 1, col - 1) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}

		// bottom neighbour
		if (row + 1 < board.rows && col + 1 < board.cols)
		{
			//nbr_i	= (row + 1) * board.cols + (col + 1) + 4;
			nbr_i	= index_from_location(row + 1, col + 1) + 4;
			nbr	= &(graph->nodes[nbr_i]);

			set_neighbouring(n, nbr);
		}
	}

	return graph;
}

void	set_neighbouring(Node *a, Node *b)
{
	bool	a_to_b	= false;
	bool	b_to_a	= false;

	for (int i = 0; i < a->count; i++)
	{
		if (a->neighbours[i] == b)
		{
			a_to_b = true;
			break;
		}
	}

	for (int i = 0; i < b->count; i++)
	{
		if (b->neighbours[i] == a)
		{
			b_to_a = true;
			break;
		}
	}

	if (!a_to_b)
	{
		a->neighbours[a->count] = b;
		a->count++;
	}

	if (!b_to_a)
	{
		b->neighbours[b->count] = a;
		b->count++;
	}

}

void	free_board()
{
	free(board.state);

	if (graph)
	{
		for (int i = 0; i < graph->size; i++)
		{
			free(graph->nodes[i].neighbours);
		}

		free(graph->nodes);
		free(graph);

		graph = NULL;
	}
}

Location	location_from_segment(int line, int segment)
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

	Location l = {row, col};

	return l;

}
