 #include "FDTD.h"

FDTD::FDTD(void)
{
	cout << "Default FDTD constructor called. You might not want this!" << endl;

	//FDTD

	this->PML_OBSTACLE_STRETCH = false;

	this->maxfreq = 1000.0f;

	this->m = 4;
	this->R0 = 0.001f;
	this->na = 10;

	this->c = 340.0f;
	this->h = c / (2.0f * maxfreq);

	this->iSizeSim = (unsigned int)ceil(xDim / h);
	this->jSizeSim = (unsigned int)ceil(yDim / h);
	this->kSizeSim = (unsigned int)ceil(zDim / h);

	this->mici = (unsigned int)floor(2.0f * (float)iSizeSim / 4.0f) + na;
	this->micj = (unsigned int)floor(1.0f * (float)jSizeSim / 4.0f) + na;
	this->mick = (unsigned int)floor(2.0f * (float)kSizeSim / 4.0f) + na;

	this->iSize = iSizeSim + 2 * na;
	this->jSize = jSizeSim + 2 * na;
	this->kSize = kSizeSim + 2 * na;

	this->sourcei = (unsigned int)((this->sourcex / this->xDim) * (float)this->iSizeSim) + na;
	this->sourcej = (unsigned int)((this->sourcey / this->yDim) * (float)this->jSizeSim) + na;
	this->sourcek = (unsigned int)((this->sourcez / this->zDim) * (float)this->kSizeSim) + na;

	this->Cmax = 1.0f;
	this->deltaTime = Cmax * h / (c * sqrt(3.0f));

	this->lambda = (c * deltaTime / h);
	this->lambda2 = lambda * lambda;

	//Create large matrices
	//cout << "Creating large matrices..." << endl;

	size_t pnSize = this->iSize * this->jSize * this->kSize;

	this->pn1 = new float[pnSize];
	this->pn2 = new float[pnSize];
	this->pn3 = new float[pnSize];
	memset(pn1,0,pnSize*sizeof(float));
	memset(pn2,0,pnSize*sizeof(float));
	memset(pn3,0,pnSize*sizeof(float));

	this->pn1JSIZE = this->jSize;
	this->pn2JSIZE = this->jSize;
	this->pn3JSIZE = this->jSize;

	this->pn1KSIZE = this->kSize;
	this->pn2KSIZE = this->kSize;
	this->pn3KSIZE = this->kSize;

	//Initialize pressure array pointers
	this->pn = this->pn1;
	this->pnp1 = this->pn2;
	this->pnm1 = this->pn3;

	this->pnJSIZE = this->jSize;
	this->pnp1JSIZE = this->jSize;
	this->pnm1JSIZE = this->jSize;

	this->pnKSIZE = this->kSize;
	this->pnp1KSIZE = this->kSize;
	this->pnm1KSIZE = this->kSize;


	//TODO: make this only inner area size, not whole grid
	this->laplacian = new float[pnSize];
	this->laplacianJSIZE = this->jSize;
	this->laplacianKSIZE = this->kSize;
	memset(this->laplacian,0,pnSize*sizeof(float));

	//Obstacle map
	this->obstaclemap = new uint_fast32_t[pnSize];
	//sizes
	this->obstaclemapJSIZE = this->jSize;
	this->obstaclemapKSIZE = this->kSize;
	//temporary version for conversion to baked map
	char* obstaclemap_simple = new char[pnSize];
	//temporary ones's sizes
	unsigned int obstaclemap_simpleJSIZE = this->jSize;
	unsigned int obstaclemap_simpleKSIZE = this->kSize;
	//set to all one bits
	memset(obstaclemap,static_cast<uint_fast32_t>(-1),pnSize*sizeof(uint_fast32_t));
	//set simple map to ones
	memset(obstaclemap_simple,1,pnSize*sizeof(char));

	////Absorptivity map
	//this->absorbmap = new float[pnSize];
	//this->absorbmapJSIZE = this->jSize;
	//this->absorbmapKSIZE = this->kSize;
	//memset(absorbmap, 0, pnSize*sizeof(float));
	


	//set simple map 0 values
	for (unsigned int i = 0; i < iSize; i++)
	{
		for (unsigned int j = 0; j < jSize; j++)
		{
			for (unsigned int k = 0; k < kSize; k++)
			{
				if ((i < na+1 || i > iSize-na)||
					(j < na+1 || j > jSize-na)||
					(k < na+1 || k > kSize-na))
				{
					i3(obstaclemap_simple,i,j,k) = 0;
				}
			}
		}
	}



	//convert simple map to baked map
	for (unsigned int i = 0; i < iSize; i++)
	{
		for (unsigned int j = 0; j < jSize; j++)
		{
			for (unsigned int k = 0; k < kSize; k++)
			{
				char obstaclemask[7] = {1,1,1,1,1,1,1};
				if ((int)i-1 < 0 || i3(obstaclemap_simple,i-1,j,k) == 0)
					obstaclemask[1] = 0;
				if (i+1 >= iSize || i3(obstaclemap_simple,i+1,j,k) == 0)
					obstaclemask[2] = 0;
				if ((int)j-1 < 0 || i3(obstaclemap_simple,i,j-1,k) == 0)
					obstaclemask[3] = 0;
				if (j+1 >= jSize || i3(obstaclemap_simple,i,j+1,k) == 0)
					obstaclemask[4] = 0;
				if ((int)k-1 < 0 || i3(obstaclemap_simple,i,j,k-1) == 0)
					obstaclemask[5] = 0;
				if (k+1 >= kSize || i3(obstaclemap_simple,i,j,k+1) == 0)
					obstaclemask[6] = 0;

				if (i3(obstaclemap_simple,i,j,k) == 0)
				{
					obstaclemask[0] = 0;
					obstaclemask[1] = 0;
					obstaclemask[2] = 0;
					obstaclemask[3] = 0;
					obstaclemask[4] = 0;
					obstaclemask[5] = 0;
					obstaclemask[6] = 0;
				}

				uint_fast32_t obstacle = ~0u;
				for (int a = 0; a < 7; a++)
				{
					if (obstaclemask[a] == 0)
						obstacle &= ~(1u << a);
				}

				i3(obstaclemap,i,j,k) = obstacle;

				//char obstaclemask2[7];
				//for (unsigned int a = 0; a < 7; a++)
				//{
				//	obstaclemask2[a] = (char)((obstacle >> a) & 1u);
				//}
				//for (unsigned int a = 0; a < 7; a++)
				//	if (obstaclemask2[a] != obstaclemask[a])
				//		cout << "Error.";
			}
		}
	}
	//clean up the simple obstacle map
	delete[] obstaclemap_simple;

	this->count = 0;

	this->simtime = 0.0f;


	//PML



	//TODO: This is overkill since the interior region is never used. Add a macro or small function to do better indexing into smaller arrays
	size_t pmlSize = 3 * iSize * jSize * kSize;
	d1n1 = new float[pmlSize];
	d1n2 = new float[pmlSize];
	//d1n3 = new float[pmlSize];
	d2n1 = new float[pmlSize];
	d2n2 = new float[pmlSize];
	//d2n3 = new float[pmlSize];
	//Initialize them to 0
	memset(d1n1,0,pmlSize*sizeof(float));
	memset(d1n2,0,pmlSize*sizeof(float));
	//memset(d1n3,0,pmlSize*sizeof(float));
	memset(d2n1,0,pmlSize*sizeof(float));
	memset(d2n2,0,pmlSize*sizeof(float));
	//memset(d2n3,0,pmlSize*sizeof(float));

	//Initialize accessible sizes for those arrays, for the index macro i4()
	d1nphalfJSIZE = iSize;
	d1nphalfKSIZE = jSize;
	d1nphalfLSIZE = kSize;
	d2nphalfJSIZE = iSize;
	d2nphalfKSIZE = jSize;
	d2nphalfLSIZE = kSize;

	//d1nphalfnewJSIZE = iSize;
	//d1nphalfnewKSIZE = jSize;
	//d1nphalfnewLSIZE = kSize;
	//d2nphalfnewJSIZE = iSize;
	//d2nphalfnewKSIZE = jSize;
	//d2nphalfnewLSIZE = kSize;

	d1nmhalfJSIZE = iSize;
	d1nmhalfKSIZE = jSize;
	d1nmhalfLSIZE = kSize;
	d2nmhalfJSIZE = iSize;
	d2nmhalfKSIZE = jSize;
	d2nmhalfLSIZE = kSize;

	//Initialize d1nphalf, etc. with valid pointers
	d1nphalf = d1n1;
	d1nmhalf = d1n2;
	//d1nphalfnew = d1n3;
	d2nphalf = d2n1;
	d2nmhalf = d2n2;
	//d2nphalfnew = d2n3;

	this->ax = 1;
	this->n2 = 1;
	this->epsilon0 = 1;

	deltadim = this->c * this->deltaTime / this->h;
	pmlupdated2factor = deltadim * deltadim / this->n2;

	sigmamax = this->n2 * this->epsilon0 * this->c * ((float)this->m+1.0f) * log(this->R0) / (2.0f * (float)this->na * this->h);
}


