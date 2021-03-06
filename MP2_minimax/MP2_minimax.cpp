// MP2_minimax.cpp : Defines the entry point for the console application.
//


#include "MP2_minimax.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>
#include <algorithm>

using namespace std;

/*initializes each block in the game*/
void setup_game(int x)
{
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		for (int j = 0; j < GAME_DIMENSION; j++)
		{
			game[i][j].value = gameboard[x][i][j];
			game[i][j].team = OPEN;
		}
	}
	blue_expanded = 0;
	green_expanded = 0;
	blue_number_moves = 0;
	green_number_moves = 0;
	blue_time = 0;
	green_time = 0;
	blue_score = 0;
	green_score = 0;
	blocks_occupied = 0;
	play_game();
}

/*outputs gameboard result to a file*/
void output_game(string filename)
{
	ofstream outFile(filename.c_str());
	if (outFile.is_open())
	{
		
		outFile << "Player Blue expanded " << blue_expanded << " nodes" << endl;
		outFile << "Player Green expanded " << green_expanded << " nodes" << endl;
		average_number_moves = float(blue_expanded) / float(blue_number_moves);
		outFile << "Average number of nodes expanded by blue per move: " << average_number_moves << endl;
		average_number_moves = float(green_expanded) / float(green_number_moves);
		outFile << "Average number of nodes expanded by green per move: " << average_number_moves << endl;
		outFile << "Player Blue took " << blue_time << " milliseconds (" << float (blue_time) / float(blue_number_moves) << "ms per move)" << endl;
		outFile << "Player Green took " << green_time << " milliseconds (" << float (green_time) / float(green_number_moves) << "ms per move)" << endl;
		outFile << "Blue total score: " << blue_score << endl;
		outFile << "Green total score: " << green_score << endl;
		for (int i = 0; i < GAME_DIMENSION; i++)
		{
			for (int j = 0; j < GAME_DIMENSION; j++)
			{
				if (game[i][j].team == BLUE)
				{
					outFile << 'B';
				}
				else
				{
					outFile << 'G';
				}
			}
			outFile << endl;
		}
		outFile.close();
	}
}

/*handles actual playing of the game*/
void play_game()
{
	int x, y;
	block** game_copy = new block*[GAME_DIMENSION];
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		game_copy[i] = new block[GAME_DIMENSION];
	}
	Type current_team = BLUE; //player blue goes first
	Type opponent = GREEN;

	/*take turns going until there are no open spaces left*/
	while (blocks_occupied < GAME_DIMENSION*GAME_DIMENSION)
	{
		//make copy before changing things
		for (int i = 0; i < GAME_DIMENSION; i++)
		{
			for (int j = 0; j < GAME_DIMENSION; j++)
			{
				game_copy[i][j].value = game[i][j].value;
				game_copy[i][j].team = game[i][j].team;
			}
		}
		
		clock_t start = clock();
		max_val(game_copy, current_team, opponent, 1, x, y); //take turn -- once this function returns x and y will hold location of where to go next
		clock_t turn_time = (clock() - start)/(CLOCKS_PER_SEC/1000);
														  //first act as if it is a para drop
		blocks_occupied++;
		game[y][x].team = current_team;
		if (current_team == BLUE)
		{
			blue_score += game[y][x].value;
			blue_time += turn_time;
			
		}
		else if (current_team == GREEN)
		{
			green_score += game[y][x].value;
			green_time += turn_time;
		}
		//check for neighbors
		if ((y > 0 && game[y - 1][x].team == current_team) || (y < GAME_DIMENSION - 1 && game[y + 1][x].team == current_team) || (x > 0 && game[y][x - 1].team == current_team) || (x < GAME_DIMENSION - 1 && game[y][x + 1].team == current_team))
		{
			if (y > 0 && game[y - 1][x].team == opponent)
			{
				game[y - 1][x].team = current_team;
				if (current_team == BLUE)
				{
					blue_score += game[y - 1][x].value;
					green_score -= game[y - 1][x].value;
				}
				else
				{
					green_score += game[y - 1][x].value;
					blue_score -= game[y - 1][x].value;
				}

			}
			if (y < GAME_DIMENSION - 1 && game[y + 1][x].team == opponent)
			{
				game[y + 1][x].team = current_team;
				if (current_team == BLUE)
				{
					blue_score += game[y + 1][x].value;
					green_score -= game[y + 1][x].value;
				}
				else
				{
					green_score += game[y + 1][x].value;
					blue_score -= game[y + 1][x].value;
				}
			}
			if (x > 0 && game[y][x - 1].team == opponent)
			{
				game[y][x - 1].team = current_team;
				if (current_team == BLUE)
				{
					blue_score += game[y][x - 1].value;
					green_score -= game[y][x - 1].value;
				}
				else
				{
					green_score += game[y][x - 1].value;
					blue_score -= game[y][x - 1].value;
				}
			}
			if (x < GAME_DIMENSION - 1 && game[y][x + 1].team == opponent)
			{
				game[y][x + 1].team = current_team;
				if (current_team == BLUE)
				{
					blue_score += game[y][x + 1].value;
					green_score -= game[y][x + 1].value;
				}
				else
				{
					green_score += game[y][x + 1].value;
					blue_score -= game[y][x + 1].value;
				}
			}
		}
		if (current_team == BLUE)
		{
			current_team = GREEN;
			opponent = BLUE;
			blue_number_moves += 1;
		}
		else
		{
			current_team = BLUE;
			opponent = GREEN;
			green_number_moves += 1;
		}
	}
	//memory cleanup
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		delete[] game_copy[i];
	}
	delete[] game_copy;

}

