// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
/*
/*BFS expands nodes in the following order if there is a tie
up -> right -> down -> left
Changing this does not really affect the output since BFS is optimal
DFS expands nodes in the following order if there is a tie
left -> down -> right -> up
Changing this order considerably changes the output we get in DFS since DFS is not optimal.
*/
#include "stdafx.h"
#include "maze.h"
#include <iostream>
#include <string>
#include <fstream> //for reading and writing to files
#include <queue> //for BFS FIFO queue
#include <stack> //for DFS LIFO stack

using namespace std;

int start_row; //start row position
int start_column; //start column position
int expanded = 0; //counts number of expanded nodes
int final_steps = 0; holds number of steps taken to get to goal
maze_block mazeArray[50][50]; //static array to hold maze info

void parse_string(string& buffer, int rowNumber)
{
	int index = 0;
	for (size_t i = 0; i < buffer.size(); i++)
	{
		char value = buffer[i];

		switch (value)
		{
		case '%':
			mazeArray[rowNumber][index].wall = true;
			break;

		case 'P':
			mazeArray[rowNumber][index].start = true;
			mazeArray[rowNumber][index].prev = NULL;
			mazeArray[rowNumber][index].up = &mazeArray[rowNumber-1][index];
			mazeArray[rowNumber][index].right = &mazeArray[rowNumber][index + 1];
			mazeArray[rowNumber][index].down = &mazeArray[rowNumber + 1][index];
			mazeArray[rowNumber][index].left = &mazeArray[rowNumber][index - 1];
			start_row = rowNumber;
			start_column = index;
			break;

		case '.':
			mazeArray[rowNumber][index].goal = true;
			mazeArray[rowNumber][index].open = true;
			break;

		default:
			mazeArray[rowNumber][index].open = true;
			mazeArray[rowNumber][index].up = &mazeArray[rowNumber - 1][index];
			mazeArray[rowNumber][index].right = &mazeArray[rowNumber][index + 1];
			mazeArray[rowNumber][index].down = &mazeArray[rowNumber + 1][index];
			mazeArray[rowNumber][index].left = &mazeArray[rowNumber][index - 1]; 
		}
		index++;

	}
}

void printSolution(maze_block *ptr)
{
	cout << "steps: " << ptr->steps << endl << "expanded nodes: " << expanded <<endl; //print cost to reach solution
	final_steps = ptr->steps;
	while (ptr->prev != NULL)
	{
		ptr = ptr->prev;
		ptr->goal = true;
	}
}
void BFS()
{
	queue <maze_block*> BFS_queue;
	BFS_queue.push(&mazeArray[start_row][start_column]);
	BFS_queue.front()->visited = true;
	BFS_queue.front()->steps = 0;

	while (!BFS_queue.empty())
	{
		if (BFS_queue.front()->up->open == true && BFS_queue.front()->up->visited == false)
		{
			BFS_queue.front()->up->visited = true;
			BFS_queue.front()->up->prev = BFS_queue.front();
			BFS_queue.front()->up->steps = BFS_queue.front()->steps + 1;
			if (BFS_queue.front()->up->goal == true)
			{
				expanded++;
				printSolution(BFS_queue.front()->up);
				return;
			}
			BFS_queue.push(BFS_queue.front()->up); //push up node to queue
		}
		if (BFS_queue.front()->right->open == true && BFS_queue.front()->right->visited == false)
		{
			BFS_queue.front()->right->visited = true;
			BFS_queue.front()->right->prev = BFS_queue.front();
			BFS_queue.front()->right->steps = BFS_queue.front()->steps + 1;
			if (BFS_queue.front()->right->goal == true)
			{
				expanded++;
				printSolution(BFS_queue.front()->right);
				return;
			}
			BFS_queue.push(BFS_queue.front()->right); //push right node to queue
		}
		if (BFS_queue.front()->down->open == true && BFS_queue.front()->down->visited == false)
		{
			BFS_queue.front()->down->visited = true;
			BFS_queue.front()->down->prev = BFS_queue.front();
			BFS_queue.front()->down->steps = BFS_queue.front()->steps + 1;
			if (BFS_queue.front()->down->goal == true)
			{
				expanded++;
				printSolution(BFS_queue.front()->down);
				return;
			}
			BFS_queue.push(BFS_queue.front()->down); //push down node to queue
		}
		if (BFS_queue.front()->left->open == true && BFS_queue.front()->left->visited == false)
		{
			BFS_queue.front()->left->visited = true;
			BFS_queue.front()->left->prev = BFS_queue.front();
			BFS_queue.front()->left->steps = BFS_queue.front()->steps + 1;
			if (BFS_queue.front()->left->goal == true)
			{
				expanded++;
				printSolution(BFS_queue.front()->left);
				return;
			}
			BFS_queue.push(BFS_queue.front()->left); //push left node to queue
		}
		expanded++;
		BFS_queue.pop();

	}

}

