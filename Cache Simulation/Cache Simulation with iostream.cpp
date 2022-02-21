// Bradley, Cody        CS230 Section 12698  5/26/20
// Fourth Laboratory Assignment - Cache Simulation

//the input was not specified, so I originally wrote this using fstream, not iostream


#include<iostream>
#include<string>
#include<fstream>

using namespace std;

void die(string msg);
void checkDuplicate(char opCode, bool ABCDReadIn[]);//ends program if opcode A,B,C, or D is being read in a second time
void validateABCD(int lineSize, int cacheSize, int cacheAssociativity, int mainMemorySize, string sizeNames[]);//ends program if any of the values are not valid, otherwise returns
void checkGZM2(int testInt, string sizeName);//ends program if the int is not greater than zero or is not a multiple of 2
void calcCacheInfo(int cacheSize, int lineSize, int cacheAssociativity, int& cacheLines, int& offsetBits, int& lineBits, int& offsetMask, int& lineMask);
//calculates additional information about the cache based on input information (all variables passed by reference are calculated)
void initializeIntArray(int* intArray, int elements);//initializes the given array with -1 for each element
int findNumBits(int testInt);//returns the minimum number of bits required to represent the given integer
int bringDataToCache(int* mainMemoryArray, int* cacheArray, int* tagArray, bool* bitArray, int mainMemorySize, int cacheAssociativity, int lineSize, int address, int offsetBits, int lineBits, int lineMask);
//brings the data of the address into the cache if it's not already there, and writes back what it replaces if it needs to
//returns associativity of the cache line where the data was stored
int findInCache(int* tagArray, bool* bitArray, int cacheAssociativity, int tag, int lineNum);//returns the associativity if found in the cache with valid bit on, otherwise returns -1
void writeToAddress(int* cacheArray, bool* bitArray, int cacheAssociativity, int lineSize, int address, int associativity, int writeValue, int offsetBits, int offsetMask, int lineMask);
//writes data in instruction to cache line that holds the data at the given address
//only works if the data at address is already in the cache
void showDataAtAddress(int* mainMemoryArray, int* cacheArray, int* tagArray, bool* bitArray, int cacheAssociativity, int lineSize, int address, int offsetBits, int offsetMask, int lineBits, int lineMask);


