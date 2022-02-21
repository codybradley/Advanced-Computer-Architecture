//CS 230 Section 12698, Lab Assignment 2, Bradley Cody

#include<iostream>
#include<fstream>
#include<string>
#include<iomanip>
using namespace std;

void readInData(string stockNames[], float stockPrices[]);//open file, read into arrays, close file
void showStocks(string stockNames[], float stockPrices[]);//extra function for testing
void clearInput();//clears input buffer and flags
bool isUpperStr(string s);//returns true if entire string is upper case or it is an empty string
int findStockIndex(string stockNames[], string userStockName);//returns the index of the stock matching the user's stock, or -1 if not found
void updatePrice(float stockPrices[], int stockIndex, float userStockPrice);//puts userStockPrice into array stockPrices[] at index stockIndex
float calcAverage(float stockPrices[]);//calculates new average in assembly language and returns result
void outputAverage(float dowAvg);//outputs message 3 with the passed average

const int TOTALSTOCKS = 30;
const double DOWDIVISOR = 0.14748071991788;

int main() {
	string stockNames[TOTALSTOCKS], userStockName;
	float stockPrices[TOTALSTOCKS], userStockPrice;
	char userContinue;
	int stockIndex;
	readInData(stockNames, stockPrices);
	outputAverage(calcAverage(stockPrices));
	do {
		cout << "\nCalculate new value of average?    Reply 'Y' or 'N'.\n" << endl;//message 4
		cin >> userContinue;
		if (userContinue == 'Y' || userContinue == 'y') {
			cout << "\nEnter a new stock symbol and value\n" << endl;//message 7
			cin >> userStockName >> userStockPrice;
			if (!isUpperStr(userStockName) || userStockName.length() < 1 || userStockName.length() > 5) {
				cout << "\nInvalid stock symbol " << userStockName << ", must be between one and five characters long and all upper case " << endl;//message 8
				clearInput();
			}
			else {
				stockIndex = findStockIndex(stockNames, userStockName);
				if (stockIndex == -1) {
					cout << "\nStock symbol " << userStockName << " was not found, re-enter" << endl;//message 9
					clearInput();
				}
				else if (cin.fail() || userStockPrice <= 0.0) {
					cout << fixed << showpoint << setprecision(2) << "\nStock price " << userStockPrice << " is not valid, re-enter" << endl;//message 10
					clearInput();
				}
				else {//user's stock symbol is in the array and user's price is positive
					updatePrice(stockPrices, stockIndex, userStockPrice);
					outputAverage(calcAverage(stockPrices));
				}
			}
		}
		else if (userContinue != 'N' && userContinue != 'n') {//none of: Y,y,N,n
			cout << "\nValue " << userContinue << " not valid, try again." << endl;//message 6
			clearInput();
		}
	} while (userContinue != 'N' && userContinue != 'n');//exits loop on 'N' or 'n'
	cout << "\nEnding execution." << endl;//message 5
}

void readInData(string stockNames[], float stockPrices[]) {
	ifstream inFile;
	int stockCount = 0;
	string tempName, filePath = "C:\\Temp\\Dow30.txt";
	float tempPrice;
	inFile.open(filePath);
	if (!inFile.is_open()) {
		cout << "\nUnable to open " << filePath << " - ending" << endl;//message 1
		exit(1);
	}
	inFile >> tempName >> tempPrice;
	while (!inFile.eof() && stockCount < TOTALSTOCKS) {//any entries after 30 are ignored
		stockNames[stockCount] = tempName;
		stockPrices[stockCount] = tempPrice;
		++stockCount; //updates stockCount to the amount successfully read into arrays
		inFile >> tempName >> tempPrice;
	}
	inFile.close();
	if (stockCount < TOTALSTOCKS) {
		cout << "\nFile " << filePath << " only contains " << stockCount << " records - ending" << endl;//message 2
		exit(1);
	}
}

void showStocks(string stockNames[], float stockPrices[]) {
	cout << fixed << showpoint << setprecision(2) << left << setw(8) << "Names:" << right << setw(8) << "Prices:" << endl;
	for (int i = 0; i < TOTALSTOCKS; ++i) {
		cout << left << setw(8) << stockNames[i] << right << setw(8) << stockPrices[i] << endl;
	}
	cout << endl;
}

void clearInput() {
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool isUpperStr(string s) {
	for (unsigned i = 0; i < s.length(); ++i)
		if (s[i]<'A' || s[i]>'Z')
			return false;
	return true;
}

int findStockIndex(string stockNames[], string userStockName) {
	int i;
	for (i = 0; i < TOTALSTOCKS; ++i)
		if (stockNames[i] == userStockName)
			return i;
	return -1;//stock name not found
}

void updatePrice(float stockPrices[], int stockIndex, float userStockPrice) {
	//stockPrices[stockIndex] = userStockPrice;
	__asm {
		FINIT
		MOV EDI, [stockPrices]//store address of first element of array stockPrices[] in EBX
		MOV EAX, stockIndex//index of where the price goes, EAX holds 0,1,2,...,29
		FLD userStockPrice//put price on top of stack
		FSTP [EDI + EAX*4]//pops top of stack and stores it into stockPrices in index EAX*4 (float = 4 bytes)
	}
}

float calcAverage(float stockPrices[]) {
	/*float dowSum = 0.0, dowAvg;
	for (int i = 0; i < TOTALSTOCKS; ++i)
		dowSum += stockPrices[i];
	dowAvg = dowSum / DOWDIVISOR;
	return dowAvg;*/
	float dowAvg;
	short sWord;
	bool overflow = false;
	__asm {
		FINIT

		MOV EDI, [stockPrices]//pointer to first element of array
		FLD [EDI]//push price at index 0 onto stack
		MOV ECX, 1//set counter to 1
		
		loop1://loops through indexes 1,2,3,...,TOTALSTOCKS-1
		FADD [EDI + ECX*4]//adds price at index ECX to the sum on top of the stack
		INC ECX
		CMP ECX, TOTALSTOCKS
		JB loop1//ends loop once ECX reaches TOTALSTOCKS, since there is no price at index TOTALSTOCKS

		FDIV DOWDIVISOR//divide top of stack by DOWDIVISOR
		FSTP dowAvg//store final result in dowAvg

		FSTSW sWord//checking if the number in the 80 bit register was too large for a 32 bit float
		AND sWord, 0x8//mask to check bit 3 (Overflow)
		CMP sWord, 0
		JE notOE//if sWord is 0 after the mask, no overflow, so skip the next instruction
		MOV overflow, 1
		notOE:
	}
	if (overflow) {
		cout << "\nThere was an overflow exception while calculating the average - ending" << endl;
		exit(1);
	}
	return dowAvg;
}

void outputAverage(float dowAvg) {
	cout << fixed << showpoint << setprecision(2) << "\nDow Jones Industrial Average is " << dowAvg << "." << endl;//message 3
}