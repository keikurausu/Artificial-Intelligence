#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <math.h>

using namespace std;

#define NUM_TRAINING_LABELS 5000
#define NUM_TEST_LABELS 1000
#define IMAGE_DIM 28
#define K 1
//K=0 gives 75.7% accuracy
//K=1 gives 77.1% accuracy
//K=2 gives 76.6% accuracy


int training_labels[NUM_TRAINING_LABELS];	//used to store 5000 labels for training set
int test_labels[NUM_TEST_LABELS];		
int estimated_test_labels[NUM_TEST_LABELS];

int digit_count[10];	//used to count how many of each digit class in training set
double digit_probability[10];

//first dimension for digit class, second(rows) and third(cols) for pixel location
int num_features0[10][IMAGE_DIM][IMAGE_DIM]; //used to store number of background features in this position from training set
int num_features1[10][IMAGE_DIM][IMAGE_DIM];
double odds_ratio[IMAGE_DIM][IMAGE_DIM];

double likelihoods0[10][IMAGE_DIM][IMAGE_DIM]; //used to store likelihoods of background feature
double likelihoods1[10][IMAGE_DIM][IMAGE_DIM]; //used to store likelihoods of foreground feature

double confusion_matrix[10][10];

double max_posterior_prob[10]; //keep track of max/min posterior probability for each digit
double min_posterior_prob[10];
int max_posterior_prob_index[10]; //keep track of index of max/min posterior probability for each digit
int min_posterior_prob_index[10];

void import_traininglabels(string filename){
	ifstream file(filename);
	int row_count = 0;
	int digit = 0;
	string line;
	if (file.is_open()){
		while (getline(file, line)){
			digit = atoi(line.substr(0, 1).c_str());

			//increment digit count and store label
			digit_count[digit]++;
			training_labels[row_count] = digit;
			row_count++;
		}
		file.close();
	}

	for (int i = 0; i < 10; i++){
		digit_probability[i] = (double) digit_count[i] / NUM_TRAINING_LABELS;
	}
}

void import_testlabels(string filename){
	ifstream file(filename);
	int row_count = 0;
	int digit = 0;
	string line;
	if (file.is_open()){
		while (getline(file, line)){
			digit = atoi(line.substr(0, 1).c_str());

			//store label
			test_labels[row_count] = digit;
			row_count++;
		}
		file.close();
	}
}

void import_trainingdata(string filename){
	ifstream file(filename);
	int row_count = 0;
	int i,j,k = 0;
	int digit = 0;
	string line;
	if (file.is_open()){
		//go through all 5000 digits
		for (i = 0; i < NUM_TRAINING_LABELS; i++){
			digit = training_labels[i];
			for (j = 0; j < IMAGE_DIM; j++){	//go through all 28 rows
				getline(file, line);
				for (k = 0; k < IMAGE_DIM; k++){	//go through all 28 columns
					char c = line[k];
					if (c == '#' || c == '+'){	//pixel part of foreground
						num_features1[digit][j][k]++;
					}
					else{	//pixel part of background
						num_features0[digit][j][k]++;
					}
				}

			}
		}
		file.close();
	}
}

void compute_likelihoods(){
	int i, j, k = 0;
	for (i = 0; i < 10; i++){
		for (j = 0; j < IMAGE_DIM; j++){
			for (k = 0; k < IMAGE_DIM; k++){
				likelihoods0[i][j][k] = (double)(num_features0[i][j][k] + K) / (digit_count[i] + 2 * K);
				likelihoods1[i][j][k] = (double)(num_features1[i][j][k] + K) / (digit_count[i] + 2 * K);
			}
		}
	}
}


