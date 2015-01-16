
#include "main.h"

#include "ImpulseResponse.h"

#include "../RunfileGenerator/runfilegenerate.h"

void preprocessScript(string scriptFile, string preprocessedScript);
void runScriptVerbatim(string preprocessedScript);
vector<string> tokenizeStringDelimiter(string str, string find);

bool startsWith(string str, string startswiththis);
bool endsWith(string str, string endswiththis);

float errorAnalysis(string file1, string file2);


//size_t lastmem;
//
//
//void* operator new (size_t size)
//{
// void *p=malloc(size); 
// //if (p==0) // did malloc succeed?
//  //throw std::bad_alloc(); // ANSI/ISO compliant behavior
// lastmem += size;
// return p;
//}
//
//void operator delete (void *p)
//{
//	free(p); 
//}


string resultLogFilename = "resultlog.csv";



//Take a set of arguments (like those passed from command line) and determine what to do with them
//This may be called recursively, for example after finding a "script" operation and treating the
//lines of a file as additional arguments for recursive invocation of this function
void actOnArguments(int argc, char* argv[])
{
	string operation;

	//Parse command line arguments

	//Depend on first argument being a filename (or dummy text) and second being /op
	string arg = argv[1];
	if (arg.substr(0,4).compare("/op:") != 0)
	{
		cout << "Error: no /op argument" << endl;
	}
	else
	{
		operation = arg.substr(4,string::npos);	//grab the rest of the string

#ifdef VERBOSE
		cout << "Operation: " << operation << endl;
#endif

		if (operation == "sim")
		{
			//START THE APPLICATION!
			AppControl app;
			app.run(argc,argv,resultLogFilename);
		}
		else if (operation == "pulsegen")	//TODO: add name parameter, add spread param, add center param
		{
			pulseGen();
		}
		else if (operation == "convolve")
		{
			//convolveRecordings("","","");
			convolveRecordingsArgsIn(argc, argv);
		}
		else if (operation == "script")
		{
			string scriptFile = argv[2];
			string preprocessedScript = "0_pre_" + scriptFile;
			preprocessScript(scriptFile,preprocessedScript);
			runScriptVerbatim(preprocessedScript);
		}//end script operation
		else if (operation == "pause")
		{
			cout << endl << "Paused! Press any key to continue." << endl << endl;
			_getch();
		}
		else if (operation == "printir")
		{
			string irfile = argv[2];
			ImpulseResponse ir = ImpulseResponse::LoadAndConvertRecordingToIR(irfile);
			ir.printIR();
		}
		else if (operation == "errorbetween")
		{
			//Calculate and print the error between two curves
			//The lengths should match
			string file1 = argv[2];
			string file2 = argv[3];
			float error = errorAnalysis(file1, file2);
			cout << "Error between " << file1 << " and " << file2 << ":" << endl << error << endl;
		}
		else if (operation == "setresultlog")
		{
			string resultlog = argv[2];
			resultLogFilename = resultlog;
			RLOG::clearFile(resultlog);
		}
		else if (operation == "print")
		{
			int i = 0;
			for (i = 2; i < argc; i++)
			{
				cout << argv[i] << " ";
			}
			cout << endl;
		}
	}//end if testing for op parameter
}

int main(int argc, char* argv[])
{
	/*lastmem = 0;*/

	runfileGenerate();

	actOnArguments(argc, argv);

	//cout << "Press any key to quit." << endl;
	//_getch();

	//getchar();
	//getwchar();

	//Goodbye world
	return 0;
}


