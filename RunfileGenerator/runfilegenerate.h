#include <iostream>
#include <fstream>

#include <cmath>

using namespace std;

void runfileGenerate()
{
	ofstream filecreate("../FDTD3D/RunFile.run", fstream::app);
	filecreate << endl;
	filecreate.close();

	ofstream file("../FDTD3D/RunFile.run", fstream::trunc);

	if (file.fail())
	{
		cout << "There was a problem opening the file!" << endl;
	}
	else
	{
		float c =  340.0f;

		file << "/op:setresultlog 1_results.csv" << endl;
		
		file << "replace ENVIRONMENT /STEREO:0 /inputduration:10.0 /deltaTime:0.1 /plotinterval:1000 /realtimeplotinterval:1.0 /MOVIEMODE:0 /zoomfactor:2 /AUTORUN:1 /PML_OBSTACLE_STRETCH:1 /m:4.0 /R0:0.001 /na:10 /c:340.0 /Cmax:1.0 /ax:1.0 /n2:1.0 /epsilon0:1.0" << endl;

		file << "replace MAP_NEWELLSIMON5CM /obstheightmapfile:\"0_MAP_newellsimon5cm_7m.bmp\"\n";
		file << "replace MAP_BLANK /obstheightmapfile:\"0_MAP_blank.bmp\"\n";
		file << "replace MAP_SPLIT_CENTER /obstheightmapfile:\"0_MAP_boxsplit_center.bmp\"\n";
		file << "replace MAP_SPLIT_OFFSET /obstheightmapfile:\"0_MAP_boxsplit_offset.bmp\"\n";
		file << "replace MAP_SPLIT_2OFFSET /obstheightmapfile:\"0_MAP_boxsplit_2offset.bmp\"\n";
		file << "replace MAP_HALFWALL /obstheightmapfile:\"0_MAP_halfwall.bmp\"\n";

		file << endl;

		file << "replace SOUND_IMPULSE /soundfile:\"0_IMPULSE.wav\"" << endl;;
		file << "replace DAFXIMPULSECOMMON /op:sim SOUND_IMPULSE ENVIRONMENT" << endl;
		file << "replace 1ECONFIG1.0 /ABSOLUTE_LIMIT:0 /ABSOLUTE_NUMBER:0 /WAVEVOLUME_LIMIT:1 /WAVEVOLUME_LIMIT_FACTOR:1.0" << endl;
		file << "replace DAFXKBEST1.0 /integrator:1E_KBEST 1ECONFIG1.0" << endl;
		file << "replace DAFXFDTD /integrator:FDTD" << endl;

		file << "/op:pulsegen  //0_IMPULSE.wav is default name" << endl;

#define forvalrange(arg) for (arg = arg##low; arg <= arg##high; arg += arg##inc)
		string methods[2];
		methods[0] = "DAFXFDTD";
		methods[1] = "DAFXKBEST1.0";

		unsigned int total = 5*5*2;
		unsigned int count = 0;

		float sizelow = 2.0f;
		float sizehigh = 50.0f;
		float sizeinc = 10.0f;
		float size;
		forvalrange(size)
		{
			float maxfreqlow = 200;
			float maxfreqhigh = 1000;
			float maxfreqinc = 200;
			float maxfreq;
			forvalrange(maxfreq)
			{
				int methodsilow = 0;
				int methodsihigh = 1;
				int methodsiinc = 1;
				int methodsi;
				forvalrange(methodsi)
				{
					count++;
					file << "/op:print" << endl;
					file << "/op:print Simulation " << count << "/" << total << endl;

					file << "DAFXIMPULSECOMMON MAP_BLANK /outputfile:\"deleteme.wav\"";

					file << " " << methods[methodsi].c_str();

					file << " /sourcex:" << size/2.0f << " /sourcey:" << 3.0f*size/4.0f << " /sourcez:" << size/4.0f;
					file << " /micx:" << size/2.0f << " /micy:" << size/4.0f << " /micz:" << size/4.0f;
					file << " /xDim:" << size << " /yDim:" << size << " /zDim:" << size/2.0f;

					float duration = sqrt(size*size+size*size+(size/2.0f)*(size/2.0f)) / c;
					file << " /duration:" << duration;

					file << " /maxfreq:" << maxfreq;

					file << endl;
				}
			}
		}

		file.close();
		cout << "Done generating run file!" << endl;
	}

}	//end runfileGenerate()