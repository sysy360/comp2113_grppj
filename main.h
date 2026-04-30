#ifndef MAIN_H
#define MAIN_H

#include "globals.h"
#include <string>

Difficulty selectDifficulty();
std::string getPlayerName();
int gameLoop(Difficulty difficulty);
const char* difficultyToString(Difficulty difficulty);

#endif
