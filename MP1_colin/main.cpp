#include <iostream>
#include <fstream>
#include <string>
#include <queue>

using namespace std;

enum Type {WALL, OPEN, START, GOAL};

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


/* determine the dimensions of the maze */
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

/* take data from txt file and store it into an array */
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
				else if(line[cur_col] == ' '){
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

void greedy_search(){
	priority_queue<point*, vector<point*>, ComparePointGreedy> pq;

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

int main(){
	
	/* test medium maze with greedy */
	/* create array for maze */
	get_maze_dimensions("maze_inputs/medium_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	/* fill in array based on input maze file */
	input_maze("maze_inputs/medium_maze.txt");
	greedy_search();
	output_maze("maze_outputs/medium_maze_greedy.txt");
	/* free memory used for maze */
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

	/* test big maze with greedy */
	get_maze_dimensions("maze_inputs/big_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_inputs/big_maze.txt");
	greedy_search();
	output_maze("maze_outputs/big_maze_greedy.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

	/* test open maze with greedy */
	get_maze_dimensions("maze_inputs/open_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_inputs/open_maze.txt");
	greedy_search();
	output_maze("maze_outputs/open_maze_greedy.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;




	/* test medium maze with A star */
	get_maze_dimensions("maze_inputs/medium_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_inputs/medium_maze.txt");
	astar_search();
	output_maze("maze_outputs/medium_maze_astar.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

	/* test big maze with A star */
	get_maze_dimensions("maze_inputs/big_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_inputs/big_maze.txt");
	astar_search();
	output_maze("maze_outputs/big_maze_astar.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

	/* test open maze with A star */
	get_maze_dimensions("maze_inputs/open_maze.txt");
	maze = new point*[maze_dimensions.rows];
	for(int i=0;i<maze_dimensions.rows;i++){
		maze[i] = new point[maze_dimensions.cols];
	}
	input_maze("maze_inputs/open_maze.txt");
	astar_search();
	output_maze("maze_outputs/open_maze_astar.txt");
	for(int i=0;i<maze_dimensions.rows;i++){
		delete [] maze[i];
	}
	delete [] maze;

  return 0;
}
