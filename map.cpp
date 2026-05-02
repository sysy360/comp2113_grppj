#include "map.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <climits>

// == How the maze is stored ==
// Grid uses a 2x expansion layout: cell (r,c) maps to grid cell (2r+1, 2c+1). Walls between adjacent cells sit in odd-indexed rows/cols.
// TILE_WALL_H '#' for horizontal walls, TILE_WALL_V '|' for vertical walls.
// The renderer inspects each wall tile's four neighbours to pick the right corner/T-junction/cross glyph.

static bool inBounds(int r, int c, int rows, int cols) {
    return r >= 0 && c >= 0 && r < rows && c < cols;
}

// == Recursive-backtracker maze carver ==
// cr,cc are logical cell coords (not grid coords). Grid must be pre-filled with TILE_WALL_H; start cell (grid[1][1]) opened before calling.
static void carveMaze(std::vector<std::vector<char>>& grid,
                      int cellRows, int cellCols,
                      int cr, int cc) {
    const int DR[] = {-1,  1,  0, 0};
    const int DC[] = { 0,  0, -1, 1};

    int order[] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--)
        std::swap(order[i], order[rand() % (i + 1)]);

    for (int i = 0; i < 4; i++) {
        int d  = order[i];
        int nr = cr + DR[d];
        int nc = cc + DC[d];
        if (nr < 0 || nc < 0 || nr >= cellRows || nc >= cellCols) continue;

        int gnr = 2*nr+1, gnc = 2*nc+1; // neighbour grid pos
        int wgr = 2*cr+1 + DR[d];        // wall tile row
        int wgc = 2*cc+1 + DC[d];        // wall tile col

        if (grid[gnr][gnc] != TILE_WALL_H) continue; // already visited

        grid[wgr][wgc] = TILE_EMPTY;
        grid[gnr][gnc]  = TILE_EMPTY;
        carveMaze(grid, cellRows, cellCols, nr, nc);
    }
}

// == Add extra loops so Medium/Hard aren't single-path death corridors ==
// Turns the perfect maze into one with loops, giving the player alternate routes when the Virus appears.
static void addExtraLoops(std::vector<std::vector<char>>& grid,
                          int rows, int cols, int loopsToOpen) {
    std::vector<Pos> candidates;
    for (int r = 1; r < rows - 1; r++) {
        for (int c = 1; c < cols - 1; c++) {
            char ch = grid[r][c];
            if (ch != TILE_WALL_H && ch != TILE_WALL_V) continue;

            bool verticalBridge   = (c > 0 && c < cols-1 && grid[r][c-1] == TILE_EMPTY && grid[r][c+1] == TILE_EMPTY);
            bool horizontalBridge = (r > 0 && r < rows-1 && grid[r-1][c] == TILE_EMPTY && grid[r+1][c] == TILE_EMPTY);
            if (verticalBridge || horizontalBridge)
                candidates.push_back({r, c});
        }
    }

    for (int i = (int)candidates.size() - 1; i > 0; i--)
        std::swap(candidates[i], candidates[rand() % (i + 1)]);

    int opened = 0;
    for (auto& p : candidates) {
        if (opened >= loopsToOpen) break;
        grid[p.row][p.col] = TILE_EMPTY;
        opened++;
    }
}

// == MapState::isWall ==
bool MapState::isWall(Pos p) const {
    char ch = grid[p.row][p.col];
    return ch == TILE_WALL_H || ch == TILE_WALL_V;
}

// == MapState::isWalkable ==
bool MapState::isWalkable(Pos p) const {
    if (p.row <= 0 || p.col <= 0 || p.row >= rows-1 || p.col >= cols-1) return false;
    char ch = grid[p.row][p.col];
    if (ch == TILE_WALL_H || ch == TILE_WALL_V) return false;
    for (auto& xg : xgates)
        if (!xg.open && xg.pos == p) return false;
    for (auto& g : gates)
        if (!g.gateOpen && g.gatePos == p) return false;
    return true;
}

