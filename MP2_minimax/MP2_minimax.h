#ifndef MP2_MINIMAX_H
#define MP2_MINIMAX_H

#define GAME_DIMENSION 6

#include <string>
using namespace std;

enum Type { BLUE, GREEN, OPEN };

/*holds data for each block on the gameboard*/
class block
{
public:
	int value;
	Type team;
};

void setup_game(int x);
void output_game(string filename);
void play_game();
int max_val(block** game_board, Type Max_team, Type Min_team, int depth, int& x, int& y);
int min_val(block** game_board, Type Max_team, Type Min_team, int depth, int& x, int& y);


block** game;  //pointer to array of gameboard blocks
int blue_expanded = 0; //keeps track of total expanded nodes by blue
int green_expanded = 0; //keeps track of total expanded nodes by green
int blue_number_moves = 0;
int green_number_moves = 0;
float average_number_moves;
int blue_score = 0;
int green_score = 0;
int blocks_occupied = 0; //keeps track of number of blocks which are not OPEN



						 /*holds value data for each location of the 5 game boards*/
int gameboard[5][6][6] =
{
	//Keren
	{
		{ 1,1,1,1,1,1 },
		{ 1,1,1,1,1,1 },
		{ 1,1,1,1,1,1 },
		{ 1,1,1,1,1,1 },
		{ 1,1,1,1,1,1 },
		{ 1,1,1,1,1,1 }
	},
	//Narvik
	{
		{ 99,1,99,1,99,1 },
		{ 1,99,1,99,1,99 },
		{ 99,1,99,1,99,1 },
		{ 1,99,1,99,1,99 },
		{ 99,1,99,1,99,1 },
		{ 1,99,1,99,1,99 }
	},
	//Sevastopol
	{
		{ 1,1,1,1,1,1 },
		{ 2,2,2,2,2,2 },
		{ 4,4,4,4,4,4 },
		{ 8,8,8,8,8,8 },
		{ 16,16,16,16,16,16 },
		{ 32,32,32,32,32,32 }
	},
	//Smolensk
	{
		{ 66,76,28,66,11,9 },
		{ 31,39,50,8,33,14 },
		{ 80,76,39,59,2,48 },
		{ 50,73,43,3,13,3 },
		{ 99,45,72,87,49,4 },
		{ 80,63,92,28,61,53 }
	},
	//Westerplatte
	{
		{ 1,1,1,1,1,1 },
		{ 1,3,4,4,3,1 },
		{ 1,4,2,2,4,1 },
		{ 1,4,2,2,4,1 },
		{ 1,3,4,4,3,1 },
		{ 1,1,1,1,1,1 }
	}
};






#endif
