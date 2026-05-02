#include "game.h"
#include "map.h"
#include "virus.h"
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <queue>
#include <vector>
#include <termios.h>
#include <unistd.h>

// let the player pick a difficulty level
Difficulty chooseDifficulty() {
    std::cout << "\n  Welcome! Which Adventure would you like to do today "
              << "(Easy/Medium/Hard)?\n  > ";

    int badCount = 0;
    std::string input;
    
    while (true) {
        std::getline(std::cin, input);

        std::string lower = input;
        for (char& c : lower) c = (char)std::tolower((unsigned char)c);

        if (lower == "easy")   return EASY;
        if (lower == "medium") return MEDIUM;
        if (lower == "hard")   return HARD;

        ++badCount;
        if (badCount >= 5) {
            std::cout << "\n  I don't get paid enough for this.\n";
            return HARD;
        }
        std::cout << "  Strange, haven't heard of that before. Seriously, "
                  << "which difficulty? (Easy/Medium/Hard)\n  > ";
    }
}


// stuff to make the terminal read keys without hitting enter
// saves the original state so we can put it back later
static struct termios g_origTermios;

static void enableRawMode() {
    tcgetattr(STDIN_FILENO, &g_origTermios);
    struct termios raw = g_origTermios;
    raw.c_lflag &= ~(ICANON | ECHO); // turns off echo and line buffering
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
}

// gets one key press
static char readKey() {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) != 1) c = 0;
    return c;
}

// wipes the console clean at the start
static void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}


// tries to move player, returns false if hit a wall
static bool applyMove(MapState& ms, Pos& pos, int dr, int dc) {
    Pos nxt{pos.row+dr, pos.col+dc};
    if (!ms.isWalkable(nxt)) return false;
    pos = nxt;
    return true;
}


// check if player stepped on anything important (keys, exit, etc)
// returns W for win, space for keep going
static char handleInteractions(MapState& ms,
                               bool& virusStunned, int& virusStunTimer,
                               int& stunReplenishTimer, int& stunIdx,
                               std::string& eventMsg) {
    Pos pp = ms.playerPos;
    char ch = ms.grid[pp.row][pp.col];

    if (ch == TILE_EXIT) return 'W';

    // picked up a key
    if (ch >= 'a' && ch <= 'i') {
        int id = (ch - 'a') + 1;
        for (auto& g : ms.gates) {
            if (g.id == id && !g.keyTaken) {
                g.keyTaken = true;
                ms.grid[pp.row][pp.col] = TILE_EMPTY;
                
                for (auto& g2 : ms.gates)
                    if (g2.id == id) {
                        g2.gateOpen = true;
                        ms.grid[g2.gatePos.row][g2.gatePos.col] = TILE_EMPTY;
                    }
                    
                eventMsg = "  [!] Key " + std::to_string(id) +
                           " collected -- Gate " + std::to_string(id) + " opened!";
                break;
            }
        }
    }

    // hit a stun tile
    if (ch == TILE_STUN) {
        for (int si = 0; si < (int)ms.stuns.size(); ++si) {
            auto& st = ms.stuns[si];
            if (st.pos == pp && st.active) {
                st.active = false;
                ms.grid[pp.row][pp.col] = TILE_EMPTY;

                int dur;
                if (ms.diff == MEDIUM)
                    dur = MEDIUM_STUN_MIN + rand()%(MEDIUM_STUN_MAX-MEDIUM_STUN_MIN+1);
                else
                    dur = HARD_STUN_MIN  + rand()%(HARD_STUN_MAX -HARD_STUN_MIN +1);

                virusStunned = true;
                virusStunTimer = dur;
                stunIdx = si;
                stunReplenishTimer = -1;
                eventMsg = "  [!] Virus stunned for " + std::to_string(dur) + " steps!";
                break;
            }
        }
    }

    return ' ';
}

// checks if the virus can move to a specific tile
static bool virusTileBlocked(const MapState& ms, Pos p) {
    char ch = ms.grid[p.row][p.col];
    if (ch == TILE_WALL_H || ch == TILE_WALL_V) return true;
    if (ch == TILE_EXIT) return true;
    for (const auto& xg : ms.xgates)
        if (!xg.open && xg.pos == p) return true;
    for (const auto& g : ms.gates)
        if (!g.gateOpen && g.gatePos == p) return true;
    return false;
}

// calculates the path finding for the virus
static int shortestVirusDistance(const MapState& ms) {
    if (ms.virusPos.row < 0) return 999999;

    std::vector<std::vector<int>> dist(ms.rows,
        std::vector<int>(ms.cols, -1));
    std::queue<Pos> q;
    q.push(ms.virusPos);
    dist[ms.virusPos.row][ms.virusPos.col] = 0;

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    while (!q.empty()) {
        Pos cur = q.front(); q.pop();
        if (cur == ms.playerPos) return dist[cur.row][cur.col];

        for (int i = 0; i < 4; ++i) {
            Pos nb{cur.row + dr[i], cur.col + dc[i]};
            if (nb.row <= 0 || nb.col <= 0 || nb.row >= ms.rows - 1 || nb.col >= ms.cols - 1)
                continue;
            if (dist[nb.row][nb.col] != -1) continue;
            if (nb != ms.playerPos && virusTileBlocked(ms, nb)) continue;
            
            dist[nb.row][nb.col] = dist[cur.row][cur.col] + 1;
            q.push(nb);
        }
    }
    return 999999;
}