//PulseGen implementation
void pulseGen()
{
	//Generate a pulse

	short* sounddatashort = new short[44100];


	////Under then over:
	//	//-2.0*((t0-T)./spread).*exp(-1.*((t0-T)./spread)^2);
	//const float spread = 0.001f;
	//const float offset = 0.0025f;
	//for (size_t i = 0; i < 44100; i++)
	//{
	//	float t = (float)i / 44100.0f;
	//	float temp = (( offset - t ) / spread);
	//	float a = -2.3f * temp * exp( -1.0f * temp * temp );
	//	sounddatashort[i] = (short)(a * (float)SHRT_MAX );
	//	//cout << a << " - " << sounddatashort[i] << endl;
	//}


	//Gaussian shape:
	const float spread = WAVESPREAD;
	const float offset = WAVEOFFSET;
	for (size_t i = 0; i < 44100; i++)
	{
		float x = (float)i / 44100.0f;
		//float a = 0.002f * (1.0f/(spread*sqrt(2.0f*PI))) * exp((-(x-offset)*(x-offset))/(2.0f*spread*spread));
		float a = (1.0f) * exp((-(x-offset)*(x-offset))/(2.0f*spread*spread));
		sounddatashort[i] = (short)(a * (float)SHRT_MAX );
		//cout << a << " - " << sounddatashort[i] << endl;
	}


	wavSave("0_IMPULSE.wav",sounddatashort,44100,44100);

	delete[] sounddatashort;
}

void convolveRecordings(string soundfilename, string irfilename, string outputfilename)
{
	//process a sample sound
	//Load input sound
#ifdef VERBOSE
	cout << "Loading input sound..." << endl;
#endif
	if (soundfilename == "")
		soundfilename = "../../matlab/sounds/john_reading_1_fullspectrum_18s.wav";
	//string soundfilename = "0_IMPULSE.wav";
	//Get the Sample Rate of the file
	int soundsamplerate = 0;
	size_t sounddatalen = 0;
	wavLoad(soundfilename, NULL, 0, sounddatalen, soundsamplerate);
	//Allocate space for the data
	float duration = 1.0f;
	size_t allocated_sounddata_size = (size_t)ceil((float)soundsamplerate * duration);
	short* sounddatashort = new short[allocated_sounddata_size];
	wavLoad(soundfilename, sounddatashort, allocated_sounddata_size, sounddatalen, soundsamplerate);

	if (sounddatalen > allocated_sounddata_size)
	{
#ifdef VERBOSE
		cout << "Input sound data length is longer than allocated_sounddata_size." << endl;
#endif
		sounddatalen = allocated_sounddata_size;
	}
	float* sounddata = new float[sounddatalen];
	for (size_t i = 0; i < sounddatalen; i++)
	{
		sounddata[i] = (float)sounddatashort[i] / SHRT_MAX;
	}
	delete[] sounddatashort;	

	ImpulseResponse ir;
	//ir.add(0.0f, 0.5f);
	//ir.add(0.05f, 0.5f);

	//Test conversion
	if (irfilename == "")
		irfilename = "0_test_ir_input.wav";
	size_t irdatalen = wavGetDataLength(irfilename);
	int irdatasamplerate = wavGetSampleRate(irfilename);
	short* irdata = new short[irdatalen];
	wavLoad(irfilename,irdata,irdatalen,irdatalen,irdatasamplerate);
#ifdef VERBOSE
	cout << "Converting recorded IR to ImpulseResponse object..." << endl;
#endif
	ir = ImpulseResponse::ConvertRecordingToIR(irdata,irdatalen,irdatasamplerate);
	
#ifdef VERBOSE
	cout << "IR size: " << ir.size() << " Max delay: " << ir.getMaxDelayTime() << endl;
#endif

	//adjust for wave offset
	ir.timeOffset(-WAVEOFFSET);

#ifdef VERBOSE
	cout << "Convolving..." << endl;
#endif

	short* outsounddata = new short[3 * 44100];
	clock_t t0 = clock();	//for all timing; this just marks the beginning of overall execution
	for (size_t i = 0; i < 3 * 44100; i++)
	{
		float time = (float)i / 44100.0f;
		float sample = ir.sampleConvolveSound(sounddata,(float)sounddatalen/(float)soundsamplerate,soundsamplerate,time);
		
		//cout << "Sample: " << sample << endl;

		outsounddata[i] = (short) (sample * (float)SHRT_MAX);
	}
	clock_t t1 = clock() - t0;
	float exectime = (float)t1 / (float)CLOCKS_PER_SEC;
	cout << endl << "Convolve time: " << exectime << endl << endl;

#ifdef VERBOSE
	cout << "Saving..." << endl;
#endif
	if (outputfilename == "")
		outputfilename = "0_FilterOutput.wav";
	wavSave(outputfilename, outsounddata, 3 * 44100, 44100);

	delete[] sounddata;
	delete[] irdata;
	delete[] outsounddata;
}

