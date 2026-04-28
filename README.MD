## Team Members
-LI DANDAN(3036458605)
-MAHMUD NAFIS BIN
-RUFINO HYUGO ETHAN (3036572437)
-WU QINGYA
-YU SZE YUI(3036391039)

--------------------------------

## Description of Game
This is a C++ terminal maze game. Player need to escape from a randomly generated maze before being 
caught by the Virus. Player needs to collect keys and open some corresponding gates.
If Player reaches Exit, user wins.
IF Virus reaches Player, user loses.

Objects:
-'P': Player. It is controlled by user.
-'V': Virus. Virus chases Player.
-'E': Exit. Player need to go to exit to win.
-'Gn': Gate. Gate will open when Player gets the key. n represents the different gates. 
-'Kn': Key.
-'X': X-Gate. A temporary Gate that stops Virus until its count decreases to zero.
-'Z': Stun tile. It freezes the Virus for limited steps.
-'*': Stunned Virus.
-'#': Horizontal wall.
-'|': Vertical wall.

This game has three Difficulty Levels: Easy/Medium/Hard
-Easy: There is no Virus. Player needs to collect keys and reach the Exit.
-Medium: Medium mode adds Virus, X-Gate and stunned Virus. X-Gate will temporarily stop Virus chasing 
         the Player. When X-Gate opens, Virus starts to move. Player needs to eat Z to freeze Virus.
-Hard: The maze is much bigger. Virus increases speed(double) when it is close to the Player. 

----------------------------------

## Features
Map:
-Random maze generation
-Different maze sizes
-Wall paths
-X-Gate
-Gate and key system
-Stun tiles

Virus:
-Virus chases the Player
-BFS path finding
-Virus is blocked by closed X-Gates
-Stunned Virus
-In Hard mode, the Virus increases speed when closing to Player


Game and Interaction:



Score:


-----------------------------------

# How 6 coding requirements are satisfied?

1.Generation of random events:
-Map is randomly picked from a library of finished maps.


2.Data structures for storing data:
- The data stored are mostly about the players, the difficulty they have picked and the scores. Which would be stored in exactly like so,
PLAYERS DIFFICULTY STEPS

The orders starting from the least step taken to the most, regardless of difficulty. Plus, the data stored this way due to scores.cpp, which uses istringstream extract(line) that would extract one by one, save it to the memory, then overwrite the file, then put it back to the .txt file again


3.Dynamic memory management:
-The virus part uses data structures such as vector<vector<Pos>>, vector<vector<bool>> and quene<Pos>


4.File input/output:
- Usage of <fstream> libraries for ifstream and ofstream, enabling outputting scores to the terminal as well inputting scores into scores.txt to save local data progress of players attempting the game.


5.Program codes in multiple files:
-The project includes 'main.cpp', 'game.cpp', 'map.cpp', 'virus.cpp', 'score.cpp' and their corresponding 
 header files.
-This enable the usage of cross file functions enabling simplicity in code reading, and easy to distinguish to which function belongs to, and how they play a role in their specific task.


6.Multiple difficulty levels:

- In globals.h, we have created a clear picture or demonstration for each difficulty minimum and maximum tools or obstacles. For example, the Stun('Z') varied among difficulties and the gates also varied for the maximum gates available in Medium and Hard Difficulty. Ensuring the difficulty of both Medium and Hard differed.
Easy difficulty,  on the other hand, is the simplest mode, as the virus doesn't exist and the player is only required to pass the game without any threats but with gate obstacles.





------------------------------------

## Non-Standard Libraries
In 'game.cpp':
<termios.h>
<unistd.h>


-------------------------------------

## Compilation Instructions
-Environment: Unix-like terminal environment.

-Build:
###```bash
   make


-------------------------------------

## Execution Instructions
-Run: ./maze

-Clean: make clean









