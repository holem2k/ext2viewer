// build.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fstream.h"

const char fileName[] = "build.no";

int main(int argc, char* argv[])
{
	
	fstream file(fileName, ios::out | ios::in | ios::nocreate);
	if (file.good())
	{
		unsigned long buildNum;
		file >> buildNum;
		file.clear();
		file.seekp(0, ios::beg);
		file << ++buildNum;
		cout << endl << "build - " << buildNum << endl;
	}
	else
	{
		file.clear();
		file.open(fileName, ios::out);
		if (file.good()) file << 0;
		else cout << endl << "Error creating file: " << fileName << endl;
	}
	return 0;
}