void DFS()
{
	stack <maze_block*> DFS_stack;
	DFS_stack.push(&mazeArray[start_row][start_column]);
	DFS_stack.top()->visited = true;
	DFS_stack.top()->steps = 0;

	while (1)
	{
		maze_block* current_node = DFS_stack.top();
		DFS_stack.pop();

		if (current_node->up->open == true && current_node->up->visited == false)
		{
			current_node->up->visited = true;
			current_node->up->prev = current_node;
			current_node->up->steps = current_node->steps + 1;
			if (current_node->up->goal == true)
			{
				expanded++;
				printSolution(current_node->up);
				return;
			}
			DFS_stack.push(current_node->up); //push up node to queue
		}
		if (current_node->right->open == true && current_node->right->visited == false)
		{
			current_node->right->visited = true;
			current_node->right->prev = current_node;
			current_node->right->steps = current_node->steps + 1;
			if (current_node->right->goal == true)
			{
				expanded++;
				printSolution(current_node->right);
				return;
			}
			DFS_stack.push(current_node->right); //push right node to queue
		}
		if (current_node->down->open == true && current_node->down->visited == false)
		{
			current_node->down->visited = true;
			current_node->down->prev = current_node;
			current_node->down->steps = current_node->steps + 1;
			if (current_node->down->goal == true)
			{
				expanded++;
				printSolution(current_node->down);
				return;
			}
			DFS_stack.push(current_node->down); //push down node to queue
		}
		if (current_node->left->open == true && current_node->left->visited == false)
		{
			current_node->left->visited = true;
			current_node->left->prev = current_node;
			current_node->left->steps = current_node->steps + 1;
			if (current_node->left->goal == true)
			{
				expanded++;
				printSolution(current_node->left);
				return;
			}
			DFS_stack.push(current_node->left); //push left node to queue
		}
		expanded++;

	}

}

int main()
{
	string inputBuffer;
	ifstream myFile("bigMaze.txt");
	//read in first line
	getline(myFile, inputBuffer);
	int mazeColumns = inputBuffer.size(); //get number of maze columns
	int mazeRows = 0;

	//create the maze array which will be a square based on the mazeColumn dimension


	//parse the line
	parse_string(inputBuffer, mazeRows);

	//read in all lines from file
	while (getline(myFile, inputBuffer))
	{
		mazeRows++;
		parse_string(inputBuffer, mazeRows);
	}
	mazeRows++;
	myFile.close();
	bool search = false; 
	cout << "BFS or DFS? (0 for BFS and 1 for DFS)" << endl;
	cin >> search;
	if (search)
	{
		DFS();
	}
	else
	{
		BFS();
	}
	cin.ignore();
	//DFS();

	
	//write solution to file
	ofstream OutFile("BFS_bigMaze.txt");
	OutFile << "steps: " << final_steps << endl << "expanded nodes: " << expanded << endl;

	for (int i = 0; i < mazeRows; i++)
	{
		for (int j = 0; j < mazeColumns; j++)
		{
			if (mazeArray[i][j].start == true)
				OutFile << 'P';
			else if (mazeArray[i][j].goal == true)
				OutFile << '.';
			else if (mazeArray[i][j].wall == true)
				OutFile << '%';
			else if (mazeArray[i][j].open == true)
				OutFile << ' ';
			if (j == mazeColumns - 1)
				OutFile << endl;
		}
	}
	OutFile.close();
	cin.get();


	return 0;
}