// Pick a random TILE_EMPTY interior cell
static Pos randomOpenCell(const MapState& ms) {
    Pos p;
    int tries = 0;
    do {
        p.row = 1 + rand() % (ms.rows - 2);
        p.col = 1 + rand() % (ms.cols - 2);
        tries++;
    } while (ms.grid[p.row][p.col] != TILE_EMPTY && tries < 20000);
    return p;
}

// == Planning helpers for "meaningful" gate placement ==
// A gate should block a route that is currently usable, not just sit on a random wall and act like an optional shortcut.

static bool passableForSearch(char ch) {
    return ch != TILE_WALL_H && ch != TILE_WALL_V && ch != 'X' && !(ch >= '1' && ch <= '9');
}

static std::vector<std::vector<bool>> reachableCells(
    const std::vector<std::vector<char>>& grid,
    int rows, int cols, Pos start, Pos blocked = {-1, -1}) {

    std::vector<std::vector<bool>> vis(rows, std::vector<bool>(cols, false));
    if (start == blocked || !inBounds(start.row, start.col, rows, cols)) return vis;
    if (!passableForSearch(grid[start.row][start.col])) return vis;

    std::queue<Pos> q;
    q.push(start);
    vis[start.row][start.col] = true;

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    while (!q.empty()) {
        Pos cur = q.front(); q.pop();
        for (int i = 0; i < 4; i++) {
            Pos nb{cur.row + dr[i], cur.col + dc[i]};
            if (nb == blocked) continue;
            if (nb.row <= 0 || nb.col <= 0 || nb.row >= rows-1 || nb.col >= cols-1) continue;
            if (vis[nb.row][nb.col] || !passableForSearch(grid[nb.row][nb.col])) continue;
            vis[nb.row][nb.col] = true;
            q.push(nb);
        }
    }
    return vis;
}

