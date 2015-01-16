#include "Model.h"

Model::Model()
{
	cout << "Default Model constructor called. You probably don't want this!" << endl;

	this->view = NULL;

	this->STEREO = false;

	this->xDim = 2.0f;
	this->yDim = 2.0f;
	this->zDim = 1.0f;

	this->duration = 0.1f;
	this->inputduration = 10.0f;
	this->deltaTime = 0.1f;

	this->plotinterval = 1000;
	this->realtimeplotinterval = 1.0f / 1.0f;
	this->MOVIEMODE = false;
	this->zoomfactor = 2;

	this->sourcex = (2.0f / 4.0f) * this->xDim;
	this->sourcey = (3.0f / 4.0f) * this->yDim;
	this->sourcez = (2.0f / 4.0f) * this->zDim;

	//cout << duration / deltaTime << endl;
	//cout << ceil(this->duration / this->deltaTime) << endl;
	//cout << (unsigned int)ceil(this->duration / this->deltaTime) << endl;
	this->listenlen = (size_t)ceil(this->duration / this->deltaTime);
	this->listen = new float[this->listenlen];

	this->AUTORUN = false;
	this->shouldStop = false;

	this->outputfile = "default_output.wav";

	this->obstheightmapfile = "0_default_obstheightmap.bmp";
}

Model::Model(ModelParams modelparams)
{
	this->view = NULL;

	//Params

	this->STEREO = modelparams.STEREO;

	this->xDim = modelparams.xDim;
	this->yDim = modelparams.yDim;
	this->zDim = modelparams.zDim;

	this->duration = modelparams.duration;
	//this->duration = sqrt() / this->c; SEE FDTD CODE

	this->inputduration = modelparams.inputduration;
	this->deltaTime = modelparams.deltaTime;

	this->plotinterval = modelparams.plotinterval;
	this->realtimeplotinterval = modelparams.realtimeplotinterval;
	this->MOVIEMODE = modelparams.MOVIEMODE;
	this->zoomfactor = modelparams.zoomfactor;

	this->sourcex = modelparams.sourcex;
	this->sourcey = modelparams.sourcey;
	this->sourcez = modelparams.sourcez;

	this->AUTORUN = modelparams.AUTORUN;

	this->shouldStop = false;

	this->outputfile = modelparams.outputfile;
	this->soundfile = modelparams.soundfile;

	this->obstheightmapfile = modelparams.obstheightmapfile;

	//in case config never gets called
	this->listen = NULL;
}

Model::~Model(void)
{
	//delete[] arrays allocated by this class
	delete[] listen;
	listen = NULL;
}


void Model::setView(View* _view)
{
	this->view = _view;
}

void Model::setSoundView(SoundView* _soundview)
{
	this->soundview = _soundview;
}

void Model::runSimConfig()
{
	//cout << duration / deltaTime << endl;
	//cout << ceil(this->duration / this->deltaTime) << endl;
	//cout << (unsigned int)ceil(this->duration / this->deltaTime) << endl;
	this->listenlen = (size_t)ceil(this->duration / this->deltaTime);
	this->listen = new float[this->listenlen];
}
