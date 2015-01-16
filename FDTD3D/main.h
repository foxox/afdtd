#include "common.h"

//#include "tbb/parallel_for.h"
//#include "tbb/blocked_range.h"

//#include "View.h"
//#include "SoundView.h"
//#include "Model.h"
//#include "FDTD.h"
//#include "FDTD_TBB.h"

#include <conio.h>

#include "ParseArgs.h"

#include "AppControl.h"

#include <string>
//#include <cctype>	//character modification functions
#include <fstream>

//Methods defined in main.cpp

//Generate and save a pulse sample to use as input to a simulation for generating impulse responses
void pulseGen();

//Convolve input sound with an impulse response recording and save the result
void convolveRecordings(string soundfilename, string irfilename, string outputfilename);
//Parse main-like arguments for convolveRecordings
void convolveRecordingsArgsIn(int argc, char* argv[]);