void import_testdata(string filename){
	ifstream file(filename);
	int row_count = 0;
	int i, j, k, cur_digit = 0;
	double probabilities[10];	//hold probabilities for each digit class
	double max_prob = 0.0;
	int max_digit = 0;
	string line;
	if (file.is_open()){
		//go through all 1000 images
		for (i = 0; i < NUM_TEST_LABELS; i++){

			//reset probabilities
			max_prob = -100000000000000.0;
			max_digit = 0;

			for (cur_digit = 0; cur_digit < 10; cur_digit++){
				probabilities[cur_digit] = log(digit_probability[cur_digit]);
			}

			for (j = 0; j < IMAGE_DIM; j++){	//go through all 28 rows
				getline(file, line);
				for (k = 0; k < IMAGE_DIM; k++){	//go through all 28 columns
					char c = line[k];
					for (cur_digit = 0; cur_digit < 10; cur_digit++){	//update probability for each digit class
						if (c == '#' || c == '+'){	//pixel part of foreground
							if (likelihoods1[cur_digit][j][k] != 0){	//log(0) is undefined and causes problems, so just dont add it
								probabilities[cur_digit] += log(likelihoods1[cur_digit][j][k]);
							}
						}
						else{	//pixel part of background
							if (likelihoods0[cur_digit][j][k] != 0){
								probabilities[cur_digit] += log(likelihoods0[cur_digit][j][k]);
							}
						}
					}
				}
			}

			//find max probability
			for (cur_digit = 0; cur_digit < 10; cur_digit++){
				if (probabilities[cur_digit] > max_prob){
					max_prob = probabilities[cur_digit];
					max_digit = cur_digit;
				}
			}
			estimated_test_labels[i] = max_digit;


			int actual_digit = test_labels[i];
			int digit_prob = probabilities[actual_digit];
			//keep track of max/min posterior prob for each digit
			if (digit_prob > max_posterior_prob[actual_digit]){
				max_posterior_prob[actual_digit] = digit_prob;
				max_posterior_prob_index[actual_digit] = i;
			}
			if (digit_prob < min_posterior_prob[actual_digit]){
				min_posterior_prob[actual_digit] = digit_prob;
				min_posterior_prob_index[actual_digit] = i;
			}
		}
		file.close();
	}
}

//returns percent accuracy
double calculate_overall_accuracy(){
	int number_correct = 0;
	for (int i = 0; i < NUM_TEST_LABELS; i++){
		if (estimated_test_labels[i] == test_labels[i]){
			number_correct++;
		}
	}
	return (double)number_correct / NUM_TEST_LABELS * 100.0;
}

void compute_confusion_matrix(){
	//start by counting how many classifies for each class
	for (int i = 0; i < NUM_TEST_LABELS; i++){
		confusion_matrix[test_labels[i]][estimated_test_labels[i]]++;
	}

	//convert counts to percentages
	int total = 0;
	for (int i = 0; i < 10; i++){
		total = 0;
		//for each row count all columns
		for (int j = 0; j < 10; j++){
			total += confusion_matrix[i][j];
		}

		//convert
		for (int j = 0; j < 10; j++){
			confusion_matrix[i][j] = confusion_matrix[i][j] / total;
		}
	}
}

void output_odds_ratio_map(int c1, int c2, string filename) {
	
	ofstream file(filename);

	file.is_open();
	
	file << "LIKELIHOOD MAP OF "<< c1 << endl;
	
	for (int i = 0; i < IMAGE_DIM; i++) {
		for (int j = 0; j <IMAGE_DIM; j++) {
			double probability = likelihoods1[c1][i][j];
			if (probability < 0.2) {
				file << ' ';
			} else if (probability < 0.5) {
				file << '-';
			} else {
				file << '+';
			}
		}
		file << '\n';
	}
	
	file << "LIKELIHOOD MAP OF "<< c2 << endl;
	
	for (int i = 0; i < IMAGE_DIM; i++) {
		for (int j = 0; j <IMAGE_DIM; j++) {
			double probability = likelihoods1[c2][i][j];
			if (probability < 0.2) {
				file << ' ';
			} else if (probability < 0.5) {
				file << '-';
			} else {
				file << '+';
			}
		}
		file << '\n';
	}
	
	file << "ODDS-RATIO MAP BETWEEN "<< c1 << " AND " << c2 << endl << endl<<endl;

	for (int i=0;i<IMAGE_DIM;i++) {
		for(int j=0;j<IMAGE_DIM;j++) {
			if (odds_ratio[i][j] <= -0.25) {
				file << '-';
			} else if ((odds_ratio[i][j] > 0.28)) {
				file << ' ';
			} else {
				file << '+';
			}
		}
		file << '\n';
	}
	
	file.close();
	return;
}

