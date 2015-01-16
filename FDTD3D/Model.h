#ifndef MODELH
#define MODELH

#include "common.h"

#include <time.h>
#include <ctime>

class View;
class SoundView;

#include "tbb/atomic.h"

#include "resultlogger.h"

struct ModelParams
{
	float duration;
	float inputduration;
	float deltaTime;
	bool STEREO;
	float xDim;
	float yDim;
	float zDim;
	float sourcex;
	float sourcey;
	float sourcez;
	int plotinterval;
	float realtimeplotinterval;
	bool MOVIEMODE;
	float zoomfactor;
	bool AUTORUN;
	string outputfile;
	string soundfile;
	string obstheightmapfile;
};

class Model
{
protected: 
	View* view;
	SoundView* soundview;

public:
	Model(void);
	Model(ModelParams modelparams);
	virtual ~Model(void);

	//Model parameters common to all models

	//PARAMETERS

	float duration;
	float inputduration;
	float deltaTime;

	bool STEREO;

	float xDim;
	float yDim;
	float zDim;
	float sourcex;
	float sourcey;
	float sourcez;

	int plotinterval;
	float realtimeplotinterval;
	bool MOVIEMODE;
	float zoomfactor;

	bool AUTORUN;

	string outputfile;
	//TODO: accomodate multiple input sounds
	string soundfile;


	//OTHER MEMBERS

	float* listen;


	//Update Method Evaluation
	tbb::atomic<unsigned long int> expansionCount;

	//Obstacle map.
	//Each element's lowest bit represents whether the cell is free (1) or occupied (0).
	//The following 18 higher bits encode the occupancy status of surrounding cells. This way, only one memory read per cell is needed, even for the laplacian estimation (up to +-3)
	uint_fast32_t* obstaclemap;	//UINT_FASTN_MAX
	size_t obstaclemapJSIZE;
	size_t obstaclemapKSIZE;
	string obstheightmapfile;
	unsigned char* obstaclemap_heightmap;
	size_t obstaclemap_heightmapJSIZE;
	static const size_t obstaclemap_heightmapKSIZE=3;

	//float* absorbmap;
	//size_t absorbmapJSIZE;
	//size_t absorbmapKSIZE;

	size_t listenlen;
	size_t count;

	//Current time in sim
	float simtime;

	void setView(View* _view);
	void setSoundView(SoundView* _view);

	virtual void runSimConfig();

	virtual void updateState(float time) = 0;
	virtual void timeshiftState() = 0;

	volatile bool shouldStop;
	
	string resultFilename;
};

//Inlined methods:
#include "View.h"
#include "SoundView.h"

#endif