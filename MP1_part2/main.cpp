#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <cmath>

using namespace std;

enum Type { WALL, OPEN, START, GOAL };
enum Dir {NONE, LEFT, RIGHT, UP, DOWN};

/* used to define a space on the maze */
struct point {
	int row;
	int col;
	int cost;
	int heuristic;
	Type type;
	bool explored;
	bool on_frontier;
	point* prev;
} start, goal;

struct dimension {
	int rows;
	int cols;
} maze_dimensions;

point** maze;
Dir direction = NONE; //keeps track of which direction we are currently moving in.
int final_cost; //holds final path cost number
int final_expanded; //holds final expanded nodes number



/* Returns true if cost+heursitic of p1 is greater than cost+heuristic p2, used for priority queue */
class ComparePointAStar {
public:
	bool operator()(point* p1, point* p2)
	{
		return (p1->heuristic + p1->cost) > (p2->heuristic + p2->cost);
	}
};


/* determine the dimensions of the maze */
void get_maze_dimensions(string filename) {
	ifstream file(filename.c_str());

	int row_count = 0;
	string line;
	if (file.is_open()) {
		/* measure length of first row since it should be consistent for every row */
		getline(file, line);
		maze_dimensions.cols = line.length();
		row_count++;

		/* count the rest of the rows */
		while (getline(file, line)) {
			row_count++;
		}
		file.close();
	}
	maze_dimensions.rows = row_count;

	return;
}

/* take data from txt file and store it into an array */
void input_maze(string filename) {

	ifstream file(filename.c_str());

	int cur_row = 0;
	string line;
	if (file.is_open()) {
		while (getline(file, line)) {
			for (int cur_col = 0; cur_col<(int)line.length(); cur_col++) {
				point p;
				p.row = cur_row;
				p.col = cur_col;
				p.cost = 1000000000;
				p.heuristic = 0;
				p.explored = false;
				p.on_frontier = false;
				if (line[cur_col] == '%') {
					p.type = WALL;
				}
				else if (line[cur_col] == '.') {
					p.type = GOAL;
					goal.row = cur_row;
					goal.col = cur_col;
				}
				else if (line[cur_col] == 'P') {
					p.type = START;
					start.row = cur_row;
					start.col = cur_col;
				}
				else if (line[cur_col] == ' ') {
					p.type = OPEN;
				}
				maze[cur_row][cur_col] = p;
			}
			cur_row++;
		}
		file.close();
	}

	return;
}

/* output based on what is in the maze array */
void output_maze(string filename) {

	ofstream file(filename.c_str());
	if (file.is_open())
	{
		/*print final cost and expanded nodes number to file*/
		file << "Cost: " << final_cost << endl;
		file << "Nodes expanded: " << final_expanded << endl;

		for (int i = 0; i<maze_dimensions.rows; i++) {
			for (int j = 0; j<maze_dimensions.cols; j++) {
				if (maze[i][j].type == WALL)
					file << '%';
				else if (maze[i][j].type == START)
					file << 'P';
				else if (maze[i][j].type == GOAL)
					file << '.';
				else if (maze[i][j].type == OPEN)
					file << ' ';
			}
			file << '\n';
		}
		file.close();
	}
	return;
}

