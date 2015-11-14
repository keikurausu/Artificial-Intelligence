#include <map>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

#define NUM_EMAIL_TRAINING 700
#define NUM_EMAIL_TEST 260
#define SPAM 1
#define NSPAM 0
#define NUM_MOVIE_TRAINING 4000
#define NUM_MOVIE_TEST 1000
#define POSITIVE_REVIEW 1
#define NEGATIVE_REVIEW -1
#define K 1 //Laplace smoothing constant

double const MoviePrior = log(0.5); //we are given the prior probability for the movie set
double SpamPrior; //find these from the email training docs
double NSpamPrior;
map<string, double> spam; //holds log of spam word probabilities
map<string, double> Nspam;
map<string, double> Positive;
map<string, double> Negative;
int EmailWords = 0; //number of unique words in email training docs
int MovieWords = 0;
int SpamWords = 0; //number of total spam words in spam doc
int NSpamWords = 0;
int NegativeWords = 0;
int PositiveWords = 0;
double numCorrect = 0; //number of correct classification
double SpamCount = 0; // count the number of spam labels
double NSpamCount = 0;

/*imports data from training documents and calculates multinomial likelihoods*/
void process_training_multinomial(string filename, int x)
{
	ifstream inFile(filename.c_str());
	if (inFile.is_open())
	{
		string line; //holds current line of document being processed
		size_t idx = 0; //holds current index in line
		string word;
		int number;
		int type; //label
		pair<map<string, double>::iterator, bool> ret;

		/*email case*/
		if (x == 0)
		{
			for (int i = 0; i < NUM_EMAIL_TRAINING; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				idx = 2;
				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					idx = end + 1;
					number = atoi((line.substr(idx, 3)).c_str());
					/*not spam case*/
					if (type == NSPAM)
					{
						NSpamCount += 1;
						ret = Nspam.insert(pair<string, double>(word, K + number)); //insert word into the dictionary with K value added to number
						if (ret.second == false)
						{
							ret.first->second += number; //if already existed just add appropriate value to current sum
						}
						else
						{
							EmailWords++;
							spam.insert(pair<string, double>(word, K)); //insert word into the spam dictionary with value K so we know we have seen it before
						}
						NSpamWords += number; //update total number of non spam words
					}
					/*spam case*/
					else if (type == SPAM)
					{
						SpamCount += 1; //increment spam count
						ret = spam.insert(pair<string, int>(word, K + number));
						if (ret.second == false)
						{
							ret.first->second += number;
						}
						else
						{
							EmailWords++;
							Nspam.insert(pair<string, int>(word, K));
						}
						SpamWords += number;
					}
					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
			}

			/*calculate prior probabilites*/
			SpamPrior = log(SpamCount / NUM_EMAIL_TRAINING);
			NSpamPrior = log(NSpamCount / NUM_EMAIL_TRAINING);

			/*calculate word probabilities*/
			map<string, double>::iterator it;
			for (it = spam.begin(); it != spam.end(); ++it)
			{
				it->second = log(it->second / (SpamWords + K*EmailWords)); //use log so we don't get underflow. K term is for laplace smoothing
			}
			for (it = Nspam.begin(); it != Nspam.end(); ++it)
			{
				it->second = log(it->second / (NSpamWords + K*EmailWords));
			}
		}

		/*movie review case*/
		else if (x == 1)
		{
			for (int i = 0; i < NUM_MOVIE_TRAINING; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				if (type == NEGATIVE_REVIEW)
					idx = 3; //account for - character
				else
					idx = 2;

				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					idx = end + 1;
					number = atoi((line.substr(idx, 2)).c_str());
					/*negative review case*/
					if (type == NEGATIVE_REVIEW)
					{
						ret = Negative.insert(pair<string, double>(word, K + number)); //insert word into the dictionary with K value added to number
						if (ret.second == false)
						{
							ret.first->second += number; //if already existed just add appropriate value to current sum
						}
						else
						{
							MovieWords++;
							Positive.insert(pair<string, double>(word, K)); //insert word into the positive dictionary with value K so we know we have seen it before
						}
						NegativeWords += number; //update total number of negative words
					}
					/*positive review case*/
					else if (type == POSITIVE_REVIEW)
					{
						ret = Positive.insert(pair<string, int>(word, K + number));
						if (ret.second == false)
						{
							ret.first->second += number;
						}
						else
						{
							MovieWords++;
							Negative.insert(pair<string, int>(word, K));
						}
						PositiveWords += number;
					}
					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
			}
			/*calculate word probabilities*/
			map<string, double>::iterator it;
			for (it = Negative.begin(); it != Negative.end(); ++it)
			{
				it->second = log(it->second / (NegativeWords + K*MovieWords)); //use log so we don't get underflow. K term is for laplace smoothing
			}
			for (it = Positive.begin(); it != Positive.end(); ++it)
			{
				it->second = log(it->second / (PositiveWords + K*MovieWords));
			}
		}
	}
	else
	{
		cout << "Error opening training doc" << endl;
	}
}

