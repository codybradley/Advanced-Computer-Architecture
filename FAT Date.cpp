//CS 230 Section 12698, Lab Assignment 1, Bradley Cody

#include <iostream>
using namespace std;

void getDate(unsigned short& month, unsigned short& day, unsigned short& year);
bool checkValidDate(unsigned short month, unsigned short day, unsigned short year);

int main() {
	unsigned short month, day, year, FATDate;
	cout << "CS 230 Section 12698     Lab Assignment 1     Bradley, Cody\n" << endl;
	
	getDate(month, day, year);
	__asm {
		//moving each part of date into a register
		mov ax,month
		mov bx,day
		mov cx,year
		//set year to 0-127, where 0 = 1980
		sub cx,1980

		shl ax,5 //month goes in bits 5-8
		//day goes in bits 0-4 (already there)
		shl cx,9 //year goes in bits 9-15
		//combining the three parts into register eax
		or ax,bx 
		or ax,cx
		//copy combined date into FATDate
		mov FATDate,ax
	}

	cout << "\nThe hexadecimal FAT date is: " << hex << FATDate << endl;
	return 0;
}

//asks user for date until the date is valid
void getDate(unsigned short& month, unsigned short& day, unsigned short& year) {
	bool validDate;
	do {
		cout << "Enter a month, day, and year between 1-1-1980 and 12-31-2107: ";
		cin >> month >> day >> year;
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			validDate = false;
		}
		else
			validDate = checkValidDate(month, day, year);
		if (!validDate)
			cout << "Invalid Date" << endl;
	} while (!validDate);
}

//checks if the month, day, and year passed make a valid date
bool checkValidDate(unsigned short month, unsigned short day, unsigned short year) {
	if (year < 1980 || year>2107 || month < 1 || month>12 || day < 1 || day>31)
		return false;
	if (month == 2)//Feb has 28 days
		return (day <= 28);//professor said ignore leap years
	if (month == 4 || month == 6 || month == 9 || month == 11)
		return (day <= 30);//months w/ 30 days
	return true;//month has 31 days
}