// heuristic calculated using manhatten distance + amount of turning required to reach goal
int calc_heuristic(point* p1){
	//goal has heuristic of 0
	if((p1->row == goal.row) && (p1->col == goal.col)){
		return 0;
	}

	int heuristic = abs(p1->row - goal.row) + abs(p1->col - goal.col);
	if(p1->col < goal.col){
		if(p1->row < goal.row){	//goal is to the right and down
			if(direction == UP || direction == LEFT){	//must turn to go right and down at some point
				heuristic+=2;
			}
			else{			//either facing right or down so may only have to turn once
				heuristic++;
			}
		}
		else if(p1->row > goal.row){	//goal is to the right and up
			if(direction == DOWN || direction == LEFT){
				heuristic+=2;
			}
			else{
				heuristic++;
			}
		}
		else{	//goal is to the right, same row
			if(direction == UP || direction == DOWN){
				heuristic++;
			}
			else if(direction == LEFT){
				heuristic+=2;
			}
		}
	}
	else if (p1->col > goal.col){
		if(p1->row < goal.row){	//goal is to the left and down
			if(direction == UP || direction == RIGHT){
				heuristic+=2;
			}
			else{
				heuristic++;
			}
		}
		else if(p1->row > goal.row){	//goal is to the left and up
			if(direction == DOWN || direction == RIGHT){
				heuristic+=2;
			}
			else{
				heuristic++;
			}
		}
		else{	//goal is to the left, same row
			if(direction == UP || direction == DOWN){
				heuristic++;
			}
			else if(direction == RIGHT){
				heuristic+=2;
			}
		}
	}
	else{	//same column
		if(p1->row < goal.row){	//goal is down
			if(direction == LEFT || direction == RIGHT){
				heuristic++;
			}
			else if(direction == UP){
				heuristic+=2;
			}
		}
		else if(p1->row > goal.row){	//goal is up
			if(direction == LEFT || direction == RIGHT){
				heuristic++;
			}
			else if(direction == DOWN){
				heuristic+=2;
			}
		}
	}
	return heuristic;
}


/*turn cost is 2*/
/*forward cost is 1*/
void astar_search_turn_penalty() {
	priority_queue<point*, vector<point*>, ComparePointAStar> pq;

	maze[start.row][start.col].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze[start.row][start.col].cost = 0;
	maze[start.row][start.col].prev = NULL;
	pq.push(&maze[start.row][start.col]);

	point* p;
	int nodes_expanded = 0;
	while (1) {
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		/*set current direction*/
		if (p->prev == NULL)
			direction = NONE;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row + 1))
			direction = UP;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row - 1))
			direction = DOWN;
		else if ((p->prev->col == p->col + 1) && (p->prev->row == p->row))
			direction = LEFT;
		else if ((p->prev->col == p->col - 1) && (p->prev->row == p->row))
			direction = RIGHT;

		nodes_expanded++;

		/* check if it is goal node */
		if (p->type == GOAL) {
			printf("PATH LENGTH: %d\n", p->cost);
			final_cost = p->cost;
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			final_expanded = nodes_expanded;
			break;
		}

		/* expand the node */
		/* try moving up */
		if (p->row > 0) {
			/* dont do anything if the node is already explored, also dont add walls */
			if (maze[p->row - 1][p->col].explored == false && maze[p->row - 1][p->col].type != WALL) {
				/*if we were already moving up then cost is only 1*/
				if (direction == UP || direction == NONE) {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 1) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 1;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = abs(goal.row - maze[p->row - 1][p->col].row) + abs(goal.col - maze[p->row - 1][p->col].col);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 1;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
				/*we were not already moving up so cost is 2*/
				else {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 2) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 2;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = abs(goal.row - maze[p->row - 1][p->col].row) + abs(goal.col - maze[p->row - 1][p->col].col);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 2;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
			}
		}

		/* try moving down */
		if (p->row < (maze_dimensions.rows - 1)) {
			if (maze[p->row + 1][p->col].explored == false && maze[p->row + 1][p->col].type != WALL) {
				/*if we were already moving down then cost is only 1*/
				if (direction == DOWN || direction == NONE) {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 1) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 1;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = abs(goal.row - maze[p->row + 1][p->col].row) + abs(goal.col - maze[p->row + 1][p->col].col);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 1;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
				/*we were not already moving down so cost is 2*/
				else {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 2) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 2;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = abs(goal.row - maze[p->row + 1][p->col].row) + abs(goal.col - maze[p->row + 1][p->col].col);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 2;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
			}
		}

		/* try moving left */
		if (p->col > 0) {
			if (maze[p->row][p->col - 1].explored == false && maze[p->row][p->col - 1].type != WALL) {
				/*if we were already moving left then cost is only 1*/
				if (direction == LEFT || direction == NONE) {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 1;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = abs(goal.row - maze[p->row][p->col - 1].row) + abs(goal.col - maze[p->row][p->col - 1].col);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 1;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}
				}
				/*we were not already moving left so cost is 2*/
				else {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 2;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = abs(goal.row - maze[p->row][p->col - 1].row) + abs(goal.col - maze[p->row][p->col - 1].col);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 2;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}

				}
			}
		}

		/* try moving right */
		if (p->col < (maze_dimensions.cols - 1)) {
			if (maze[p->row][p->col + 1].explored == false && maze[p->row][p->col + 1].type != WALL) {
				/*if we were already moving right then cost is only 1*/
				if (direction == RIGHT || direction == NONE) {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 1;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = abs(goal.row - maze[p->row][p->col + 1].row) + abs(goal.col - maze[p->row][p->col + 1].col);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 1;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
				/*we were not already moving right so cost is 2*/
				else {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 2;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = abs(goal.row - maze[p->row][p->col + 1].row) + abs(goal.col - maze[p->row][p->col + 1].col);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 2;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
			}
		}
	}

	/* mark the solution on the maze */
	while (p->prev != NULL) {
		p->type = GOAL;
		p = p->prev;
	}
}

