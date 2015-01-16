#include "ImageIO.h"

#include <iostream>
#include <fstream>
using namespace std;

void ImageOutRGB2BMP(const string& filename, unsigned int width, unsigned int height, unsigned char* image)
{
	//allocate space to store the bmp file data before writing it out
	//byte* bmpData = new byte[ 100 + ( width * height * 3 ) ];

	//start writing bmp data
	ofstream file;
	file.open(filename.c_str(), ios::out | ios::binary | ios::trunc);

	//Bitmap File Header
	//signature
	file << 'B' << 'M';
	//filesize
	int filesize = (14 + 40 + width * height * 3);
	file.write(reinterpret_cast<const char *>(&filesize), 4);
	//reserved1 reserved2
	short reserved1 = 0;
	short reserved2 = 0;
	file.write(reinterpret_cast<const char *>(&reserved1), 2);
	file.write(reinterpret_cast<const char *>(&reserved2), 2);
	//file offset to pixel array
	int fileoffsettopixelarray = 40+14;
	file.write(reinterpret_cast<const char *>(&fileoffsettopixelarray), 4);

//file.write(reinterpret_cast<const char *>(&), 4);

	//DIB Header
	//BITMAPINFOHEADER
	//size of header
	int dibheadersize = 40;
	file.write(reinterpret_cast<const char *>(&dibheadersize), 4);
	//width
	file.write(reinterpret_cast<const char *>(&width), 4);
	//height
	file.write(reinterpret_cast<const char *>(&height), 4);
	//number of color planes (must be 1)
	short numcolorplanes = 1;
	file.write(reinterpret_cast<const char *>(&numcolorplanes), 2);
	//bits per pixel
	short bitsperpixel = 24;
	file.write(reinterpret_cast<const char *>(&bitsperpixel), 2);
	//compression scheme
	int compressionscheme = 0;
	file.write(reinterpret_cast<const char *>(&compressionscheme), 4);
	//image size	//can be 0 for uncompressed image fields
	int imagesize = 0;
	file.write(reinterpret_cast<const char *>(&imagesize), 4);
	//horizontal resolution
	int horizontalres = 1;
	file.write(reinterpret_cast<const char *>(&horizontalres), 4);
	//vertical resolution
	int verticalres = 1;
	file.write(reinterpret_cast<const char *>(&verticalres), 4);
	//number of colors in palette
	//0 is default to max
	int colorsinpalette = 0;
	file.write(reinterpret_cast<const char *>(&colorsinpalette), 4);
	//number of important colors (generally 0, for all)
	int numimportantcolors = 0;
	file.write(reinterpret_cast<const char *>(&numimportantcolors), 4);

	unsigned int i, j, padding, k;
	padding = ((width % 4) != 0) ? ((width * 3 + 4) / 4) : 0;
	//cout << "width height padding" << width << ", " << height << ", " << padding << endl;
	char paddingzero = 0;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			file.write(reinterpret_cast<const char *>(image + (height - j - 1) * width * 3 + i * 3 + 0), 1);
			file.write(reinterpret_cast<const char *>(image + (height - j - 1) * width * 3 + i * 3 + 1), 1);
			file.write(reinterpret_cast<const char *>(image + (height - j - 1) * width * 3 + i * 3 + 2), 1);
		}
		for (k = 0; k < padding; k++)
		{
			file.write(reinterpret_cast<const char *>(&paddingzero), 1);
		}
	}

	file.close();
}


void ImageInReadSizes(const string& filename, unsigned int& width, unsigned int& height)
{
	//start reading bmp data
	ifstream file;
	file.open(filename.c_str(), ios::in | ios::binary);// | ios::trunc);

	char data[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};

	//Bitmap File Header
	//signature
	//file >> data[0] >> data[1];
	//file.read(data,2); data[2]='\0';
	file.read(data,2); data[2]='\0';
	if (strcmp(data,"BM") != 0)
		cout << "Not a BMP file!" << endl;

	//filesize
	int filesize = 0;
	file.read(reinterpret_cast<char *>(&filesize),4);

	//reserved1 reserved2
	short reserved1 = 0;
	short reserved2 = 0;
	file.read(reinterpret_cast<char *>(&reserved1), 2);
	file.read(reinterpret_cast<char *>(&reserved2), 2);

	//file offset to pixel array
	int fileoffsettopixelarray = 0;
	file.read(reinterpret_cast<char *>(&fileoffsettopixelarray), 4);

	//DIB Header
	//BITMAPINFOHEADER
	//size of header
	int dibheadersize = 40;
	file.read(reinterpret_cast<char *>(&dibheadersize), 4);
	//width
	file.read(reinterpret_cast<char *>(&width), 4);
	//height
	file.read(reinterpret_cast<char *>(&height), 4);

	file.close();
}


