#ifndef APPCONTROLH
#define APPCONTROLH

//#include "common.h"

#pragma warning (disable: 4005)

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <tbb/compat/thread>

#include "Model.h"
#include "View.h"
#include "SoundView.h"

#include "FDTD.h"
#include "FDTD_TBB.h"
#include "FDTD_TBB_1E_THRESHOLD.h"
#include "FDTD_1E_PQUEUE.h"
#include "FDTD_1E_PQUEUE2.h"
#include "FDTD_1E_PQUEUE3.h"

#include "FDTD_1E_SBPLHEAP.h"
//#include "FDTD_1E_SBPLHEAP2.h"
//#include "FDTD_1E_SBPLHEAP3.h"

//#include "FDTD_1E_KBEST.h"
#include "FDTD_1E_KBESTF.h"

#include "ParseArgs.h"

#include <ctime>

#include <map>

#include <string>

using namespace std;

//This class provides a central container in which the Model and View reside.
//It also doubles as the "controller" in the MVC architecture 
class AppControl
{
private:
	//TODO: change this FDTD* to Model* when I add more model types
	FDTD* model;
	View* view;
	//SoundView* soundview;

	static map<GLFWwindow*, AppControl*> windowToAppControl;

	class runSimOnModelFunctor
	{
	public:
		runSimOnModelFunctor(FDTD* const _model, volatile bool* const _shouldStop) : model(_model), shouldStop(_shouldStop) {};
		~runSimOnModelFunctor() {};
		FDTD* const model;
		volatile bool* const shouldStop;
		void operator() (void)
		{
			model->runSim();
		}
	};

	//runSimOnModelFunctor runsimonmodelfunctor;
	std::thread* simThread;
	volatile bool* simThreadShouldStop;
	//TODO: should this be "volatile" or tbb volatile type?


	//AUTORUN STOPPER
	class AUTORUNStopper
	{
	public:
		AUTORUNStopper(/*std::thread* _joinme, */AppControl* _stopme) : /*joinme(_joinme), */stopme(_stopme) {};
		~AUTORUNStopper() {};
		//std::thread* joinme;
		AppControl* stopme;
		void operator() (void)
		{
			//joinme->join();
			//stopme->stopSimThread();
			stopme->simThread->join();
			delete stopme->simThread;
			stopme->simThread = NULL;
#ifdef VERBOSE
			cout << "AUTORUN: Sim thread done. Closing AppControl and moving on." << endl;
#endif
			//Stop one of the windows as if we hit ESC. Either one; doesn't matter which.
			if (stopme->view->windowWavefield!=NULL)
			{
				glfwSetWindowShouldClose(stopme->view->windowWavefield, GL_TRUE);
			}
			stopme->view->shouldstop = true;
		}
	};

	std::thread* autorunStopperThread;

public:
	AppControl();
	virtual ~AppControl(void);

	void run(int argc, char* argv[], string _resultlogfilename);

	static void key_callback(GLFWwindow* windowWavefield, int key, int scancode, int action, int mods);

	void stopSimThread();
};

#endif