/*turn cost is 1*/
/*forward cost is 2*/
void astar_search_forward_penalty() {
	priority_queue<point*, vector<point*>, ComparePointAStar> pq;

	maze[start.row][start.col].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze[start.row][start.col].cost = 0;
	maze[start.row][start.col].prev = NULL;
	pq.push(&maze[start.row][start.col]);

	point* p;
	int nodes_expanded = 0;
	while (1) {
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		/*set current direction*/
		if (p->prev == NULL)
			direction = NONE;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row + 1))
			direction = UP;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row - 1))
			direction = DOWN;
		else if ((p->prev->col == p->col + 1) && (p->prev->row == p->row))
			direction = LEFT;
		else if ((p->prev->col == p->col - 1) && (p->prev->row == p->row))
			direction = RIGHT;

		nodes_expanded++;

		/* check if it is goal node */
		if (p->type == GOAL) {
			printf("PATH LENGTH: %d\n", p->cost);
			final_cost = p->cost;
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			final_expanded = nodes_expanded;
			break;
		}

		/* expand the node */
		/* try moving up */
		if (p->row > 0) {
			/* dont do anything if the node is already explored, also dont add walls */
			if (maze[p->row - 1][p->col].explored == false && maze[p->row - 1][p->col].type != WALL) {
				/*if we were already moving up then cost is only 1*/
				if (direction == UP || direction == NONE) {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 2) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 2;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = abs(goal.row - maze[p->row - 1][p->col].row) + abs(goal.col - maze[p->row - 1][p->col].col);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 2;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
				/*we were not already moving up so cost is 2*/
				else {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 1) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 1;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = abs(goal.row - maze[p->row - 1][p->col].row) + abs(goal.col - maze[p->row - 1][p->col].col);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 1;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
			}
		}

		/* try moving down */
		if (p->row < (maze_dimensions.rows - 1)) {
			if (maze[p->row + 1][p->col].explored == false && maze[p->row + 1][p->col].type != WALL) {
				/*if we were already moving down then cost is only 1*/
				if (direction == DOWN || direction == NONE) {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 2) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 2;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = abs(goal.row - maze[p->row + 1][p->col].row) + abs(goal.col - maze[p->row + 1][p->col].col);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 2;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
				/*we were not already moving down so cost is 2*/
				else {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 1) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 1;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = abs(goal.row - maze[p->row + 1][p->col].row) + abs(goal.col - maze[p->row + 1][p->col].col);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 1;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
			}
		}

		/* try moving left */
		if (p->col > 0) {
			if (maze[p->row][p->col - 1].explored == false && maze[p->row][p->col - 1].type != WALL) {
				/*if we were already moving left then cost is only 1*/
				if (direction == LEFT || direction == NONE) {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 2;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = abs(goal.row - maze[p->row][p->col - 1].row) + abs(goal.col - maze[p->row][p->col - 1].col);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 2;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}
				}
				/*we were not already moving left so cost is 2*/
				else {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 1;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = abs(goal.row - maze[p->row][p->col - 1].row) + abs(goal.col - maze[p->row][p->col - 1].col);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 1;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}

				}
			}
		}

		/* try moving right */
		if (p->col < (maze_dimensions.cols - 1)) {
			if (maze[p->row][p->col + 1].explored == false && maze[p->row][p->col + 1].type != WALL) {
				/*if we were already moving right then cost is only 1*/
				if (direction == RIGHT || direction == NONE) {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 2;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = abs(goal.row - maze[p->row][p->col + 1].row) + abs(goal.col - maze[p->row][p->col + 1].col);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 2;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
				/*we were not already moving right so cost is 2*/
				else {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 1;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = abs(goal.row - maze[p->row][p->col + 1].row) + abs(goal.col - maze[p->row][p->col + 1].col);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 1;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
			}
		}
	}

	/* mark the solution on the maze */
	while (p->prev != NULL) {
		p->type = GOAL;
		p = p->prev;
	}
}