int max_val(block** game_board, Type Max_team, Type Min_team, int depth, int& x, int& y)
{
	int best = -1000; //best value so far is held here
	int best_evaluation = -1000; //used for evaluation function
	
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		for (int j = 0; j < GAME_DIMENSION; j++)
		{
			//check if we have reached a terminal node
			if (depth <= 3 && blocks_occupied + depth - 1 == GAME_DIMENSION*GAME_DIMENSION - 1)
			{
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//set location values which will be sent back
					x = j;
					y = i;
					int utility = game_board[i][j].value;
					//check for neighbors
					if ((i > 0 && game_board[i - 1][j].team == Max_team) || (i < GAME_DIMENSION - 1 && game_board[i + 1][j].team == Max_team) || (j > 0 && game_board[i][j - 1].team == Max_team) || (j < GAME_DIMENSION - 1 && game_board[i][j + 1].team == Max_team))
					{
						if (i > 0 && game_board[i - 1][j].team == Min_team)
						{
							utility += game_board[i - 1][j].value;
						}
						if (i < GAME_DIMENSION - 1 && game_board[i + 1][j].team == Min_team)
						{
							utility += game_board[i + 1][j].value;
						}
						if (j > 0 && game_board[i][j - 1].team == Min_team)
						{
							utility += game_board[i][j - 1].value;
						}
						if (j < GAME_DIMENSION - 1 && game_board[i][j + 1].team == Min_team)
						{
							utility += game_board[i][j + 1].value;
						}
					}
					return utility;
				}
			}
			//not at terminal node and depth limit not reached
			else if (depth < 3)
			{
				int local_best;
				int x_loc; //used to hold return value
				int y_loc;
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//MAKE COPY EACH TIME
					block** copy = new block*[GAME_DIMENSION];
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						copy[k] = new block[GAME_DIMENSION];
					}
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							copy[k][m].value = game_board[k][m].value;
							copy[k][m].team = game_board[k][m].team;
						}
					}
					//perform para drop
					copy[i][j].team = Max_team;
					//check for neighbors
					if ((i > 0 && copy[i - 1][j].team == Max_team) || (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Max_team) || (j > 0 && copy[i][j - 1].team == Max_team) || (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Max_team))
					{
						if (i > 0 && copy[i - 1][j].team == Min_team)
						{
							copy[i - 1][j].team = Max_team;
						}
						if (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Min_team)
						{
							copy[i + 1][j].team = Max_team;
						}
						if (j > 0 && copy[i][j - 1].team == Min_team)
						{
							copy[i][j - 1].team = Max_team;
						}
						if (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Min_team)
						{
							copy[i][j + 1].team = Max_team;
						}
					}
					local_best = min_val(copy, Max_team, Min_team, depth + 1, x_loc, y_loc);
					
					//update best location
					if (local_best > best)
					{
						best = local_best;
						x = x_loc;
						y = y_loc;
					}
					//memory cleanup
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						delete[] copy[k];
					}
					delete[] copy;
				}
			}
			/*perform evaluation function since depth limit reached*/
			else if (depth == 3)
			{
				int max_total = 0;
				int min_total = 0;
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//MAKE COPY EACH TIME
					int local_best;
					block** copy = new block*[GAME_DIMENSION];
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						copy[k] = new block[GAME_DIMENSION];
					}
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							copy[k][m].value = game_board[k][m].value;
							copy[k][m].team = game_board[k][m].team;
						}
					}
					//perform para drop
					copy[i][j].team = Max_team;
					//check for neighbors
					if ((i > 0 && copy[i - 1][j].team == Max_team) || (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Max_team) || (j > 0 && copy[i][j - 1].team == Max_team) || (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Max_team))
					{
						if (i > 0 && copy[i - 1][j].team == Min_team)
						{
							copy[i - 1][j].team = Max_team;
						}
						if (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Min_team)
						{
							copy[i + 1][j].team = Max_team;
						}
						if (j > 0 && copy[i][j - 1].team == Min_team)
						{
							copy[i][j - 1].team = Max_team;
						}
						if (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Min_team)
						{
							copy[i][j + 1].team = Max_team;
						}
					}

					/*add up all current values on board of max_team and min_team and compute the difference*/
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							if (copy[k][m].team == Max_team)
							{
								max_total += copy[k][m].value;
							}
							else if (copy[k][m].team == Min_team)
							{
								min_total += copy[k][m].value;
							}
						}
					}
					local_best = max_total - min_total;
					if (local_best > best_evaluation)
					{
						best_evaluation = local_best;
						x = j;
						y = i;
					}
					//memory cleanup
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						delete[] copy[k];
					}
					delete[] copy;
				}
			}
		}
	}
	if (depth < 3)
	{
		return best;
	}
	else
	{
		return best_evaluation;
	}

}