// self explanatory
static bool allNumberedGatesOpen(const MapState& ms) {
    for (const auto& g : ms.gates)
        if (!g.gateOpen) return false;
    return true;
}


// main game loop
int runGame(Difficulty diff) {
    MapState ms = generateMap(diff);

    bool virusStunned      = false;
    int  virusStunTimer    = 0;
    int  stunReplenishTimer= 0;
    int  stunIdx           = -1;
    int  playerSteps       = 0;
    int  virusSpeed        = 1;
    std::string eventMsg;

    clearScreen();
    // hide the cursor so it looks cleaner
    std::cout << "\033[?25l" << std::flush;

    printMap(ms, virusStunned, virusStunTimer, playerSteps);
    enableRawMode();

    int result = 0; // 0 is playing, 1 is win, -1 is lose/quit

    while (result == 0) {

        char key = readKey();
        char cmd = (char)std::toupper((unsigned char)key);

        if (cmd == 'Q') {
            result = -1;
            break;
        }

        int dr = 0, dc = 0;
        if      (cmd == 'W') dr = -1;
        else if (cmd == 'S') dr =  1;
        else if (cmd == 'A') dc = -1;
        else if (cmd == 'D') dc =  1;
        else continue; 

        bool moved = applyMove(ms, ms.playerPos, dr, dc);
        if (!moved) {
            // hit a wall, just redraw to keep the prompt up
            printMap(ms, virusStunned, virusStunTimer, playerSteps);
            continue;
        }
        
        ++playerSteps;
        eventMsg.clear();

        // tick down the x-gate timers
        for (auto& xg : ms.xgates) {
            if (!xg.open) {
                --xg.stepsLeft;
                if (xg.stepsLeft <= 0) {
                    xg.open = true;
                    ms.grid[xg.pos.row][xg.pos.col] = TILE_EMPTY;
                    eventMsg = "  [!] X-Gate opened -- Virus is now FREE!";
                }
            }
        }

        bool allXgatesClosed = false;
        for (const auto& xg : ms.xgates)
            if (!xg.open) { allXgatesClosed = true; break; }

        // process what the player stepped on
        char iResult = handleInteractions(ms, virusStunned, virusStunTimer,
                                          stunReplenishTimer, stunIdx, eventMsg);
        if (iResult == 'W') { result = 1; break; }

        // virus movement logic
        if (diff != EASY && !virusStunned && !allXgatesClosed) {
            int currentVirusSpeed = virusSpeed;

            if (diff == HARD) {
                int chaseDist = shortestVirusDistance(ms);
                bool endgamePressure = allNumberedGatesOpen(ms);

                // speed up the virus if the player is close, but keep it readable
                if (endgamePressure && chaseDist <= 3 && (playerSteps % 2 == 0)) {
                    currentVirusSpeed = 2;
                } else if (endgamePressure && chaseDist <= 7 && (playerSteps % 3 == 0)) {
                    currentVirusSpeed = 2;
                }
            }

            bool caught = moveVirus(ms, currentVirusSpeed);
            if (caught) { result = -1; break; }
        }

        // lower the stun timer (has to happen after virus moves so it actually works)
        if (virusStunned) {
            --virusStunTimer;
            if (virusStunTimer <= 0) {
                virusStunned = false;
                stunReplenishTimer = STUN_REPLENISH_DELAY;
            }
        } else if (stunReplenishTimer > 0) {
            --stunReplenishTimer;
            if (stunReplenishTimer == 0 && stunIdx >= 0) {
                ms.stuns[stunIdx].active = true;
                ms.grid[ms.stuns[stunIdx].pos.row]
                       [ms.stuns[stunIdx].pos.col] = TILE_STUN;
                stunIdx = -1;
            }
        }

        // update screen
        printMap(ms, virusStunned, virusStunTimer, playerSteps);
    }

    disableRawMode();
    std::cout << "\033[?25h" << std::flush; // bring cursor back

    // print final results
    clearScreen();
    printMap(ms, false, 0, playerSteps);

    if (result == 1) {
        std::cout << "\n  *** YOU ESCAPED THE MAZE! Congratulations! ***\n";
        std::cout << "  Total steps taken: " << playerSteps << "\n\n";
        return playerSteps;
    } else if (result == -1 && diff != EASY) {
        std::cout << "\n  *** THE VIRUS GOT YOU! Game Over. ***\n\n";
    } else {
        std::cout << "\n  You fled the maze. Farewell, Adventurer!\n\n";
    }
    return -1;
}
