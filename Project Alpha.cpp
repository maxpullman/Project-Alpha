// Project Alpha.cpp : Defines the entry point for the console application.
//

//Max Pullman
//February 16, 2017
//ME 493: Autonomy 

//Work completed over the last 6 days with a great deal of hours spent on self reviewing the code with miniature tweaks and collaborating with Honi Ahmadian

#include <iostream>
#include <time.h>
#include <random>
#include <fstream>
#include <assert.h>
#include <vector>
#include <cstdlib>  //Box-Muller library
#include <cmath>  //Box-Muller library
#include <limits>  //Box-Muller library
#include <stdexcept> //out_of_range errors

//Defines the randomness of mean, standarad deviation, and the probability for the action learner
#define MEAN 5+(double)rand()/RAND_MAX*95  //Makes sure the reward value of each arm can reach the mean of that arm by making the mean a value between 5-100
#define STD (double)rand()/RAND_MAX*2   //Sets a random standard deviation between 0-2
#define PROB (double)rand()/RAND_MAX

using namespace std; 


class bandit {
public:
	double mean;
	double std;
	double value;
	int pulls;

	void init();
	void setmean(); //sets the initial mean for the bandit
	void setstd();  //sets the initial standard deviation for the bandit
	double pull(); //pulls the arm once and returns a reward double
	void newvalue(double newval);
};

void bandit::init() {
	//Initializes the values of the bandit to have both a negative mean and standard deviation
	//Sets the value of the bandit at 0
	pulls = 0;
	value = 100;
	mean = -1.0;
	std = -1.0;
}

void bandit::setmean() {
	//ensures the number from MEAN is a double
	double m = MEAN;
	//Sets a mean for one particular instance of bandit
	mean = m;
}

void bandit::setstd() {
	//ensures the number from STD is a double variable
	double s = STD;
	//sets the standard deviation for one instance of bandit
	std = s;
}

//The Box-Muller Transform which generates values from the standard normal distribution with mean 0 and stdev 1.  
//The following function was grabbed from the Wikipedia page about Box-Muller transforms
double generateGaussianNoise(double mu, double sigma)
{
	const double epsilon = std::numeric_limits<double>::min();
	const double two_pi = 2.0*3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate)
		return z1 * sigma + mu;

	double u1, u2;
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}

double bandit::pull() {
	//sets the reward parameters
	double reward;

	//returns the value from the Box-Muller Transform
	reward = generateGaussianNoise(mean, std);

	//increases pulls
	pulls = pulls + 1;

	//return reward;
	return reward;
}

//Reward function for the action value learner
void bandit::newvalue(double newval) {
	double alpha = .1; //setting the learning priority to cover only 10 % of new data and 90 % old data
	value = newval*alpha + value*(1 - alpha);
}


//TestA using asserts to determine the average of many pulls from a single arm converges to that arms mean value
void TestA(bandit ARM) {

	ARM.value = 0;
	int runs = 1000; //setting the number of iterations

	ofstream fout;  //creating the function outfile for the learning curve
	fout.clear();   //clearing the file first
	fout.open("Learning Curve 1.txt");   //opening the learning curve text file for the data points

	double average_reward = 0;  //setting the average reward for a certain amount of runs
	double reward_value;  //The reward from each individual pull
	double new_value;  //The updated value from each seperate pull
	fout << "Reward:" << '\t' << "Runs:" << endl;

	for (int t = 1; t <= runs; t++) {

		//reads out the first reward at 0 and then iterates the average reward value leading up to the mean of that particular bandit arm (or close to it)
		fout << average_reward << '\t' << t - 1 << endl;   
		reward_value = ARM.pull();   //pulls the arm
		ARM.newvalue(reward_value);   //makes sure to update the reward value
		average_reward = ARM.value;  //grabbing the new value of the bandit arm
	}

	fout << "Mean: " << '\t' << "Std: " << endl;
	fout << ARM.mean << '\t' << ARM.std << endl;

	fout.close();

	int test;   //sets the assert test up
	// Spits out the correct assert it the average value of the reward from the pulls is relatively close to the mean of that arm (within a 5% range) 
	if (average_reward > .9*ARM.mean && average_reward < 1.1*ARM.mean) {  

		test = 1;
	}
	else {
		test = 0;
	}

	assert(test == 1);

	cout << "Test A successfully completed.  The average value reward from an arm converges to the mean after a set number of runs." << endl;

}

