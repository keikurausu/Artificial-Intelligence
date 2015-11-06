#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

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

double likelihoods0[10][IMAGE_DIM][IMAGE_DIM]; //used to store likelihoods of background feature
double likelihoods1[10][IMAGE_DIM][IMAGE_DIM]; //used to store likelihoods of foreground feature

double confusion_matrix[10][10];

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

void main(){
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
}