/*imports test data and estimates the conditional probabilities for multinomial*/
void test_multinomial(string filename, int x)
{
	ifstream inFile(filename.c_str());
	if (inFile.is_open())
	{
		string line; //holds current line of document being processed
		size_t idx = 0; //holds current index in line
		string word;
		int number;
		int type; //label

		/*email case*/
		if (x == 0)
		{
			numCorrect = 0;
			double spamProb;
			double NspamProb;
			for (int i = 0; i < NUM_EMAIL_TEST; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				idx = 2;
				spamProb = SpamPrior;
				NspamProb = NSpamPrior;
				//cout << type << endl;
				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					idx = end + 1;
					number = atoi((line.substr(idx, 3)).c_str());
					map<string, double>::iterator it;
					it = spam.find(word);
					if (it != spam.end())
					{
						spamProb += number*it->second;
					}
					it = Nspam.find(word);
					if (it != Nspam.end())
					{
						NspamProb += number*it->second;
					}

					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
				if (spamProb > NspamProb && type == SPAM)
				{
					numCorrect++; //we got it right!
				}
				else if (spamProb < NspamProb && type == NSPAM)
				{
					numCorrect++;
				}
			}
			cout << "Accuracy for email set " << 100 * numCorrect / NUM_EMAIL_TEST << "%" << endl; //classification accurracy
		}
		/*movie review case*/
		else if (x == 1)
		{
			numCorrect = 0;
			double positiveProb;
			double negativeProb;
			for (int i = 0; i < NUM_MOVIE_TEST; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				if (type == NEGATIVE_REVIEW)
					idx = 3;
				else
					idx = 2;

				positiveProb = MoviePrior;
				negativeProb = MoviePrior;
				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					idx = end + 1;
					number = atoi((line.substr(idx, 3)).c_str());
					map<string, double>::iterator it;
					it = Negative.find(word);
					if (it != Negative.end())
					{
						negativeProb += number*it->second;
					}
					it = Positive.find(word);
					if (it != Positive.end())
					{
						positiveProb += number*it->second;
					}

					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
				//cout << spamProb << endl;
				//cout << NspamProb << endl;
				if (negativeProb > positiveProb && type == NEGATIVE_REVIEW)
				{
					numCorrect++; //we got it right!
				}
				else if (negativeProb < positiveProb && type == POSITIVE_REVIEW)
				{
					numCorrect++;
				}
			}
			cout << "Accuracy for movie set " << 100 * numCorrect / NUM_MOVIE_TEST << "%" << endl; //classification accurracy
		}
	}
	else
	{
		cout << "Error opening test doc" << endl;
	}
}

/*imports data from training documents and calculates the Bernoulli likelihoods*/
void process_training_Bernoulli(string filename, int x)
{
	ifstream inFile(filename.c_str());
	if (inFile.is_open())
	{
		string line; //holds current line of document being processed
		size_t idx = 0; //holds current index in line
		string word;
		int number;
		int type; //label
		pair<map<string, double>::iterator, bool> ret;

		/*email case*/
		if (x == 0)
		{
			for (int i = 0; i < NUM_EMAIL_TRAINING; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				idx = 2;
				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					/*not spam case*/
					if (type == NSPAM)
					{
						ret = Nspam.insert(pair<string, double>(word, K + 1)); //insert word into the dictionary with K value added to number
						if (ret.second == false)
						{
							ret.first->second += 1; //if already existed just add 1
						}
						else
						{
							spam.insert(pair<string, double>(word, K)); //insert word into the spam dictionary with value K so we know we have seen it before
						}
					}
					/*spam case*/
					else if (type == SPAM)
					{
						ret = spam.insert(pair<string, int>(word, K + 1));
						if (ret.second == false)
						{
							ret.first->second += 1;
						}
						else
						{
							Nspam.insert(pair<string, int>(word, K));
						}
					}
					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
			}
			/*calculate word probabilities*/
			map<string, double>::iterator it;
			for (it = spam.begin(); it != spam.end(); ++it)
			{
				it->second = log(it->second / (SpamCount + K)); //use log so we don't get underflow. K term is for laplace smoothing
			}
			for (it = Nspam.begin(); it != Nspam.end(); ++it)
			{
				it->second = log(it->second / (NSpamCount + K));
			}
		}

		/*movie review case*/
		else if (x == 1)
		{
			for (int i = 0; i < NUM_MOVIE_TRAINING; i++)
			{
				getline(inFile, line);
				size_t end;
				type = atoi(line.c_str()); //get label
				if (type == NEGATIVE_REVIEW)
					idx = 3; //account for - character
				else
					idx = 2;

				while (1)
				{
					end = line.find(":", idx);
					word = line.substr(idx, end - idx);
					/*negative review case*/
					if (type == NEGATIVE_REVIEW)
					{
						ret = Negative.insert(pair<string, double>(word, K + 1)); //insert word into the dictionary with K value added to number
						if (ret.second == false)
						{
							ret.first->second += 1; //if already existed just add appropriate value to current sum
						}
						else
						{
							Positive.insert(pair<string, double>(word, K)); //insert word into the positive dictionary with value K so we know we have seen it before
						}
					}
					/*positive review case*/
					else if (type == POSITIVE_REVIEW)
					{
						ret = Positive.insert(pair<string, int>(word, K + 1));
						if (ret.second == false)
						{
							ret.first->second += 1;
						}
						else
						{
							Negative.insert(pair<string, int>(word, K));
						}
					}
					idx = line.find(" ", idx);
					if (idx == string::npos)
						break;
					idx++;
				}
			}
			/*calculate word probabilities*/
			map<string, double>::iterator it;
			for (it = Negative.begin(); it != Negative.end(); ++it)
			{
				it->second = log(it->second / (NUM_MOVIE_TRAINING/2 + K)); //use log so we don't get underflow. K term is for laplace smoothing
			}
			for (it = Positive.begin(); it != Positive.end(); ++it)
			{
				it->second = log(it->second / (NUM_MOVIE_TRAINING / 2 + K));
			}
		}
	}
	else
	{
		cout << "Error opening training doc" << endl;
	}
}

