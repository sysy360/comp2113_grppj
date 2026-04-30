#ifndef GAME_H
#define GAME_H

#include "globals.h"

// main function to start the game loop
// returns the total steps if you win, or -1 if you lose/quit
int runGame(Difficulty diff);

// asks the player what difficulty they want to play on
// defaults to hard if they type it wrong too many times
Difficulty chooseDifficulty();

#endif // GAME_H