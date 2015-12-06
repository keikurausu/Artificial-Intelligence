#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <algorithm>

using namespace std;

enum Type {WALL, OPEN, START, GOAL, GHOST, WAIT};

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
	int ghost_possibleStates;
} maze_dimensions, maze_ghosts_dimensions;

point*** maze_ghosts;
point** maze;

int max_ghost_length, ghost_row;

int ghost_direction = 1; /* 0 for left, 1 for right*/

int number_of_ghosts;
int* ghost_row_array;
int* ghost_col_array;

/* Returns true if cost of p1 is greater than p2, used for priority queue */
class ComparePointGreedy {
    public:
    bool operator()(point* p1, point* p2)
    {
       return p1->heuristic > p2->heuristic;
    }
};

/* Returns true if cost+heursitic of p1 is greater than cost+heuristic p2, used for priority queue */
class ComparePointAStar {
    public:
    bool operator()(point* p1, point* p2)
    {
       return (p1->heuristic + p1->cost) > (p2->heuristic + p2->cost);
    }
};

int get_ghostCol(int cost, int row) {
	
	ghost_direction = 1;
	int index = -1;
	
	for (int i = 0; i<number_of_ghosts; i++) {
		if (ghost_row_array[i] == row) {
			index = i;
		}
	}
	
	
	if (index != -1) {
		int cur_ghost_column = ghost_col_array[index];
		for (int i = 0; i < cost; i++) {
			if (ghost_direction == 1) {
				if (maze_ghosts[ghost_row_array[index]][cur_ghost_column+1][0].type == WALL) {
					//change  movement to left and move left after hitting wall
					ghost_direction = 0;
					cur_ghost_column--;
				} else {
					cur_ghost_column++;
				}
			} else {
				if (maze_ghosts[ghost_row_array[index]][cur_ghost_column-1][0].type == WALL) {
					//change  movement to right and move right after hitting wall
					ghost_direction = 1;
					cur_ghost_column++;
				} else {
					cur_ghost_column--;
				}
			}
		}
		
		return cur_ghost_column;
	}
	else
		return -1;
}

void get_maze_dimensions(string filename){
	ifstream file(filename);
	
	int row_count = 0;
	string line;
	if (file.is_open()){
		/* measure length of first row since it should be consistent for every row */
		getline(file,line);
		maze_dimensions.cols = line.length();
		row_count++;
		
		/* count the rest of the rows */
		while(getline(file,line)){
			row_count++;
		}
		file.close();
	}
	maze_dimensions.rows = row_count;
	
	return;
}

/* determine the dimensions of the maze */
void get_maze_dimensions_with_ghosts(string filename){
	ifstream file(filename);

	int row_count = 0;
	string line;
	if (file.is_open()){
		/* measure length of first row since it should be consistent for every row */
		getline(file,line);
		maze_ghosts_dimensions.cols = line.length();
		row_count++;

		/* count the rest of the rows */
    while(getline(file,line)){
      row_count++;
    }
    file.close();
  }
	maze_ghosts_dimensions.rows = row_count;
	
	ifstream file2(filename);
	
	int cur_row = 0;
	int ghost_length = 0;
	string line2;
	number_of_ghosts = 0;
	if (file2.is_open()){
		while(getline(file2,line2)){
			if (ghost_length > max_ghost_length) {
				max_ghost_length = ghost_length;
			}
			ghost_length = 0;
			for(int cur_col=0;cur_col<(int)line2.length();cur_col++){
				 if(line2[cur_col] == 'G') {
					ghost_length++;
					number_of_ghosts++;

				}
				else if(line2[cur_col] == 'g'){
					ghost_length++;
				}
			}
			cur_row++;
		}
		file2.close();
	}
	
	maze_ghosts_dimensions.ghost_possibleStates = 2*max_ghost_length - 2;

	return;
}

