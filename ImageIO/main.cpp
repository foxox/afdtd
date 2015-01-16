#include <iostream>
using namespace std;

#include "ImageIO.h"

int main(char argc, char** argv)
{
	unsigned char saveme[3][9] =
		{{1,1,1,0,0,0,2,2,2},
		 {0,0,0,255,255,255,0,0,0},
		 {3,3,3,0,0,0,4,4,4}};
	ImageOutRGB2BMP("test.bmp", 3, 3, &(saveme[0][0]));


	unsigned int size1,size2;
	ImageInReadSizes("test2.bmp", size1, size2);
	cout << "Sizes: " << size1 << ", " << size2 << endl;
	unsigned char* image = new unsigned char[size1*size2*3];
	ImageInBMP2RGB("test2.bmp", size1, size2, image);
	ImageOutRGB2BMP("test2_out.bmp", size1, size2, image);
	delete image;


	cout << "Done.";

	//getchar();
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