void calculate_odds_ratios (int c1, int c2, string filename) {
	for (int i = 0; i < IMAGE_DIM; i++) {
		for (int j = 0; j <IMAGE_DIM; j++) {
			odds_ratio[i][j] = (likelihoods1[c1][i][j])/likelihoods1[c2][i][j];
			odds_ratio[i][j] = log(odds_ratio[i][j]);
		}
	}
	
	output_odds_ratio_map(c1, c2, filename);
}

int main(){
	//clear out arrays
	for (int i = 0; i < 10; i++){
		max_posterior_prob[i] = (double)(INT_MIN);
		min_posterior_prob[i] = (double)(INT_MAX);
		max_posterior_prob_index[i] = 0;
		min_posterior_prob_index[i] = 0;
	}

	import_traininglabels("Inputs/traininglabels");
	import_trainingdata("Inputs/trainingimages");
	compute_likelihoods();
	import_testlabels("Inputs/testlabels");
	import_testdata("Inputs/testimages");

	double accuracy = calculate_overall_accuracy();
	printf("Overall accuracy: %f percent\n", accuracy);
	
	compute_confusion_matrix();

	//print out classification rate of each digit
	for (int i = 0; i < 10; i++){
		printf("%f percentage of test images of digit %d correctly classified\n", confusion_matrix[i][i], i);
	}

	//print out confusion matrix
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 10; j++){
			printf("%f ", confusion_matrix[i][j]);
		}
		printf("\n");
	}
	
	for (int i = 0; i < 10; i++){
		printf("Digit: %d\n", i);
		printf("Max posterior prob: %f\n", max_posterior_prob[i]);
		printf("Max posterior prob index: %d\n", max_posterior_prob_index[i]);
		printf("Min posterior prob: %f\n", min_posterior_prob[i]);
		printf("Min posterior prob index: %d\n", min_posterior_prob_index[i]);
	}
	
	float diff1 = 100000000.0;
	int diff1_x = 0;
	int diff1_y = 0;
	
	float diff2 = 100000000.0;
	int diff2_x = 0;
	int diff2_y = 0;
	
	float diff3 = 100000000.0;
	int diff3_x = 0;
	int diff3_y = 0;
	
	float diff4 = 100000000.0;
	int diff4_x = 0;
	int diff4_y = 0;
	
	for (int i = 0; i <  10; i++) {
		for (int j = 0; j < 10; j++) {
			if (i == j)
				continue;
			
			float diff = confusion_matrix[i][i]/confusion_matrix[i][j];
			
			if (diff < diff1) {
				diff4 = diff3;
				diff4_x = diff3_x;
				diff4_y = diff3_y;
				
				diff3 = diff2;
				diff3_x = diff2_x;
				diff3_y = diff2_y;
				
				diff2 = diff1;
				diff2_x = diff1_x;
				diff2_y = diff1_y;
				
				diff1 = diff;
				diff1_x = i;
				diff1_y = j;
				
			} else if (diff < diff2) {
				diff4 = diff3;
				diff4_x = diff3_x;
				diff4_y = diff3_y;
				
				diff3 = diff2;
				diff3_x = diff2_x;
				diff3_y = diff2_y;
				
				diff2 = diff;
				diff2_x = i;
				diff2_y = j;
				
			} else if (diff < diff3) {
				diff4 = diff3;
				diff4_x = diff3_x;
				diff4_y = diff3_y;
				
				diff3 = diff;
				diff3_x = i;
				diff3_y = j;
				
			} else if (diff < diff4) {
				diff4 = diff;
				diff4_x = i;
				diff4_y = j;
				
			}
		}
	}
	
	printf("The most confused digits are:\n\t%d and %d (ratio in percentages is %f)\n\t%d and %d (ratio in percentages is %f)\n\t%d and %d (ratio in percentages is %f)\n\t%d and %d (ratio in percentages is %f)\n", diff4_x, diff4_y, diff4, diff3_x, diff3_y, diff3, diff2_x, diff2_y, diff2, diff1_x, diff1_y, diff1);

	calculate_odds_ratios(1, 8, "Map0.txt");
	calculate_odds_ratios(diff1_x, diff1_y, "Map1.txt");
	calculate_odds_ratios(diff2_x, diff2_y, "Map2.txt");
	calculate_odds_ratios(diff3_x, diff3_y, "Map3.txt");
	calculate_odds_ratios(diff4_x, diff4_y, "Map4.txt");
	
	return 0;

}