/*prints the 20 most likely words from each case*/
void print_best_20(int x)
{
	if (x == 0)
	{
		map<string, double>::iterator it;
		string max;
		double max_value;
		cout << "Most likely 20 words in spam class are from most likely to less likely:" << endl;
		for (int i = 0; i < 20; i++)
		{
			it = spam.begin();
			max = it->first;
			max_value = it->second;
			for (; it != spam.end(); ++it)
			{
				if (it->second > max_value)
				{
					max_value = it->second;
					max = it->first;
				}
			}
			cout << max << ", ";
			spam.erase(max);
		}
		cout << endl << "Most likely 20 words in NON spam class are from most likely to less likely:" << endl;
		for (int i = 0; i < 20; i++)
		{
			it = Nspam.begin();
			max = it->first;
			max_value = it->second;
			for (; it != Nspam.end(); ++it)
			{
				if (it->second > max_value)
				{
					max_value = it->second;
					max = it->first;
				}
			}
			cout << max << ", ";
			Nspam.erase(max);
		}
		cout << endl;
		//free up memory
		spam.clear();
		Nspam.clear();
	}
	else if (x == 1)
	{
		map<string, double>::iterator it;
		string max;
		double max_value;
		cout << "Most likely 20 words in Negative review class are from most likely to less likely:" << endl;
		for (int i = 0; i < 20; i++)
		{
			it = Negative.begin();
			max = it->first;
			max_value = it->second;
			for (; it != Negative.end(); ++it)
			{
				if (it->second > max_value)
				{
					max_value = it->second;
					max = it->first;
				}
			}
			cout << max << ", ";
			Negative.erase(max);
		}
		cout << endl << "Most likely 20 words in Positive review class are from most likely to less likely:" << endl;
		for (int i = 0; i < 20; i++)
		{
			it = Positive.begin();
			max = it->first;
			max_value = it->second;
			for (; it != Positive.end(); ++it)
			{
				if (it->second > max_value)
				{
					max_value = it->second;
					max = it->first;
				}
			}
			cout << max << ", ";
			Positive.erase(max);
		}
		cout << endl;
		//free up memory
		Negative.clear();
		Positive.clear();
	}
}


int main()
{
	/*multinomial email datasets*/
	cout << "Multinomial email case:" << endl;
	process_training_multinomial("train_email.txt", 0);
	test_multinomial("test_email.txt", 0);
	print_best_20(0);

	/*multinomial movie datasets*/
	cout << "Multinomial movie case:" << endl;
	process_training_multinomial("rt-train.txt", 1);
	test_multinomial("rt-test.txt", 1);
	print_best_20(1);

	/*Bernoulli email datasets*/
	cout << "Bernoulli email case:" << endl;
	process_training_Bernoulli("train_email.txt", 0);
	test_multinomial("test_email.txt", 0);
	print_best_20(0);

	/*Bernoulli movie datasets*/
	cout << "Bernoulli movie case:" << endl;
	process_training_Bernoulli("rt-train.txt", 1);
	test_multinomial("rt-test.txt", 1);
	print_best_20(1);

	return 0;
}

