// GridWorldMDP.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#define GRID_DIMENSION 8
#define IT_NUM 10000

enum Type { WALL, TERMINAL, NON_TERMINAL };
enum Direction { RIGHT, LEFT, UP, DOWN, NONE };
double const rewardValues[GRID_DIMENSION][GRID_DIMENSION] = { { 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, -0.04, -1, -0.04, -0.04, -0.04, -0.04,0 },
{ 0, -0.04, -0.04, -0.04, 0, -1, -0.04,0 },
{ 0, -0.04, -0.04, -0.04, 0, -0.04, 3,0 },
{ 0, -0.04, -0.04, -0.04, 0, -0.04, -0.04,0 },
{ 0, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04,0 },
{ 0, 1, -1, -0.04, 0, -1, -1,0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 } };
double const forwardProb = 0.8; // probability of moving in desired direction
double const turnProb = 0.1;
double const gamma = 0.99; //discount factor
class block
{
public:
	double reward; //reward for visiting this block
	Type type;
	Direction policy; //the current policy for this block
	double utility; //holds utility which gets updated every iteration
	double utilityNext;

};

block** gridWorld;

/*finds the best direction to go in based on the expected utilities. Returns best utility value and changes policy variable*/
double best(int i, int j)
{
	Direction currentPolicy;
	double currentBest;
	double localBest;
	double forward, right, left;
	/*try going up*/
	if (gridWorld[i - 1][j].type == WALL)
		forward = forwardProb*gridWorld[i][j].utility;
	else
		forward = forwardProb*gridWorld[i - 1][j].utility;
	if (gridWorld[i][j + 1].type == WALL)
		right = turnProb*gridWorld[i][j].utility;
	else
		right = turnProb*gridWorld[i][j + 1].utility;
	if (gridWorld[i][j - 1].type == WALL)
		left = turnProb*gridWorld[i][j].utility;
	else
		left = turnProb*gridWorld[i][j - 1].utility;
	currentBest = forward + right + left;
	currentPolicy = UP;

	/*try going down*/
	if (gridWorld[i + 1][j].type == WALL)
		forward = forwardProb*gridWorld[i][j].utility;
	else
		forward = forwardProb*gridWorld[i + 1][j].utility;
	if (gridWorld[i][j + 1].type == WALL)
		right = turnProb*gridWorld[i][j].utility;
	else
		right = turnProb*gridWorld[i][j + 1].utility;
	if (gridWorld[i][j - 1].type == WALL)
		left = turnProb*gridWorld[i][j].utility;
	else
		left = turnProb*gridWorld[i][j - 1].utility;
	localBest = forward + right + left;
	if (localBest > currentBest)
	{
		currentBest = localBest;
		currentPolicy = DOWN;
	}

	/*try going right*/
	if (gridWorld[i][j + 1].type == WALL)
		forward = forwardProb*gridWorld[i][j].utility;
	else
		forward = forwardProb*gridWorld[i][j + 1].utility;
	if (gridWorld[i + 1][j].type == WALL)
		right = turnProb*gridWorld[i][j].utility;
	else
		right = turnProb*gridWorld[i + 1][j].utility;
	if (gridWorld[i - 1][j].type == WALL)
		left = turnProb*gridWorld[i][j].utility;
	else
		left = turnProb*gridWorld[i - 1][j].utility;
	localBest = forward + right + left;
	if (localBest > currentBest)
	{
		currentBest = localBest;
		currentPolicy = RIGHT;
	}

	/*try going left*/
	if (gridWorld[i][j - 1].type == WALL)
		forward = forwardProb*gridWorld[i][j].utility;
	else
		forward = forwardProb*gridWorld[i][j - 1].utility;
	if (gridWorld[i - 1][j].type == WALL)
		right = turnProb*gridWorld[i][j].utility;
	else
		right = turnProb*gridWorld[i - 1][j].utility;
	if (gridWorld[i + 1][j].type == WALL)
		left = turnProb*gridWorld[i][j].utility;
	else
		left = turnProb*gridWorld[i + 1][j].utility;
	localBest = forward + right + left;
	if (localBest > currentBest)
	{
		currentBest = localBest;
		currentPolicy = LEFT;
	}
	gridWorld[i][j].policy = currentPolicy;
	return currentBest;
}


