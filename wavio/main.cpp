#include <iostream>
using namespace std;

#include "wavio.h"

const size_t NUMSAMPLES = 10000000; 

int main(char argc, char** argv)
{
	short* loadme = new short[NUMSAMPLES];
	size_t length;

	loadme[0] = 0;

	int samplerate = 0;

	wavLoad("../../matlab/sounds/john_reading_1_fullspectrum_18s.wav", loadme, NUMSAMPLES, length, samplerate);

	//cout << "Sample: " << loadme[0] << endl;
	cout << endl;

	wavSave("testsave.wav", loadme, length, 44100);

	getchar();
}




void primitiveTypesCheck()
{
	cout << "sizeof(char)" << sizeof(char) << endl;
	//cout << "sizeof(char16_t)" << sizeof(char16_t) << endl;
	//cout << "sizeof(char32_t)" << sizeof(char32_t) << endl;
	cout << "sizeof(short)" << sizeof(short) << endl;
	cout << "sizeof(int)" << sizeof(int) << endl;
	cout << "sizeof(long)" << sizeof(long) << endl;

	unsigned char a = (unsigned char)-1;
	unsigned long long b = a;
	cout << "Max value: " << b << endl;
	unsigned int i = 0;
	while(b != 0)
	{
		b = b & -2;
		b >>= 1;
		i++;
	}
	cout << "Bits: " << i << endl;
}