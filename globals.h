#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

// Tile Management
#define TILE_EMPTY ' ' //Passable Pathway
#define TILE_WALL_H '#' // Horizontal Wall
#define TILE_WALL_V '|' //Vertical Wall
#define TILE_PLAYER 'P' // Player
#define TILE_VIRUS 'V' // VIRUS
#define TILE_EXIT 'E' //Exit / Completion
#define TILE_STUN 'Z' // Freeze/Stun Virus Progression

//Difficulty section
enum Difficulty {EASY, MEDIUM, HARD};

//GRID SIZES PER DIFFICULTY
#define EASY_SIZE 7 //7X7 Grid
#define MEDIUM_SIZE 12 // 12 x 12 Grid
#define HARD_SIZE 22 // 22 x 22 Grid

//Gate Differences per difficulty
#define EASY_GATES_MIN 1
#define EASY_GATES_MAX 3
#define MEDIUM_GATES_MIN 1
#define MEDIUM_GATES_MAX 3
#define HARD_GATES_MIN 4
#define HARD_GATES_MAX 6

//Stun Duration ranges (based on player steps)
#define MEDIUM_STUN_MIN 1
#define MEDIUM_STUN_MAX 3
#define HARD_STUN_MIN 1
#define  HARD_STUN_MAX 5

//Defining when the stun will be repleneished
#define STUN_REPLENISH_DELAY 3

//File for score saving and output/input
#define SCORE_FILE "scores.txt"

//Variable set up for score saving
struct ScoreRecord
{
    std::string username;
    std::string difficulty;
    int steps;
};

#endif