void convolveRecordingsArgsIn(int argc, char* argv[])
{
	if (ParseArgs::checkForDuplicateArgs(argc,argv))
	{
		cout << "Duplicate arguments detected! The argument parse may not be good. Check your input." << endl;
	}
	string soundfile, irfile, outputfile;
	map<string,string> argmap = ParseArgs::parseArgs(argc,argv);
	for (map<string,string>::iterator i = argmap.begin(); i != argmap.end(); i++)
	{
		//switch on map key
		if (i->first == "soundfile")
		{
			soundfile = i->second;
		}
		else if (i->first == "irfile")
		{
			irfile = i->second;
		}
		else if (i->first == "outputfile")
		{
			outputfile = i->second;
		}
	}
	convolveRecordings(soundfile, irfile, outputfile);
}

string findReplace(string str, string find, string replace)
{
	if (find == "")
		return str;

	////make a list of the positions of all of the replacements that need to be made
	////later when replacing them, tack on the lendiff each time to keep the offset correct
	//list<size_t> positions;
	//size_t lendiff = replace.length() - find.length();

	//size_t start = 0;
	//while (start < str.length())
	//{
	//	size_t found = str.find(find,start);
	//	if (found != string::npos)
	//	{
	//		positions.push_back(found);
	//		start += found;
	//	}
	//	else
	//	{
	//		start = str.length();	//stop it
	//	}
	//}
	//
	////Now make the replacements
	//string returnme = str;
	//size_t nextReplacementPos = 0;
	//for (list<size_t>::iterator it = positions.begin(); it != positions.end(); it++)
	//{
	//	nextReplacementPos += *it;

	//}

	string start = "";
	string end = "";
	if (startsWith(str, find))
		start = replace;
	if (endsWith(str, find))
		end = replace;

	vector<string> tokenized = tokenizeStringDelimiter(str, find);

	//detokenize
	string output = "";

	if (tokenized.size() > 0)
	{
		for (size_t i = 0; i < tokenized.size()-1; i++)
		{
			output += tokenized[i] + replace;
		}
		output += tokenized[tokenized.size()-1];
	}

	return start + output + end;
}

bool startsWith(string str, string startswiththis)
{
	if (str.length() < startswiththis.length()) return false;
	return str.substr(0,startswiththis.length()) == startswiththis;
}

bool endsWith(string str, string endswiththis)
{
	if (str.length() < endswiththis.length()) return false;
	return str.substr(str.length()-endswiththis.length()) == endswiththis;
}

vector<string> tokenizeStringDelimiter(string str, string delimiter)
{
	vector<string> tokens;
	size_t pos = 0;
	while (pos < str.length())
	{
		//find the next delimiter or end of string, then save out the chunk between

		size_t nextDelimiterPos = str.find(delimiter, pos);

		if (nextDelimiterPos-pos > 0)
			tokens.push_back(str.substr(pos,nextDelimiterPos-pos));
		
		if (nextDelimiterPos != string::npos)
		{
			pos = nextDelimiterPos + delimiter.length();
		}
		else
		{
			pos = string::npos;
		}
	}
	return tokens;
}