void input_maze(string filename){
	
	ifstream file(filename);
	
	int cur_row = 0;
	string line;
	if (file.is_open()){
		while(getline(file,line)){
			for(int cur_col=0;cur_col<(int)line.length();cur_col++){
				point p;
				p.row = cur_row;
				p.col = cur_col;
				p.cost = 1000000000;
				p.heuristic = 0;
				p.explored = false;
				p.on_frontier = false;
				if(line[cur_col] == '%'){
					p.type = WALL;
				}
				else if(line[cur_col] == '.'){
					p.type = GOAL;
					goal.row = cur_row;
					goal.col = cur_col;
				}
				else if(line[cur_col] == 'P'){
					p.type = START;
					start.row = cur_row;
					start.col = cur_col;
				}
				else if(line[cur_col] == ' ' || line[cur_col] == 'G' || line[cur_col] == 'g'){
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

/* take data from txt file and store it into an array */
void input_maze_with_ghosts(string filename){

	ifstream file(filename);
	
	int ghost_index = 0;

	int cur_row = 0;
	string line;
	if (file.is_open()) {
		while(getline(file,line)){
			for(int cur_col=0;cur_col<(int)line.length();cur_col++){
				point p;
				p.row = cur_row;
				p.col = cur_col;
				p.cost = 1000000000;
				p.heuristic = 0;
				p.explored = false;
				p.on_frontier = false;
				if(line[cur_col] == '%'){
					p.type = WALL;
				}
				else if(line[cur_col] == '.'){
					p.type = GOAL;
					goal.row = cur_row;
					goal.col = cur_col;
				}
				else if(line[cur_col] == 'P'){
					p.type = START;
					start.row = cur_row;
					start.col = cur_col;
				}
				else if(line[cur_col] == 'G'){
					ghost_row_array[ghost_index] = cur_row;
					ghost_col_array[ghost_index] = cur_col;
					ghost_index++;
					p.type = GHOST;
					
				}
				else if(line[cur_col] == 'g'){
					p.type = GHOST;
				}
				else if(line[cur_col] == ' '){
					p.type = OPEN;
				}
				maze_ghosts[cur_row][cur_col][0] = p;
			}
			cur_row++;
    }
    file.close();
  }
	
	for (int i = 0; i < maze_ghosts_dimensions.rows; i++) {
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			for (int k = 0; k < maze_ghosts_dimensions.ghost_possibleStates; k++) {
				point p;
				p.row = i;
				p.col = j;
				p.cost = 1000000000;
				p.heuristic = 0;
				p.explored = false;
				p.on_frontier = false;
				p.type = maze_ghosts[i][j][0].type;
				maze_ghosts[i][j][k] = p;
			}
		}
	}

	return;
}

void output_maze(string filename){
	
	ofstream file (filename);
	if (file.is_open())
	{
		for (int i=0;i<maze_dimensions.rows;i++){
			for(int j=0;j<maze_dimensions.cols;j++){
				if(maze[i][j].type == WALL)
					file << '%';
				else if(maze[i][j].type == START)
					file << 'P';
				else if(maze[i][j].type == GOAL)
					file << '.';
				else if(maze[i][j].type == OPEN)
					file<< ' ';
			}
			file << '\n';
		}
		file.close();
	}
	return;
}

/* output based on what is in the maze array */
void output_maze_with_ghosts(string filename){

	ofstream file (filename);
	if (file.is_open())
  {
    for (int i=0;i<maze_ghosts_dimensions.rows;i++){
			for(int j=0;j<maze_ghosts_dimensions.cols;j++){
				if(maze_ghosts[i][j][0].type == WALL)
					file << '%';
				else if(maze_ghosts[i][j][0].type == START)
					file << 'P';
				else {
					bool pcman_path = false;
					for (int k = 0; k < maze_ghosts_dimensions.ghost_possibleStates; k++) {
						if (maze_ghosts[i][j][k].type == GOAL) {
							pcman_path = true;
						}
					}
					
					if (pcman_path) {
						file<< '.';
					} else {
						file<< ' ';
					}
				}
			}
			file << '\n';
		}
    file.close();
  }
	return;
}

void astar_search(){
	priority_queue<point*, vector<point*>, ComparePointAStar> pq;
	
	maze[start.row][start.col].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze[start.row][start.col].cost = 0;
	maze[start.row][start.col].prev = NULL;
	pq.push(&maze[start.row][start.col]);
	
	point* p;
	int nodes_expanded = 0;
	while(1){
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		nodes_expanded++;
		
		/* check if it is goal node */
		if(p->type == GOAL){
			printf("PATH LENGTH: %d\n", p->cost);
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			break;
		}
		
		/* expand the node */
		/* try moving up */
		if(p->row > 0){
			/* dont do anything if the node is already explored, also dont add walls */
			if(maze[p->row-1][p->col].explored == false && maze[p->row-1][p->col].type != WALL){
				
				if(maze[p->row-1][p->col].on_frontier){
					/* if already on the forntier and the path cost is less, update it */
					if((p->cost+1) < maze[p->row-1][p->col].cost){
						maze[p->row-1][p->col].cost = p->cost+1;
						maze[p->row-1][p->col].prev = p;
					}
				}
				else{
					/* calculate the heuristic and add to frontier */
					maze[p->row-1][p->col].heuristic = abs(goal.row - maze[p->row-1][p->col].row) + abs(goal.col - maze[p->row-1][p->col].col);
					maze[p->row-1][p->col].on_frontier = true;
					maze[p->row-1][p->col].cost = p->cost+1;
					maze[p->row-1][p->col].prev = p;
					pq.push(&maze[p->row-1][p->col]);
				}
			}
		}
		
		/* try moving down */
		if(p->row < (maze_dimensions.rows-1)){
			if(maze[p->row+1][p->col].explored == false && maze[p->row+1][p->col].type != WALL){
				
				if(maze[p->row+1][p->col].on_frontier){
					if((p->cost+1) < maze[p->row+1][p->col].cost){
						maze[p->row+1][p->col].cost = p->cost+1;
						maze[p->row+1][p->col].prev = p;
					}
				}
				else{
					maze[p->row+1][p->col].heuristic = abs(goal.row - maze[p->row+1][p->col].row) + abs(goal.col - maze[p->row+1][p->col].col);
					maze[p->row+1][p->col].on_frontier = true;
					maze[p->row+1][p->col].cost = p->cost+1;
					maze[p->row+1][p->col].prev = p;
					pq.push(&maze[p->row+1][p->col]);
				}
			}
		}
		
		/* try moving left */
		if(p->col > 0){
			if(maze[p->row][p->col-1].explored == false && maze[p->row][p->col-1].type != WALL){
				
				if(maze[p->row][p->col-1].on_frontier){
					if((p->cost+1) < maze[p->row][p->col-1].cost){
						maze[p->row][p->col-1].cost = p->cost+1;
						maze[p->row][p->col-1].prev = p;
					}
				}
				else{
					maze[p->row][p->col-1].heuristic = abs(goal.row - maze[p->row][p->col-1].row) + abs(goal.col - maze[p->row][p->col-1].col);
					maze[p->row][p->col-1].on_frontier = true;
					maze[p->row][p->col-1].cost = p->cost+1;
					maze[p->row][p->col-1].prev = p;
					pq.push(&maze[p->row][p->col-1]);
				}
			}
		}
		
		/* try moving right */
		if(p->col < (maze_dimensions.cols-1)){
			if(maze[p->row][p->col+1].explored == false && maze[p->row][p->col+1].type != WALL){
				
				if(maze[p->row][p->col+1].on_frontier){
					if((p->cost+1) < maze[p->row][p->col+1].cost){
						maze[p->row][p->col+1].cost = p->cost+1;
						maze[p->row][p->col+1].prev = p;
					}
				}
				else{
					maze[p->row][p->col+1].heuristic = abs(goal.row - maze[p->row][p->col+1].row) + abs(goal.col - maze[p->row][p->col+1].col);
					maze[p->row][p->col+1].on_frontier = true;
					maze[p->row][p->col+1].cost = p->cost+1;
					maze[p->row][p->col+1].prev = p;
					pq.push(&maze[p->row][p->col+1]);
				}
			}
		}
	}
	
	/* mark the solution on the maze */
	while(p->prev != NULL){
		p->type = GOAL;
		p = p->prev;
	}
}

void astar_search_with_ghosts(bool wait_enable) {

	priority_queue<point*, vector<point*>, ComparePointAStar> pq;

	maze_ghosts[start.row][start.col][0].heuristic = abs(start.row - goal.row) + abs(start.col - goal.col);
	maze_ghosts[start.row][start.col][0].cost = 0;
	maze_ghosts[start.row][start.col][0].prev = NULL;
	pq.push(&maze_ghosts[start.row][start.col][0]);

	point* p;
	int nodes_expanded = 0;
	while(1){
		/* expand node on frontier */
		p = pq.top();
		p->explored = true;
		pq.pop();
		nodes_expanded++;
		
		/* check if it is goal node */
		if(p->type == GOAL){
			printf("PATH LENGTH: %d\n", p->cost);
			printf("NODES EXPANDED: %d\n", nodes_expanded);
			break;
		}
		
		/* if collided with ghost, skip to next best node on the frontier */
		if (p->col == get_ghostCol(p->cost, p->row)) {
			continue;
		}
		 
		/* if passed through ghost, skip to next best node on the frontier */
		if ((p->col == get_ghostCol(p->cost - 1, p->row)) && ((p->prev)->col == get_ghostCol(p->cost, p->row))) {
			continue;
		}
		/* expand the node */
		/* try moving up */
		if(p->row > 0){
			/* dont do anything if the node is already explored, also dont add walls */
			if(maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].explored == false && maze_ghosts[p->row-1][p->col][0].type != WALL){
				
				if(maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier){
					/* if already on the forntier and the path cost is less, update it */
					if((p->cost+1) < maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost){
						maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
						maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					}
				}
				else{
					/* calculate the heuristic and add to frontier */
					maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].heuristic = abs(goal.row - maze_ghosts[p->row-1][p->col][0].row) + abs(goal.col - maze_ghosts[p->row-1][p->col][0].col);
					maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier = true;
					maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
					maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					pq.push(&maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates]);
				}
			}
		}
		
		/* try moving down */
		if(p->row < (maze_ghosts_dimensions.rows-1)){
			if(maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].explored == false && maze_ghosts[p->row+1][p->col][0].type != WALL){
				
				if(maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier){
					if((p->cost+1) < maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost){
						maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
						maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					}
				}
				else{
					maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].heuristic = abs(goal.row - maze_ghosts[p->row+1][p->col][0].row) + abs(goal.col - maze_ghosts[p->row+1][p->col][0].col);
					maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier = true;
					maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
					maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					pq.push(&maze_ghosts[p->row+1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates]);
				}
			}
		}
		
		/* try moving left */
		if(p->col > 0){
			if(maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].explored == false && maze_ghosts[p->row][p->col-1][0].type != WALL){
				
				if(maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier){
					if((p->cost+1) < maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost){
						maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
						maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					}
				}
				else{
					maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].heuristic = abs(goal.row - maze_ghosts[p->row][p->col-1][0].row) + abs(goal.col - maze_ghosts[p->row][p->col-1][0].col);
					maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier = true;
					maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
					maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					pq.push(&maze_ghosts[p->row][p->col-1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates]);
				}
			}
		}
		
		/* try moving right */
		if(p->col < (maze_ghosts_dimensions.cols-1)){
			if(maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].explored == false && maze_ghosts[p->row][p->col+1][0].type != WALL){
				
				if(maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier){
					if((p->cost+1) < maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost){
						maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
						maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					}
				}
				else{
					maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].heuristic = abs(goal.row - maze_ghosts[p->row][p->col+1][0].row) + abs(goal.col - maze_ghosts[p->row][p->col+1][0].col);
					maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier = true;
					maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
					maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					pq.push(&maze_ghosts[p->row][p->col+1][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates]);
				}
			}
		}
		
		/* check if waiting at spot is enabled*/
		if (wait_enable) {
		
			/* try waiting at current spot */
			/* Don't wait if already visited that ghost state before at this pos*/
			if(maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].explored == false) {
				
				if(maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier){
					/* if already on the forntier and the path cost is less, update it */
					if((p->cost+1) < maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost){
						maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
						maze_ghosts[p->row-1][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					}
				}
				else {
					/* calculate the heuristic and add to frontier */
					maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].heuristic = abs(goal.row - maze_ghosts[p->row][p->col][0].row) + abs(goal.col - maze_ghosts[p->row][p->col][0].col);
					maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].on_frontier = true;
					maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].cost = p->cost+1;
					maze_ghosts[p->row][p->col][(p->cost + 1)%maze_ghosts_dimensions.ghost_possibleStates].prev = p;
					pq.push(&maze_ghosts[p->row][p->col][(p->cost +1)%maze_ghosts_dimensions.ghost_possibleStates]);
				}
			}
		}
	}

	/* mark the solution on the maze */
	while(p->prev != NULL){
		p->type = GOAL;
		p = p->prev;
	}
}

