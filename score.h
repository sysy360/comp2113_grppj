#ifndef SCORE_H
#define SCORE_H

#include "globals.h"
#include <vector>
#include <string>
#include <cctype>

std::vector <ScoreRecord> loadScores();

void saveScores(const std::vector<ScoreRecord> & records);

void addScore(std::vector<ScoreRecord> &records, const std::string &username, const std::string &diff, int steps);

void displayScores(const std::vector<ScoreRecord> &records);

std::string promptUsername();

#endif