/*turn cost is 2*/
/*forward cost is 1*/
/*new heuristic*/
void astar_search_turn_penalty_new_heuristic() {
	priority_queue<point*, vector<point*>, ComparePointAStar> pq;

	maze[start.row][start.col].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze[start.row][start.col].cost = 0;
	maze[start.row][start.col].prev = NULL;
	pq.push(&maze[start.row][start.col]);

	point* p;
	int nodes_expanded = 0;
	while (1) {
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		/*set current direction*/
		if (p->prev == NULL)
			direction = NONE;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row + 1))
			direction = UP;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row - 1))
			direction = DOWN;
		else if ((p->prev->col == p->col + 1) && (p->prev->row == p->row))
			direction = LEFT;
		else if ((p->prev->col == p->col - 1) && (p->prev->row == p->row))
			direction = RIGHT;

		nodes_expanded++;

		/* check if it is goal node */
		if (p->type == GOAL) {
			printf("PATH LENGTH: %d\n", p->cost);
			final_cost = p->cost;
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			final_expanded = nodes_expanded;
			break;
		}

		/* expand the node */
		/* try moving up */
		if (p->row > 0) {
			/* dont do anything if the node is already explored, also dont add walls */
			if (maze[p->row - 1][p->col].explored == false && maze[p->row - 1][p->col].type != WALL) {
				/*if we were already moving up then cost is only 1*/
				if (direction == UP || direction == NONE) {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 1) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 1;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = calc_heuristic(&maze[p->row-1][p->col]);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 1;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
				/*we were not already moving up so cost is 2*/
				else {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 2) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 2;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = calc_heuristic(&maze[p->row-1][p->col]);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 2;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
			}
		}

		/* try moving down */
		if (p->row < (maze_dimensions.rows - 1)) {
			if (maze[p->row + 1][p->col].explored == false && maze[p->row + 1][p->col].type != WALL) {
				/*if we were already moving down then cost is only 1*/
				if (direction == DOWN || direction == NONE) {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 1) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 1;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = calc_heuristic(&maze[p->row+1][p->col]);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 1;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
				/*we were not already moving down so cost is 2*/
				else {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 2) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 2;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = calc_heuristic(&maze[p->row+1][p->col]);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 2;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
			}
		}

		/* try moving left */
		if (p->col > 0) {
			if (maze[p->row][p->col - 1].explored == false && maze[p->row][p->col - 1].type != WALL) {
				/*if we were already moving left then cost is only 1*/
				if (direction == LEFT || direction == NONE) {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 1;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = calc_heuristic(&maze[p->row][p->col-1]);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 1;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}
				}
				/*we were not already moving left so cost is 2*/
				else {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 2;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = calc_heuristic(&maze[p->row][p->col-1]);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 2;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}

				}
			}
		}

		/* try moving right */
		if (p->col < (maze_dimensions.cols - 1)) {
			if (maze[p->row][p->col + 1].explored == false && maze[p->row][p->col + 1].type != WALL) {
				/*if we were already moving right then cost is only 1*/
				if (direction == RIGHT || direction == NONE) {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 1;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = calc_heuristic(&maze[p->row][p->col+1]);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 1;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
				/*we were not already moving right so cost is 2*/
				else {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 2;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = calc_heuristic(&maze[p->row][p->col+1]);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 2;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
			}
		}
	}

	/* mark the solution on the maze */
	while (p->prev != NULL) {
		p->type = GOAL;
		p = p->prev;
	}
}


