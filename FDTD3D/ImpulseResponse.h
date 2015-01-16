#ifndef IMPULSERESPONSEH
#define IMPULSERESPONSEH

#include <vector>
using namespace std;

#include "common.h"

//Represents an Impulse Response for use in convolutions
class ImpulseResponse
{
public:
	//Struct representing an impulse with a delay time and an amplitude
	struct Impulse
	{
		float time;
		float amplitude;

		Impulse(float _time, float _amplitude) : time(_time), amplitude(_amplitude) {};
	};

protected:
	vector<ImpulseResponse::Impulse> impulseList;

public:
	ImpulseResponse(void);
	virtual ~ImpulseResponse(void);

	//Get the number of impulses stored in this IR
	size_t size(void);

	//Add an impulse to this IR
	void add(Impulse _addme)
	{
		this->impulseList.push_back(_addme);
	}

	//Add an impulse to this IR
	void add(float _time, float _amplitude)
	{
		this->impulseList.push_back(ImpulseResponse::Impulse(_time, _amplitude));
	}

	//Make the sum of impulse amplitudes equal to 1
	void normalize();

	//Apply an offset to all impulse times
	void timeOffset(float offset);

	//Get the largest delay time among all impulses
	float getMaxDelayTime();

	//Convolve a sound array with this IR at the specified time, yielding the convolved sample for that time
	float sampleConvolveSound(float* sound, float length, int samplerate, float time);

	//Convert a sound recording to an IR by detecting peaks
	static ImpulseResponse ConvertRecordingToIR(float* data, size_t length, int samplerate);
	//Convert a sound recording to an IR by detecting peaks
	static ImpulseResponse ConvertRecordingToIR(short* data, size_t length, int samplerate);

	//Load a sound recording, then convert it as above
	static ImpulseResponse LoadAndConvertRecordingToIR(string filename);

	//Print the IR info
	void printIR();
};

#endif