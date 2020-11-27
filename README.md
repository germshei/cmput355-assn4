# CMPUT 355 Assignment 4

A basic hex player.

This is a basic implementation of the game Hex. The game allows two players to play, 
these can be two human players, a human player against a computer, or two computer players.

A Makefile for building the program is included in the repository.

Program options:

  -r=         Specify the number of rows on the board (integer - default: 11)
              1-26
              
  -c=         Specify the number of columns on the board (integer - default: 11)
              1-26
              
  -computer=  Specify which turn the computer plays
              0:  Computer is player 1
              1:  Computer is player 2
              2:  Both players are computer
              <0: No computer player
              
  -pie=       Specify if the pie rule is being used
              true:  enabled
              false: disabled
              
  -ab-depth=  Specify the maximum depth the computer can search for an optimal move
              >0

  -time       Collect and display statistics for the computer player(s)
  
  -print=     Specify whether the game state should be printed before each turn.
              true:  print enabled
              false: print disabled
              
Additionally, there are options located in the header file options.h, some of these include 
default values for the above options, while others are options not included in the above list.
These are:

  PLAYER_ONE_COLOUR ANSI colour code to colour player 1, red by default
  PLAYER_TWO_COLOUR ANSI colour code to colour player 2, blue by default
  
  PLAYER_ONE_CHAR   Marker for player 1 - 'X' by default
  PLAYER_TWO_CHAR   Marker for player 2 - 'O' by default
  
  USE_COLOURS       Whether or not to use colours, disable this if ANSI escape sequences don't work
