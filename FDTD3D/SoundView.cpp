#include "SoundView.h"

SoundView::SoundView(void)
{
	this->initialized = false;
	this->model = NULL;

	this->dev = -1;
	
	this->lastLowerBound = 0;
	this->lastUpperBound = 0;
	this->notFirstBufferDump = true;
}

SoundView::~SoundView(void)
{
	if (initialized)
	{
		SDL_PauseAudioDevice(this->dev, 1);
		//TODO: figure out why this hangs
		//SDL_CloseAudioDevice(this->dev);
	}
}

void SoundView::setModel(FDTD* _model)
{
	if (this->model != NULL)
	{
		//cout << "You cannot reuse a View with a new Model at this time. Please delete this View and create a new one." << endl;
		cout << "Closing old audio device and re-initializing." << endl;
		this->model = NULL;
		this->initialized = false;
		SDL_PauseAudioDevice(this->dev,1);
		SDL_CloseAudioDevice(this->dev);
		this->dev = NULL;
		this->model = _model;
		this->initView();
		//this->initialized = true;

	}
	else
	{
		if (_model == NULL)
		{
			cout << "You are trying to set a NULL model to this view. This is not allowed." << endl;
		}
		else
		{
			this->model = _model;
			this->initView();
		}
	}
}

void SoundView::initView(void)
{
#ifdef VERBOSE
	cout << "Init auralization..." << endl;
#endif

	if ( SDL_Init(SDL_INIT_AUDIO) != 0)
	{
		cout << "SDL could not initialize. Audio output will not work." << endl;
	}
	else 
	{
#ifdef VERBOSE
		cout << "SDL initialized. Audio ready!" << endl;
#endif
	}

	if (SDL_GetNumAudioDevices(0) < 1) // 0 parameter = playback, nonzero = recording
	{
		cout << "No playback device found. No audio can play." << endl;
		return;
	}

	SDL_AudioSpec desiredSpec;
	desiredSpec.callback = &this->audio_fill;
	desiredSpec.channels = 1;
	desiredSpec.format = AUDIO_F32SYS;
	//desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.freq = (int)(1.0f / this->model->deltaTime);
	//desiredSpec.padding = 0;
	//desiredSpec.samples = 16384;
	desiredSpec.samples = 4096;
	desiredSpec.userdata = this;	//I could set this. it gets passed to the callback. not required

	SDL_AudioSpec obtainedSpec;

	string devname(SDL_GetAudioDeviceName(0,0));
	//cout << "Will attempt to use audio device: " << devname << "..." << endl;

	//set last parameter to 0 to let SDL convert between my format and the hardware format
	this->dev = SDL_OpenAudioDevice(devname.c_str(),0,&desiredSpec,&obtainedSpec,1);
	SDL_PauseAudioDevice(dev,0);

	this->initialized = true;
}

void SoundView::audio_fill(void* userdata, Uint8* stream, int len)
{
	SoundView* thisSV = static_cast<SoundView*>(userdata);
	
	//Cast the stream buffer as a float, since that is the type I am using in this code
	float* streamf = reinterpret_cast<float*>(stream);
	//Adjust the length input accordingly
	len=len / ( sizeof(float) / sizeof(Uint8) );
	
	long int thislowerbound = 0;
	long int thisupperbound = 0;

	bool dataAvailable = true;
	
	//First set lower bound to be last upper bound + 1 to ensure that progress is made
	if (thisSV->notFirstBufferDump)
	{
		thislowerbound = (long)thisSV->lastUpperBound + 1;
	}
	else
	{
		thislowerbound = 0;
		thisSV->notFirstBufferDump = false;
	}

	//If we cannot advance a whole "len" amount, advance as far as possible
	thisupperbound = min<long>((long)thisSV->model->count, thislowerbound + len);
	//if (thisSV->model->count < thisSV->lastUpperBound + len)
	//{	
	//	thisupperbound = (long)thisSV->model->count;
	//}
	//else	//otherwise, advance the whole amount
	//{
	//	thisupperbound = thislowerbound + len;
	//}

	//If we have moved beyond the last of the data, don't play anything
	//Remember that lowerbound progress is always made, so this WILL happen
	if (thislowerbound >= thisSV->model->listenlen)
	{
		dataAvailable = false;
		//TODO: should I pause the device here?
	}

	//TODO: remove, shouldn't happen
	if (thislowerbound < 0)
		thislowerbound = 0;
	
	//TODO: remove, shouldn't happen
	if (thisupperbound > thisSV->model->listenlen)
	{
		thisupperbound = (long)thisSV->model->listenlen - 1;
		dataAvailable = false;
	}
	
	for (int i = 0; i < len; i++)
	{
		if ( (i <= (thisupperbound - thislowerbound)) && dataAvailable)
		{
			//streamf[i] = sin(2.0f * math_pi * 500.0f * ((float)i/(float)len) );
			streamf[i] = 1000.0f * thisSV->model->listen[thislowerbound+i];
		}
		else
		{
			streamf[i] = 0.0f;
		}
	}
	thisSV->lastLowerBound = thislowerbound;
	thisSV->lastUpperBound = thisupperbound;
	
	//SDL_MixAudio(soundbuffer, stream, len, SDL_MIX_MAXVOLUME);
}

void SoundView::updateView()
{
	
}