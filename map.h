#ifndef MAP_H
#define MAP_H

#include "globals.h"
#include <vector>
#include <string>

// == Position helper ==
struct Pos {
    int row, col;
    bool operator==(const Pos& o) const { return row == o.row && col == o.col; }
    bool operator!=(const Pos& o) const { return !(*this == o); }
};

// == Gate/Key pair info stored inside the map ==
struct GateInfo {
    int  id;        // 1-based index (matches digit on board)
    Pos  gatePos;
    Pos  keyPos;
    bool gateOpen;  // true once player collected the key
    bool keyTaken;  // true once player stepped on key
};

// == XGate info (virus blocker near player spawn) ==
struct XGateInfo {
    Pos  pos;
    int  stepsLeft; // countdown; gate opens when reaches 0
    bool open;
};

// == Stun tile info ==
struct StunInfo {
    Pos  pos;
    bool active;         // tile is currently available to pick up
    int  replenishTimer; // steps remaining before it respawns (0 = ready)
};

// == Full map state ==
struct MapState {
    int rows, cols;
    std::vector<std::vector<char>> grid; // base terrain (walls, gates, keys...)
    Pos playerPos;
    Pos virusPos;
    Pos exitPos;
    std::vector<GateInfo>  gates;
    std::vector<XGateInfo> xgates;
    std::vector<StunInfo>  stuns;
    Difficulty diff;

    // returns true when pos is inside borders and not a wall/closed gate
    bool isWalkable(Pos p) const;
    // returns true when pos is a wall tile (#, |)
    bool isWall(Pos p) const;
};

// == Public API ==
// generateMap: builds a randomised maze for the requested difficulty level
MapState generateMap(Difficulty diff);

// printMap: renders the current game state to stdout (virusStunSteps shown in HUD)
void printMap(const MapState& ms, bool virusStunned, int virusStunSteps, int playerSteps);

#endif // MAP_H
