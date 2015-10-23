#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

//used to hold list of all words for category
struct category{
	string name;
	vector<string> words;
};

//holds the category and positions of letters for word
struct puzzle_category{
	category group;
	int letters[3];
};

struct Puzzle{
	int length;
	vector<puzzle_category> parts;
};

struct arc{
	int start_part;
	int end_part;
};

Puzzle puzzle;
vector<arc> word_arcs;	//hold arcs of current puzzle
vector<arc> letter_arcs;	//hold arcs of current puzzle
vector<category> categories;	//hold all categories
char alphabet[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
int alphabet_size = 26;

void import_wordlist(string filename){
	ifstream file(filename);

	int row_count = 0;
	string line;
	if (file.is_open()){
		while (getline(file, line)){
			category c;
			int colon_pos = line.find(':');
			string category_name = line.substr(0,colon_pos);	//parse category name
			string all_words = line.substr(colon_pos + 2);		//parse string with list of words
			c.name = category_name;

			int num_of_words = (all_words.length() / 5) + 1;
			for (int i = 0; i < num_of_words; i++){
				int begin_word = i * 5;
				string new_word = all_words.substr(begin_word, 3);
				c.words.push_back(new_word);
			}
			categories.push_back(c);
		}
		file.close();
	}
}

void import_puzzle(string filename){
	ifstream file(filename);

	int row_count = 0;
	string line;
	if (file.is_open()){
		//get first number specifying length of puzzle
		getline(file, line);
		puzzle.length = atoi(line.c_str());

		//get categories
		while (getline(file, line)){
			puzzle_category p;
			int colon_pos = line.find(':');
			string category_name = line.substr(0, colon_pos);	//parse category name
			for (int i = 0; i < (int)categories.size(); i++){
				if (category_name.compare(categories[i].name) == 0){	//category matches
					p.group = categories[i];
					break;
				}
			}
			p.letters[0] = atoi(line.substr(colon_pos + 2, 1).c_str()) - 1;
			p.letters[1] = atoi(line.substr(colon_pos + 5, 1).c_str()) - 1;
			p.letters[2] = atoi(line.substr(colon_pos + 8, 1).c_str()) - 1;
			puzzle.parts.push_back(p);
		}
		file.close();
	}
}

// returns 1 if there is a solution, 0 if there isn't
bool verify_solution(string solution){
	for (int i = 0; i < (int)puzzle.parts.size(); i++){
		//construct 3 letter word based on requirement and current solution, word will contain '1' for letters not yet filled in
		string word = string(1,solution[puzzle.parts[i].letters[0]]) + string(1,solution[puzzle.parts[i].letters[1]]) + string(1,solution[puzzle.parts[i].letters[2]]);
		
		//go through all possible words in category looking for a match
		bool match_found = false;
		for (int j = 0; j < (int)puzzle.parts[i].group.words.size(); j++){
			string word_to_check = puzzle.parts[i].group.words[j];
			if (word[0] == '1' && word[1] == '1' && word[2] == '1'){	//no letters filled in, so anything works
				match_found = true;
				break;
			}
			else if (word[0] == '1' && word[1] == '1' && word[2] == word_to_check[2]){
				match_found = true;
				break;
			}
			else if (word[0] == '1' && word[1] == word_to_check[1] && word[2] == '1'){
				match_found = true;
				break;
			}
			else if (word[0] == word_to_check[0] && word[1] == '1' && word[2] == '1'){
				match_found = true;
				break;
			}
			else if (word[0] == word_to_check[0] && word[1] == word_to_check[1] && word[2] == '1'){
				match_found = true;
				break;
			}
			else if (word[0] == word_to_check[0] && word[1] == '1' && word[2] == word_to_check[2]){
				match_found = true;
				break;
			}
			else if (word[0] == '1' && word[1] == word_to_check[1] && word[2] == word_to_check[2]){
				match_found = true;
				break;
			}
			else if (word[0] == word_to_check[0] && word[1] == word_to_check[1] && word[2] == word_to_check[2]){
				match_found = true;
				break;
			}
		}
		if (!match_found){	//if no word in category list fits, there is no solution
			return false;
		}
	}
	//at least one word fits for each category so return true
	return true;
}

//fill in arcs based on current puzzle constraints for word based search
void generate_arcs_words(){
	for (int i = 0; i < (int)puzzle.parts.size(); i++){
		for (int j = i+1; j < (int)puzzle.parts.size(); j++){
			puzzle_category p1 = puzzle.parts[i];
			puzzle_category p2 = puzzle.parts[j];
			//check if any letters overlap, if so there is an arc between these two word constraints
			if (p1.letters[0] == p2.letters[0] || p1.letters[0] == p2.letters[1] || p1.letters[0] == p2.letters[2]
				|| p1.letters[1] == p2.letters[0] || p1.letters[1] == p2.letters[1] || p1.letters[1] == p2.letters[2]
				|| p1.letters[2] == p2.letters[0] || p1.letters[2] == p2.letters[1] || p1.letters[2] == p2.letters[2]){
				arc a1, a2;
				a1.start_part = i;
				a1.end_part = j;
				a2.start_part = j;
				a2.end_part = i;
				word_arcs.push_back(a1);
				word_arcs.push_back(a2);
			}
		}
	}
}

//fill in arcs based on current puzzle constraints for letter based search
void generate_arcs_letters(){
	for (int i = 0; i < (int)puzzle.parts.size(); i++){
		int letter0 = puzzle.parts[i].letters[0];
		int letter1 = puzzle.parts[i].letters[1];
		int letter2 = puzzle.parts[i].letters[2];
		bool found_01=false, found_12=false, found_02=false;
		//check for duplicates
		for (int j = 0; j < (int)letter_arcs.size();j++){
			if (letter_arcs[j].start_part == letter0 && letter_arcs[j].end_part == letter1){
				found_01 = true;
			}
			if (letter_arcs[j].start_part == letter1 && letter_arcs[j].end_part == letter2){
				found_12 = true;
			}
			if (letter_arcs[j].start_part == letter0 && letter_arcs[j].end_part == letter2){
				found_02 = true;
			}
		}

		//add arcs if not duplicates
		if (!found_01){
			arc a1, a2;
			a1.start_part = letter0;
			a1.end_part = letter1;
			a2.start_part = letter1;
			a2.end_part = letter0;
			letter_arcs.push_back(a1);
			letter_arcs.push_back(a2);
		}
		if (!found_12){
			arc a1, a2;
			a1.start_part = letter1;
			a1.end_part = letter2;
			a2.start_part = letter2;
			a2.end_part = letter1;
			letter_arcs.push_back(a1);
			letter_arcs.push_back(a2);
		}
		if (!found_02){
			arc a1, a2;
			a1.start_part = letter0;
			a1.end_part = letter2;
			a2.start_part = letter2;
			a2.end_part = letter0;
			letter_arcs.push_back(a1);
			letter_arcs.push_back(a2);
		}
	}
}

//check if two strings are equal with free characters being 1
bool string_equal(string s1, string s2){
	if (s1[0] == '1' && s1[1] == '1' && s1[2] == '1'){	//no letters filled in, so anything works
		return true;
	}
	else if (s1[0] == '1' && s1[1] == '1' && s1[2] == s2[2]){
		return true;
	}
	else if (s1[0] == '1' && s1[1] == s2[1] && s1[2] == '1'){
		return true;
	}
	else if (s1[0] == s2[0] && s1[1] == '1' && s1[2] == '1'){
		return true;;
	}
	else if (s1[0] == s2[0] && s1[1] == s2[1] && s1[2] == '1'){
		return true;
	}
	else if (s1[0] == s2[0] && s1[1] == '1' && s1[2] == s2[2]){
		return true;
	}
	else if (s1[0] == '1' && s1[1] == s2[1] && s1[2] == s2[2]){
		return true;
	}
	else if (s1[0] == s2[0] && s1[1] == s2[1] && s1[2] == s2[2]){
		return true;
	}
	return false;
}

vector<vector<string>> check_arcs_words(vector<vector<string>> possible_words, string solution){
	//construct queue of arcs
	queue<arc> q;
	for (int i = 0; i < (int)word_arcs.size(); i++){
		q.push(word_arcs[i]);
	}

	while (!q.empty()){
		arc a = q.front();
		q.pop();
		bool value_removed = false;
		//try every possible word for start of arc and make sure there is an allowed value for end of arc
		for (int i = 0; i < (int)possible_words[a.start_part].size(); i++){
			string test = solution;

			//fill in first word
			test[puzzle.parts[a.start_part].letters[0]] = possible_words[a.start_part][i][0];
			test[puzzle.parts[a.start_part].letters[1]] = possible_words[a.start_part][i][1];
			test[puzzle.parts[a.start_part].letters[2]] = possible_words[a.start_part][i][2];

			//check that second word is allowed
			string word2 = string(1, test[puzzle.parts[a.end_part].letters[0]]) + string(1, test[puzzle.parts[a.end_part].letters[1]]) + string(1, test[puzzle.parts[a.end_part].letters[2]]);
			bool found_match = false;
			for (int j = 0; j < (int)possible_words[a.end_part].size(); j++){
				if (string_equal(word2, possible_words[a.end_part][j])){
					found_match = true;
					break;
				}
			}
			if (!found_match){	//no possible choices for second word, remove from first list
				value_removed = true;
				possible_words[a.start_part].erase(possible_words[a.start_part].begin() + i);
				i--;	//everything shifts if it is taken out
			}
		}

		if (value_removed){
			//re add arcs to recheck
			for (int i = 0; i < (int)word_arcs.size(); i++){
				if (word_arcs[i].end_part == a.start_part){
					q.push(word_arcs[i]);
				}
			}
		}
	}
	return possible_words;
}


vector<vector<char>> check_arcs_letters(vector<vector<char>> possible_letters, string solution){
	//construct queue of arcs
	queue<arc> q;
	for (int i = 0; i < (int)letter_arcs.size(); i++){
		q.push(letter_arcs[i]);
	}

	while (!q.empty()){
		arc a = q.front();
		q.pop();
		bool value_removed = false;
		//try every possible letter for start of arc and make sure there is an allowed value for end of arc
		for (int i = 0; i < (int)possible_letters[a.start_part].size(); i++){
			string test = solution;

			//fill in test letters
			test[a.start_part] = possible_letters[a.start_part][i];

			//check that a second letter is allowed
			bool found_match = false;
			for (int j = 0; j < (int)possible_letters[a.end_part].size(); j++){
				test[a.end_part] = possible_letters[a.end_part][j];
				if (verify_solution(test)){
					found_match = true;
				}
			}
			if (!found_match){	//no possible choices for second letter, remove from first list
				value_removed = true;
				possible_letters[a.start_part].erase(possible_letters[a.start_part].begin() + i);
				i--;	//everything shifts if it is taken out
			}
		}

		if (value_removed){
			//re add arcs to recheck
			for (int i = 0; i < (int)letter_arcs.size(); i++){
				if (letter_arcs[i].end_part == a.start_part){
					q.push(letter_arcs[i]);
				}
			}
		}
	}
	return possible_letters;
}


//remove impossible words based on what is currently filled in the solution string
vector<vector<string>> remove_words(vector<vector<string>> possible_words, string solution){
	for (int i = 0; i < (int)possible_words.size(); i++){
		string word = string(1, solution[puzzle.parts[i].letters[0]]) + string(1, solution[puzzle.parts[i].letters[1]]) + string(1, solution[puzzle.parts[i].letters[2]]);
		for (int j = 0; j < (int)possible_words[i].size(); j++){
			if (!string_equal(word, possible_words[i][j])){
				possible_words[i].erase(possible_words[i].begin() + j);
				j--;
			}
		}
	}
	return possible_words;
}

//remove impossible letters based on what is currently filled in the solution string
vector<vector<char>> remove_letters(vector<vector<char>> possible_letters, string solution){
	for (int i = 0; i < (int)possible_letters.size(); i++){
		char letter = solution[i];
		for (int j = 0; j < (int)possible_letters[i].size(); j++){
			//check if letter is already filled in
			if (letter != '1' && letter != possible_letters[i][j]){
				possible_letters[i].erase(possible_letters[i].begin() + j);
				j--;
			}
			else{
				//check if solution can be valid
				string test = solution;
				test[i] = possible_letters[i][j];
				if (!verify_solution(test)){
					possible_letters[i].erase(possible_letters[i].begin() + j);
					j--;
				}
			}
		}
	}
	return possible_letters;
}

//create a list of possible letter choices, from possible words left
vector<char> find_possible_letters(vector<vector<string>> possible_words, int solution_pos){
	vector<char> list;
	for (int i = 0; i < (int)possible_words.size(); i++){
		int word_pos = -1;
		if (puzzle.parts[i].letters[0] == solution_pos){
			word_pos = 0;
		}
		else if (puzzle.parts[i].letters[1] == solution_pos){
			word_pos = 1;
		}
		else if (puzzle.parts[i].letters[2] == solution_pos){
			word_pos = 2;
		}

		if (word_pos >= 0){
			for (int j = 0; j < (int)possible_words[i].size(); j++){
				char c = possible_words[i][j][word_pos];
				bool found = false;
				for (int k = 0; k < (int)list.size(); k++){
					if (c == list[k]){
						found = true;
					}
				}
				if (!found){
					list.push_back(c);
				}
			}
		}
	}
	return list;
}

void solve_puzzle_word_based(string solution, int word_num, vector<vector<string>> possible_words){
	if (word_num == (int)puzzle.parts.size() && verify_solution(solution)){	//if all letters are filled in and solution is valid
		printf(" (Found Solution: %s)\n", solution.c_str());
	}
	else if (verify_solution(solution)){
		//remove impossible words then check arc consistency
		vector<vector<string>> possible_words2 = remove_words(possible_words, solution);
		vector<vector<string>> possible_words3 = check_arcs_words(possible_words2, solution);

		if ((int)possible_words3[word_num].size() == 0){
			printf("	BACKTRACK\n");
			return;
		}
		//try all remaining possible words for this constraint
		for (int i = 0; i < (int)possible_words3[word_num].size(); i++){
			printf(" -> %s", possible_words3[word_num][i].c_str());
			//assign and recurse
			solution[puzzle.parts[word_num].letters[0]] = possible_words3[word_num][i][0];
			solution[puzzle.parts[word_num].letters[1]] = possible_words3[word_num][i][1];
			solution[puzzle.parts[word_num].letters[2]] = possible_words3[word_num][i][2];
			solve_puzzle_word_based(solution, word_num + 1, possible_words3);
		}
	}
	else{
		printf("	BACKTRACK\n");
	}
}


void solve_puzzle_letter_based(string solution, int letter_num, vector<vector<char>> possible_letters){
	if (letter_num == puzzle.length && verify_solution(solution)){	//if all letters are filled in and solution is valid
		printf(" (Found Solution: %s)\n", solution.c_str());
	}
	else if (verify_solution(solution)){
		//remove impossible letters then check arc consistency
		vector<vector<char>> possible_letters2 = remove_letters(possible_letters, solution);
		vector<vector<char>> possible_letters3 = check_arcs_letters(possible_letters2, solution);
		
		if ((int)possible_letters3[letter_num].size() == 0){
			printf("	BACKTRACK\n");
			return;
		}
		//try all remaining possible words for this constraint
		for (int i = 0; i < (int)possible_letters3[letter_num].size(); i++){
			printf(" -> %c", possible_letters3[letter_num][i]);
			//assign and recurse
			solution[letter_num] = possible_letters3[letter_num][i];
			solve_puzzle_letter_based(solution, letter_num + 1, possible_letters3);
		}
	}
	else{
		printf("	BACKTRACK\n");
	}
}

void main(){
	import_wordlist("Inputs/WordList.txt");

	/*
	Puzzle1 - 4 solutions, NNEMANDYE, NNESAYDYE, NWEMANDYE, NWESAYDYE
	Puzzle2 - 4 solutions, HSIAIWNCS, HSIAIWNPS, HSIOIWNDS, HSIOIWNYS
	Puzzle3 - 2 solutions, ASULPEA, ASULPIE
	Puzzle4 - 3 solutions, HEDITYRE, HELITYRE, HETITYRE
	Puzzle5 - 4 solutions, IHTTNOIEN, IHTTYOIEN, THTTNOIEN, THTTYOIEN
	*/
	import_puzzle("Inputs/Puzzle5.txt");


	//word based assigment
	string solution = ""; 
	for (int i = 0; i < puzzle.length; i++){
		solution += '1';
	}
	generate_arcs_words();
	//fill in all possible words with all words from category
	vector<vector<string>> possible_words;
	for (int i = 0; i < (int)puzzle.parts.size(); i++){
		possible_words.push_back(puzzle.parts[i].group.words);
	}
	//do arc consistency on all initial values
	possible_words = remove_words(possible_words, solution);
	possible_words = check_arcs_words(possible_words, solution);
	printf("root");
	solve_puzzle_word_based(solution, 0, possible_words);


	//letter based assignment
	solution = "";
	for (int i = 0; i < puzzle.length; i++){
		solution += '1';
	}
	printf("\n\n");
	generate_arcs_letters();
	//fill in all possible letters with the entire alphabet to start
	vector<vector<char>> possible_letters;
	for (int i = 0; i < puzzle.length; i++){
		vector<char> a;
		for (int j = 0; j < alphabet_size; j++){
			a.push_back(alphabet[j]);
		}
		possible_letters.push_back(a);
	}
	//do arc consistency on all initial values
	possible_letters = remove_letters(possible_letters, solution);
	possible_letters = check_arcs_letters(possible_letters, solution);
	printf("root");
	solve_puzzle_letter_based(solution, 0, possible_letters);
}