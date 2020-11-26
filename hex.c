#include "hex.h"
#include "ansi_colour.h"
#include "utility.h"
#include "board.h"
#include "options.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

#define COLOUR_ONE	ANSI_STYLE(FG_BRIGHT PLAYER_ONE_COLOUR)
#define	COLOUR_TWO	ANSI_STYLE(FG_BRIGHT PLAYER_TWO_COLOUR)

#define	STRING_TRUE	"true"

// strings for specifying arguments
// e.g. ./hex -r=5 -c=6 -computer=1
#define	ARG_ROW	"r="
#define	ARG_COL	"c="
#define ARG_COMPUTER	"computer="
#define	ARG_PIE	"pie="
#define ARG_AB_DEPTH	"ab-depth="

extern	const char	*alphabet;
extern	GameBoard	board;

bool	using_pie	= DEFAULT_PIE;

int	AB_DEPTH	= MAX_AB_DEPTH;

int	main(int argc, char **argv)
{
	if (!process_arguments(argc, argv))
	{
		return 0;
	}

	board.turn		= -1;

	size_t	size		= board.rows * board.cols;
	board.size		= size;
	board.state		= malloc(board.size * sizeof(char));

	board.empty_cell	= ' ';
	board.players[0]	= PLAYER_ONE_CHAR;
	board.colours[0]	= COLOUR_ONE;
	board.players[1]	= PLAYER_TWO_CHAR;
	board.colours[1]	= COLOUR_TWO;

	memset(board.state, board.empty_cell, size * sizeof(char));

	// number of possible moves = number of cells + 1 (incase someone calls pie)
	board.history_size	= size + 1;
	board.history_length	= 0;
	board.history		= malloc(board.history_size * sizeof(Location));

	start_game();
	free_board();
}

/*
 * Process argv to see if user specified rows, columns, or
 * computer player
 */
bool	process_arguments(int argc, char **argv)
{
	int	r		= -1,
		c		= -1,
		computer	= COMPUTER_PLAYER,
		max_size	= strlen(alphabet),
		ab_depth	= MAX_AB_DEPTH;
	bool	pie		= DEFAULT_PIE;

	for (int i = 1; i < argc; i++)
	{
		char *arg = argv[i];
		if (*arg == '-')
		{
			arg++;
			if (strncmp(arg, ARG_ROW, strlen(ARG_ROW)) == 0)		//ROW
			{
				arg	+= strlen(ARG_ROW);
				r	= atoi(arg);
			}
			else if (strncmp(arg, ARG_COL, strlen(ARG_COL)) == 0)		//COL
			{
				arg	+= strlen(ARG_COL);
				c	= atoi(arg);
			}
			else if (strncmp(arg, ARG_COMPUTER, strlen(ARG_COMPUTER)) == 0)	//COMPUTER
			{
				arg		+= strlen(ARG_COMPUTER);
				computer	= atoi(arg);
			}
			else if (strncmp(arg, ARG_PIE, strlen(ARG_PIE)) == 0)		//PIE
			{
				arg	+= strlen(ARG_PIE);
				for (char *c = arg; *c; c++) *c = tolower(*c);
				pie	= strncmp(arg, STRING_TRUE, strlen(STRING_TRUE)) == 0;
			}
			else if (strncmp(arg, ARG_AB_DEPTH, strlen(ARG_AB_DEPTH)) == 0)	// MAX AB DEPTH
			{
				arg		+= strlen(ARG_AB_DEPTH);
				ab_depth	= atoi(arg);
			}
			else
			{
				printf("Invalid argument: %s\n", argv[i]);
				return false;
			}
		}
	}

	if (	!(r > 0 && c > 0) ||
		!(r <= max_size && c <= max_size))
	{
		// If given r & c not in bounds, use default values
		board.rows	= BOARD_ROWS;
		board.cols	= BOARD_COLS;
	}
	else
	{
		// rows always > cols
		if (c > r)
		{
			int t	= c;
			c	= r;
			r	= t;
		}

		board.rows	= r;
		board.cols	= c;
	}

	if (computer == 0 || computer == 1)
	{
		// use given computer player
		board.computer_player = computer;
	}
	else
	{
		// use default computer player
		board.computer_player = COMPUTER_PLAYER;
	}

	using_pie	= pie;

	AB_DEPTH	= max(ab_depth, 0);

	return true;
}

/*
 * Game loop
 */