//TestB using asserts to test when one arm has a higher mean and standard deviation that the action learner has a very high likely hood of chooising that arm after many pulls
void TestB(vector<bandit> testMAB, int bandits) {
	
	ofstream TestBout;
	TestBout.clear();
	TestBout.open("Action Curve Test.txt");  //Sets up one trial for the action curve 

	TestBout << "Pulls:" << '\t';

	for (int a = 0; a < bandits+1; a++) {
		TestBout << "Arm" << a << ":" << '\t' << '\t' << '\t' << '\t';
	}

	TestBout << endl;

	for (int b = 0; b < bandits; b++) {
		testMAB.at(b).value = 100;
		testMAB.at(b).pulls = 0;
	}

	//Hard coded to create the best bandit arm and puts it in the MAB at the last poisition
	bandit best_arm;
	best_arm.init();
	best_arm.mean = 200;
	best_arm.std = 1;

	testMAB.push_back(best_arm);

	double total = 0; //Total number of pulls

	int sets = 1000; //The amount of iterations for the action learner
	double epsilon2 = .1;  //Setting the greedy variable again for the function
	double functionreward; //reward for inside the function

	int highestarm = 0; //makes the highest arm at the start of the vector
	
	//Action-Learner for Test B
	for (int k = 0; k < sets; k++) {
		if (PROB < epsilon2) { //will pull a random arm if the probability is under .1 (epsilon -> set above)
			int decision = rand() % bandits+1;  //random integer creator
			functionreward = testMAB.at(decision).pull();
			testMAB.at(decision).newvalue(functionreward);
			total++;
			TestBout << k << '\t';
			for (int i = 0; i < bandits; i++) {
				TestBout << testMAB.at(i).pulls / total << '\t' << '\t' << '\t' << '\t';
			}
			TestBout << endl;
		}
		else {
			//The procedure for how the pick the highest reward bandit given every bandits value (optomistic approach) 
			for (int l = 0; l < bandits+1; l++) {
				if (testMAB.at(l).value > testMAB.at(highestarm).value) {
					highestarm = l;
				}
			}
			functionreward = testMAB.at(highestarm).pull();
			testMAB.at(highestarm).newvalue(functionreward);
			total++;  //counts the number of pulls every time the loop continues
			TestBout << k << '\t';
			for (int i = 0; i < bandits+1; i++) {
				TestBout << testMAB.at(i).pulls / total << '\t' << '\t' << '\t' << '\t';
			}
			TestBout << endl;
		}
	}

	int testB = 0;  //Initial variable declared for the assert test

	//Determines if the create best bandit arm will be converge to the percent value 
	if (testMAB.at(bandits).pulls /total >= .95*((1 - epsilon2) + (epsilon2 / bandits)) && testMAB.at(bandits).pulls / total <= 1.05*((1 - epsilon2) + (epsilon2 / bandits))) {
		testB = 1;
	}
	
	//Assert test to see if the highest reward bandit will be picked the most times 
	assert (testB == 1);

	cout << "Test B completed and successful. The highest rewarded bandit arm was pulled the majority amount of times." << endl;
}


//Create a vector of bandits called MAB based on an input from the user (function?)
void MAB(vector<bandit> test) {
}


