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

int training_labels[NUM_TRAINING_LABELS];	//used to store 5000 labels for training set
int test_labels[NUM_TEST_LABELS];		
int estimated_test_labels[NUM_TEST_LABELS];
double weight_vectors[10][IMAGE_DIM][IMAGE_DIM];
double current_digit[IMAGE_DIM][IMAGE_DIM];
double weight_bias[10];
int digits_accessed[NUM_TRAINING_LABELS];

double epoch = 0;
double alpha = 1;
double accuracy = 0;
double total_incorrect = 0;

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
			training_labels[row_count] = digit;
			row_count++;
		}
		file.close();
	}
}

void classify(int * predicted_digit) {
	long int max_likelihood = 0;
	for (int i = 0; i < 10; i++) { // Go through all ten weight vectors to compute likelihood
		long int current_digit_likelihood = 0;
		for (int j = 0; j < IMAGE_DIM; j++) {
			for (int k = 0; k <IMAGE_DIM; k++) {
				current_digit_likelihood = current_digit_likelihood + weight_vectors[i][j][k]*current_digit[j][k];
			}
		} //output for the current digit likelihood has been calculated

		current_digit_likelihood = current_digit_likelihood + weight_bias[i]*1;
		
		if (current_digit_likelihood > max_likelihood) {
			max_likelihood = current_digit_likelihood;
			*predicted_digit = i;
		}
	}
}

void update_weights(int predicted_digit, int actual_digit) {
	for (int i = 0; i < IMAGE_DIM; i++) {
		for (int j = 0; j <IMAGE_DIM; j++) {
			weight_vectors[actual_digit][i][j] = weight_vectors[actual_digit][i][j] + alpha*current_digit[i][j];
			weight_vectors[predicted_digit][i][j] = weight_vectors[predicted_digit][i][j] - alpha*current_digit[i][j];
		}
		weight_bias[actual_digit] = weight_bias[actual_digit] + alpha*1;
		weight_bias[predicted_digit] = weight_bias[predicted_digit] - alpha*1;
	}
}

void process_trainingdata(string filename){
	ifstream file(filename);
	int predicted_digit = 0;
	string line;
	double incorrect = 0;
	
	if (file.is_open()){
		
		while (epoch < 150 && accuracy != 100.0) {
			incorrect = 0;
			
			for (int i =0; i <NUM_TRAINING_LABELS; i++) {
				digits_accessed[i] = 0;
			}
			//go through all 5000 digits
			for (int i = 0; i < NUM_TRAINING_LABELS; i++){
				
				int random_digit_index = rand() % 5001;
				
				while (digits_accessed[random_digit_index] == 1) {
					random_digit_index = rand() % 5001;
				}
				
				int actual_digit = training_labels[random_digit_index];
				digits_accessed[random_digit_index] = 1;
				
				file.seekg(29*28*random_digit_index, ios::beg);

				for (int j = 0; j < IMAGE_DIM; j++){	//go through all 28 rows
					getline(file, line);
					for (int k = 0; k < IMAGE_DIM; k++){	//go through all 28 columns
						char c = line[k];
						if (c == '#' || c == '+'){	//pixel part of foreground
							current_digit[j][k] = 1;
						}
						else{	//pixel part of background
							current_digit[j][k] = 0;
						}
					}
					
				} // Current Image has been imported at this point
				
				classify(&predicted_digit);
				
				if (predicted_digit != actual_digit) {
					incorrect++;
					total_incorrect++;
					update_weights(predicted_digit, actual_digit);
				}
				
				file.clear();
				file.seekg(0, ios::beg);
			}
			accuracy = 100.0 - (incorrect*100.0/NUM_TRAINING_LABELS);
			printf("Epoch %.0f, Accuracy Percentage: %.2f\n", epoch, accuracy);
			
			//Go back to beginning of file
			file.clear();
			file.seekg(0, ios::beg);
			
			epoch++;
			alpha = 1000/(epoch+1000);
		}
		
		printf("Total Accuracy Percentage: %.2f\n", (100.0 - (total_incorrect*100.0/(NUM_TRAINING_LABELS*epoch))));
		
	}
	file.close();
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

void process_testdata(string filename) {
	ifstream file(filename);
	int predicted_digit = 0;
	string line;
	double incorrect = 0;
	
	if (file.is_open()) {
		for (int i = 0; i < NUM_TEST_LABELS; i++){
			for (int j = 0; j < IMAGE_DIM; j++){	//go through all 28 rows
				getline(file, line);
				for (int k = 0; k < IMAGE_DIM; k++){	//go through all 28 columns
					char c = line[k];
					if (c == '#' || c == '+'){	//pixel part of foreground
						current_digit[j][k] = 1;
					}
					else{	//pixel part of background
						current_digit[j][k] = 0;
					}
				}
				
			} // Current Image has been imported at this point
			
			classify(&predicted_digit);
			estimated_test_labels[i] = predicted_digit;

			int actual_digit = test_labels[i];
			
			if (predicted_digit != actual_digit) {
				incorrect++;
			}
		}
	}
	printf("Total Accuracy Percentage on Test Data: %.2f\n", (100.0 - (incorrect*100.0/(NUM_TEST_LABELS))));
	file.close();
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


int main(){
	srand (time(NULL));

	//set initial weight wectors for each class
	for (int i = 0; i < 10; i++){
		weight_bias[i] = -1.0 + ((double)rand() / RAND_MAX)*2.0;
		for (int j = 0; j <IMAGE_DIM; j++) {
			for (int k = 0; k <IMAGE_DIM; k++) {
				weight_vectors[i][j][k] = -1.0 + ((double)rand() / RAND_MAX)*2.0;
			}
		}
	}

	import_traininglabels("Inputs/traininglabels");
	process_trainingdata("Inputs/trainingimages");
	import_testlabels("Inputs/testlabels");
	process_testdata("Inputs/testimages");
	
	compute_confusion_matrix();

	//print out classification rate of each digit
	for (int i = 0; i < 10; i++){
		printf("%.2f percentage of test images of digit %d correctly classified\n", confusion_matrix[i][i]*100, i);
	}
	
	//print out confusion matrix
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 10; j++){
			printf("%.2f ", confusion_matrix[i][j]*100);
		}
		printf("\n");
	}
	
	return 0;

}