/*turn cost is 1*/
/*forward cost is 2*/
/*new heuristic*/
void astar_search_forward_penalty_new_heuristic() {
	priority_queue<point*, vector<point*>, ComparePointAStar> pq;

	maze[start.row][start.col].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze[start.row][start.col].cost = 0;
	maze[start.row][start.col].prev = NULL;
	pq.push(&maze[start.row][start.col]);

	point* p;
	int nodes_expanded = 0;
	while (1) {
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		/*set current direction*/
		if (p->prev == NULL)
			direction = NONE;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row + 1))
			direction = UP;
		else if ((p->prev->col == p->col) && (p->prev->row == p->row - 1))
			direction = DOWN;
		else if ((p->prev->col == p->col + 1) && (p->prev->row == p->row))
			direction = LEFT;
		else if ((p->prev->col == p->col - 1) && (p->prev->row == p->row))
			direction = RIGHT;

		nodes_expanded++;

		/* check if it is goal node */
		if (p->type == GOAL) {
			printf("PATH LENGTH: %d\n", p->cost);
			final_cost = p->cost;
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			final_expanded = nodes_expanded;
			break;
		}

		/* expand the node */
		/* try moving up */
		if (p->row > 0) {
			/* dont do anything if the node is already explored, also dont add walls */
			if (maze[p->row - 1][p->col].explored == false && maze[p->row - 1][p->col].type != WALL) {
				/*if we were already moving up then cost is only 1*/
				if (direction == UP || direction == NONE) {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 2) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 2;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = calc_heuristic(&maze[p->row-1][p->col]);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 2;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
				/*we were not already moving up so cost is 2*/
				else {
					if (maze[p->row - 1][p->col].on_frontier) {
						/* if already on the forntier and the path cost is less, update it */
						if ((p->cost + 1) < maze[p->row - 1][p->col].cost) {
							maze[p->row - 1][p->col].cost = p->cost + 1;
							maze[p->row - 1][p->col].prev = p;
						}
					}
					else {
						/* calculate the heuristic and add to frontier */
						maze[p->row - 1][p->col].heuristic = calc_heuristic(&maze[p->row-1][p->col]);
						maze[p->row - 1][p->col].on_frontier = true;
						maze[p->row - 1][p->col].cost = p->cost + 1;
						maze[p->row - 1][p->col].prev = p;
						pq.push(&maze[p->row - 1][p->col]);
					}
				}
			}
		}

		/* try moving down */
		if (p->row < (maze_dimensions.rows - 1)) {
			if (maze[p->row + 1][p->col].explored == false && maze[p->row + 1][p->col].type != WALL) {
				/*if we were already moving down then cost is only 1*/
				if (direction == DOWN || direction == NONE) {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 2) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 2;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = calc_heuristic(&maze[p->row+1][p->col]);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 2;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
				/*we were not already moving down so cost is 2*/
				else {
					if (maze[p->row + 1][p->col].on_frontier) {
						if ((p->cost + 1) < maze[p->row + 1][p->col].cost) {
							maze[p->row + 1][p->col].cost = p->cost + 1;
							maze[p->row + 1][p->col].prev = p;
						}
					}
					else {
						maze[p->row + 1][p->col].heuristic = calc_heuristic(&maze[p->row+1][p->col]);
						maze[p->row + 1][p->col].on_frontier = true;
						maze[p->row + 1][p->col].cost = p->cost + 1;
						maze[p->row + 1][p->col].prev = p;
						pq.push(&maze[p->row + 1][p->col]);
					}
				}
			}
		}

		/* try moving left */
		if (p->col > 0) {
			if (maze[p->row][p->col - 1].explored == false && maze[p->row][p->col - 1].type != WALL) {
				/*if we were already moving left then cost is only 1*/
				if (direction == LEFT || direction == NONE) {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 2;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = calc_heuristic(&maze[p->row][p->col-1]);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 2;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}
				}
				/*we were not already moving left so cost is 2*/
				else {
					if (maze[p->row][p->col - 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col - 1].cost) {
							maze[p->row][p->col - 1].cost = p->cost + 1;
							maze[p->row][p->col - 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col - 1].heuristic = calc_heuristic(&maze[p->row][p->col-1]);
						maze[p->row][p->col - 1].on_frontier = true;
						maze[p->row][p->col - 1].cost = p->cost + 1;
						maze[p->row][p->col - 1].prev = p;
						pq.push(&maze[p->row][p->col - 1]);
					}

				}
			}
		}

		/* try moving right */
		if (p->col < (maze_dimensions.cols - 1)) {
			if (maze[p->row][p->col + 1].explored == false && maze[p->row][p->col + 1].type != WALL) {
				/*if we were already moving right then cost is only 1*/
				if (direction == RIGHT || direction == NONE) {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 2) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 2;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = calc_heuristic(&maze[p->row][p->col+1]);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 2;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
				/*we were not already moving right so cost is 2*/
				else {
					if (maze[p->row][p->col + 1].on_frontier) {
						if ((p->cost + 1) < maze[p->row][p->col + 1].cost) {
							maze[p->row][p->col + 1].cost = p->cost + 1;
							maze[p->row][p->col + 1].prev = p;
						}
					}
					else {
						maze[p->row][p->col + 1].heuristic = calc_heuristic(&maze[p->row][p->col+1]);
						maze[p->row][p->col + 1].on_frontier = true;
						maze[p->row][p->col + 1].cost = p->cost + 1;
						maze[p->row][p->col + 1].prev = p;
						pq.push(&maze[p->row][p->col + 1]);
					}
				}
			}
		}
	}

	/* mark the solution on the maze */
	while (p->prev != NULL) {
		p->type = GOAL;
		p = p->prev;
	}
}