int main() {
	bool ABCDReadIn[4] = { 0 };//each index 0,1,2,3 corresponds to A,B,C,D respectively having been read in
	bool validAddress;
	char opCode, WRCode;
	int tempInt, lineSize, cacheSize, cacheAssociativity, mainMemorySize, cacheLines;
	int lineBits, offsetBits, lineMask, offsetMask;
	int address, writeValue, associativity;
	string sizeNames[4] = { "line size", "cache size", "cache associativity", "main memory size" };
	/*string filePath = "C:\\Temp\\L4Data.txt";
	ifstream inFile;
	inFile.open(filePath);
	if (!inFile.is_open()) {
		cout << "\nUnable to open " << filePath << " - ending" << endl;
		exit(1);
	}
	*/
	do {
		cin >> opCode >> tempInt;
		/*if (inFile.eof())
			die("Not enough instructions in input file\nPlease use opcodes A, B, C, and D once each");*/
		checkDuplicate(opCode, ABCDReadIn);
		switch (opCode) {
		case 'A':
			lineSize = tempInt;
			ABCDReadIn[0] = true;
			break;
		case 'B':
			cacheSize = tempInt;
			ABCDReadIn[1] = true;
			break;
		case 'C':
			cacheAssociativity = tempInt;
			ABCDReadIn[2] = true;
			break;
		case 'D':
			mainMemorySize = tempInt;
			ABCDReadIn[3] = true;
			break;
		case 'E':
		case 'F':
			die("Please use op codes A, B, C, and D once each before using opcodes E and F");
		default:
			cout << "\nInvalid op code '" << opCode << "'\nEnding program" << endl;
			exit(1);
		}
	} while (!ABCDReadIn[0] || !ABCDReadIn[1] || !ABCDReadIn[2] || !ABCDReadIn[3]);//continues loop while at least one has not been read in
	
	validateABCD(lineSize, cacheSize, cacheAssociativity, mainMemorySize, sizeNames);
	//ABCD read in and validated, can now use information to create arrays to simulate cache and main memory
	calcCacheInfo(cacheSize, lineSize, cacheAssociativity, cacheLines, offsetBits, lineBits, offsetMask, lineMask);

	int* mainMemoryArray = new (nothrow) int[mainMemorySize / 4];//each element of the array is 4 bytes, so divide mainMemorySize by 4 to get total elements of array
	if (mainMemoryArray == nullptr)
		die("Failed to allocate memory for main memory array");
	initializeIntArray(mainMemoryArray, mainMemorySize / 4);

	int* cacheArray = new (nothrow) int[cacheSize / 4];//same as main memory, cache size / 4 gives total data slots to hold integers
	if (cacheArray == nullptr)
		die("Failed to allocate memory for cache array");
	initializeIntArray(cacheArray, cacheSize / 4);

	int* tagArray = new (nothrow) int[cacheLines * cacheAssociativity];
	//acts like a two dimensional array: tagArray[cacheLines][cacheAssociativity]
	if (tagArray == nullptr)
		die("Failed to allocate memory for tag array");
	initializeIntArray(tagArray, cacheLines * cacheAssociativity);

	bool* bitArray = new (nothrow) bool[cacheLines * cacheAssociativity * 2]();
	//acts like a three dimensional array: bitArray[cacheLines][cacheAssociativity][2]
	//there are two elements for each combination of cache line # and associativity # to hold the valid bit and dirty bit
	if (bitArray == nullptr)
		die("Failed to allocate array to hold valid and dirty bits");
	
	//arrays have been created and initialized (bitArray to zero, everything else to -1), can now process op codes E and F

	cin >> opCode >> address;
	while (opCode == 'E' || opCode == 'F') {
		validAddress = true;
		if (address < 0 || address > mainMemorySize || address % 4 != 0)//address must be between 0 and the size of main memory
			validAddress = false;//the address must also be evenly divisible by 4, so that the offset is a multiple of 4 (cache & main memory hold ints)
		if (opCode == 'E') {
			cin >> WRCode >> writeValue;//ASSUMING THERE IS A DUMMY VARIABLE AFTER 'R' SINCE NO EXAMPLES WITH 'R' WERE GIVEN IN THE SAMPLE INPUT
			if (WRCode == 'W') {
				if (validAddress) {
					associativity = bringDataToCache(mainMemoryArray, cacheArray, tagArray, bitArray, mainMemorySize, cacheAssociativity, lineSize, address, offsetBits, lineBits, lineMask);
					//the address that needs to be written to is in the cache if it wasn't already there
					writeToAddress(cacheArray, bitArray, cacheAssociativity, lineSize, address, associativity, writeValue, offsetBits, offsetMask, lineMask);
				}
				else
					cout << "\nInput address " << address << " invalid" << endl;
			}
			else if (WRCode == 'R') {
				if (validAddress) {
					associativity = bringDataToCache(mainMemoryArray, cacheArray, tagArray, bitArray, mainMemorySize, cacheAssociativity, lineSize, address, offsetBits, lineBits, lineMask);
				}
				else
					cout << "\nInput address " << address << " invalid" << endl;
			}
			else {
				cout << "\nInvalid code '" << WRCode << "'\nPlease use either 'W' or 'R' after the address when using op code 'E'\nEnding program" << endl;
				exit(1);
			}
		}
		else if (opCode == 'F') {
			if (validAddress) {
				showDataAtAddress(mainMemoryArray, cacheArray, tagArray, bitArray, cacheAssociativity, lineSize, address, offsetBits, offsetMask, lineBits, lineMask);
			}
			else
				cout << "\nInput address " << address << " invalid" << endl;
		}
		else {
			cout << "\nInvalid op code '" << opCode << "'\nEnding program" << endl;
			exit(1);
		}

		cin >> opCode >> address;
	}


	delete[] mainMemoryArray;
	delete[] cacheArray;
	delete[] tagArray;
	delete[] bitArray;

	//inFile.close();
}

void die(string msg) {
	cout << "\n" << msg << "\nEnding program" << endl;
	exit(1);
}

void checkDuplicate(char opCode, bool ABCDReadIn[]) {
	if ((opCode == 'A' && ABCDReadIn[0]) || (opCode == 'B' && ABCDReadIn[1]) || (opCode == 'C' && ABCDReadIn[2]) || (opCode == 'D' && ABCDReadIn[3])) {
		cout << "\nThe op code '" << opCode << "' has been used multiple times in the input file\nPlease only use each of the op codes A, B, C, and D once\nEnding Program" << endl;
		exit(1);
	}
}