void preprocessScript(string scriptin, string scriptout)
{
	//Apply one pass of preprocessing to a script file
	//This allows #define behavior in scripts

	ifstream filein(scriptin.c_str());//, ios::in);
	ofstream fileout(scriptout.c_str(), ios::trunc);//, ios::out || ios::trunc);

	map<string,string> replacements;

	//Output file buffer
	string output = "";

	while (!filein.eof())
	{
		string line;
		char aoeu[10000];
		filein.getline(aoeu, 10000);
		line = aoeu;

		//Perform existing replacements
		for (map<string,string>::iterator it = replacements.begin(); it != replacements.end(); it++)
		{
			line = findReplace(line,it->first,it->second);
		}

		//Replacements done

		//Tokenize the line on spaces and see if any preprocess commands are in it
		vector<string> tokens = tokenizeStringDelimiter(line," ");
		if (tokens.size() > 0 && tokens[0] == "replace" && tokens.size() > 2)
		{
			//compile remaining tokens into one string
			string the_rest = tokens[2];
			for (int i = 3; i < tokens.size(); i++)
				the_rest += " " + tokens[i];
			replacements[tokens[1]] = the_rest;
		}
		//elseif...
		else	//not a command line, so output it
		{
			//cout << output << endl;
			//cout << line << endl;
			output += line + "\n";
		}

	}//loop over filein

	filein.close();

	//write fileout
	fileout.write(output.c_str(),output.length());
	fileout.close();
}

void runScriptVerbatim(string scriptfile)
{
	//string scriptfile = argv[2];
	fstream script(scriptfile.c_str(), ios_base::in);
	script >> std::noskipws;
	//load the script and execute its items
	while (!script.eof())
	{
		string line;
		string word;
		char sep;
		int argcount = 0;
		if (script.peek() == '\n')
		{
			script.get();
			continue;
		}
		while (word != "\n" && !script.eof())
		{
			argcount++;
			script >> word;
			script >> sep >> std::ws;
			line = line + word;
			line = line + " ";
			if (/*word == "" || */sep == '\n')
				break;
		}
		if (line == "" || line == " " || line.substr(0,2) == "//")
			continue;
		argcount++;	//add space for a dummy filename argument like the OS gives even though it's a script line
		char** arguments = new char*[argcount];
		//Put in the dummy filename argument in spot 0
		arguments[0] = new char[100]; strcpy_s(arguments[0],100,"dummy.exe");
		size_t pos = 0;
		for (size_t i = 1; i < argcount; i++)
		{
			size_t posnextsep = line.find(" ",pos);
			string nextarg = line.substr(pos,posnextsep-pos);
			arguments[i] = new char[100];
			strcpy_s(arguments[i],100,nextarg.c_str());
			pos = posnextsep + 1;
		}
#ifdef VERBOSE
		cout << line << endl;
#endif
		
		actOnArguments(argcount, arguments);

		//delete the arguments array
		for (size_t i = 0; i < argcount; i++)	//note that this starts at 0, deleting even the dummy file arg
		{
			delete[] arguments[i];
		}
		delete[] arguments;
	}//end while script line scan
}

float errorAnalysis(string file1, string file2)
{
	size_t file1len = wavGetDataLength(file1);
	size_t file2len = wavGetDataLength(file2);
	int samplerate1 = 0;
	int samplerate2 = 0;
	if (file1len != file2len)
	{
		cout << "Error! Error analysis file input lengths do not match!" << endl;
		return 0;
	}
	short* file1data = new short[file1len];
	short* file2data = new short[file2len];
	wavLoad(file1, file1data, file1len, file1len, samplerate1);
	wavLoad(file2, file2data, file2len, file2len, samplerate2);

	float squaredDifferenceSum = 0.0f;
	for (size_t i = 0; i < file1len; i++)
	{
		short sample1 = file1data[i];
		short sample2 = file2data[i];
		float fsample1 = static_cast<float>(sample1) / 255.0f;
		float fsample2 = static_cast<float>(sample2) / 255.0f;
		float difference = fsample1 - fsample2;
		squaredDifferenceSum += difference * difference;
	}

	return squaredDifferenceSum;
}