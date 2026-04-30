#include "main.h"
#include "score.h"

#include <cctype>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace {
bool isUpperAlphaName(const string& name) {
    if (name.empty()) {
        return false;
    }
    for (char ch : name) {
        if (!isupper(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}
} // namespace

const char* difficultyToString(Difficulty difficulty) {
    switch (difficulty) {
    case EASY:
        return "Easy";
    case MEDIUM:
        return "Medium";
    case HARD:
    default:
        return "Hard";
    }
}

Difficulty selectDifficulty() {
    cout << "Welcome! Which adventure would you like (Easy/Medium/Hard)?\n";

    int wrongAttempts = 0;
    string input;
    while (true) {
        cin >> input;

        for (char& ch : input) {
            ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
        }

        if (input == "EASY") {
            return EASY;
        }
        if (input == "MEDIUM") {
            return MEDIUM;
        }
        if (input == "HARD") {
            return HARD;
        }

        wrongAttempts++;
        if (wrongAttempts >= 5) {
            cout << "Too many invalid attempts. Defaulting to Hard.\n";
            return HARD;
        }

        cout << "Unknown difficulty. Please enter Easy, Medium, or Hard.\n";
    }
}

string getPlayerName() {
    cout << "What's your name, Adventurer? (Uppercase letters only)\n";

    int wrongAttempts = 0;
    string name;
    while (true) {
        cin >> name;

        if (isUpperAlphaName(name)) {
            return name;
        }

        wrongAttempts++;
        if (wrongAttempts >= 3) {
            cout << "Too many invalid attempts. Name saved as BOB.\n";
            return "BOB";
        }

        cout << "Input the correct name accordingly! (Uppercase letters only)\n";
    }
}

int gameLoop(Difficulty difficulty) {
    int stepsTaken = 0;

    // The actual round-by-round game logic should be provided by game/map modules.
    // Replace this block with your real game engine call when game.h is available.
    cout << "\n[Game Start] Difficulty: " << difficultyToString(difficulty) << "\n";
    cout << "Game module hook not linked yet. Enter steps taken for this run: ";
    cin >> stepsTaken;
    if (stepsTaken < 0) {
        stepsTaken = 0;
    }

    return stepsTaken;
}

int main() {
    Difficulty difficulty = selectDifficulty();
    int steps = gameLoop(difficulty);
    string username = getPlayerName();

    vector<ScoreRecord> records = loadScores();
    addScore(records, username, difficultyToString(difficulty), steps);
    saveScores(records);

    cout << "\nSaved Score: " << username
         << " | " << difficultyToString(difficulty)
         << " | Steps: " << steps << "\n";
    displayScores(records);

    return 0;
}
