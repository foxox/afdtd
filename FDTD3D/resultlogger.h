#ifndef RESULTLOGGER_H
#define RESULTLOGGER_H

//#include "common.h"
//#include <iostream>

#include <fstream>
#include <string>

using namespace std;

namespace RLOG
{

inline void clearFile(string filename)
{
	ofstream filestream(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
	filestream.close();
}

inline void appendTo(string filename, string text)
{
	ofstream filestream(filename.c_str(), std::ofstream::out | std::ofstream::app);
	
	filestream << text;

	filestream.close();
}

};

#endif