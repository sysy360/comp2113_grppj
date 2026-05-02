#include <iostream>
#include <cstdlib>
#include <ctime>

#include "globals.h"
#include "game.h"
#include "score.h"

// ─────────────────────────────────────────────────────────────
//  main – program entry point
//
//  Flow:
//    1. Seed random number generator
//    2. Print title banner
//    3. Player chooses difficulty
//    4. Run game loop
//    5. On win: prompt username, save score, show leaderboard
//    6. On lose: show encouragement, show leaderboard
// ─────────────────────────────────────────────────────────────
int main() {
    // Seed RNG with current time for randomised maps each run
    srand((unsigned int)time(nullptr));

    // ── Title Banner ──────────────────────────────────────────
    std::cout << "\n";
    std::cout << "  ╔═══════════════════════════════════════════╗\n";
    std::cout << "  ║         M A Z E   A D V E N T U R E       ║\n";
    std::cout << "  ║   Escape the maze before the Virus wins!   ║\n";
    std::cout << "  ╚═══════════════════════════════════════════╝\n";

    // ── Choose Difficulty ─────────────────────────────────────
    Difficulty diff = chooseDifficulty();

    std::string diffStr;
    switch (diff) {
        case EASY:   diffStr = "Easy";   break;
        case MEDIUM: diffStr = "Medium"; break;
        case HARD:   diffStr = "Hard";   break;
    }
    std::cout << "\n  Difficulty selected: " << diffStr << "\n";
    std::cout << "  Generating maze... Good luck!\n";

    // ── Run Game ──────────────────────────────────────────────
    int steps = runGame(diff);

    // ── Load existing leaderboard ─────────────────────────────
    std::vector<ScoreRecord> scores = loadScores();

    if (steps > 0) {
        // Player won – record score
        std::string username = promptUsername();
        addScore(scores, username, diffStr, steps);
        saveScores(scores);
        std::cout << "\n  Score saved!\n";
    } else {
        std::cout << "  Better luck next time, Adventurer!\n";
    }

    // ── Display leaderboard ───────────────────────────────────
    displayScores(scores);

    return 0;
}