void validateABCD(int lineSize, int cacheSize, int cacheAssociativity, int mainMemorySize, string sizeNames[]) {
	checkGZM2(lineSize, sizeNames[0]);
	checkGZM2(cacheSize, sizeNames[1]);
	checkGZM2(cacheAssociativity, sizeNames[2]);
	checkGZM2(mainMemorySize, sizeNames[3]);
	if (lineSize * 10 > cacheSize) {
		cout << "\nline size " << lineSize << " too large\nEnding Program" << endl;
		exit(1);
	}
	if (cacheSize * 10 > mainMemorySize) {
		cout << "\ncache size " << cacheSize << " too large\nEnding Program" << endl;
		exit(1);
	}
	/*//making sure the 2 and 3 dimensional arrays are correct size
	if (cacheSize % lineSize != 0)
		die("cache size must be evenly divisible by line size");
	if ((cacheSize / lineSize) % cacheAssociativity != 0)
		die("(cache size / line size) must be evenly divisible by cache associativity");*/
}

void checkGZM2(int testInt, string sizeName) {
	if (testInt <= 0 || (testInt & 1 == 1)) {
		cout << "\nInvalid value " << testInt << " for " << sizeName << "\nEnding program" << endl;
		exit(1);
	}
}

void calcCacheInfo(int cacheSize, int lineSize, int cacheAssociativity, int& cacheLines, int& offsetBits, int& lineBits, int& offsetMask, int& lineMask) {
	cacheLines = (cacheSize / lineSize) / cacheAssociativity;
	offsetBits = findNumBits(lineSize);
	lineBits = findNumBits(cacheLines);
	offsetMask = pow(2, offsetBits) - 1;
	//in the example, cache line size is 16 bytes (or 16),
	//and you need 4 bits to represent that,
	//so the offset mask would be 00...001111
	lineMask = pow(2, lineBits) - 1;
	//in the example, there are 64 lines
	//and you need 6 bits to represent that
	//so the lineMask would be 00...00111111
}

void initializeIntArray(int* intArray, int elements) {
	for (int i = 0; i < elements; ++i)
		intArray[i] = -1;
}

int findNumBits(int testInt) {//17 returns 5, 16 returns 4, 15 returns 4, etc
	int count, tempInt = 2;
	for (count = 1; testInt > tempInt; count++)//loops until (2^count >= testInt)
		tempInt = tempInt << 1;
	return count;
}

int bringDataToCache(int* mainMemoryArray, int* cacheArray, int* tagArray, bool* bitArray, int mainMemorySize, int cacheAssociativity, int lineSize, int address, int offsetBits, int lineBits, int lineMask) {
	int tag, lineNum, associativity, lineAddress;
	int availableAssociativity = -1;
	int writeBackAddress, writeBackTag, cacheLineIndex;//writeBackLineNum = lineNum, writeBackAssociativity = associativity
	lineNum = (address >> offsetBits) & lineMask;//line number for the address in the instruction
	tag = address >> (offsetBits + lineBits);//tag for the address in the instruction
	lineAddress = (address >> offsetBits) << offsetBits;//address of the beginning of the line that the instruction asked for
	associativity = findInCache(tagArray, bitArray, cacheAssociativity, tag, lineNum);
	if (associativity == -1) {//data is not already in cache or in cache but not valid, need to bring it in
		for (int i = 0; i < cacheAssociativity && availableAssociativity == -1; ++i) {
			//loop ends as soon as an available line is found or if none of the lines are available
			if (!(bitArray[(lineNum*cacheAssociativity * 2) + (i * 2) + 0]
				&& bitArray[(lineNum*cacheAssociativity * 2) + (i * 2) + 1]))
				//if one or both the valid and dirty bit are zero, the line is available
				availableAssociativity = i;
		}
		if (availableAssociativity == -1) {//no lines available, need to write back a line
			availableAssociativity = tag % cacheAssociativity;//somewhat evenly distributes which lines get evicted
			writeBackTag = tagArray[lineNum * cacheAssociativity + availableAssociativity];
			writeBackAddress = (writeBackTag << (lineBits + offsetBits)) + (lineNum << offsetBits);
			cacheLineIndex = (lineNum * cacheAssociativity * lineSize / 4) + (availableAssociativity * lineSize / 4);
			for (int i = 0; i < lineSize / 4; ++i)//if line size is 16 bytes, only 4 ints must be copied from cache to main memory
				//i is taking the place of offset/4, since it only goes from 0 to lineSize/4
				mainMemoryArray[writeBackAddress / 4 + i] = cacheArray[cacheLineIndex + i];
			//after exiting loop, line we will copy to has been written back to main memory
		}//after evicting a line, the code to pull from main memory is the same as if we didn't evict a line

		tagArray[lineNum * cacheAssociativity + availableAssociativity] = tag;//update tag in the tag array
		cacheLineIndex = (lineNum * cacheAssociativity * lineSize / 4) + (availableAssociativity * lineSize / 4);
		for (int i = 0; i < lineSize / 4; ++i)//copying the line from main memory to cache
			cacheArray[cacheLineIndex + i] = mainMemoryArray[lineAddress / 4 + i];

		bitArray[(lineNum * cacheAssociativity * 2) + (availableAssociativity * 2) + 0] = 1;
		bitArray[(lineNum * cacheAssociativity * 2) + (availableAssociativity * 2) + 1] = 0;
		//set valid bit to true, dirty bit to false

		associativity = availableAssociativity;
	}
	else
		;//data at address is already in cache and valid
	return associativity;
}

