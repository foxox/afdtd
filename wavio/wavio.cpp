#include "wavio.h"

//#include <cmath>

using namespace std;

//Fills wavdata with up to maxlength amount of wave file sound samples
//assumes 44.1k sample rate, 16bit signed short samples
void wavLoad(const string& filename, short* wavdata, size_t maxlength, size_t& length, int& samplerate)
{
	//cout << "Loading file " << filename << "..." << endl;
	ifstream file;
	file.open(filename.c_str(), ios::in | ios::binary);
	if (!file.is_open())
	{
		cout << "File could not be opened!" << endl;
		length = 0;
		return;
	}

	char data[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};

	//write RIFF header
	file.read(data, 4);
	data[4]='\0';
	//cout << data[0] << data[1] << data[2] << data[3] << endl;
	if (strcmp(data, "RIFF") != 0)
		cout << "Not a RIFF file" << endl;
	//size
	file.read(data, 4);
	int size;
	size = *((int*)data);
	//cout << "Size: " << size << endl;
	//cout << data[0] << data[1] << data[2] << data[3] << endl;

	//wave
	file.read(data, 4);
	//cout << data[0] << data[1] << data[2] << data[3] << endl;
	if (strcmp(data, "WAVE") != 0)
		cout << "Not a WAVE file!" << endl;

	//write fmt chunk
	//for this purpose, require that it be a fmt chunk.
	file.read(data, 4);
	//cout << data[0] << data[1] << data[2] << data[3] << endl;
	if (strcmp(data, "fmt ") != 0)
		cout << "fmt chunk not found in file where expected." << endl;
	int fmtchunksize = 0;
	file.read((char*)&fmtchunksize, 4);
	//cout << "FMT Chunk Size: " << fmtchunksize << endl;
	int compressioncode = 0;
	file.read((char*)&compressioncode, 2);
	//cout << "Compression Code: " << compressioncode << endl;
	if (fmtchunksize != 16 || (compressioncode != 0 && compressioncode != 1))
		cout << "Unexpected WAVE format encountered!" << endl;
	short numchans = 0;
	file.read((char*)&numchans, 2);
	//cout << "Channel Count: " << numchans << endl;
	if (numchans > 1)
		cout << "Too many channels!" << endl;
	//int samplerate = 0;
	file.read((char*)&samplerate, 4);
	//cout << "Sample Rate: " << samplerate << endl;
	int bytespersec = 0;
	file.read((char*)&bytespersec, 4);
	//cout << "Bytes Per Sec: " << bytespersec << endl;
	short blockalign = 0;
	file.read((char*)&blockalign, 2);
	//cout << "Block Alignment: " << blockalign << endl;
	short sigbits = 0;
	file.read((char*)&sigbits, 2);
	//cout << "Significant Bits: " << sigbits << endl;
	//Extra format bytes
	//If I uncomment this, it needs a check with fmtchunksize above to see if it's even necessary at run-time
	//short extraformatbytes = 0;
	//file.read((char*)&extraformatbytes, 2);
	//cout << extraformatbytes << endl;

	//data
	file.read(data, 4);
	//cout << data[0] << data[1] << data[2] << data[3] << endl;
	if (strcmp(data, "data") != 0)
		cout << "data chunk not encountered where it was expected!" << endl;
	int datasize = 0;
	file.read((char*)&datasize, 4);
	//cout << "Data Size: " << datasize << endl;

	size_t i = 0;
	size_t stophere = min<size_t>(maxlength, (size_t)datasize);
	for (i = 0; i < stophere; i++)
	//for (i = 0; i < 100; i++)
	{
		short sample = 0;
		file.read((char*)&sample, sigbits/8);
		//cout << sample << endl;

		wavdata[i] = sample;

		//in case stophere is the max value
		if (i == stophere && stophere == ((unsigned int)0-1))
			break;
	}

	file.close();

	length = datasize / numchans / (sigbits / 8);
	//cout << "Done." << endl;
}





void wavSave(const string& filename, short* data, size_t length, int samplerate)
{
#ifdef VERBOSE
	cout << "Saving file \"" << filename << "\"..." << endl;
#endif
	ofstream file;
	file.open(filename.c_str(), ios::in | ios::binary | ios::trunc);
	if (!file.is_open())
	{
		cout << "File could not be opened!" << endl;
		length = 0;
		return;
	}

	//write RIFF header
	file.write("RIFF", 4);
	//size
	//RIFF header 12 + 24 wave subchunk + 8 data subchunk + data size
	int size = 12 + 16 + 8 + (int)length * 2;
	//cout << "Size: " << size << endl;
	file.write((char*)&size, 4);

	//wave
	file.write("WAVE", 4);

	//write fmt chunk
	file.write("fmt ", 4);
	int fmtchunksize = 16;
	file.write((char*)&fmtchunksize, 4);
	//cout << "FMT Chunk Size: " << fmtchunksize << endl;

	int compressioncode = 1;
	file.write((char*)&compressioncode, 2);
	//cout << "Compression Code: " << compressioncode << endl;

	short numchans = 1;
	file.write((char*)&numchans, 2);
	//cout << "Channel Count: " << numchans << endl;
	
	//int samplerate = 44100;
	file.write((char*)&samplerate, 4);
	//cout << "Sample Rate: " << samplerate << endl;
	
	int bytespersec = samplerate * numchans * 2;
	file.write((char*)&bytespersec, 4);
	//cout << "Bytes Per Sec: " << bytespersec << endl;
	
	short blockalign = numchans * 2;
	file.write((char*)&blockalign, 2);
	//cout << "Block Alignment: " << blockalign << endl;

	short sigbits = 16;
	file.write((char*)&sigbits, 2);
	//cout << "Significant Bits: " << sigbits << endl;
	
	//Extra format bytes
	//If I uncomment this, it needs a check with fmtchunksize above to see if it's even necessary at run-time
	//short extraformatbytes = 0;
	//file.write((char*)&extraformatbytes, 2);
	//cout << extraformatbytes << endl;

	//data
	file.write("data", 4);
	//cout << data[0] << data[1] << data[2] << data[3] << endl;
	int datasize = (int)length * numchans * 2;
	file.write((char*)&datasize, 4);
	//cout << "Data Size: " << datasize << endl;

	size_t i = 0;
	size_t stophere = length;// min<int>(maxlength, datasize);
	for (i = 0; i < stophere; i++)
	//for (i = 0; i < 100; i++)
	{
		short sample = data[i];
		file.write((char*)&sample, sigbits/8);
		//cout << sample << endl;

		//in case stophere is the max value
		if (i == stophere && stophere == ((unsigned int)0-1))
			break;
	}

	file.close();
#ifdef VERBOSE
	cout << "Done." << endl;
#endif
}

//Loads a WAV file and returns the length of the data section
size_t wavGetDataLength(const string& filename)
{
	size_t length;
	int samplerate;
	//reuse load function
	wavLoad(filename,NULL,0,length,samplerate);
	return length;
}

//Loads a WAV file and returns the sample rate of the data
int wavGetSampleRate(const string& filename)
{
	size_t length;
	int samplerate;
	//reuse load function
	wavLoad(filename,NULL,0,length,samplerate);
	return samplerate;
}