FDTD::FDTD(ModelParams modelparams, FDTDParams params) : Model(modelparams)
{
	//FDTD

	this->PML_OBSTACLE_STRETCH = params.PML_OBSTACLE_STRETCH;

	this->maxfreq = params.maxfreq;

	this->m = params.m;
	this->R0 = params.R0;
	this->na = params.na;

	this->c = params.c;
	//////////////////////////////////
	//FOR DAFX
	//OVERRIDE MODEL DURATION
	//this->duration = sqrt(this->xDim*this->xDim+this->yDim*this->yDim+this->zDim*this->zDim) / this->c;
	//this->duration = 0.15f;
	cout << "Duration: " << this->duration << endl;
	//////////////////////////////////
	this->h = c / (2.0f * maxfreq);

	this->iSizeSim = (unsigned int)ceil(xDim / h);
	this->jSizeSim = (unsigned int)ceil(yDim / h);
	this->kSizeSim = (unsigned int)ceil(zDim / h);

	this->iSize = iSizeSim + 2 * na;
	this->jSize = jSizeSim + 2 * na;
	this->kSize = kSizeSim + 2 * na;

	//this->mici = (unsigned int)floor(2.0f * (float)iSizeSim / 4.0f) + na;
	//this->micj = (unsigned int)floor(1.0f * (float)jSizeSim / 4.0f) + na;
	//this->mick = (unsigned int)floor(2.0f * (float)kSizeSim / 4.0f) + na;
	this->mici = (unsigned int)((params.micx / this->xDim) * (float)this->iSizeSim) + na;;
	this->micj = (unsigned int)((params.micy / this->xDim) * (float)this->jSizeSim) + na;;
	this->mick = (unsigned int)((params.micz / this->xDim) * (float)this->kSizeSim) + na;;

	this->sourcei = (unsigned int)((this->sourcex / this->xDim) * (float)this->iSizeSim) + na;
	this->sourcej = (unsigned int)((this->sourcey / this->yDim) * (float)this->jSizeSim) + na;
	this->sourcek = (unsigned int)((this->sourcez / this->zDim) * (float)this->kSizeSim) + na;

	this->Cmax = params.Cmax;
	this->deltaTime = Cmax * h / (c * sqrt(3.0f));

	this->lambda = (c * deltaTime / h);
	this->lambda2 = lambda * lambda;

	//Create large matrices
	//cout << "Creating large matrices..." << endl;

	size_t pnSize = this->iSize * this->jSize * this->kSize;

	//TODO: ERROR: bad alloc
	this->pn1 = new float[pnSize];
	this->pn2 = new float[pnSize];
	this->pn3 = new float[pnSize];
	memset(pn1,0,pnSize*sizeof(float));
	memset(pn2,0,pnSize*sizeof(float));
	memset(pn3,0,pnSize*sizeof(float));

	this->pn1JSIZE = this->jSize;
	this->pn2JSIZE = this->jSize;
	this->pn3JSIZE = this->jSize;

	this->pn1KSIZE = this->kSize;
	this->pn2KSIZE = this->kSize;
	this->pn3KSIZE = this->kSize;

	//Initialize pressure array pointers
	this->pn = this->pn1;
	this->pnp1 = this->pn2;
	this->pnm1 = this->pn3;

	this->pnJSIZE = this->jSize;
	this->pnp1JSIZE = this->jSize;
	this->pnm1JSIZE = this->jSize;

	this->pnKSIZE = this->kSize;
	this->pnp1KSIZE = this->kSize;
	this->pnm1KSIZE = this->kSize;


	//TODO: make this only inner area size, not whole grid
	this->laplacian = new float[pnSize];
	this->laplacianJSIZE = this->jSize;
	this->laplacianKSIZE = this->kSize;
	memset(this->laplacian,0,pnSize*sizeof(float));

	//Obstacle map
	this->obstaclemap = new uint_fast32_t[pnSize];
	//sizes
	this->obstaclemapJSIZE = this->jSize;
	this->obstaclemapKSIZE = this->kSize;
	//temporary version for conversion to baked map
	char* obstaclemap_simple = new char[pnSize];
	//temporary ones's sizes
	unsigned int obstaclemap_simpleJSIZE = this->jSize;
	unsigned int obstaclemap_simpleKSIZE = this->kSize;
	//set to all one bits
	memset(obstaclemap,static_cast<uint_fast32_t>(-1),pnSize*sizeof(uint_fast32_t));
	//set simple map to ones
	memset(obstaclemap_simple,1,pnSize*sizeof(char));
	//Load obstmap image
	unsigned int w,h;
	ImageInReadSizes(modelparams.obstheightmapfile, w, h);
	obstaclemap_heightmap = new unsigned char[w*h*3];
	obstaclemap_heightmapJSIZE = h;
	//	obstaclemap_heightmapKSIZE = 3;
	ImageInBMP2RGB(modelparams.obstheightmapfile, w, h, obstaclemap_heightmap);

	////Absorptivity map
	//this->absorbmap = new float[pnSize];
	//this->absorbmapJSIZE = this->jSize;
	//this->absorbmapKSIZE = this->kSize;
	//memset(absorbmap, 0, pnSize*sizeof(float));

	//set simple map obstacle locations. it has already been set to all 1s above
	for (unsigned int i = na; i < iSize-na; i++)
	{
		for (unsigned int j = na; j < jSize-na; j++)
		{
			//Find normalized image space coordinates for this column of cells
			float s = static_cast<float>(i-na) / static_cast<float>(iSizeSim);
			float t = static_cast<float>(j-na) / static_cast<float>(jSizeSim);
			
			//read from image here
			unsigned char obstKHeight = ImageNNSample(obstaclemap_heightmap,w,h,s,t,0);

			//unsigned char absorb = ImageNNSample(obstaclemap_heightmap,w,h,s,t,1);
			//float absorbf = (float)absorb / 255.0f;
			////if (absorb != 0)
			//	//cout << "waa" << endl;

			float obstKHeightScaled = static_cast<float>(obstKHeight) / 255.0f;
			obstKHeight = static_cast<unsigned char>(obstKHeightScaled * static_cast<float>(kSizeSim));
			
			//
			for (unsigned int k = na; k < obstKHeight + na; k++)
			{
				//if ((i < na+1 || i > iSize-na-1)||
				//	(j < na+1 || j > jSize-na-1)||
				//	(k < na+1 || k > kSize-na-1))
				//if (na < i && i < na+2)
				//{
					i3(obstaclemap_simple,i,j,k) = 0;	//obstacle
				//}
			}

			//vertical pml obstacle stretch
			if (this->PML_OBSTACLE_STRETCH)
			{
				for(size_t k = 0; k < na; k++)
				{
					i3(obstaclemap_simple,i,j,k) = i3(obstaclemap_simple,i,j,na);
				}
				for(size_t k = kSize-na; k < kSize; k++)
				{
					i3(obstaclemap_simple,i,j,k) = i3(obstaclemap_simple,i,j,kSize-na-1);
				}
			}


			//if (absorb != 0)
			//{
			//	for (size_t k = na; k < kSizeSim+na; k++)
			//		i3(this->absorbmap,i,j,k) = absorbf;
			//}

			////Ceiling and floor
			////if it's in the sim space and not the pml zones
			//if (i >= na && i < iSize-na && j >= na && j < jSize-na)
			//{
			//	//floor
			//	i3(obstaclemap_simple,i,j,na) = 0;
			//	//ceiling
			//	i3(obstaclemap_simple,i,j,kSize-na-1) = 0;
			//}
		}
	}

	//convert simple map to baked map
	for (unsigned int i = 0; i < iSize; i++)
	{
		for (unsigned int j = 0; j < jSize; j++)
		{
			for (unsigned int k = 0; k < kSize; k++)
			{
				char obstaclemask[7] = {1,1,1,1,1,1,1};
				if ((int)i-1 < 0 || i3(obstaclemap_simple,i-1,j,k) == 0)
					obstaclemask[1] = 0;
				if (i+1 >= iSize || i3(obstaclemap_simple,i+1,j,k) == 0)
					obstaclemask[2] = 0;
				if ((int)j-1 < 0 || i3(obstaclemap_simple,i,j-1,k) == 0)
					obstaclemask[3] = 0;
				if (j+1 >= jSize || i3(obstaclemap_simple,i,j+1,k) == 0)
					obstaclemask[4] = 0;
				if ((int)k-1 < 0 || i3(obstaclemap_simple,i,j,k-1) == 0)
					obstaclemask[5] = 0;
				if (k+1 >= kSize || i3(obstaclemap_simple,i,j,k+1) == 0)
					obstaclemask[6] = 0;

				if (i3(obstaclemap_simple,i,j,k) == 0)
				{
					obstaclemask[0] = 0;
					obstaclemask[1] = 0;
					obstaclemask[2] = 0;
					obstaclemask[3] = 0;
					obstaclemask[4] = 0;
					obstaclemask[5] = 0;
					obstaclemask[6] = 0;
				}

				uint_fast32_t obstacle = ~0u;
				for (int a = 0; a < 7; a++)
				{
					if (obstaclemask[a] == 0)
						obstacle &= ~(1u << a);
				}

				i3(obstaclemap,i,j,k) = obstacle;

				//char obstaclemask2[7];
				//for (unsigned int a = 0; a < 7; a++)
				//{
				//	obstaclemask2[a] = (char)((obstacle >> a) & 1u);
				//}
				//for (unsigned int a = 0; a < 7; a++)
				//	if (obstaclemask2[a] != obstaclemask[a])
				//		cout << "Error.";
			}
		}
	}
	//clean up the simple obstacle map
	delete[] obstaclemap_simple;

	this->count = 0;

	this->simtime = 0.0f;

	//PML



	//TODO: This is overkill since the interior region is never used. Add a macro or small function to do better indexing into smaller arrays
	size_t pmlSize = 3 * iSize * jSize * kSize;
	d1n1 = new float[pmlSize];
	d1n2 = new float[pmlSize];
	//d1n3 = new float[pmlSize];
	d2n1 = new float[pmlSize];
	d2n2 = new float[pmlSize];
	//d2n3 = new float[pmlSize];
	//Initialize them to 0
	memset(d1n1,0,pmlSize*sizeof(float));
	memset(d1n2,0,pmlSize*sizeof(float));
	//memset(d1n3,0,pmlSize*sizeof(float));
	memset(d2n1,0,pmlSize*sizeof(float));
	memset(d2n2,0,pmlSize*sizeof(float));
	//memset(d2n3,0,pmlSize*sizeof(float));

	//Initialize accessible sizes for those arrays, for the index macro i4()
	d1nphalfJSIZE = iSize;
	d1nphalfKSIZE = jSize;
	d1nphalfLSIZE = kSize;
	d2nphalfJSIZE = iSize;
	d2nphalfKSIZE = jSize;
	d2nphalfLSIZE = kSize;

	//d1nphalfnewJSIZE = iSize;
	//d1nphalfnewKSIZE = jSize;
	//d1nphalfnewLSIZE = kSize;
	//d2nphalfnewJSIZE = iSize;
	//d2nphalfnewKSIZE = jSize;
	//d2nphalfnewLSIZE = kSize;

	d1nmhalfJSIZE = iSize;
	d1nmhalfKSIZE = jSize;
	d1nmhalfLSIZE = kSize;
	d2nmhalfJSIZE = iSize;
	d2nmhalfKSIZE = jSize;
	d2nmhalfLSIZE = kSize;

	//Initialize d1nphalf, etc. with valid pointers
	d1nphalf = d1n1;
	d1nmhalf = d1n2;
	//d1nphalfnew = d1n3;
	d2nphalf = d2n1;
	d2nmhalf = d2n2;
	//d2nphalfnew = d2n3;

	this->ax = params.ax;
	this->n2 = params.n2;
	this->epsilon0 = params.epsilon0;

	deltadim = this->c * this->deltaTime / this->h;
	pmlupdated2factor = deltadim * deltadim / this->n2;

	sigmamax = this->n2 * this->epsilon0 * this->c * ((float)this->m+1.0f) * log(this->R0) / (2.0f * (float)this->na * this->h);
}



