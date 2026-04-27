#include "virus.h"
#include <queue>
#include <vector>
using namespace std;

//返回virus的下一步位置
static Pos bfsNextStep(const MapState& ms) { //Pos和MapState都是map定义的struct
    Pos src = ms.virusPos; 
    Pos dst = ms.playerPos;

    //parent表示前一格
    vector<vector<Pos>> parent(ms.rows);//二维vector，内层表示col，外层表示row
    for (int i=0;i<ms.rows;i++){
        for(int j=0;j<ms.cols;j++){
            parent[i].push_back({-1,-1});
        }
    }
    vector<vector<bool>> visited(ms.rows);
    //每个格子都初始化为false
    for (int i=0;i<ms.rows;i++){
        for(int j=0;j<ms.cols;j++){
            visited[i].push_back(false);
        }
    }
    
    //创建一个名字叫q的队列
    queue<Pos> q;
    //把起点的格子放进去
    q.push(src);
    //起点的格子已经visited过了
    visited[src.row][src.col] = true;

    //上下左右定义成array
    const int lr[4] = {-1,1,0,0};
    const int ud[4] = {0,0,-1,1};

    //found：找到P
    bool found = false;
    while (!q.empty() && !found) {
        Pos current = q.front(); 
        q.pop();
        for (int d = 0; d < 4; ++d) {
            Pos nb{
                current.row+lr[d],
                current.col+ud[d]
            };
            //如果超过边界就跳过
            if (nb.row<=0 || nb.col<=0 || nb.row>=ms.rows-1 || nb.col>=ms.cols-1){
               continue;
            }
            //如果格子访问过就跳过
            if (visited[nb.row][nb.col]){
                continue;
            }
            
            
            //检查nb的char是什么
            char ch = ms.grid[nb.row][nb.col];
            //如果是墙就跳过
            if (ch == TILE_WALL_H || ch == TILE_WALL_V){
                continue;
            } 
            //如果是出口也跳过
            if (ch == TILE_EXIT){
                continue;
            }
            
            
            //检查所有xgates，如果是nb又没开就跳过
            bool blocked = false;
            for (int i=0;i<ms.xgates.size();i++){
                if (!ms.xgates[i].open && ms.xgates[i].pos == nb) {
                     blocked=true; 
                     break; 
                    }
                }
            if (blocked==true){
                continue;
            }
            
            
            //检查所有gates，跟xgates一样
            for (int i=0;i<ms.gates.size();i++){
                if (!ms.gates[i].gateOpen && ms.gates[i].gatePos == nb){ 
                    blocked=true; 
                    break;
                }
            }
            if (blocked==true){
                 continue;
            } 

            visited[nb.row][nb.col] = true;
            parent[nb.row][nb.col] = current;
            if (nb == dst) { 
                found=true; 
                break; 
            }
            //接下来从这个nb开始搜
            q.push(nb);
        }
    }
    
    //如果没有找到P，found=false。就保持原地不动
    if (!found){
        return src;
    } 

    //如果已经找到P，found=true。我们开始决定V下一步的位置
    Pos current_ = dst;
    //找的思路：倒着找，从P开始倒退回V。当P的parent格子==src时，就是V的下一步
    while (parent[current_.row][current_.col] != src) {
        Pos prev = parent[current_.row][current_.col];
        if (prev.row == -1){ //保险措施。如果有问题就直接返回V的位置
            return src;
        }
        current_ = prev;
    }
    return current_;
}

//返回抓没抓到P
bool moveVirus(MapState& ms, int steps) {
    for (int s = 0; s < steps; ++s) {
        Pos next = bfsNextStep(ms);
        ms.virusPos = next;
        if (ms.virusPos == ms.playerPos){
            return true;
        }
    }
    return false;
}