int main() {


	/* test small turns maze with A star forward cost 1 turn cost 2 */
	get_maze_dimensions("smallTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("smallTurns.txt");
	astar_search_turn_penalty();
	output_maze("output files/smallTurns_f1_t2.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test small turns maze with A star forward cost 2 turn cost 1 */
	get_maze_dimensions("smallTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("smallTurns.txt");
	astar_search_forward_penalty();
	output_maze("output files/smallTurns_f2_t1.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test big turns maze with A star forward cost 1 turn cost 2*/
	get_maze_dimensions("bigTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("bigTurns.txt");
	astar_search_turn_penalty();
	output_maze("output files/bigTurns_f1_t2.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test big turns maze with A star forward cost 2 turn cost 1*/
	get_maze_dimensions("bigTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("bigTurns.txt");
	astar_search_forward_penalty();
	output_maze("output files/bigTurns_f2_t1.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;




	/* test small turns maze with A star forward cost 1 turn cost 2, new heuristic */
	get_maze_dimensions("smallTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("smallTurns.txt");
	astar_search_turn_penalty_new_heuristic();
	output_maze("output files/smallTurns_f1_t2_new_heuristic.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test small turns maze with A star forward cost 2 turn cost 1, new heuristic */
	get_maze_dimensions("smallTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("smallTurns.txt");
	astar_search_forward_penalty_new_heuristic();
	output_maze("output files/smallTurns_f2_t1_new_heuristic.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test big turns maze with A star forward cost 1 turn cost 2, new_heursitic*/
	get_maze_dimensions("bigTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("bigTurns.txt");
	astar_search_turn_penalty_new_heuristic();
	output_maze("output files/bigTurns_f1_t2_new_heuristic.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	/* test big turns maze with A star forward cost 2 turn cost 1, new_heuristic*/
	get_maze_dimensions("bigTurns.txt");
	maze = new point*[maze_dimensions.rows];
	for (int i = 0; i<maze_dimensions.rows; i++) {
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("bigTurns.txt");
	astar_search_forward_penalty_new_heuristic();
	output_maze("output files/bigTurns_f2_t1_new_heuristic.txt");
	for (int i = 0; i<maze_dimensions.rows; i++) {
		delete[] maze[i];
	}
	delete[] maze;

	return 0;
}
