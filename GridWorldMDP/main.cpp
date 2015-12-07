// GridWorldMDP.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
using namespace std;

#define GRID_DIMENSION 8
#define IT_NUM 3000
#define IT_NUM_Q 4000
#define N_E 800

enum Type { WALL, TERMINAL, NON_TERMINAL };
enum Direction { RIGHT, LEFT, UP, DOWN, NONE };
double const rewardValues[GRID_DIMENSION][GRID_DIMENSION] = { { 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, -0.04, -1, -0.04, -0.04, -0.04, -0.04, 0 },
{ 0, -0.04, -0.04, -0.04, 0, -1, -0.04, 0 },
{ 0, -0.04, -0.04, -0.04, 0, -0.04, 3, 0 },
{ 0, -0.04, -0.04, -0.04, 0, -0.04, -0.04, 0 },
{ 0, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, 0 },
{ 0, 1, -1, -0.04, 0, -1, -1, 0 },
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

struct state{
	int row;
	int col;
};

state start = { 4, 2 };	//starts at row 4, column 2
double q_values[GRID_DIMENSION][GRID_DIMENSION][4];	//keep track of q values for all actions at each state
int num_actions[GRID_DIMENSION][GRID_DIMENSION][4]; //keep track of how many times weve taken action from state
double utilities[GRID_DIMENSION][GRID_DIMENSION];

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

//given an action and current state, perform the given action and return the new state
state perform_action(state cur_state, Direction action){
	state new_state;
	new_state.row = 0;
	new_state.col = 0;

	/*	generate a random number between 0 and 99
	if r is between 0 and 79, it does correct action
	if r is between 80 and 89, it goes left of correct action
	if r is between 90 and 99, it goes right of correct action*/
	int r = std::rand() % 100;
	if (action == RIGHT){
		if (r <= 79){	//actually goes right
			if (gridWorld[cur_state.row][cur_state.col + 1].type == WALL){	//cant run into wall, so stay put
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{	//no obstacle so move right
				new_state.row = cur_state.row;
				new_state.col = cur_state.col + 1;
			}
		}
		else if (r <= 89){	//goes up
			if (gridWorld[cur_state.row - 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row - 1;
				new_state.col = cur_state.col;
			}
		}
		else{	//goes down
			if (gridWorld[cur_state.row + 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row + 1;
				new_state.col = cur_state.col;
			}
		}
	}
	else if (action == LEFT){
		if (r <= 79){	//actually goes left
			if (gridWorld[cur_state.row][cur_state.col - 1].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row;
				new_state.col = cur_state.col - 1;
			}
		}
		else if (r <= 89){	//goes down
			if (gridWorld[cur_state.row + 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row + 1;
				new_state.col = cur_state.col;
			}
		}
		else{	//goes up
			if (gridWorld[cur_state.row - 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row - 1;
				new_state.col = cur_state.col;
			}
		}
	}
	else if (action == UP){
		if (r <= 79){	//actually goes up
			if (gridWorld[cur_state.row - 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row - 1;
				new_state.col = cur_state.col;
			}
		}
		else if (r <= 89){	//goes left
			if (gridWorld[cur_state.row][cur_state.col - 1].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row;
				new_state.col = cur_state.col - 1;
			}
		}
		else{	//goes right
			if (gridWorld[cur_state.row][cur_state.col + 1].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row;
				new_state.col = cur_state.col + 1;
			}
		}
	}
	else if (action == DOWN){
		if (r <= 79){	//actually goes down
			if (gridWorld[cur_state.row + 1][cur_state.col].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row + 1;
				new_state.col = cur_state.col;
			}
		}
		else if (r <= 89){	//goes right
			if (gridWorld[cur_state.row][cur_state.col + 1].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row;
				new_state.col = cur_state.col + 1;
			}
		}
		else{	//goes left
			if (gridWorld[cur_state.row][cur_state.col - 1].type == WALL){
				new_state.row = cur_state.row;
				new_state.col = cur_state.col;
			}
			else{
				new_state.row = cur_state.row;
				new_state.col = cur_state.col - 1;
			}
		}
	}

	return new_state;
}

//select action to do for current iteration in TD Q-learning
Direction select_action_exploration(state s){
	Direction best_action = RIGHT;
	double max_value = -100.0;
	double cur_value = 0.0;
	for (int i = 0; i < 4; i++){
		Direction d = (Direction)i;
		if (num_actions[s.row][s.col][i] < N_E){	//exploration function, force it to try each direction N_E times
			cur_value = 3 + (std::rand() % 100) / 100.0;	//best possible reward, add some noise
		}
		else{
			//if it collides with wall, use current spots utility
			if (d == RIGHT){
				if (gridWorld[s.row][s.col + 1].type == WALL)
					cur_value = q_values[s.row][s.col][i];
				else
					cur_value = q_values[s.row][s.col + 1][i];
			}
			else if (d == LEFT){
				if (gridWorld[s.row][s.col - 1].type == WALL)
					cur_value = q_values[s.row][s.col][i];
				else
					cur_value = q_values[s.row][s.col - 1][i];
			}
			else if (d == UP){
				if (gridWorld[s.row - 1][s.col].type == WALL)
					cur_value = q_values[s.row][s.col][i];
				else
					cur_value = q_values[s.row - 1][s.col][i];
			}
			else if (d == DOWN){
				if (gridWorld[s.row + 1][s.col].type == WALL)
					cur_value = q_values[s.row][s.col][i];
				else
					cur_value = q_values[s.row + 1][s.col][i];
			}
		}

		if (cur_value > max_value){
			max_value = cur_value;
			best_action = d;
		}
	}
	return best_action;
}

void calculate_utility(){
	double max_reward = 0.0;
	Direction best_policy = RIGHT;
	for (int i = 1; i < GRID_DIMENSION - 1; i++){
		for (int j = 1; j < GRID_DIMENSION - 1; j++){
			max_reward = std::max(q_values[i][j][0], q_values[i][j][1]);
			max_reward = std::max(max_reward, q_values[i][j][2]);
			max_reward = std::max(max_reward, q_values[i][j][3]);

			if (max_reward == q_values[i][j][0]){
				best_policy = RIGHT;
			}
			else if (max_reward == q_values[i][j][1]){
				best_policy = LEFT;
			}
			else if (max_reward == q_values[i][j][2]){
				best_policy = UP;
			}
			else if (max_reward == q_values[i][j][3]){
				best_policy = DOWN;
			}

			gridWorld[i][j].utility = max_reward;

			if (gridWorld[i][j].type == NON_TERMINAL)
				gridWorld[i][j].policy = best_policy;
			else
				gridWorld[i][j].policy = NONE;
		}
	}
}

double calculate_rms_error(){
	double error = 0;
	double N = 36.0;
	for (int i = 1; i < GRID_DIMENSION - 1; i++){
		for (int j = 1; j < GRID_DIMENSION - 1; j++){
			error += pow(gridWorld[i][j].utility - utilities[i][j], 2);
		}
	}
	return sqrt(error / N);
}

void reinforcement_iterate(){
	state cur_state, next_state;
	int time = 0;
	double learning_rate = 0.0;
	double max_reward = 0.0;
	for (int i = 0; i < IT_NUM_Q; i++){
		cur_state = start;
		while (gridWorld[cur_state.row][cur_state.col].type != TERMINAL){	//continue until terminal state is reached
			Direction action = select_action_exploration(cur_state);
			next_state = perform_action(cur_state, action);

			//calc new q
			max_reward = std::max(q_values[next_state.row][next_state.col][0], q_values[next_state.row][next_state.col][1]);
			max_reward = std::max(max_reward, q_values[next_state.row][next_state.col][2]);
			max_reward = std::max(max_reward, q_values[next_state.row][next_state.col][3]);

			learning_rate = 60.0 / (59.0 + time);
			q_values[cur_state.row][cur_state.col][action] = q_values[cur_state.row][cur_state.col][action] + 0.25*(gridWorld[cur_state.row][cur_state.col].reward + gamma*max_reward - q_values[cur_state.row][cur_state.col][action]);

			num_actions[cur_state.row][cur_state.col][action]++;
			time++;
			cur_state = next_state;
		}

		//update and output utility
		calculate_utility();
		if (i % 50 == 0)
			printf("%d: %f\n", i, calculate_rms_error());
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

	//store utilities
	for (int i = 1; i < GRID_DIMENSION - 1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION - 1; j++)
		{
			utilities[i][j] = gridWorld[i][j].utility;
		}
	}

	/*reset utilities*/
	for (int i = 1; i < GRID_DIMENSION - 1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION - 1; j++)
		{
			gridWorld[i][j].utility = 0;
		}
	}

	/*do value iteration for no stopping at terminal states*/
	ValueIterate(1);
	Output("MDPoutput2.txt");


	/*reset utilities*/
	for (int i = 1; i < GRID_DIMENSION - 1; i++)
	{
		for (int j = 1; j < GRID_DIMENSION - 1; j++)
		{
			gridWorld[i][j].utility = 0;
		}
	}

	//start reinforcement learning
	srand(time(NULL));

	//clear array
	for (int i = 0; i < GRID_DIMENSION; i++){
		for (int j = 0; j < GRID_DIMENSION; j++){
			for (int k = 0; k < 4; k++){
				q_values[i][j][k] = 0.0;
				num_actions[i][j][k] = 0;
			}
		}
	}
	//calculate initial q values
	for (int i = 1; i < GRID_DIMENSION - 1; i++){
		for (int j = 1; j < GRID_DIMENSION - 1; j++){
			if (gridWorld[i][j].type == TERMINAL || gridWorld[i][j].type == WALL){	//reward doesnt change
				q_values[i][j][0] = gridWorld[i][j].reward;
				q_values[i][j][1] = gridWorld[i][j].reward;
				q_values[i][j][2] = gridWorld[i][j].reward;
				q_values[i][j][3] = gridWorld[i][j].reward;
			}
			else{
				//right, left, up, down
				q_values[i][j][0] = 0;
				q_values[i][j][1] = 0;
				q_values[i][j][2] = 0;
				q_values[i][j][3] = 0;
			}
		}
	}

	reinforcement_iterate();
	calculate_utility();
	Output("MDPoutput3.txt");

	/*memory cleanup*/
	for (int i = 0; i < GRID_DIMENSION; i++)
	{
		delete[] gridWorld[i];
	}
	delete[] gridWorld;

	return 0;
}