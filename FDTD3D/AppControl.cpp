#include "AppControl.h"

//make sure static member exists in file scope
map<GLFWwindow*, AppControl*> AppControl::windowToAppControl;

AppControl::AppControl()
{
	this->model = NULL;
	this->view = NULL;
	//this->soundview = NULL;
	
	this->simThread = NULL;
	this->simThreadShouldStop = NULL;
}

AppControl::~AppControl(void)
{
	//cout << "AppControl cleanup!" << endl;
}

void AppControl::run(int argc, char* argv[], string _resultlogfilename)
{
	//cout << endl << "Starting up new AppControl!" << endl;

	//TODO: figure out whether it's worth it to bring view stuff into here, since the view now depends so much on the model and to move any of it into here would mean moving all of it into here
	//Perhaps it is enough to define the keyboard callback here?

	this->simThread = NULL;
	this->autorunStopperThread = NULL;

	//Parse arguments
	//We only need to switch on integrator type here, but to invoke the integrators we need their parameters
	string integrator;

	//MODEL
	ModelParams modelparams;

	//FDTD
	FDTDParams fdtdparams;

	//1EPQUEUE
	FDTD1EPQUEUEParams fdtd1epqueueparams;

	unsigned int numberOfBasicArguments = 1;
	unsigned int numberOfArgumentsModel = 18;
	unsigned int numberOfArgumentsFDTD = 13;
	unsigned int argumentsAccepted = 0;
	unsigned int numberOfArguments1EPQUEUE = 4;

	map<string,string> args = ParseArgs::parseArgs(argc, argv);

	if (argc != args.size())
	{
		cout << "Problem parsing arguments!" << endl;
	}

	string firstArg = argv[0];

	//Loop over all map elements
	for (map<string,string>::iterator it = args.begin(); it != args.end(); it++)
	{
		//ignore first arg
		if (it->first == firstArg)
		{
			continue;
		}


		//string arg = argv[i];
		//cout << arg << "\t\t";
		//size_t colonidx = arg.find_first_of(":");

		string lhs, rhs;

		//lhs = arg.substr(1,colonidx-1);
		lhs = it->first;
		rhs = it->second;

		//if (colonidx == string::npos)
		//{
		//	rhs = "";
		//}
		//else
		//{
		//	rhs = arg.substr(colonidx+1,string::npos);
		//}
		//cout << lhs << "\t" << rhs << endl;

		if (lhs == "" || lhs == "op" || lhs == "script")
		{
			//cout << "Ignoring argument " << arg << endl;
			continue;
		}
		//Basic Arguments
		else if (lhs == "integrator")
			integrator = rhs;

		//Model Arguments
		else if (lhs == "duration")
			modelparams.duration = (float)atof(rhs.c_str());
		else if (lhs == "inputduration")
			modelparams.inputduration = (float)atof(rhs.c_str());
		else if (lhs == "deltaTime")
			modelparams.deltaTime = (float)atof(rhs.c_str());
		else if (lhs == "STEREO")
			modelparams.STEREO = atoi(rhs.c_str()) != 0;
		else if (lhs == "xDim")
			modelparams.xDim = (float)atof(rhs.c_str());
		else if (lhs == "yDim")
			modelparams.yDim = (float)atof(rhs.c_str());
		else if (lhs == "zDim")
			modelparams.zDim = (float)atof(rhs.c_str());
		else if (lhs == "sourcex")
			modelparams.sourcex = (float)atof(rhs.c_str());
		else if (lhs == "sourcey")
			modelparams.sourcey = (float)atof(rhs.c_str());
		else if (lhs == "sourcez")
			modelparams.sourcez = (float)atof(rhs.c_str());
		else if (lhs == "plotinterval")
			modelparams.plotinterval = atoi(rhs.c_str());
		else if (lhs == "realtimeplotinterval")
			modelparams.realtimeplotinterval = (float)atof(rhs.c_str());
		else if (lhs == "MOVIEMODE")
			modelparams.MOVIEMODE = atoi(rhs.c_str()) != 0;
		else if (lhs == "zoomfactor")
			modelparams.zoomfactor = (float)atof(rhs.c_str());
		else if (lhs == "AUTORUN")
			modelparams.AUTORUN = atoi(rhs.c_str()) != 0;
		else if (lhs == "outputfile")
			modelparams.outputfile = rhs;
		else if (lhs == "soundfile")
			modelparams.soundfile = rhs;
		else if (lhs == "obstheightmapfile")
			modelparams.obstheightmapfile = rhs;

		//FDTD Arguments
		else if (lhs == "PML_OBSTACLE_STRETCH")
			fdtdparams.PML_OBSTACLE_STRETCH = atoi(rhs.c_str()) != 0;
		else if (lhs == "maxfreq")
			fdtdparams.maxfreq = (float)atof(rhs.c_str());
		else if (lhs == "m")
			fdtdparams.m = (float)atof(rhs.c_str());
		else if (lhs == "R0")
			fdtdparams.R0 = (float)atof(rhs.c_str());
		else if (lhs == "na")
			fdtdparams.na = atoi(rhs.c_str());
		else if (lhs == "c")
			fdtdparams.c = (float)atof(rhs.c_str());
		else if (lhs == "Cmax")
			fdtdparams.Cmax = (float)atof(rhs.c_str());
		else if (lhs == "ax")
			fdtdparams.ax = (float)atof(rhs.c_str());
		else if (lhs == "n2")
			fdtdparams.n2 = (float)atof(rhs.c_str());
		else if (lhs == "epsilon0")
			fdtdparams.epsilon0 = (float)atof(rhs.c_str());
		else if (lhs == "micx")
			fdtdparams.micx = (float)atof(rhs.c_str());
		else if (lhs == "micy")
			fdtdparams.micy = (float)atof(rhs.c_str());
		else if (lhs == "micz")
			fdtdparams.micz = (float)atof(rhs.c_str());

		//1EPQUEUE Argumets
		else if (lhs == "ABSOLUTE_LIMIT")
			fdtd1epqueueparams.ABSOLUTE_LIMIT = atoi(rhs.c_str()) != 0;
		else if (lhs == "ABSOLUTE_NUMBER")
			fdtd1epqueueparams.ABSOLUTE_NUMBER = atoi(rhs.c_str());
		else if (lhs == "WAVEVOLUME_LIMIT")
			fdtd1epqueueparams.WAVEVOLUME_LIMIT = atoi(rhs.c_str()) != 0;
		else if (lhs == "WAVEVOLUME_LIMIT_FACTOR")
			fdtd1epqueueparams.WAVEVOLUME_LIMIT_FACTOR = (float)atof(rhs.c_str());

		else
		{
			cout << "Unknown FDTD argument: " << it->first << ":" << it->second << "\t" << lhs << "\t" << rhs << endl;
			continue;
		}
		//TODO: keep track of these better to deal with case of duplicate arguments
		//cout << "Argument accepted: " << arg << endl;
		argumentsAccepted++;

		//if (arg.substr(0,4).compare("/op:") == 0)
		//	operation = arg.substr(4,string::npos);	//grab the rest of the string
	}

	if (integrator == "FDTD" || integrator == "FDTDTBB" || integrator == "1E_THRESHOLD")
	{
		if (argumentsAccepted < (numberOfBasicArguments + numberOfArgumentsModel + numberOfArgumentsFDTD))
			cout << "WARNING: Some FDTD argument(s) have not been specified!" << endl;
	}
	else if (integrator == "1E_PQUEUE" || integrator == "1E_PQUEUE2" || integrator == "1E_PQUEUE3" || integrator == "1E_SBPLHEAP" || integrator == "1E_KBEST")
	{
		if (argumentsAccepted < (numberOfBasicArguments + numberOfArgumentsModel + numberOfArgumentsFDTD + numberOfArguments1EPQUEUE))
			cout << "WARNING: Some 1EPQUEUE argument(s) have not been specified!" << endl;
	}
	else
	{
		cout << "Integrator unknown. Argument set status unknown." << endl;
	}


	for (int i = 0; i < argc; i++)
	{
		string argi = argv[i];
		size_t colonidxi = argi.find_first_of(":");
		string lhsi = argi.substr(1,colonidxi-1);
		for (int j = i+1; j < argc; j++)
		{
			string argj = argv[j];
			size_t colonidxj = argj.find_first_of(":");
			string lhsj = argj.substr(1,colonidxj-1);
			if (lhsi == lhsj)
			{
				cout << "WARNING! Duplicate input arguments detected. This may mean that some sim config values have not been configured correctly! STRONGLY ADVISE REVIEWING YOUR INPUT." << endl;
				i = argc;
				j = argc;
			}
		}
	}



	//initialize GLFW
	if (!glfwInit())
	{
		cout << "GLFW Initialization failed." << endl;
		return;
	}

	//Hint at parameters to use when opening windows
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



	//Initialize

	//These two are pretty static since I only have one choice for each
	view = new View();
	//soundview = new SoundView();

	
	//Though for the Model, I have multiple implementations to compare running times
	//FDTD_TBB is faster because it is multithreaded but relies on Intel's TBB

	//Pick and configure the integrator to use

	if (integrator == "FDTD")
	{
		model = new FDTD(modelparams, fdtdparams);
	}
	else if (integrator == "FDTDTBB")
	{
		model = new FDTD_TBB(modelparams, fdtdparams);
	}
	else if (integrator == "1E_THRESHOLD")
	{
		model = new FDTD_TBB_1E_THRESHOLD(modelparams, fdtdparams);
	}
	else if (integrator == "1E_PQUEUE")
	{
		model = new FDTD_1E_PQUEUE(modelparams, fdtdparams, fdtd1epqueueparams);
	}
	else if (integrator == "1E_PQUEUE2")
	{
		model = new FDTD_1E_PQUEUE2(modelparams, fdtdparams, fdtd1epqueueparams);
	}
	else if (integrator == "1E_PQUEUE3")
	{
		model = new FDTD_1E_PQUEUE3(modelparams, fdtdparams, fdtd1epqueueparams);
	}
	else if (integrator == "1E_SBPLHEAP")
	{
		model = new FDTD_1E_SBPLHEAP(modelparams, fdtdparams, fdtd1epqueueparams);
//		model = new FDTD_1E_SBPLHEAP2(modelparams, fdtdparams, fdtd1epqueueparams);
//		model = new FDTD_1E_SBPLHEAP3(modelparams, fdtdparams, fdtd1epqueueparams);
	}
	else if (integrator == "1E_KBEST")
	{
		model = new FDTD_1E_KBEST(modelparams, fdtdparams, fdtd1epqueueparams);
	}
	else
	{
		cout << "Integrator unknown. Using default FDTDTBB." << endl;
		model = new FDTD_TBB(modelparams, fdtdparams);
	}

	//Wire them up
	view->setModel(model);
	//soundview->setModel(model); //(also inits view)
	model->setView(view);
	//model->setSoundView(soundview);


	this->model->resultFilename = _resultlogfilename;


	if (view->initialized && view->windowWavefield != NULL)
	{
		//Set up keyboard callbacks
		//this->keyCallbackForAllGLFWWindows = new AppControl::KeyCallback(this);
		glfwSetKeyCallback(this->view->windowWavefield, AppControl::key_callback);
		AppControl::windowToAppControl[this->view->windowWavefield] = this;

		//WINDOWWORLDCOMMENT glfwSetKeyCallback(this->view->windowWorld, AppControl::key_callback);
		//WINDOWWORLDCOMMENT AppControl::windowToAppControl[this->view->windowWorld] = this;
	}

	//If AUTORUN, do it
	if (model->AUTORUN)
	{
#ifdef VERBOSE
		cout << "AUTORUN: Initiating sim and tracking threads for this sim. Will advance when computation completes." << endl;
#endif
		//Run simulation
		this->simThreadShouldStop = &this->model->shouldStop;
		*this->simThreadShouldStop = false;
		this->simThread = new std::thread( ( runSimOnModelFunctor(this->model, this->simThreadShouldStop) ) );
		this->autorunStopperThread = new std::thread( AUTORUNStopper(this) );
	}

	//clock_t t0 = clock();
	//model->runSim();
	//clock_t t1 = clock() - t0;
	//cout << endl << "Sim time: " << t1 << endl << endl;

	//main loop
	while (1)
	{
		//Tell views to update
		this->view->updateView();

		//Poll for windowWavefield events (like hitting the close button, keys I think)
		glfwPollEvents();
		//If the windowWavefield should close, close it
		if ((this->view->initialized && this->view->windowWavefield!=NULL && glfwWindowShouldClose(this->view->windowWavefield)) || this->view->shouldstop)
			break;
		//WINDOWWORLDCOMMENT if (glfwWindowShouldClose(this->view->windowWorld))
			//WINDOWWORLDCOMMENT break;
	}


	if (model->AUTORUN)
	{
		delete this->simThread;
		delete this->autorunStopperThread;
	}

	//Cleanup
	delete this->view;
	//delete this->soundview;
	delete this->model;

//	delete this->keyCallbackForAllGLFWWindows;

	//Done running application
}