void	start_game()
{
	char	winner;
	char	computer_player	= board.computer_player > 0? '\0' : board.players[board.computer_player];
	
	// Coloured strings
	char	*player_one_name	= COLOUR_ONE "Player 1" ANSI_CLEAR();
	char	*player_two_name	= COLOUR_TWO "Player 2" ANSI_CLEAR();
	char	*computer_name;

	if (board.computer_player == 0)
	{
		computer_name	= COLOUR_ONE "Computer" ANSI_CLEAR();
	}
	else if (board.computer_player == 1)
	{
		computer_name	= COLOUR_TWO "Computer" ANSI_CLEAR();
	}

	while ( !(winner = check_winner(board.state)) )
	{
		print_board();

		board.turn++;
		
		int	row		= -1;
		int	col		= -1;
		bool	valid_cell	= false;

		int	player_turn	= board.turn % 2;
		char	player		= board.players[player_turn];

		// Second player can choose to invoke pie
		if (board.turn == 1 && using_pie)
		{
			bool c = (player_turn == board.computer_player);
			if (!c)
			{
				printf("%s would you like to invoke pie y/N? ", player_two_name);
			}

			if (invoke_pie(c))
			{
				if (c)
				{
					printf("%s invoked pie\n", computer_name);
				}

				// pie is recorded as row = col = -1
				record_turn(-1, -1);
				continue;
			}
		}

		// Computer moves
		if (player_turn == board.computer_player)
		{
			Location	move	= find_optimal_move(player);

			row	= move.row;
			col	= move.col;

			printf("%s chose cell %c%d\n", computer_name, alphabet[row], col + 1);
		}
		else
		{
			do
			{
				// Prompt player for cell
				if (player_turn == 0)
				{
					printf("%s make a move: ", player_one_name);
				}
				else
				{
					printf("%s make a move: ", player_two_name);
				}

				int	c	= -1;

				char	r	= '\0',
					*line	= NULL;

				size_t	size	= 0;

				getline(&line, &size, stdin);

				sscanf(line, "%c%d", &r, &c);

				free(line);

				row	= r - alphabet[0];
				col	= c - 1;

				// check cell is valid
				valid_cell = validate_move(row, col);

				if (!valid_cell)
				{
					printf("A move must be in the form of a row followed by a column (%c1 - %c%d). "
						"A move can only be made in an empty cell.\n",
						alphabet[0], alphabet[board.rows - 1], board.cols);
				}

			}	while(!valid_cell);
		}

		// mark the selected cell and record the move
		record_turn(row, col);
		set_cell(row, col, player);
	}

	print_board();

	if		(winner == computer_player)
	{
		printf("The computer won! Better luck next time.\n");
	}
	else if		(winner == board.players[0])
	{
		printf("%s wins!\n", player_one_name);
	}
	else
	{
		printf("%s wins!\n", player_two_name);
	}

	char	*history	= serialize_history();
	printf("Turn history: %s\n", history);
	free(history);
}

/*
 * Check if a given coordinate is within the bounds
 * of the board's contents
 */
bool	validate_move(int row, int col)
{
	// cell isn't out of bounds
	bool invalid =	row >= board.rows ||
			row < 0 ||
			col >= board.cols || 
			col < 0 ;

	// cell is unoccupied
	invalid	|=	get_cell(row, col) != board.empty_cell;

	return !invalid;

}

/*
 * Asks the player (if computer == false) if they would like to invoke pie
 * otherwise, computer determines
 *
 * if yes, then mirror the board and swap the first move 
 */
bool	invoke_pie(bool computer)
{
	// always player two
	char	player = board.players[1];

	if (computer)
	{
		if (!should_computer_invoke_pie())	return false;
	}
	else
	{
		char	c;

		//printf("Player 1, you have the option to invoke the pie rule.\n");
		//printf("Would you like to do so (y/N)? ");

		//fflush(stdin);
		//scanf("%c", &c);
		//fflush(stdin);

		c = getchar();
		flush_input();

		if (!(c == 'y' || c == 'Y'))		return false;
	}

	// get transposed board and swap it with the current one
	char	*new_state	= compute_pie();
	free(board.state);
	board.state		= new_state;

	return true;
}

/*
 */
bool	should_computer_invoke_pie()
{
	char	computer	= board.players[board.computer_player],
		opponent	= board.players[(board.computer_player + 1) % 2],
		*pie_state	= compute_pie();

	int	ab_current	= alphabeta(board.state, AB_DEPTH, INT_MIN, INT_MAX, computer, computer),
		ab_pie		= alphabeta(board.state, AB_DEPTH, INT_MIN, INT_MAX, computer, opponent);

	free(pie_state);

	return ab_pie > ab_current;
}

/*
 * This checks if the game currently has a winner. This is determined
 * by checking if a given player's shortest-path is 0
 */
char	check_winner(char *state)
{
	if (shortest_path(state, board.players[0]) == 0) return board.players[0];
	if (shortest_path(state, board.players[1]) == 0) return board.players[1];

	return '\0';
}