FDTD::~FDTD(void)
{
	delete[] pn1;
	//pn1 = NULL;
	delete[] pn2;
	//pn2 = NULL;
	delete[] pn3;
	//pn3 = NULL;
	
	//Clean up PML arrays
	delete[] d1n1;
	delete[] d1n2;
	//delete[] d1n3;
	delete[] d2n1;
	delete[] d2n2;
	//delete[] d2n3;

	delete[] laplacian;

	delete[] obstaclemap;
	delete[] obstaclemap_heightmap;
}

void FDTD::runSim()
{
	if (this->view == NULL)
	{
		cout << "This Model's View is not configured. Make sure you call ->setView(__) before starting the sim." << endl;
		return;
	}

	this->runSimConfig();

	//Load input sound
#ifdef VERBOSE
	cout << "Loading input sound..." << endl;
#endif
	//string soundfilename = "../../matlab/sounds/john_reading_1_fullspectrum_18s.wav";
	//string soundfilename = "0_IMPULSE.wav";
	//Get the Sample Rate of the file
	int soundsamplerate = wavGetSampleRate(this->soundfile);
	size_t sounddatalen = 0;
	//wavLoad(this->soundfile, NULL, 0, sounddatalen, soundsamplerate);
	//Allocate space for the data
	size_t allocated_sounddata_size = (size_t)ceil((float)soundsamplerate*this->duration);
	short* sounddatashort = new short[allocated_sounddata_size];
	wavLoad(this->soundfile, sounddatashort, allocated_sounddata_size, sounddatalen, soundsamplerate);


	if (sounddatalen > allocated_sounddata_size)
	{
		//cout << "Input sound data length is longer than allocated_sounddata_size." << endl;
		sounddatalen = allocated_sounddata_size;
	}
	float* sounddata = new float[sounddatalen];
	for (size_t i = 0; i < sounddatalen; i++)
	{
		sounddata[i] = (float)sounddatashort[i] / std::numeric_limits<short>::max();
	}
	delete[] sounddatashort;	

#ifdef VERBOSE
	cout << "Beginning simulation..." << endl;
#endif
	//cout << iSize << endl;

	this->expansionCount = 0;

	string computationProfile = "";

	//Simulation time loop
	//float version drifted due to error
	//for (float simtime = 0.0f; simtime <= duration; simtime += deltaTime)
	clock_t t0 = clock();	//for all timing; this just marks the beginning of overall execution
	for (count = 0; count < listenlen; count++)
	{
		simtime = (float)count * deltaTime;

		//Update the simulation state
		updateState(simtime);

		//Sound source
		if (simtime < this->inputduration)
		{
			//i3(this->pnp1,sourcei,sourcej,sourcek) = 1.0f * (float)sin(simtime * simtime * 2.0f * math_pi * 100.0f);
			float timetimessamplerate = simtime * (float)soundsamplerate;
			size_t soundindex = (size_t)timetimessamplerate;
			//cout << (size_t)(13.6f) << endl;
			if (soundindex < sounddatalen)
			{
				i3(this->pnp1,sourcei,sourcej,sourcek) += 
				90.0f * sounddata[soundindex];
				//i3(this->pn1,sourcei,sourcej,sourcek) = 
				//sounddata[soundindex];
			}
		}

		//Record to listen array
		this->listen[count] = i3(pnp1,mici,micj,mick);

		timeshiftState();

		//If it's time to update the view, update it.
		if (count % plotinterval == 0)
		{
			this->view->updateView();
			this->soundview->updateView();
		}

		//Show runtime estimation
#define RUNTIME_ESTIMATION_FRAMES 100
		if (count == RUNTIME_ESTIMATION_FRAMES)
		{
			clock_t t_re = clock() - t0;
			float frametime = (float)t_re / (float)CLOCKS_PER_SEC / (float)RUNTIME_ESTIMATION_FRAMES;
			if (!this->MOVIEMODE)
			{
				this->plotinterval = (int)ceil(this->realtimeplotinterval / frametime);
			}
			float seconds = frametime * (float)listenlen;
			float minutes = frametime * (float)listenlen / 60.0f;
			float hours = frametime * (float)listenlen / 3600.0f;
			if (seconds < 60)
				cout << "Estimated simulation time (seconds): " << seconds << endl;
			else if (minutes < 60)
				cout << "Estimated simulation time (minutes): " << minutes << endl;
			else
				cout << "Estimated simulation time (hours): " << hours << endl;
		}

		char strbuf[100];
		sprintf_s(strbuf, 100, "%i", this->expansionCount);
		string strbufstr(strbuf);
		computationProfile += strbufstr + "\n";

		if (shouldStop)
			break;
	}
	clock_t t1 = clock() - t0;
	float exectime = (float)t1 / (float)CLOCKS_PER_SEC;

	cout << /*endl << */"Sim time: " << exectime << endl /*<< endl*/;
	
	string tempstring;
	strprintf_s(tempstring, 1000, "%f,%f,%f,%f,%f,%f,%i,%f,%f", this->xDim,this->yDim,this->zDim,this->duration,exectime,this->maxfreq,this->na,this->h,this->Cmax);
	RLOG::appendTo(this->resultFilename, tempstring);

	float expansions = (float)this->iSize * (float)this->jSize * (float)this->kSize * (float)count;
	cout << "Expansions: " << (100.0f * (float)expansionCount / expansions) << "% " << endl /*<< endl*/;

	strprintf_s(tempstring, 100, ",%u,%f\n", expansionCount, expansions);
	RLOG::appendTo(this->resultFilename, tempstring);

	ofstream fileout("profile.txt");
	fileout << computationProfile << endl;
	fileout.close();

	//cout << "Saving wav file from listen data..." << endl;
	//Find the max and min values
	float* listenscaled = new float[this->listenlen];
	short* listenscaledshort = new short[this->listenlen];
	float listenmin = 0;//std::numeric_limits<float>::max();
	float listenmax = 0;//std::numeric_limits<float>::min();
	//cout << "Listenlen: " << listenlen << endl;
	for (size_t i = 0; i < this->listenlen; i++)
	{
		if (listenmin < this->listen[i])
			listenmin = this->listen[i];
		if (listenmax > this->listen[i])
			listenmax = this->listen[i];
		//if (listen[i] > 10000.0f || listen[i] < -10000.0f)
			//cout << "Bad i: " << i << endl;
	}
	float scalefactor = 1.0f;//1.0f / max(abs(listenmin), abs(listenmax));
	//cout << "Scale factor " << scalefactor << " " << listenmin << " " << listenmax << endl;
	for (size_t i = 0; i < listenlen; i++)
	{
		listenscaled[i] = scalefactor*listen[i];
		listenscaledshort[i] = (short)(listenscaled[i] * (float)std::numeric_limits<short>::max());
	}


	//Save data as a wav file
	string filename = "";
	char stringbuf[1000];

	tm thetime;
	__time64_t longtime;

	_time64(&longtime);
	_localtime64_s(&thetime,&longtime);

	sprintf_s<1000>(stringbuf,"%04i-%02i-%02i-%02i-%02i-%02i",thetime.tm_year+1900,thetime.tm_mon+1,thetime.tm_mday,thetime.tm_hour,thetime.tm_min,thetime.tm_sec);
	filename = stringbuf;

	string simparameters;
	sprintf_s<1000>(stringbuf,"x=%.4g y=%.4g z=%.4g d=%.4g t=%.4g f=%.4g n=%i h=%.4g c=%.4g",
		this->xDim,this->yDim,this->zDim,this->duration,exectime,this->maxfreq,this->na,this->h,this->Cmax);
	simparameters = stringbuf;
	filename = filename + " " + simparameters;

	filename = filename + ".wav";

	//filename override
	if (this->outputfile != "")
		filename = this->outputfile;

	//cout << filename << endl;
	wavSave(filename, listenscaledshort, this->listenlen, (int)(1.0f / deltaTime));

	//delete temporary copy of listen
	delete[] listenscaled;
	delete[] listenscaledshort;

	delete[] sounddata;

}