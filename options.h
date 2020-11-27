#pragma once

// set this to 0 if colours don't work
#define	USE_COLOURS		1

// default dimensions of board
#define	BOARD_COLS		11
#define	BOARD_ROWS		11

// player colours, for more options see ansi_colour.h
#define	PLAYER_ONE_COLOUR	RED
#define PLAYER_TWO_COLOUR	BLUE

// player markers
#define	PLAYER_ONE_CHAR		'X'
#define	PLAYER_TWO_CHAR		'O'

// default computer player
// <0	= No computer player
//  0	= Computer player 1
//  1	= Computer player 2
//  2	= Computer player 1 and 2
#define	COMPUTER_PLAYER		-1

// default value to enable pie rule
#define	DEFAULT_PIE		true

// default max depth to search minmax tree
#define	MAX_AB_DEPTH		1