void AppControl::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//NOTE: The following few lines are here because GLFW does not support passing a void* pointer into the callback nor with C++ can i pass a functor in place of the callback function pointer, but I needed access to the AppControl for which this callback was registered. To accomplish this, I am making it such that each window can have only one associated AppControl (reasonable) and describing the mapping with a private static map object in the AppControl class

	//Get the appropriate AppControl from the window->AppControl map
	AppControl* thisAppControl;
	thisAppControl = AppControl::windowToAppControl[window];

	if (action == GLFW_PRESS)
	{

    switch (key)
	{
	case GLFW_KEY_ESCAPE:
		cout << "Escaping!" << endl;
        glfwSetWindowShouldClose(window, GL_TRUE);
		if (thisAppControl->simThread != NULL)
		{
#ifdef VERBOSE
			cout << "There is an existing sim thread! Attempting to terminate it and join now." << endl;
#endif
			if (thisAppControl->autorunStopperThread != NULL)
			{
#ifdef VERBOSE
				cout << "Autorun thread also detected! Attempting to stop sim thread now and let Autorun thread clean up the mess." << endl;
#endif
				*thisAppControl->simThreadShouldStop = true;
				thisAppControl->autorunStopperThread->join();
				//It should have deleted and null'd the sim thread, so just clean up the stopper thread's resources
				delete thisAppControl->autorunStopperThread;
				thisAppControl->autorunStopperThread = NULL;
			}
			else
			{
				*thisAppControl->simThreadShouldStop = true;
				thisAppControl->simThread->join();
				delete thisAppControl->simThread;
				thisAppControl->simThread = NULL;
			}
#ifdef VERBOSE
			cout << "Successfully stopped sim thread." << endl;
#endif

			//TODO: stop autorun thread too? if it exists?
		}
		break;
	case GLFW_KEY_S:
		{
			//If there is an existing sim running, stop it.
			thisAppControl->stopSimThread();
			
			//Run simulation
			thisAppControl->simThreadShouldStop = &thisAppControl->model->shouldStop;
			*thisAppControl->simThreadShouldStop = false;
			//thisAppControl->runsimonmodelfunctor(thisAppControl->model, thisAppControl->simThreadShouldStop);
			//thisAppControl->simThread = std::thread( thisAppControl->runsimonmodelfunctor );
			thisAppControl->simThread = new std::thread( ( runSimOnModelFunctor(thisAppControl->model, thisAppControl->simThreadShouldStop) ) );
		}
		break;

	case GLFW_KEY_Q:
		{
			thisAppControl->stopSimThread();
		}
		break;

	//Cases for switching model
	case GLFW_KEY_1:
		{
			//If there is an existing sim running, stop it.
			thisAppControl->stopSimThread();

			if (thisAppControl->model != NULL)
			{
				delete thisAppControl->model;
				thisAppControl->model = new FDTD();
				cout << endl << "FDTD" << endl << endl;

				//Wire them up
				thisAppControl->view->setModel(thisAppControl->model);
				//thisAppControl->soundview->setModel(thisAppControl->model); //(also inits view)
				thisAppControl->model->setView(thisAppControl->view);
				//thisAppControl->model->setSoundView(thisAppControl->soundview);
			}
			else
				cout << "uh oh!" << endl;
		}
		break;

		case GLFW_KEY_2:
		{
			//If there is an existing sim running, stop it.
			thisAppControl->stopSimThread();

			if (thisAppControl->model != NULL)
			{
				delete thisAppControl->model;
				thisAppControl->model = new FDTD_TBB();
				cout << endl << "FDTD TBB" << endl << endl;

				//Wire them up
				thisAppControl->view->setModel(thisAppControl->model);
				//thisAppControl->soundview->setModel(thisAppControl->model); //(also inits view)
				thisAppControl->model->setView(thisAppControl->view);
				//thisAppControl->model->setSoundView(thisAppControl->soundview);
			}
			else
				cout << "uh oh!" << endl;
		}
		break;

		case GLFW_KEY_3:
		{
			//If there is an existing sim running, stop it.
			thisAppControl->stopSimThread();

			if (thisAppControl->model != NULL)
			{
				delete thisAppControl->model;
				thisAppControl->model = new FDTD_TBB_1E_THRESHOLD();
				cout << endl << "FDTD TBB 1E THRESHOLD" << endl << endl;

				//Wire them up
				thisAppControl->view->setModel(thisAppControl->model);
				//thisAppControl->soundview->setModel(thisAppControl->model); //(also inits view)
				thisAppControl->model->setView(thisAppControl->view);
				//thisAppControl->model->setSoundView(thisAppControl->soundview);
			}
			else
				cout << "uh oh!" << endl;
		}
		break;

		case GLFW_KEY_4:
		{
			//If there is an existing sim running, stop it.
			thisAppControl->stopSimThread();

			if (thisAppControl->model != NULL)
			{
				delete thisAppControl->model;
				thisAppControl->model = new FDTD_1E_PQUEUE();
				cout << endl << "FDTD TBB 1E PQUEUE" << endl << endl;

				//Wire them up
				thisAppControl->view->setModel(thisAppControl->model);
				//thisAppControl->soundview->setModel(thisAppControl->model); //(also inits view)
				thisAppControl->model->setView(thisAppControl->view);
				//thisAppControl->model->setSoundView(thisAppControl->soundview);
			}
			else
				cout << "uh oh!" << endl;
		}
		break;

	}//switch

	}//if action is a keypress

	//if action is a mousebutton press
	else if (action == GLFW_MOUSE_BUTTON_LEFT)
	{
	
	}
	else if (action == GLFW_MOUSE_BUTTON_RIGHT)
	{
	
	}
	else if (action == GLFW_MOUSE_BUTTON_MIDDLE)
	{
	
	}
}

void AppControl::stopSimThread()
{
	if (this->simThread != NULL)
	{
#ifdef VERBOSE
		cout << "There is an existing sim thread! Attempting to terminate it and join now." << endl;
#endif
		if (this->autorunStopperThread != NULL)
		{
#ifdef VERBOSE
			cout << "Autorun thread also detected! Attempting to stop sim thread now and let Autorun thread clean up the mess." << endl;
#endif
			*this->simThreadShouldStop = true;
			this->autorunStopperThread->join();
			//It should have deleted and null'd the sim thread, so just clean up the stopper thread's resources
			delete this->autorunStopperThread;
			this->autorunStopperThread = NULL;
		}
		else
		{
			*this->simThreadShouldStop = true;
			this->simThread->join();
			delete this->simThread;
			this->simThread = NULL;
		}
#ifdef VERBOSE
		cout << "Successfully stopped sim thread." << endl;
#endif
	}
	else
	{
#ifdef VERBOSE
		cout << "There is no existing sim thread to stop." << endl;
#endif
	}
}
