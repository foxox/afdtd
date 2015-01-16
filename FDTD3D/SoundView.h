#ifndef SOUNDVIEWH
#define SOUNDVIEWH

#pragma warning (disable: 4985)

#include "SDL.h"
#include "SDL_audio.h"

//class Model;
//#include "Model.h"

class FDTD;
#include "FDTD.h"

#include <iostream>
using namespace std;

#include <cmath>

//TODO: Make SoundView a subclass of View, move View code into View3D or like

class SoundView
{
private:
	FDTD* model;
	bool initialized;

	SDL_AudioDeviceID dev;

	size_t lastLowerBound;
	size_t lastUpperBound;
	bool notFirstBufferDump;

	void initView(void);

public:
	SoundView(void);
	virtual ~SoundView(void);

	void setModel(FDTD* _model);

	void updateView();


	static void audio_fill(void* userdata, Uint8* stream, int len);
};

#endif
