#ifndef WAVIO
#define WAVIO

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

__declspec(dllexport) void wavLoad(const string& filename, short* wavdata, size_t maxlength, size_t& length, int& samplerate);
__declspec(dllexport) void wavSave(const string& filename, short* data, size_t length, int samplerate);

__declspec(dllexport) size_t wavGetDataLength(const string& filename);
__declspec(dllexport) int wavGetSampleRate(const string& filename);

#endif