int main(){
	
	printf("\n\n\n\n*********TESTING BIG MAZE WITH SINGLE GHOST*********\n");
	/*test big ghost maze with ghosts*/
	printf("\n*****WITH ghosts and waiting ENABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/bigGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];

		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/bigGhost.txt");
	astar_search_with_ghosts(true);
	output_maze_with_ghosts("maze_ghost_outputs/big_with_ghost_wait_enable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	printf("\n*****WITH ghosts and waiting DISABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/bigGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/bigGhost.txt");
	astar_search_with_ghosts(false);
	output_maze_with_ghosts("maze_ghost_outputs/big_with_ghost_wait_disable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;

	/*test big ghost maze without ghosts*/
	printf("\n*****WITHOUT ghosts*****\n\n");
	get_maze_dimensions("maze_ghost_inputs/bigGhost.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	
	input_maze("maze_ghost_inputs/bigGhost.txt");
	astar_search();
	output_maze("maze_ghost_outputs/big_without_ghost.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;
	
	printf("\n\n\n\n*********TESTING BIG MAZE WITH MULTIPLE GHOSTS*********\n");
	/*test big ghost maze with ghosts*/
	printf("\n*****WITH ghosts and waiting ENABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/bigMultipleGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/bigMultipleGhost.txt");
	astar_search_with_ghosts(true);
	output_maze_with_ghosts("maze_ghost_outputs/big_with_multiple_ghost_wait_enable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	printf("\n*****WITH ghosts and waiting DISABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/bigMultipleGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/bigMultipleGhost.txt");
	astar_search_with_ghosts(false);
	output_maze_with_ghosts("maze_ghost_outputs/big_with_multiple_ghost_wait_disable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	/*test big ghost maze without ghosts*/
	printf("\n*****WITHOUT ghosts*****\n\n");
	get_maze_dimensions("maze_ghost_inputs/bigMultipleGhost.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	
	input_maze("maze_ghost_inputs/bigMultipleGhost.txt");
	astar_search();
	output_maze("maze_ghost_outputs/big_without_multiple_ghosts.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;
	
	printf("\n\n\n\n*********TESTING MEDIUM MAZE*********\n");
	/*test medium ghost maze with ghosts*/
	printf("\n*****WITH ghosts and waiting ENABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/mediumGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/mediumGhost.txt");
	astar_search_with_ghosts(true);
	output_maze_with_ghosts("maze_ghost_outputs/medium_with_ghost_wait_enable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	printf("\n*****WITH ghosts and waiting DISABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/mediumGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/mediumGhost.txt");
	astar_search_with_ghosts(false);
	output_maze_with_ghosts("maze_ghost_outputs/medium_with_ghost_wait_disable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	/*test medium ghost maze without ghosts*/
	printf("\n*****WITHOUT ghosts*****\n\n");
	get_maze_dimensions("maze_ghost_inputs/mediumGhost.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_ghost_inputs/mediumGhost.txt");
	astar_search();
	output_maze("maze_ghost_outputs/medium_without_ghost.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;
	
	printf("\n\n\n\n*********TESTING SMALL MAZE*********\n");
	/*test small ghost maze with ghosts*/
	printf("\n*****WITH ghosts and waiting ENABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/smallGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/smallGhost.txt");
	astar_search_with_ghosts(true);
	output_maze_with_ghosts("maze_ghost_outputs/small_with_ghost_wait_enable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	printf("\n*****WITH ghosts and waiting DISABLED*****\n\n");
	get_maze_dimensions_with_ghosts("maze_ghost_inputs/smallGhost.txt");
	maze_ghosts = new point**[maze_ghosts_dimensions.rows];
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		maze_ghosts[i] = new point*[maze_ghosts_dimensions.cols];
		for (int j = 0; j<maze_ghosts_dimensions.cols; j++) {
			maze_ghosts[i][j] = new point[maze_ghosts_dimensions.ghost_possibleStates];
			
		}
	}
	ghost_row_array = new int[number_of_ghosts];
	ghost_col_array = new int[number_of_ghosts];
	input_maze_with_ghosts("maze_ghost_inputs/smallGhost.txt");
	astar_search_with_ghosts(false);
	output_maze_with_ghosts("maze_ghost_outputs/small_with_ghost_wait_disable.txt");
	for(int i=0;i<maze_ghosts_dimensions.rows;i++){
		for (int j = 0; j < maze_ghosts_dimensions.cols; j++) {
			delete [] maze_ghosts[i][j];
		}
		delete [] maze_ghosts[i];
	}
	delete [] maze_ghosts;
	delete [] ghost_row_array;
	delete [] ghost_col_array;
	
	/*test small ghost maze without ghosts*/
	printf("\n*****WITHOUT ghosts*****\n\n");
	get_maze_dimensions("maze_ghost_inputs/smallGhost.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_ghost_inputs/smallGhost.txt");
	astar_search();
	output_maze("maze_ghost_outputs/small_without_ghost.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

  return 0;
}
