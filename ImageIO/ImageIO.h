#ifndef IMAGEIO
#define IMAGEIO

#include <string>
using namespace std;

//Saves an RGB int array out as a BMP file, using the sizes to shape it.
__declspec(dllexport) void ImageOutRGB2BMP(const string& filename, unsigned int width, unsigned int height, unsigned char* image);

//Reads the sizes (width, height) of a BMP image file
__declspec(dllexport) void ImageInReadSizes(const string& filename, unsigned int& width, unsigned int& height);
//Loads a BMP file into a pre-allocated array of ints. The sizes must match or this will fail.
__declspec(dllexport) void ImageInBMP2RGB(const string& filename, unsigned int width, unsigned int height, unsigned char* image);

__declspec(dllexport) char ImageNNSample(unsigned char* image, size_t w, size_t h, float s, float t, size_t c);

#define i2s(arr,arrJSIZE,i,j) arr[((i) * (arrJSIZE)) + (j)]
#define i3s(arr,arrJSIZE,arrKSIZE,i,j,k) arr[((i) * ((arrJSIZE) * (arrKSIZE))) + (j) * (arrKSIZE) + (k)]
//#define i4(arr,i,j,k,l) arr[(  (i) * ( (arr ## JSIZE) * (arr ## KSIZE) * (arr ## LSIZE) )  ) + (j) * (  (arr ## KSIZE) * (arr ## LSIZE)  ) + (k) * (arr ## LSIZE) + (l)]

#endif