int findInCache(int* tagArray, bool* bitArray, int cacheAssociativity, int tag, int lineNum) {
	int tempAssociativity = -1;
	for (int i = 0; i < cacheAssociativity; ++i) {
		//if the cache associativity is 2, it means the cache has 2 columns so it should only check i=0 and i=1
		if (tagArray[lineNum*cacheAssociativity + i] == tag)//if the tag is in the tagArray at line lineNum, associativity i
			if (bitArray[(lineNum*cacheAssociativity * 2) + (i * 2) + 0])//if the valid bit is on for the given lineNum and associativity
				//the 0 in the index represents valid bit, whereas 1 would be the dirty bit
				tempAssociativity = i;
	}
	return tempAssociativity;
}

void writeToAddress(int* cacheArray, bool* bitArray, int cacheAssociativity, int lineSize, int address, int associativity, int writeValue, int offsetBits, int offsetMask, int lineMask) {
	int lineNum, offset;
	lineNum = (address >> offsetBits) & lineMask;//line number for the address in the instruction
	offset = address & offsetMask;

	cacheArray[(lineNum * cacheAssociativity * lineSize / 4) + (associativity * lineSize / 4) + (offset / 4)] = writeValue;

	bitArray[(lineNum * cacheAssociativity * 2) + (associativity * 2) + 1] = 1;
	//set dirty bit to 1 after writing to cache line
}

void showDataAtAddress(int* mainMemoryArray, int* cacheArray, int* tagArray, bool* bitArray, int cacheAssociativity, int lineSize, int address, int offsetBits, int offsetMask, int lineBits, int lineMask) {
	int mainMemoryData, cacheData, offset, lineNum, tag, associativity;
	offset = address & offsetMask;
	lineNum = (address >> offsetBits) & lineMask;
	tag = address >> (offsetBits + lineBits);

	mainMemoryData = mainMemoryArray[address / 4];

	associativity = findInCache(tagArray, bitArray, cacheAssociativity, tag, lineNum);
	if (associativity != -1 && bitArray[(lineNum * cacheAssociativity * 2) + (associativity * 2) + 0]) {
		//if the address is in cache and valid
		cacheData = cacheArray[(lineNum * cacheAssociativity * lineSize / 4) + (associativity * lineSize / 4) + (offset / 4)];
	}
	else//address is not in cache or cache line is not valid
		cacheData = -1;

	cout << "\nAddress: " << address << " memory: " << mainMemoryData << " cache: " << cacheData << endl;
}


/*

accessing mainMemoryArray[address/4]:
in this case, the address is either given or reverse engineered from the tag, line number, and offset
when calculating the address of the whole line, ignore offset:
address = (tag << (lineBits + offsetBits)) + (lineNum << offsetBits)

accessing cacheArray[lineNum][associativity][offset/4]:
cacheArray[(lineNum * cacheAssociativity * lineSize / 4) + (associativity * lineSize / 4) + (offset / 4)]
ie: cacheArray[(5 * cacheAssociativity * lineSize / 4) + (3 * lineSize / 4) + (0 / 4)]
	is line 5, associativity 3, offset 0 (or 1st data slot)
ie: cacheArray[(5 * cacheAssociativity * lineSize / 4) + (3 * lineSize / 4) + (12 / 4)]
	is line 5, associativity 3, offset 12 (or 4th data slot)

accessing tagArray[lineNum][associativity]:
tagArray[lineNum * cacheAssociativity + associativity]
(where cacheAssociativity is the max associativity of the cache)
ie: tagArray[7 * cacheAssociativity + 0] is line 7, associativity 0
ie: tagArray[7 * cacheAssociativity + 1] is line 7, associativity 1

accessing bitArray[lineNum][associativity][2]
bitArray[(lineNum * cacheAssociativity * 2) + (associativity * 2) + (valid or dirty)]
(where (valid or dirty) is replaced with 0 for valid bit or 1 for dirty bit)
ie: bitArray[(3 * cacheAssociativity * 2) + (1 * 2) + 0] is the valid bit of line 3, associativity 1
ie: bitArray[(3 * cacheAssociativity * 2) + (1 * 2) + 1] is the dirty bit of line 3, associativity 1

*/