void ImageInBMP2RGB(const string& filename, unsigned int width, unsigned int height, unsigned char* image)
{
	//start reading bmp data
	ifstream file;
	file.open(filename.c_str(), ios::in | ios::binary);// | ios::trunc);

	char data[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};

	//Bitmap File Header
	//signature
	//file >> data[0] >> data[1];
	//file.read(data,2); data[2]='\0';
	file.read(data,2); data[2]='\0';
	if (strcmp(data,"BM") != 0)
		cout << "Not a BMP file!" << endl;

	//filesize
	int filesize = 0;
	file.read(reinterpret_cast<char *>(&filesize),4);

	//reserved1 reserved2
	short reserved1 = 0;
	short reserved2 = 0;
	file.read(reinterpret_cast<char *>(&reserved1), 2);
	file.read(reinterpret_cast<char *>(&reserved2), 2);

	//file offset to pixel array
	int fileoffsettopixelarray = 0;
	file.read(reinterpret_cast<char *>(&fileoffsettopixelarray), 4);

	//DIB Header
	//BITMAPINFOHEADER
	//size of header
	int dibheadersize = 40;
	file.read(reinterpret_cast<char *>(&dibheadersize), 4);
	//width
	file.read(reinterpret_cast<char *>(&width), 4);
	//height
	file.read(reinterpret_cast<char *>(&height), 4);
	//number of color planes (must be 1)
	short numcolorplanes = 1;
	file.read(reinterpret_cast<char *>(&numcolorplanes), 2);
	//bits per pixel
	short bitsperpixel = 24;
	file.read(reinterpret_cast<char *>(&bitsperpixel), 2);
	//compression scheme
	int compressionscheme = 0;
	file.read(reinterpret_cast<char *>(&compressionscheme), 4);
	//image size	//can be 0 for uncompressed image fields
	int imagesize = 0;
	file.read(reinterpret_cast<char *>(&imagesize), 4);
	//horizontal resolution
	int horizontalres = 1;
	file.read(reinterpret_cast<char *>(&horizontalres), 4);
	//vertical resolution
	int verticalres = 1;
	file.read(reinterpret_cast<char *>(&verticalres), 4);
	//number of colors in palette
	//0 is default to max
	int colorsinpalette = 0;
	file.read(reinterpret_cast<char *>(&colorsinpalette), 4);
	//number of important colors (generally 0, for all)
	int numimportantcolors = 0;
	file.read(reinterpret_cast<char *>(&numimportantcolors), 4);


	//traverse the gap space between the header and the image data
	for (int i = 0; i < fileoffsettopixelarray - 14 - 40; i++)
		file.read(data,1);
	


	unsigned int i, j, padding, k;
	padding = ((width % 4) != 0) ? ((width * 3 + 4) / 4) : 0;
	//cout << "width height padding" << width << ", " << height << ", " << padding << endl;
	char paddingzero = 0;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			file.read(reinterpret_cast<char*>(&image[(height - j - 1) * width * 3 + i * 3 + 2]), 1);
			file.read(reinterpret_cast<char*>(&image[(height - j - 1) * width * 3 + i * 3 + 1]), 1);
			file.read(reinterpret_cast<char*>(&image[(height - j - 1) * width * 3 + i * 3 + 0]), 1);
			//cout << image[(height-j-1)*width*3+i*3] << endl;
		}
		for (k = 0; k < padding; k++)
		{
			file.read(reinterpret_cast<char *>(&paddingzero), 1);
		}
	}

	file.close();
}

char ImageNNSample(unsigned char* image, size_t w, size_t h, float s, float t, size_t c)
{
	//map to the correct range (width,height) and make sure they aren't over the top
	size_t is = static_cast<size_t>(s * static_cast<float>(w)); is = is==w?w-1:is;
	size_t it = static_cast<size_t>(t * static_cast<float>(h)); it = it==h?h-1:it;
	return i3s(image,h,3,is,it,c);
}
