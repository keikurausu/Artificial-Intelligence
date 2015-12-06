#pragma once
#ifndef MAZE_H
#define MAZE_H

class maze_block
{

public:
	bool wall = false;  //is this block a wall?
	bool open = false;   //is this block open?
	bool goal = false;
	bool start = false;
	bool visited = false; //has this block been visited?
	bool expanded = false;
	int steps = 0; //number of steps to get to this maze block
	maze_block *up; //pointer to node above current node
	maze_block *right;
	maze_block *down;
	maze_block *left;
	maze_block * prev; //pointer to previous maze_block


private:

};



#endif