/*
 * Perform an alpha-beta pruned search to find the move
 * with the best chances for player
 */
Location	find_optimal_move(char player)
{
	size_t		size		= board.size;
	char		*temp_state	= malloc(sizeof(char) * size),
			opponent	= player == board.players[0] ? board.players[1] : board.players[0];
	Location	best_move	= {-1, -1};
	int		best_score	= INT_MIN;

	memcpy(temp_state, board.state, sizeof(char) * size);

	/*
	 * For every cell, check if placing a marker there would be
	 * advantageous for player
	 */
	for (int i = 0; i < size; i++)
	{
		// skip occupied cells
		if (temp_state[i] != board.empty_cell)	continue;

		temp_state[i]	= player;
		int	score	= alphabeta(temp_state, AB_DEPTH, INT_MIN, INT_MAX, player, opponent);

		if (score > best_score)
		{
			best_score = score;
			int	col	= i % board.cols;
			int	row	= (i - col) / board.cols;
			best_move.row	= row;
			best_move.col	= col;
		}
	}

	free(temp_state);
	return best_move;
}

/*
 * Perform alpha-beta pruned search on a given state
 *
 * This searches the tree of game states, where every node
 * is a possible move by each player. This searches all possible
 * moves from a given state
 */
int	alphabeta(char *state, int depth, int alpha, int beta, char maxp, char p)
{
	char	winner	= check_winner(state);
	char	minp	= maxp == board.players[0] ? board.players[1] : board.players[0];
	size_t	size	= board.size;

	if	(winner == maxp)
	{
		return INT_MAX;	// maxp wins, best possible score
	}
	else if	(winner == minp)
	{
		return INT_MIN;	// minp wins, worst possible score
	}

	if	(depth == 0)
	{
		return alphabeta_heuristic(state, maxp);
	}


	if	(p == maxp)
	{
		int v = INT_MIN;

		for (int i = 0; i < size; i++)
		{
			if (state[i] == board.empty_cell)
			{
				state[i] = maxp;
				v = max(v, alphabeta(state, depth - 1, alpha, beta, maxp, minp));
				state[i] = board.empty_cell;
			}

			alpha = max(v, alpha);

			if (alpha >= beta)	break;
		}

		return v;
	}
	else
	{
		int v = INT_MAX;

		for (int i = 0; i < size; i++)
		{
			if (state[i] == board.empty_cell)
			{
				state[i] = minp;
				v = min(v, alphabeta(state, depth - 1, alpha, beta, maxp, maxp));
				state[i] = board.empty_cell;
			}

			beta = min(v, beta);

			if (alpha >= beta)	break;
		}

		return v;
	}
}

/*
 * heuristic for alpha-beta search, returns the shortest path of opponent - the shortest
 * path of the maximizing player
 */
int	alphabeta_heuristic(char *state, char player)
{
	char opponent = player == board.players[0] ? board.players[1] : board.players[0];

	return shortest_path(state, opponent) - shortest_path(state, player);
}

/*
 * Implementation of Djikstra's algorithm for finding the shortest
 * path between two opposing edges for a given player 
 */
int	shortest_path(char *state, char player)
{
	size_t	size		= board.size + 4;

	// Create a graph representation of the state
	Graph	*graph		= graph_from_state(state);
	Node	*node,
		*dest;

	if (player == board.players[0]) // Player One's goal is to connect L-R (NW-SE on the printout)
	{
		node	= graph->left;
		dest	= graph->right;
	}
	else				// Player Two's goal is to connect T-b (NE-SW on the printout)
	{
		node	= graph->top;
		dest	= graph->bottom;
	}

	node->dist	= 0;

	for (int i = 0; i < size; i++)
	{
		int shortest = INT_MAX;

		// select node with shortest distance
		for (int j = 0; j < size; j++)
		{
			Node *n = &(graph->nodes[j]);
			if (!n->visited && n->dist < shortest)
			{
				node		= n;
				shortest	= n->dist;
			}
		}

		node->visited = true;

		// update neighbour distances
		for (int j = 0; j < node->count; j++)
		{
			Node	*nbr	= node->neighbours[j];
			// don't update visited nodes
			if (nbr->visited) continue;

			if (nbr->state == board.empty_cell &&
				node->dist + 1 < nbr->dist)
			{
				// empty cells cost 1 to mark
				nbr->dist	= node->dist + 1;
			}
			else if (nbr->state == player &&
				node->dist < nbr->dist)
			{
				// own cells cost 0, already marked
				nbr->dist	= node->dist;
			}
		}
	}

	int shortest = dest->dist;

	return shortest;
}
