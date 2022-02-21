// Bradley, Cody        CS230 Section 12698  5/12/20
// Third Laboratory Assignment - Map NYC Taxi Destinations

#include<iostream>
#include<fstream>
#include<string>
#include"windows.h"
using namespace std;

void initHeaders(BITMAPFILEHEADER& bmfh, BITMAPINFOHEADER& bmih, char colorTable[], int BMArraySize);//initialize bit map headers and color table
void addPairs(char BMArray[], float coordPairs[], int pairsReadIn);//makes the pixel white for each pair in coordPairs that corresponds to a pixel in BMArray

const int IMAGE_SIZE = 1024;//dimensions of the square bit map
const float fpIMAGE_SIZE = 1024.0;//gets rid of warning about converting from int to float
const int READIN_SIZE = 1604;//# of pairs to read in at a time, each pair being 8 bytes (2 floats)
const float SOUTHBOUNDARY = 40.700455f;//furthest down
const float NORTHBOUNDARY = 40.830509f;//furthest up
const float EASTBOUNDARY = -73.914979f;//furthest right (should be negative)
const float WESTBOUNDARY = -74.045033f;//furthest left (should be negative)
const float MAXRANGE = 0.130054f;//distance from max east to max west or max south to max north

int main() {
	float coordPairs[READIN_SIZE * 2];//will hold READIN_SIZE*2 floats (or READIN_SIZE pairs)
	int pairsReadIn = 0;
	char* BMArray = new (nothrow) char[IMAGE_SIZE*IMAGE_SIZE]();
	if (BMArray == nullptr) {
		cout << "\nCould not allocate array of " << IMAGE_SIZE << " by " << IMAGE_SIZE << ", ending." << endl;
		exit(1);
	}

	ifstream inFile("C:\\Temp\\L2Data10K.dat", ios::in + ios::binary);
	if (!inFile) {
		cout << "\nCould not open input file, ending." << endl;
		exit(1);
	}
	inFile.read(reinterpret_cast<char*>(coordPairs), READIN_SIZE * sizeof(float) * 2);//put READIN_SIZE pairs from inFile to coordPairs array
	pairsReadIn = (int)inFile.gcount()/(sizeof(float)*2);//# of pairs read in this time around
	while (!(inFile.eof() && pairsReadIn == 0)) {//exits loop once no pairs were read in
		addPairs(BMArray, coordPairs, pairsReadIn);
		/*for (int i = 0; i < pairsReadIn * 2; i += 2) {//testing to see the floats being read in
			cout << coordPairs[i] << "   " << coordPairs[i + 1] << endl;
		}
		cout << pairsReadIn << endl;*/
		inFile.read(reinterpret_cast<char*>(coordPairs), READIN_SIZE * sizeof(float) * 2);//put READIN_SIZE pairs from inFile to coordPairs array
		pairsReadIn = (int)inFile.gcount()/(sizeof(float)*2);//# of pairs read in this time around
	}

	inFile.close();
	//BMArray is done being edited

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	char colorTable[1024];
	initHeaders(bmfh, bmih, colorTable, sizeof(BMArray));
	
	ofstream bmpOut("foo.bmp", ios::out + ios::binary);
	if (!bmpOut) {
		cout << "\nCould not open output file, ending." << endl;
		exit(1);
	}

	bmpOut.write((char*)&bmfh, 14);//write file header to bit map
	bmpOut.write((char*)&bmih, 40);//write info header to bit map
	bmpOut.write(&colorTable[0], sizeof(colorTable));//write color map to header
	bmpOut.write(&BMArray[0], IMAGE_SIZE*IMAGE_SIZE);//write the array to bit map
	bmpOut.close();

	delete[] BMArray;//dealloc array

	system("mspaint foo.bmp");//show end result of bit map
}

