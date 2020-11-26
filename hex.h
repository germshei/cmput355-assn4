#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "board.h"

bool	process_arguments(int argc, char **argv);
void	start_game();
bool	validate_move(int row, int col);
bool	invoke_pie(bool computer_turn);					// invokes pi, moves the board accordingly
bool	should_computer_invoke_pie();
char	check_winner(char * state);
Location	find_optimal_move(char player);
int	alphabeta(char *state, int depth, int alpha, int beta, char maxp, char p);
int	alphabeta_heuristic(char *state, char maxp);
int	shortest_path(char *state, char player);