int min_val(block** game_board, Type Max_team, Type Min_team, int depth, int& x, int& y)
{
	int best = 1000; //best value (in this case lowest value) so far is held here
	int best_evaluation = 1000; //used for evaluation function
	
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		for (int j = 0; j < GAME_DIMENSION; j++)
		{
			//check if we have reached a terminal node
			if (depth <= 3 && blocks_occupied + depth - 1 == GAME_DIMENSION*GAME_DIMENSION - 1)
			{
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//set location values which will be sent back
					x = j;
					y = i;
					int utility = game_board[i][j].value;
					//check for neighbors
					if ((i > 0 && game_board[i - 1][j].team == Min_team) || (i < GAME_DIMENSION - 1 && game_board[i + 1][j].team == Min_team) || (j > 0 && game_board[i][j - 1].team == Min_team) || (j < GAME_DIMENSION - 1 && game_board[i][j + 1].team == Min_team))
					{
						if (i > 0 && game_board[i - 1][j].team == Max_team)
						{
							utility += game_board[i - 1][j].value;
						}
						if (i < GAME_DIMENSION - 1 && game_board[i + 1][j].team == Max_team)
						{
							utility += game_board[i + 1][j].value;
						}
						if (j > 0 && game_board[i][j - 1].team == Max_team)
						{
							utility += game_board[i][j - 1].value;
						}
						if (j < GAME_DIMENSION - 1 && game_board[i][j + 1].team == Max_team)
						{
							utility += game_board[i][j + 1].value;
						}
					}
					return utility;
				}
			}
			//not at terminal node and depth limit not reached
			else if (depth < 3)
			{
				int local_best;
				int x_loc; //used to hold return value
				int y_loc;
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//MAKE COPY EACH TIME
					block** copy = new block*[GAME_DIMENSION];
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						copy[k] = new block[GAME_DIMENSION];
					}
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							copy[k][m].value = game_board[k][m].value;
							copy[k][m].team = game_board[k][m].team;
						}
					}
					//perform para drop
					copy[i][j].team = Min_team;
					//check for neighbors
					if ((i > 0 && copy[i - 1][j].team == Min_team) || (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Min_team) || (j > 0 && copy[i][j - 1].team == Min_team) || (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Min_team))
					{
						if (i > 0 && copy[i - 1][j].team == Max_team)
						{
							copy[i - 1][j].team = Min_team;
						}
						if (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Max_team)
						{
							copy[i + 1][j].team = Min_team;
						}
						if (j > 0 && copy[i][j - 1].team == Max_team)
						{
							copy[i][j - 1].team = Min_team;
						}
						if (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Max_team)
						{
							copy[i][j + 1].team = Min_team;
						}
					}
					local_best = max_val(copy, Max_team, Min_team, depth + 1, x_loc, y_loc);

					//update best location if necessary
					if (local_best < best)
					{
						best = local_best;
						x = x_loc;
						y = y_loc;
					}
					//memory cleanup
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						delete[] copy[k];
					}
					delete[] copy;
				}
			}
			/*perform evaluation function since depth limit reached*/
			else if (depth == 3)
			{
				int max_total = 0;
				int min_total = 0;
				if (game_board[i][j].team == OPEN)
				{
					// This open node will be expanded.
					if (Max_team == BLUE) {
						blue_expanded += 1;
					} else if (Max_team == GREEN) {
						green_expanded += 1;
					}
					
					//MAKE COPY EACH TIME
					int local_best;
					block** copy = new block*[GAME_DIMENSION];
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						copy[k] = new block[GAME_DIMENSION];
					}
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							copy[k][m].value = game_board[k][m].value;
							copy[k][m].team = game_board[k][m].team;
						}
					}
					//perform para drop
					copy[i][j].team = Min_team;
					//check for neighbors
					if ((i > 0 && copy[i - 1][j].team == Min_team) || (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Min_team) || (j > 0 && copy[i][j - 1].team == Min_team) || (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Min_team))
					{
						if (i > 0 && copy[i - 1][j].team == Max_team)
						{
							copy[i - 1][j].team = Min_team;
						}
						if (i < GAME_DIMENSION - 1 && copy[i + 1][j].team == Max_team)
						{
							copy[i + 1][j].team = Min_team;
						}
						if (j > 0 && copy[i][j - 1].team == Max_team)
						{
							copy[i][j - 1].team = Min_team;
						}
						if (j < GAME_DIMENSION - 1 && copy[i][j + 1].team == Max_team)
						{
							copy[i][j + 1].team = Min_team;
						}
					}

					/*add up all current values on board of max_team and min_team and compute the difference*/
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						for (int m = 0; m < GAME_DIMENSION; m++)
						{
							if (copy[k][m].team == Max_team)
							{
								max_total += copy[k][m].value;
							}
							else if (copy[k][m].team == Min_team)
							{
								min_total += copy[k][m].value;
							}
						}
					}
					/*we want to minimize the difference*/
					local_best = max_total - min_total;
					if (local_best < best_evaluation)
					{
						best_evaluation = local_best;
						x = j;
						y = i;
					}
					//memory cleanup
					for (int k = 0; k < GAME_DIMENSION; k++)
					{
						delete[] copy[k];
					}
					delete[] copy;
				}
			}
		}
	}
	if (depth < 3)
	{
		return best;
	}
	else
	{
		return best_evaluation;
	}
}

int main()
{
	game = new block*[GAME_DIMENSION];
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		game[i] = new block[GAME_DIMENSION];
	}

	//play game using Keren map
	setup_game(0);
	output_game("minimax_outputs/Keren_output.txt");

	//play game using Narvik map
	setup_game(1);
	output_game("minimax_outputs/Narvik_output.txt");

	//play game using Sevastopol map
	setup_game(2);
	output_game("minimax_outputs/Sevastopol_output.txt");

	//play game using Smolensk map
	setup_game(3);
	output_game("minimax_outputs/Smolesnk_output.txt");

	//play game using Westerplatte map
	setup_game(4);
	output_game("minimax_outputs/Westerplatte_output.txt");

	//memory cleanup
	for (int i = 0; i < GAME_DIMENSION; i++)
	{
		delete[] game[i];
	}
	delete[] game;

	return 0;
}

