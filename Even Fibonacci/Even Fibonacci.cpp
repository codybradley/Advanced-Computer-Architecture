// Bradley, Cody        CS230 Section 12698  6/17/20
// Fifth Laboratory Assignment - Even Fibonacci

// Solution Platform must be set to x64 (default is x86, found on toolbar)

#include<iostream>

using namespace std;

extern "C" unsigned long long Euler2(unsigned long long n);
void clearInput();

#define MAX_LIMIT unsigned int 100;

int main() {
	unsigned int limitingNum = 101;
	bool validLimit;
	unsigned long long sumN, totalSum = 0;
	
	cout << "CS230 Spring 2020 Section - Lab 5 - Bradley, Cody" << endl;//message 1

	do {
		validLimit = true;
		cout << "\nEnter the count of Fibonacci numbers to evaluate" << endl;//message 2
		cin >> limitingNum;
		if (cin.fail() || limitingNum < 1 || limitingNum>100) {
			clearInput();
			cout << "\nInput invalid, must be between 1 and 100" << endl;//message 3
			validLimit = false;
		}
	} while (!validLimit);

	for (unsigned int n = 1; n <= limitingNum; ++n) {
		sumN = Euler2(n);
		if (sumN == 0) {
			cout << "\nFibonacci number is too large to calculate, ending." << endl;//message 5
			exit(1);
		}
		if (!(sumN & 1))
			totalSum += sumN;
	}

	cout << "\nSum of even Fibonacci numbers is " << totalSum << endl;//message 4
	return 0;
}

void clearInput() {
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}