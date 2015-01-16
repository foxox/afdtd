#include "ParseArgs.h"

bool ParseArgs::checkForDuplicateArgs(int argc, char* argv[])
{
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
				return true;
			}
		}
	}
	return false;
}

map<string,string> ParseArgs::parseArgs(int argc, char* argv[])
{
	map<string,string> returnme;
	for (int i = 0; i < argc; i++)
	{
		string arg = argv[i];
		//cout << arg << "\t\t";
		size_t colonidx = arg.find_first_of(":");
		string lhs, rhs;
		if (arg[0] == '/')
			lhs = arg.substr(1,colonidx-1);
		else
			lhs = arg;
		if (colonidx == string::npos)
		{
			rhs = "";
		}
		else
		{
			rhs = arg.substr(colonidx+1,string::npos);
		}
		//trim "" from rhs with ""

		if (rhs.length() > 1 && rhs[0] == '\"' && rhs[rhs.length()-1] == '\"')
			rhs = rhs.substr(1,rhs.length()-2);
		returnme[lhs]=rhs;
	}
	return returnme;
}