void initHeaders(BITMAPFILEHEADER& bmfh, BITMAPINFOHEADER& bmih, char colorTable[], int BMArraySize) {
	// Initialize the bit map file header with static values.
	bmfh.bfType = 0x4d42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
	bmfh.bfSize = bmfh.bfOffBits + BMArraySize;

	// Initialize the bit map information header with static values.
	bmih.biSize = 40;
	bmih.biWidth = IMAGE_SIZE;
	bmih.biHeight = IMAGE_SIZE;
	bmih.biPlanes = 1;
	bmih.biBitCount = 8;
	bmih.biCompression = 0;
	bmih.biSizeImage = IMAGE_SIZE * IMAGE_SIZE;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;

	// Build color table.
	for (int i = 0; i < 256; i++) {
		int j = i * 4;
		colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
	}
}

void addPairs(char BMArray[], float coordPairs[], int pairsReadIn) {
	for (int processedBytes = 0; processedBytes < pairsReadIn * 8; processedBytes += 8) {
		float tempFloat;
		__asm {
			FINIT

			MOV ECX, processedBytes
			MOV ESI, coordPairs//EDI now holds address of the beginning of the array coordPairs
			ADD ESI, ECX//EDI now holds the address of the current pair that needs to be processed
			
			FLD [ESI]//push latitude onto floating point stack (~40 degrees)
			FLD NORTHBOUNDARY
			FCOMIP ST(0), ST(1)
			JB skipCoord//if ST(0)<ST(1) (in other words, if north boundary < coord) skip processing this coord
			FLD SOUTHBOUNDARY
			FCOMI ST(0), ST(1)//don't pop because will use south boundary for arithmetic
			JA skipCoord//if ST(0)>ST(1) (in other words, if south boundary > coord) skip processing this coord
			
			FLD [ESI+4]//push longitude onto floating point stack (~-73 degrees)
			FLD EASTBOUNDARY
			FCOMIP ST(0), ST(1)
			JB skipCoord//if ST(0)<ST(1) (in other words, if east boundary < coord) skip processing this coord
			FLD WESTBOUNDARY
			FCOMI ST(0), ST(1)
			JA skipCoord//if ST(0)>ST(1) (in other words, if west boundary > coord) skip processing this coord
			
			//ST(0)==WESTBOUNDARY, ST(1)==Longitude, ST(2)==SOUTHBOUNDARY, ST(3)==latitude
			FSUBP ST(1), ST(0)//ST(1)-ST(0), put result in ST(1), then pop stack (so the result is now on top of the stack)
			FDIV MAXRANGE//divide ST(0) by MAXRANGE and put it in ST(0)
			FMUL fpIMAGE_SIZE//multiply ST(0) by IMAGE_SIZE and put result in ST(0)
			FSTP tempFloat//pop the result and put it in tempFloat
			CVTSS2SI EBX, tempFloat//convert the result into an int to be used as index of longitude and store in EBX
			
			//ST(0)==SOUTHBOUNDARY, ST(1)==latitude
			FSUBP ST(1), ST(0)//ST(1)-ST(0), put result in ST(1), then pop stack (so the result is now on top of the stack)
			FDIV MAXRANGE//divide ST(0) by MAXRANGE and put it in ST(0)
			FMUL fpIMAGE_SIZE//multiply ST(0) by IMAGE_SIZE and put result in ST(0)
			FSTP tempFloat//pop the result and put it in tempFloat
			CVTSS2SI EAX, tempFloat//convert the result into an int to be used as index of longitude and store in EAX

			//EAX==Latitude index, EBX==longitude index
			MUL IMAGE_SIZE//multiply latitude index by IMAGE_SIZE and store it in EAX
			ADD EAX, EBX//(latitude index*IMAGE_SIZE)+longitude index = coordinate's index in 1D array
			MOV EDI, BMArray
			MOV [EDI+EAX], 255//set the value BMArray[EAX] to 255

			skipCoord://skip to here if the coordinate is invalid
		}
	}
}

//CVTSS2SI: convert floating point (32 bit) to int (32 bit)