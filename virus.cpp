#include "virus.h"
#include <queue>
#include <climits>
#include <vector>

// ─────────────────────────────────────────────────────────────
//  BFS one step toward the player
//  Returns the next Pos the virus should move to, or current pos
//  if no path exists.
// ─────────────────────────────────────────────────────────────
static Pos bfsNextStep(const MapState& ms) {
    Pos src = ms.virusPos;
    Pos dst = ms.playerPos;

    // BFS to build a parent map
    std::vector<std::vector<Pos>> parent(ms.rows,
        std::vector<Pos>(ms.cols, {-1,-1}));
    std::vector<std::vector<bool>> visited(ms.rows,
        std::vector<bool>(ms.cols, false));

    std::queue<Pos> q;
    q.push(src);
    visited[src.row][src.col] = true;

    const int dr[] = {-1,1,0,0};
    const int dc[] = {0,0,-1,1};

    bool found = false;
    while (!q.empty() && !found) {
        Pos cur = q.front(); q.pop();
        for (int d = 0; d < 4; ++d) {
            Pos nb{cur.row+dr[d], cur.col+dc[d]};
            if (nb.row<=0 || nb.col<=0 ||
                nb.row>=ms.rows-1 || nb.col>=ms.cols-1) continue;
            if (visited[nb.row][nb.col]) continue;

            // Virus obeys walls and CLOSED X-gates but ignores
            // player-only locked gates (virus cannot open keys)
            // Also virus cannot enter exit tile
            char ch = ms.grid[nb.row][nb.col];
            if (ch == TILE_WALL_H || ch == TILE_WALL_V) continue;
            if (ch == TILE_EXIT) continue;

            // Check closed X-gates
            bool blocked = false;
            for (const auto& xg : ms.xgates)
                if (!xg.open && xg.pos == nb) { blocked=true; break; }
            if (blocked) continue;

            // Check closed numbered gates (virus cannot pass them)
            for (const auto& g : ms.gates)
                if (!g.gateOpen && g.gatePos == nb) { blocked=true; break; }
            if (blocked) continue;

            visited[nb.row][nb.col] = true;
            parent[nb.row][nb.col] = cur;
            if (nb == dst) { found=true; break; }
            q.push(nb);
        }
    }

    if (!found) return src; // No path

    // Trace back from dst to find the first step from src
    Pos step = dst;
    while (parent[step.row][step.col] != src) {
        Pos prev = parent[step.row][step.col];
        if (prev.row == -1) return src; // safety
        step = prev;
    }
    return step;
}

// ─────────────────────────────────────────────────────────────
//  moveVirus – public API
// ─────────────────────────────────────────────────────────────
bool moveVirus(MapState& ms, int steps) {
    for (int s = 0; s < steps; ++s) {
        Pos next = bfsNextStep(ms);
        ms.virusPos = next;

        // Check catch condition
        if (ms.virusPos == ms.playerPos) return true;
    }
    return false;
}
