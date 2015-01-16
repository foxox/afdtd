#ifndef PARSEARGSH
#define PARSEARGSH

#include <string>
#include <map>

using namespace std;

namespace ParseArgs
{
	bool checkForDuplicateArgs(int argc, char* argv[]);

	map<string,string> parseArgs(int argc, char* argv[]);
}

#endif