static std::vector<Pos> shortestPath(const std::vector<std::vector<char>>& grid,
                                     int rows, int cols, Pos start, Pos goal) {
    std::vector<std::vector<bool>> vis(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Pos>> parent(rows, std::vector<Pos>(cols, {-1, -1}));

    std::queue<Pos> q;
    q.push(start);
    vis[start.row][start.col] = true;

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    while (!q.empty()) {
        Pos cur = q.front(); q.pop();
        if (cur == goal) break;
        for (int i = 0; i < 4; i++) {
            Pos nb{cur.row + dr[i], cur.col + dc[i]};
            if (nb.row <= 0 || nb.col <= 0 || nb.row >= rows-1 || nb.col >= cols-1) continue;
            if (vis[nb.row][nb.col] || !passableForSearch(grid[nb.row][nb.col])) continue;
            vis[nb.row][nb.col] = true;
            parent[nb.row][nb.col] = cur;
            q.push(nb);
        }
    }

    std::vector<Pos> path;
    if (!vis[goal.row][goal.col]) return path;
    for (Pos cur = goal; cur.row != -1; cur = parent[cur.row][cur.col]) {
        path.push_back(cur);
        if (cur == start) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

static bool canReachOnGrid(const std::vector<std::vector<char>>& grid,
                           int rows, int cols, Pos start, Pos goal) {
    if (!inBounds(start.row, start.col, rows, cols) || !inBounds(goal.row, goal.col, rows, cols)) return false;
    if (!passableForSearch(grid[start.row][start.col])) return false;

    std::vector<std::vector<bool>> vis(rows, std::vector<bool>(cols, false));
    std::queue<Pos> q;
    q.push(start);
    vis[start.row][start.col] = true;

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    while (!q.empty()) {
        Pos cur = q.front(); q.pop();
        if (cur == goal) return true;
        for (int i = 0; i < 4; i++) {
            Pos nb{cur.row + dr[i], cur.col + dc[i]};
            if (nb.row <= 0 || nb.col <= 0 || nb.row >= rows-1 || nb.col >= cols-1) continue;
            if (vis[nb.row][nb.col] || !passableForSearch(grid[nb.row][nb.col])) continue;
            vis[nb.row][nb.col] = true;
            q.push(nb);
        }
    }
    return false;
}

static bool allClosedGatesStillMatter(const std::vector<std::vector<char>>& grid,
                                      int rows, int cols, Pos start, Pos goal,
                                      const std::vector<Pos>& gates) {
    for (int gi = 0; gi < (int)gates.size(); gi++) {
        auto work = grid;
        for (int j = 0; j < (int)gates.size(); j++)
            if (j != gi) work[gates[j].row][gates[j].col] = TILE_EMPTY;
        if (canReachOnGrid(work, rows, cols, start, goal)) return false;
    }
    return true;
}

static void addExtraLoopsPreservingGates(std::vector<std::vector<char>>& grid,
                                         int rows, int cols, int loopsToOpen,
                                         Pos start, Pos goal,
                                         const std::vector<Pos>& gates) {
    std::vector<Pos> candidates;
    for (int r = 1; r < rows - 1; r++) {
        for (int c = 1; c < cols - 1; c++) {
            char ch = grid[r][c];
            if (ch != TILE_WALL_H && ch != TILE_WALL_V) continue;

            bool verticalBridge   = (c > 0 && c < cols-1 && passableForSearch(grid[r][c-1]) && passableForSearch(grid[r][c+1]));
            bool horizontalBridge = (r > 0 && r < rows-1 && passableForSearch(grid[r-1][c]) && passableForSearch(grid[r+1][c]));
            if (!verticalBridge && !horizontalBridge) continue;

            bool nearGate = false;
            for (auto& gp : gates) {
                if (std::abs(gp.row - r) + std::abs(gp.col - c) <= 3) { nearGate = true; break; }
            }
            if (!nearGate) candidates.push_back({r, c});
        }
    }

    for (int i = (int)candidates.size() - 1; i > 0; i--)
        std::swap(candidates[i], candidates[rand() % (i + 1)]);

    int opened = 0;
    for (auto& p : candidates) {
        if (opened >= loopsToOpen) break;
        char old = grid[p.row][p.col];
        grid[p.row][p.col] = TILE_EMPTY;
        if (allClosedGatesStillMatter(grid, rows, cols, start, goal, gates))
            opened++;
        else
            grid[p.row][p.col] = old;
    }
}

// == generateMap ==
MapState generateMap(Difficulty diff) {
    MapState ms;
    ms.diff = diff;

    int cellRows = 3, cellCols = 3;
    switch (diff) {
        case EASY:   cellRows = 3;  cellCols = 3;  break;  //  7x7  grid
        case MEDIUM: cellRows = 6;  cellCols = 6;  break;  // 13x13 grid
        case HARD:   cellRows = 11; cellCols = 11; break;  // 23x23 grid
    }
    ms.rows = 2*cellRows + 1;
    ms.cols = 2*cellCols + 1;

    // 1. fill with wall
    ms.grid.assign(ms.rows, std::vector<char>(ms.cols, TILE_WALL_H));

    // 2. carve perfect maze
    ms.grid[1][1] = TILE_EMPTY;
    carveMaze(ms.grid, cellRows, cellCols, 0, 0);

    // 2b. add loops on Medium so the player has alternate routes instead of guaranteed head-on collisions
    if (diff == MEDIUM) addExtraLoops(ms.grid, ms.rows, ms.cols, 7);

    // 3. mark vertical wall segments so the renderer draws | correctly
    // heuristic: a wall tile in an even column separates left/right cells, so it's structural vertical
    for (int r = 0; r < ms.rows; r++)
        for (int c = 0; c < ms.cols; c++)
            if (ms.grid[r][c] == TILE_WALL_H && c % 2 == 0)
                ms.grid[r][c] = TILE_WALL_V;

    // 4. player at top-left cell
    ms.playerPos = {1, 1};

    // 5. exit placement
    // easy/medium keep the classic "farthest cell" exit. Hard picks from the upper-distance band
    // instead of always the single farthest point, since that felt too marathon-like for human players.
    {
        const int dr[] = {-1, 1, 0, 0}, dc[] = {0, 0, -1, 1};
        std::vector<std::vector<int>> dist(ms.rows, std::vector<int>(ms.cols, INT_MAX));
        std::queue<Pos> q;
        dist[1][1] = 0;
        q.push({1, 1});
        Pos farthest{1, 1};
        int maxD = 0;
        std::vector<Pos> reachable;
        reachable.push_back({1, 1});

        while (!q.empty()) {
            Pos cur = q.front(); q.pop();
            for (int i = 0; i < 4; i++) {
                Pos nb{cur.row + dr[i], cur.col + dc[i]};
                if (!inBounds(nb.row, nb.col, ms.rows, ms.cols)) continue;
                if (ms.grid[nb.row][nb.col] != TILE_EMPTY) continue;
                if (dist[nb.row][nb.col] != INT_MAX) continue;
                dist[nb.row][nb.col] = dist[cur.row][cur.col] + 1;
                reachable.push_back(nb);
                if (dist[nb.row][nb.col] > maxD) { maxD = dist[nb.row][nb.col]; farthest = nb; }
                q.push(nb);
            }
        }

        Pos chosenExit = farthest;
        if (diff == HARD) {
            int threshold = (maxD * 4) / 5; // top ~20% of distances
            std::vector<Pos> hardChoices;
            for (auto& p : reachable)
                if (dist[p.row][p.col] >= threshold)
                    hardChoices.push_back(p);
            if (!hardChoices.empty())
                chosenExit = hardChoices[rand() % hardChoices.size()];
        }

        ms.exitPos = chosenExit;
        ms.grid[chosenExit.row][chosenExit.col] = TILE_EXIT;
    }

    // 6. gates and keys
    int requestedGates;
    if (diff == EASY)
        requestedGates = EASY_GATES_MIN + rand() % (EASY_GATES_MAX - EASY_GATES_MIN + 1);
    else if (diff == MEDIUM)
        requestedGates = 1 + rand() % 2; // 1-2 meaningful gates
    else
        requestedGates = 2;              // Hard: exactly 2 meaningful, well-spaced gates

    std::vector<Pos> pathToExit = shortestPath(ms.grid, ms.rows, ms.cols, ms.playerPos, ms.exitPos);
    std::vector<Pos> chosenGatePos;

    if (diff == HARD) {
        // Hard gets two gates deliberately spaced along the route so they're not piled near the spawn
        int pathLen = (int)pathToExit.size();
        if (pathLen >= 14) {
            int firstIdx  = std::max(4,  pathLen * 32 / 100);
            int secondIdx = std::min(pathLen - 5, pathLen * 66 / 100);
            if (secondIdx - firstIdx < 4) secondIdx = std::min(pathLen - 5, firstIdx + 4);
            if (firstIdx > 1 && firstIdx < pathLen - 2)
                chosenGatePos.push_back(pathToExit[firstIdx]);
            if (secondIdx > firstIdx + 2 && secondIdx < pathLen - 2)
                chosenGatePos.push_back(pathToExit[secondIdx]);
        }

        for (int i = 0; i < (int)chosenGatePos.size(); i++) {
            GateInfo gi;
            gi.id = i + 1;
            gi.gateOpen = false;
            gi.keyTaken = false;
            gi.gatePos = chosenGatePos[i];

            int gateIndexOnPath = 0;
            for (int idx = 0; idx < (int)pathToExit.size(); idx++) {
                if (pathToExit[idx] == gi.gatePos) { gateIndexOnPath = idx; break; }
            }

            int segStart = (i == 0) ? 2 : 0;
            if (i > 0) {
                for (int idx = 0; idx < (int)pathToExit.size(); idx++) {
                    if (pathToExit[idx] == chosenGatePos[i-1]) { segStart = idx + 2; break; }
                }
            }
            int segEnd = gateIndexOnPath - 2;
            if (segEnd < segStart) continue;

            std::vector<Pos> keyChoices;
            for (int idx = segStart; idx <= segEnd; idx++) {
                Pos kp = pathToExit[idx];
                if (kp == ms.playerPos || kp == ms.exitPos || kp == gi.gatePos) continue;
                bool conflict = false;
                for (auto& eg : ms.gates)
                    if (eg.gatePos == kp || eg.keyPos == kp) { conflict = true; break; }
                if (!conflict) keyChoices.push_back(kp);
            }
            if (keyChoices.empty()) continue;

            int pick = (int)keyChoices.size() / 2;
            gi.keyPos = keyChoices[pick];
            ms.grid[gi.gatePos.row][gi.gatePos.col] = '1' + i;
            ms.grid[gi.keyPos.row][gi.keyPos.col] = 'a' + i;
            ms.gates.push_back(gi);
        }

        std::vector<Pos> actualGatePos;
        for (auto& g : ms.gates) actualGatePos.push_back(g.gatePos);
        if (!actualGatePos.empty())
            addExtraLoopsPreservingGates(ms.grid, ms.rows, ms.cols, 14, ms.playerPos, ms.exitPos, actualGatePos);
        else
            addExtraLoops(ms.grid, ms.rows, ms.cols, 10);
    } else {
        // Easy/Medium keep the previous meaningful-gate logic
        std::vector<Pos> gateCandidates;
        for (int idx = 2; idx + 2 < (int)pathToExit.size(); idx++) {
            Pos p = pathToExit[idx];
            if (ms.grid[p.row][p.col] != TILE_EMPTY) continue;
            auto vis = reachableCells(ms.grid, ms.rows, ms.cols, ms.playerPos, p);
            if (!vis[ms.exitPos.row][ms.exitPos.col])
                gateCandidates.push_back(p);
        }

        int numGates = std::min(requestedGates, (int)gateCandidates.size());
        if (numGates > 0) {
            for (int k = 1; k <= numGates; k++) {
                int pick = (k * (int)gateCandidates.size()) / (numGates + 1);
                if (pick >= (int)gateCandidates.size()) pick = (int)gateCandidates.size() - 1;
                if (!chosenGatePos.empty() && gateCandidates[pick] == chosenGatePos.back()) {
                    for (int j = pick + 1; j < (int)gateCandidates.size(); j++) {
                        if (gateCandidates[j] != chosenGatePos.back()) { pick = j; break; }
                    }
                }
                if (chosenGatePos.empty() || gateCandidates[pick] != chosenGatePos.back())
                    chosenGatePos.push_back(gateCandidates[pick]);
            }
        }

        for (int i = 0; i < (int)chosenGatePos.size(); i++) {
            GateInfo gi;
            gi.id = i + 1;
            gi.gateOpen = false;
            gi.keyTaken = false;
            gi.gatePos = chosenGatePos[i];

            auto vis = reachableCells(ms.grid, ms.rows, ms.cols, ms.playerPos, gi.gatePos);
            std::vector<Pos> keyChoices;
            for (int r = 1; r < ms.rows - 1; r++) {
                for (int c = 1; c < ms.cols - 1; c++) {
                    if (!vis[r][c]) continue;
                    Pos kp{r, c};
                    char ch = ms.grid[r][c];
                    if (ch != TILE_EMPTY) continue;
                    if (kp == ms.playerPos || kp == ms.exitPos || kp == gi.gatePos) continue;

                    bool conflict = false;
                    for (auto& eg : ms.gates)
                        if (eg.gatePos == kp || eg.keyPos == kp) { conflict = true; break; }
                    for (auto& gp : chosenGatePos)
                        if (gp == kp) { conflict = true; break; }
                    if (!conflict) keyChoices.push_back(kp);
                }
            }
            if (keyChoices.empty()) continue;

            gi.keyPos = keyChoices[rand() % keyChoices.size()];
            ms.grid[gi.gatePos.row][gi.gatePos.col] = '1' + i;
            ms.grid[gi.keyPos.row][gi.keyPos.col] = 'a' + i;
            ms.gates.push_back(gi);
        }
    }

    // 7. X-Gates (Medium/Hard)
    if (diff == MEDIUM || diff == HARD) {
        for (int att = 0; att < 600; att++) {
            int r = ms.playerPos.row + (rand() % 5) - 1;
            int c = ms.playerPos.col + 1 + rand() % 4;
            if (!inBounds(r, c, ms.rows, ms.cols)) continue;
            char ch = ms.grid[r][c];
            if (ch != TILE_WALL_H && ch != TILE_WALL_V) continue;
            bool hB = (r > 0 && r < ms.rows-1 && ms.grid[r-1][c] == TILE_EMPTY && ms.grid[r+1][c] == TILE_EMPTY);
            bool vB = (c > 0 && c < ms.cols-1 && ms.grid[r][c-1] == TILE_EMPTY && ms.grid[r][c+1] == TILE_EMPTY);
            if (!hB && !vB) continue;
            XGateInfo xg;
            xg.pos = {r, c};
            xg.stepsLeft = (diff == MEDIUM) ? 16 + rand() % 7 : 34 + rand() % 9; // medium: 16-22, hard: 34-42
            xg.open = false;
            ms.grid[r][c] = 'X';
            ms.xgates.push_back(xg);
            break;
        }
    }

    // 8. Stun tiles (Medium/Hard)
    if (diff == MEDIUM || diff == HARD) {
        int numStuns = (diff == MEDIUM) ? 2 : 5;
        for (int s = 0; s < numStuns; s++) {
            for (int att = 0; att < 300; att++) {
                Pos sp = randomOpenCell(ms);
                if (sp == ms.playerPos || sp == ms.exitPos) continue;
                bool conflict = false;
                for (auto& g : ms.gates)
                    if (g.gatePos == sp || g.keyPos == sp) { conflict = true; break; }
                for (auto& st : ms.stuns)
                    if (st.pos == sp) { conflict = true; break; }
                if (conflict) continue;
                StunInfo si;
                si.pos = sp; si.active = true; si.replenishTimer = 0;
                ms.grid[sp.row][sp.col] = TILE_STUN;
                ms.stuns.push_back(si);
                break;
            }
        }
    }

    // 9. Virus (Medium/Hard)
    if (diff == MEDIUM || diff == HARD) {
        for (int r = ms.rows-2; r >= 1; r--) {
            bool found = false;
            for (int c = ms.cols-2; c >= 1; c--)
                if (ms.grid[r][c] == TILE_EMPTY) { ms.virusPos = {r, c}; found = true; break; }
            if (found) break;
        }
    } else {
        ms.virusPos = {-1, -1};
    }

    return ms;
}

// == Rendering - box-drawing wall characters ==

// returns true when a cell should visually connect like a wall (used for picking glyph)
static bool isWallLike(const std::vector<std::vector<char>>& d,
                        int rows, int cols, int r, int c) {
    if (r < 0 || c < 0 || r >= rows || c >= cols) return true;
    char ch = d[r][c];
    return ch == TILE_WALL_H || ch == TILE_WALL_V || (ch >= '1' && ch <= '9') || ch == 'X';
}

// returns a 2-char wide string (some box chars are 3 bytes UTF-8 + 1 space)
static std::string wallGlyph(const std::vector<std::vector<char>>& d,
                              int rows, int cols, int r, int c) {
    bool U = isWallLike(d, rows, cols, r-1, c);
    bool D = isWallLike(d, rows, cols, r+1, c);
    bool L = isWallLike(d, rows, cols, r, c-1);
    bool R = isWallLike(d, rows, cols, r, c+1);

    if (U&&D&&L&&R)    return "\xe2\x94\xbc\xe2\x94\x80"; // ┼─
    if (U&&D&&R&&!L)   return "\xe2\x94\x9c\xe2\x94\x80"; // ├─
    if (U&&D&&L&&!R)   return "\xe2\x94\xa4 ";             // ┤
    if (L&&R&&D&&!U)   return "\xe2\x94\xac\xe2\x94\x80"; // ┬─
    if (L&&R&&U&&!D)   return "\xe2\x94\xb4\xe2\x94\x80"; // ┴─
    if (!U&&!L&&D&&R)  return "\xe2\x94\x8c\xe2\x94\x80"; // ┌─
    if (!U&&!R&&D&&L)  return "\xe2\x94\x90 ";             // ┐
    if (!D&&!L&&U&&R)  return "\xe2\x94\x94\xe2\x94\x80"; // └─
    if (!D&&!R&&U&&L)  return "\xe2\x94\x98 ";             // ┘
    if ((U||D)&&!L&&!R) return "\xe2\x94\x82 ";            // │
    return "\xe2\x94\x80\xe2\x94\x80";                     // ──
}

// == printMap - renders full frame in one write to avoid flicker ==
void printMap(const MapState& ms, bool virusStunned, int virusStunSteps, int playerSteps) {
    // build overlay display grid
    std::vector<std::vector<char>> disp = ms.grid;

    // X-gate visibility
    for (auto& xg : ms.xgates) {
        if (!xg.open) disp[xg.pos.row][xg.pos.col] = 'X';
        else if (disp[xg.pos.row][xg.pos.col] == 'X') disp[xg.pos.row][xg.pos.col] = TILE_EMPTY;
    }

    // stun tile visibility
    for (auto& st : ms.stuns) {
        if (st.active) disp[st.pos.row][st.pos.col] = TILE_STUN;
        else if (disp[st.pos.row][st.pos.col] == TILE_STUN) disp[st.pos.row][st.pos.col] = TILE_EMPTY;
    }

    // entities drawn last so they always appear on top
    disp[ms.playerPos.row][ms.playerPos.col] = TILE_PLAYER;
    if (ms.diff != EASY && ms.virusPos.row != -1)
        disp[ms.virusPos.row][ms.virusPos.col] = virusStunned ? '*' : TILE_VIRUS;

    // accumulate into string buffer - single write eliminates flicker
    std::string buf;
    buf.reserve(8192);

    // ANSI: cursor to home - overwrites previous frame without clearing, which is flicker-free
    buf += "\033[H";

    // HUD
    buf += "\n  Steps: ";
    buf += std::to_string(playerSteps);
    if (ms.diff != EASY) {
        if (virusStunned) {
            buf += "  |  Virus STUNNED (";
            buf += std::to_string(virusStunSteps);
            buf += " steps)          ";
        } else {
            // If any X-Gate is still closed, the virus is held back.
            // Show its countdown instead of pretending it's chasing.
            int xgateStepsLeft = -1;
            for (const auto& xg : ms.xgates) {
                if (!xg.open) { xgateStepsLeft = xg.stepsLeft; break; }
            }
            if (xgateStepsLeft > 0) {
                buf += "  |  Virus: BLOCKED (X-Gate opens in ";
                buf += std::to_string(xgateStepsLeft);
                buf += ")   ";
            } else {
                buf += "  |  Virus: CHASING                  ";
            }
        }
    }
    buf += "\n";
    buf += "  P=You  E=Exit  V=Virus  Z=Stun  Gn=Gate  Kn=Key  X=XGate\n\n";

    // Grid
    for (int r = 0; r < ms.rows; r++) {
        buf += "  ";
        for (int c = 0; c < ms.cols; c++) {
            char ch = disp[r][c];
            if (ch == TILE_WALL_H || ch == TILE_WALL_V)
                buf += wallGlyph(disp, ms.rows, ms.cols, r, c);
            else if (ch == TILE_PLAYER)              buf += "P ";
            else if (ch == TILE_VIRUS)               buf += "V ";
            else if (ch == '*')                      buf += "* "; // stunned
            else if (ch == TILE_EXIT)                buf += "E ";
            else if (ch == TILE_STUN)                buf += "Z ";
            else if (ch == 'X')                      buf += "X "; // X-gate
            else if (ch >= '1' && ch <= '9')         { buf += 'G'; buf += ch; }       // G1...G9
            else if (ch >= 'a' && ch <= 'i')         { buf += 'K'; buf += (char)('1' + (ch - 'a')); } // K1...K9
            else                                     buf += "  "; // empty corridor
        }
        buf += "\n";
    }

    buf += "\n  Controls: W=Up  S=Down  A=Left  D=Right  Q=Quit\n";
    buf += "  Move: ";

    // single atomic write
    std::cout << buf << std::flush;
}
