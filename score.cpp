#include "score.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
using namespace std;

//=========================================================
// Loading the Scores (loading scores from txt.file)
//=========================================================
vector<ScoreRecord> loadScores()
{
    vector<ScoreRecord> records;
    ifstream input(SCORE_FILE);

    // Edge-Case if file may not exists, while checking failure when opening file is attempted
    if (!input.is_open())
    {
        return records;
    }

    string line;

    // adding the scores into the leaderboard
    while (getline(input, line))
    {
        if (line.empty())
        {
            continue;
        }
        istringstream extract(line);
        ScoreRecord new_player;

        if (extract >> new_player.username >> new_player.difficulty >> new_player.steps)
        {
            records.push_back(new_player);
        }
    }

    return records;
}

//=========================================================
// Saving the Scores (Storing the scores into a .txt file)
//=========================================================
void saveScores(const vector<ScoreRecord> &records)
{
    // Rewriting the scores everytime, for later addScores to add the scores and sort it out
    ofstream output(SCORE_FILE);

    // Any fail to open the file, will produce an error warning
    if (!output.is_open())
    {
        cerr << "Warning! Could not open" << SCORE_FILE << "for writing." << endl;
        return;
    }

    // Every available elements in the records will then be saved into the .txt file.
    for (const auto &r : records)
    {
        output << r.username << " " << r.difficulty << " " << r.steps << endl;
    }
}

//========================================================================
// Adding score
//========================================================================
void addScore(vector<ScoreRecord> &records, const string &username, const string &diff, int steps)
{
    // adding newly added player progress into the memory.
    ScoreRecord inputs;
    inputs.username = username;
    inputs.difficulty = diff;
    inputs.steps = steps;
    records.push_back(inputs);

    // sorting the player progress leaderboard with fewer steps as higher in rank
    sort(records.begin(), records.end(),
         [](const ScoreRecord &a, const ScoreRecord &b)
         {
             return a.steps < b.steps;
         });
}

//=============================================================================================
// Display Outlay / Positioning
//============================================================================================
void displayScores(const std::vector<ScoreRecord> &records)
{
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║            HALL  OF  ADVENTURERS         ║\n";
    std::cout << "  ╠═══════╦═══════════╦════════════╦════════╣\n";
    std::cout << "  ║ Rank  ║ Name      ║ Difficulty ║ Steps  ║\n";
    std::cout << "  ╠═══════╬═══════════╬════════════╬════════╣\n";

    if (records.empty())
    {
        std::cout << "  ║          No records yet!                ║\n";
    }
    else
    {
        for (int i = 0; i < (int)records.size(); ++i)
        {
            const auto &r = records[i];
            // Column widths: rank=5, name=9, diff=10, steps=6
            char buf[120];
            snprintf(buf, sizeof(buf),
                     "  ║  %-4d ║ %-9s ║ %-10s ║ %-6d ║",
                     i + 1, r.username.c_str(), r.difficulty.c_str(), r.steps); // avoid buffer overflow
            std::cout << buf << "\n";
        }
    }
    std::cout << "  ╚═══════╩═══════════╩════════════╩════════╝\n\n";
}

//===============================================================================================
// Prompt Username
//=============================================================================================

static bool isValidUsername(const string &s)
{
    if (s.empty())
    {
        return false;
    }

    for (char c : s) // iterating for char element, for upper-case checking
    {
        if (!isupper((unsigned char)c)) // unsigned char as an edge safety ensuring only 0-255 char values is allowed, and isupper to only accept capital letters.
        {
            return false;
        }
    }
    return true;
}

string promptUsername() // prompting player to input their username.
{
    cout << "\n What is your name, Hero?" << "(Accept Uppercase only, and alphabetical only)\n";

    int wrong_attempts = 0;
    string name;
    while (true)
    {
        cin >> name;
        if (isValidUsername(name))
        {
            return name;
        }

        wrong_attempts++;

        if (wrong_attempts >= 3)
        {
            cout << "\n I Don't Really Get Paid Enough for this, Well, anyway. You seem like a person who would be named as BOB, so let's keep it that way. \n";
            return "BOB"; // yeah, I think this is clear enough. BOB :) , get it? Well, you can laugh now.
        }

        cout << "Input name accordingly! " << "(Accept Uppercase only, and alphabetical only)\n ";
    }
}