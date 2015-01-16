#include "ImpulseResponse.h"

ImpulseResponse::ImpulseResponse(void)
{
}

ImpulseResponse::~ImpulseResponse(void)
{
}

size_t ImpulseResponse::size(void)
{
	return this->impulseList.size();
}

void ImpulseResponse::normalize()
{
	//Sum absolute values of amplitudes, for worst case
	float sum = 0.0f;
	size_t i = 0;
	for (i = 0; i < this->impulseList.size(); i++)
	{
		sum += abs(this->impulseList[i].amplitude);
	}

	//Divide amplitudes by sum
	for (i = 0; i < this->impulseList.size(); i++)
	{
		this->impulseList[i].amplitude /= sum;
	}
}

void ImpulseResponse::timeOffset(float offset)
{
	//Apply offsets
	size_t i = 0;
	for (i = 0; i < this->impulseList.size(); i++)
	{
		this->impulseList[i].time += offset;
	}	
}

float ImpulseResponse::getMaxDelayTime()
{
	float maxTime = -FLT_MAX;
	size_t i = 0;
	for (i = 0; i < this->impulseList.size(); i++)
	{
		float thisTime = this->impulseList[i].time;
		if (thisTime > maxTime)
		{
			maxTime = thisTime;
		}
	}
	return maxTime;
}

float ImpulseResponse::sampleConvolveSound(float* sound, float length, int samplerate, float time)
{
	float returnSample = 0.0f;

	//TODO: use sorted IR here and stop calculating when beyond limits to save some calculation time

	//forall impulses in IR
	for (std::vector<ImpulseResponse::Impulse>::iterator imp = this->impulseList.begin(); imp < this->impulseList.end(); imp++)
	{
		//check if enough time has passed for some impulse to have an effect (for longer sounds this should be the case most of the time)
		if (time >= imp->time)
		{
			//adjust time with impulse 
			float timeToSampleForThisImpulse = time - imp->time;
			//and make sure that it isn't past the beginning of the sound
			if ( timeToSampleForThisImpulse >= 0.0 )
			{
				//ensure it's also not past the end of the input sound (silence)
				if ( timeToSampleForThisImpulse <= length )
				{
					returnSample += imp->amplitude * sound[ (size_t)floor( (float)samplerate * timeToSampleForThisImpulse ) ];
				}
			}
		}
	}

	//TODO: figure out why this happens in DEBUG mode but not RELEASE... probably some bad pointer somewhere??
	//if (returnSample == 0.0f)
	//	cout << "uh oh!" << endl;

	return returnSample;
}

ImpulseResponse ImpulseResponse::ConvertRecordingToIR(float* data, size_t length, int samplerate)
{
	ImpulseResponse returnme;

	size_t i = 0;
	size_t stophere = length;

	float currdelta = 0.0f;
	float lastdelta = 0.0f;
	float currsample = 0.0f;
	float lastsample = 0.0f;

	if (length > 2)
	{
		for (i = 1; i < stophere; i++)
		{
			currsample = data[i];
			currdelta = currsample - lastsample;
			
			//Test if a peak or valley has just been passed
			if (lastdelta >= 0.0f && currdelta < 0.0f ||
				lastdelta <= 0.0f && currdelta > 0.0f)
			{
				//Add an impulse corresponding to the last time and last amplitude
				returnme.add(Impulse((float)(i-1)/(float)samplerate, lastsample));
			}

			lastsample = currsample;
			lastdelta = currdelta;
		}
	}

	return returnme;
}

ImpulseResponse ImpulseResponse::ConvertRecordingToIR(short *data, size_t length, int samplerate)
{
	//Convert short array to float array and then call float version of this method
	float* data_as_floats = new float[length];
	size_t i = 0;
	for (i = 0; i < length; i++)
	{
		data_as_floats[i] = (float)data[i] / (float)SHRT_MAX;	//MAXSHORT too
	}
	ImpulseResponse returnme = ConvertRecordingToIR(data_as_floats, length, samplerate);
	delete[] data_as_floats;
	return returnme;
}

ImpulseResponse ImpulseResponse::LoadAndConvertRecordingToIR(string filename)
{
	ImpulseResponse returnme;

	size_t irdatalen = wavGetDataLength(filename);
	int irdatasamplerate = wavGetSampleRate(filename);
	short* irdata = new short[irdatalen];
	wavLoad(filename,irdata,irdatalen,irdatalen,irdatasamplerate);
#ifdef VERBOSE
	cout << "Converting recorded IR to ImpulseResponse object..." << endl;
#endif
	returnme = ImpulseResponse::ConvertRecordingToIR(irdata,irdatalen,irdatasamplerate);
	delete[] irdata;
	return returnme;
}

void ImpulseResponse::printIR()
{
	size_t i = 0;
	for (i = 0; i < this->impulseList.size(); i++)
	{
		ImpulseResponse::Impulse impulse = this->impulseList[i];
		cout << "t: " << impulse.time << " a: " << impulse.amplitude << endl;
	}
}