int main()
{
	srand(time(NULL));
	//Creating the variables for number of arms and basic program values
	int arms;
	double epsilon = .1; //Creating the greedy variable epsilon
	//The number on the arm the user wishes to pull
	int n;
	//the number associated with the user's specific arm pull
	double arm_reward;
	
	//Testing for a single reward using the class bandit
	/*bandit test;
	double get;
	test.setmean();
	test.setstd();
	get = test.pull();
	cout << get << endl;*/

	//Asks for the number of bandit arms the user wants to create
	cout << "How many bandit arms would you like to create? ";
	cin >> arms;

	//sets a vector of bandits dictated by the number of arms designated by the user
	vector<bandit> MAB;

	//goes through a for loop to create a vector of bandits equal to the user number
	for (int i = 0; i < arms; i++) {
		bandit test2;

		test2.init(); //initializes the bandit values
	    //sets a mean and standard deviation for each seperate bandit 
		test2.setmean(); 
		test2.setstd();

		MAB.push_back(test2);
	}

	int runs = 1000;
	int stat_runs = 30;
	int total_pulls = 0;
	int highestarm = 0; //makes the highest arm at the start of the vector
	
	//Code that dictated the text file for a new action curve (not used)
	//ofstream fout;
	//fout.clear();
	//fout.open("Action Curve.txt");  //Sets up one trial for the action curve 

	//fout << "Pulls:" << '\t';

	//for (int a = 0; a < arms ; a++) {
	//	fout << "Arm" << a << ":" << '\t' << '\t' << '\t';
	//}

	//
	//fout << endl;
	//fout.close();

	
	//Creating the action learner
	//for a certain number of runs the learner will decide to pull a random bandit or the highest value bandit
	for (int k = 0; k < runs; k++) {
		if (PROB < epsilon) { //will pull a random arm if the probability is under .1 (epsilon -> set above)
			int decision = rand() % arms;  //random integer creator
			arm_reward = MAB.at(decision).pull();
			MAB.at(decision).newvalue(arm_reward);
			total_pulls++;
						
		}
		else {
			for (int l = 0; l < arms; l++) {
				if (MAB.at(l).value > MAB.at(highestarm).value) {
					highestarm = l;
				}
			}
			arm_reward = MAB.at(highestarm).pull();
			MAB.at(highestarm).newvalue(arm_reward);
			total_pulls++;

		}
			//Code to test for number of pulls and the values being pulled
			/*for (int p = 0; p < arms; p++) {
				cout << MAB.at(p).value << '\t';
			}
			cout << endl;
			for (int p = 0; p < arms; p++) {
				cout << MAB.at(p).pulls << '\t';
			}
			cout << endl;*/
	}

	//Code to test what the mean was compared to the reward value the action learner was choosing
	/*for (int p = 0; p < arms; p++) {
		cout << MAB.at(p).mean << '\t';
	}
	cout << endl;*/

	//Runs TestA
	TestA(MAB.at(0));

	//Runs TestB
	TestB(MAB, arms);





	//User Pull code

	//making a loop counter for the user
	int counter = 0;

	//need a for loop to determine the amount of pulls the user wishes to complete
	do {
		cout << "Which arm would you like to pull? Please input an integer between 0 and " << arms-1 << "."; //asks for which arm to pull
		try {
			cin >> n; //reads the exact arm
			arm_reward = MAB.at(n).pull(); //outputs a reward
			cout << arm_reward << endl; 
		} 
		catch (const out_of_range& oor) {
			cerr << "Out of range error. Try again." << endl; //if user inputs arm larger than the vector outputs an error
		}
		
		counter++; //increases counter
		
	} while (counter <= arms);

	int counter2 = 0; //second loop counter
	int answer; //answer for user if he/she wants to continue
	do {
		cout << "Would you like to continue pulling? (1 = yes, anything else = no) "; //quick user response for playing the game
		cin >> answer;
		if (answer == 1) {  //if the answer is the 1 the user then chooses which arm they would like to pull again
			cout << "Which arm would you like to pull? Please input an integer between 0 and " << arms - 1 << ".";
			try {
				cin >> n;
				arm_reward = MAB.at(n).pull();
				cout << arm_reward << endl;
			}
			catch (const out_of_range& oor) {
				cerr << "Out of range error. Try again." << endl;
			}
		} 
		else {
			counter2 = 100; //sets counter to automatically close the loop
			cout << "Program will now close." << endl;
		}
	} while (counter2 < 100);

	//Has a system pause to make sure the program console stays open at the end if need be
	system("pause");
    return 0;
}