/*do value iteration over the entire grid IT_NUM nubmer of times to get utilities and optimal policy
arg = 1 is for no stopping at terminal states*/
void ValueIterate(int arg)
{
	for (int k = 0; k < IT_NUM; k++)
	{
		/*compute next utilities*/
		for (int i = 1; i < GRID_DIMENSION - 1; i++)
		{
			for (int j = 1; j < GRID_DIMENSION - 1; j++)
			{
				if (gridWorld[i][j].type == NON_TERMINAL || (gridWorld[i][j].type == TERMINAL && arg == 1))
				{
					gridWorld[i][j].utilityNext = gridWorld[i][j].reward + gamma*best(i, j);
				}
			}
		}
		/*update utilities*/
		for (int i = 1; i < GRID_DIMENSION - 1; i++)
		{
			for (int j = 1; j < GRID_DIMENSION - 1; j++)
			{
				if (gridWorld[i][j].type == NON_TERMINAL || (gridWorld[i][j].type == TERMINAL && arg == 1))
				{
					gridWorld[i][j].utility = gridWorld[i][j].utilityNext;
				}
			}
		}
	}
}

void Output(string filename)
{
	ofstream myFile(filename.c_str());
	myFile << fixed << setprecision(6);
	myFile << "Utilities of states" << endl;
	for (int i = 1; i < GRID_DIMENSION - 1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION - 1; j++)
		{
			myFile << gridWorld[i][j].utility << " ";
			if (j == GRID_DIMENSION - 2)
				myFile << endl;
		}
	}
	myFile << endl << "Policies" << endl;
	for (int i = 1; i < GRID_DIMENSION - 1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION - 1; j++)
		{
			if (gridWorld[i][j].policy == UP)
				myFile << "UP ";
			else if (gridWorld[i][j].policy == DOWN)
				myFile << "DN ";
			else if (gridWorld[i][j].policy == RIGHT)
				myFile << "RT ";
			else if (gridWorld[i][j].policy == LEFT)
				myFile << "LT ";
			else
				myFile << "   ";
			if (j == GRID_DIMENSION - 2)
				myFile << endl;
		}
	}
}

int main()
{
	/*initialize gridWorld*/
	gridWorld = new block*[GRID_DIMENSION];
	for (int i = 0; i < GRID_DIMENSION; i++)
	{
		gridWorld[i] = new block[GRID_DIMENSION];
	}
	for (int i = 0; i < GRID_DIMENSION; i++)
	{
		for (int j = 0; j < GRID_DIMENSION; j++)
		{
			gridWorld[i][j].reward = rewardValues[i][j];
			gridWorld[i][j].utility = 0;
			if (rewardValues[i][j] == -0.04) {
				gridWorld[i][j].type = NON_TERMINAL;
			}
			else if (rewardValues[i][j] == 0) {
				gridWorld[i][j].type = WALL;
				gridWorld[i][j].policy = NONE;
			}
			else {
				gridWorld[i][j].type = TERMINAL;
				gridWorld[i][j].policy = NONE;
				gridWorld[i][j].utility = gridWorld[i][j].reward;
			}
		}
	}
	/*do value iteration for stopping at terminal states*/
	ValueIterate(0);
	Output("MDPoutput1.txt");

	/*reset utilities*/
	for (int i = 1; i < GRID_DIMENSION-1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION-1; j++)
		{
			gridWorld[i][j].utility = 0;
		}
	}

	/*do value iteration for no stopping at terminal states*/
	ValueIterate(1);
	Output("MDPoutput2.txt");

	/*memory cleanup*/
	for (int i = 0; i < GRID_DIMENSION; i++)
	{
		delete[] gridWorld[i];
	}
	delete[] gridWorld;

	return 0;
}

