#ifndef FDTD_1E_KBESTH
#define FDTD_1E_KBESTH

#include "FDTD.h"

#include <list>
#include <vector>
#include <queue>

//#include "sbpl/heap.h"

class FDTD_1E_KBEST : public FDTD
{

public:
	bool ABSOLUTE_LIMIT;
	int ABSOLUTE_NUMBER;
	bool WAVEVOLUME_LIMIT;
	float WAVEVOLUME_LIMIT_FACTOR;

protected:

	inline virtual void updateCellPhase1(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateCellPhase2(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateState(float time);

	float threshold;
	inline virtual float metric(unsigned int i, unsigned int j, unsigned int k);

	vector<IndexMetricTuple*> candidates;
	//list<IndexMetricTuple*> updateList;
	IndexMetricTuple* indexarr;
	size_t indexarrJSIZE;
	size_t indexarrKSIZE;

	IndexMetricTuple** updateList;
	size_t updateListCount;

	IndexMetricTuple** calcthese;
	//size_t calctheseCount;

public:

	FDTD_1E_KBEST() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD_1E_KBEST constructor called. You might not want this!" << endl;
	}

	FDTD_1E_KBEST(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1eheapparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
	{
		this->ABSOLUTE_LIMIT = fdtd1eheapparams.ABSOLUTE_LIMIT;
		this->ABSOLUTE_NUMBER = fdtd1eheapparams.ABSOLUTE_NUMBER;
		this->WAVEVOLUME_LIMIT = fdtd1eheapparams.WAVEVOLUME_LIMIT;
		this->WAVEVOLUME_LIMIT_FACTOR = fdtd1eheapparams.WAVEVOLUME_LIMIT_FACTOR;

		//allocate index state array so it's not always rebuilt
		indexarr = new IndexMetricTuple[this->iSize * this->jSize * this->kSize];
		this->indexarrJSIZE = this->pnJSIZE;
		this->indexarrKSIZE = this->pnKSIZE;
		//initialize it
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					IndexMetricTuple* thisindtup = &i3(this->indexarr, i,j,k);
					thisindtup->i = i;
					thisindtup->j = j;
					thisindtup->k = k;
					thisindtup->metric = INFINITECOST;
					//thisindtup->neighborsUpdated = 0; //Not needed because it isn't used until after it's cleared in the frame update anyway
				}
			}
		}

		//put some of the cells into the candidate list to seed the simulation
		//put all or just the ones around the source(s)
		//put all
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					//put some in candidate list
					IndexMetricTuple* thisindtup = &i3(this->indexarr, i,j,k);

					if ((i3(this->obstaclemap,i,j,k)&1u))
						this->candidates.push_back(thisindtup);
				}
			}
		}

		//Put them all in the update list so they all get processed once
		//TODO:
		//OPTIMIZE
		//alternatively put only the neighbors of the source cell in here to start
		this->updateList = new IndexMetricTuple*[this->iSize*this->jSize*this->kSize * 10];

		this->updateListCount = 0;
		for (size_t i = 0; i < this->iSize*this->jSize*this->kSize; i++)
		{
			this->updateList[i] = this->indexarr+i;
		}

		this->calcthese = new IndexMetricTuple*[this->iSize*this->jSize*this->kSize];
		//calctheseCount = 0;
	}

	virtual ~FDTD_1E_KBEST(void)
	{
		delete[] this->calcthese;
		delete[] this->updateList;
		delete[] this->indexarr;
	}

	static bool cmpLT_indexmetrictuple(IndexMetricTuple* &a, IndexMetricTuple* &b)
	{
		return a->metric < b->metric;
	}

};


float FDTD_1E_KBEST::metric(unsigned int i, unsigned int j, unsigned int k)
{
	//return (float)rand() / (float)RAND_MAX;

	//return -((float)i-(float)this->sourcei)*((float)i-(float)this->sourcei) -
	//	   ((float)j-(float)this->sourcej)*((float)j-(float)this->sourcej) -
	//	   ((float)k-(float)this->sourcek)*((float)k-(float)this->sourcek);


	//Laplacian
	//return i3(this->laplacian,i,j,k);

	////Sum of absolute values of neighbors
	//float returnme = 0.0f;
	//returnme += abs(i3(this->pn,i,j,k));
	//
	//returnme += i>0?abs(i3(this->pn,i-1,j,k)):0.0f;
	//returnme += j>0?abs(i3(this->pn,i,j-1,k)):0.0f;
	//returnme += k>0?abs(i3(this->pn,i,j,k-1)):0.0f;
	//returnme += i+1<iSize-1?abs(i3(this->pn,i+1,j,k)):0.0f;
	//returnme += j+1<jSize-1?abs(i3(this->pn,i,j+1,k)):0.0f;
	//returnme += k+1<kSize-1?abs(i3(this->pn,i,j,k+1)):0.0f;

	////float cellposi = ((float)i-(float)na)*h-this->sourcex;
	////float cellposj = ((float)j-(float)na)*h-this->sourcey;
	////float cellposk = ((float)k-(float)na)*h-this->sourcez;
	////returnme -= abs(simtime-(sqrt(cellposi*cellposi+cellposj*cellposj+cellposk*cellposk)/c));
	////returnme = max(0,returnme);

	//return returnme;


	////ability to reach end
	float adjustment = 1.0f;
	float cellposi = (float)i-(float)this->mici;
	float cellposj = (float)j-(float)this->micj;
	float cellposk = (float)k-(float)this->mick;
	float dist = sqrt(cellposi*cellposi+cellposj*cellposj+cellposk*cellposk) * h;
	float timedist = dist / this->c;
	if (timedist > (this->duration-this->simtime))
		adjustment = 0.0f;


	//Absolute value of largest difference
	float min = FLT_MAX;
	float max = FLT_MIN;
	float center = i3(this->pn,i,j,k)/6.0f;
	float neighbors[] = {center,
	i>0?i3(this->pn,i-1,j,k):center,
	j>0?i3(this->pn,i,j-1,k):center,
	k>0?i3(this->pn,i,j,k-1):center,
	i+1<iSize-1?i3(this->pn,i+1,j,k):center,
	j+1<jSize-1?i3(this->pn,i,j+1,k):center,
	k+1<kSize-1?i3(this->pn,i,j,k+1):center
	};
	for (int a = 0; a < 7; a++)
	{
		if (neighbors[a] < min)
			min = neighbors[a];
		if (neighbors[a] > max)
			max = neighbors[a];
	}
	return adjustment*(max-min);// * (i3(this->pn,i,j,k) < 0.0f ? 0.0f : 1.0f);
}

void FDTD_1E_KBEST::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase1(i, j, k);
	this->updateObstacleMaskAndLaplacian(i,j,k);
}

void FDTD_1E_KBEST::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase2(i, j, k);
}

void FDTD_1E_KBEST::updateState(float time)
{
	//Update metrics so the correct "most important" cells are pulled later
	for (size_t i = 0; i < this->updateListCount; i++)
	{
		if (updateList[i]->metricMeasuredAt == this->count)
			continue;
		updateList[i]->metricMeasuredAt = this->count;

		float metricv = this->metric(this->updateList[i]->i,this->updateList[i]->j,this->updateList[i]->k);

		//update value stored in list
		this->updateList[i]->metric = static_cast<long>(1 + INFINITECOST - (metricv * 0.1f * INFINITECOST));

#ifdef DEBUG
		if (this->updateList[i]->metric < 0)
			cout << metricv << "  " << this->updateList[i]->metric << endl;
#endif
	}
	//reset update list now that it's done being processed
	this->updateListCount = 0;


	//Calculate the number of cells to compute this frame based on estimated wavefront volume
	size_t numberToExpand = 0;
	//if (this->ABSOLUTE_LIMIT)
	//{
	//	numberToExpand = ABSOLUTE_NUMBER;
	//}
	//else if (this->WAVEVOLUME_LIMIT)
	//{
		float pulseCenterTime = WAVEOFFSET;
		float pulseHalfWidthTime = pulseCenterTime;
		float pulseHalfWidthWorldUnits = pulseHalfWidthTime * this->c;
		float timeWaveCenter = this->simtime - pulseCenterTime;
		//float timeAfterFirstExpansion = timeWaveCenter + pulseHalfWidthTime;

		float rcenter = this->c * timeWaveCenter;
		float ro3 = rcenter + pulseHalfWidthWorldUnits; ro3 = ro3 * ro3 * ro3;
		float ri3 = rcenter - pulseHalfWidthWorldUnits; ri3 = ri3 * ri3 * ri3;
		ri3 = max(ri3,0); ro3 = max(ro3,0);	//limit to 0; no negative area
		float volume = (4.0f / 3.0f) * PI * (ro3 - ri3);
		float volume_in_cells = volume / (this->h * this->h * this->h);
		volume_in_cells *= this->WAVEVOLUME_LIMIT_FACTOR;
		volume_in_cells = max(0, volume_in_cells);
		numberToExpand = static_cast<size_t>(volume_in_cells);
	//}
	//else
	//{
	//	cout << "Something is wrong with the configuration." << endl;
	//	this->shouldStop = true;
	//}

#ifdef DEBUG
		//if (this->count % 10 == 0)
	//debug highlighting
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));
#endif


	//TODO: HACK FIX LATER
	//If numberToExpand is <7, bump it up to 7
	if (numberToExpand < 7) numberToExpand = 7;

	//Also, force the source & neighbor cells to be important through the whole wave period
	if (this->simtime < 1.0f * WAVEOFFSET)
	{
		i3(indexarr,this->sourcei,this->sourcej,this->sourcek).metric = 0;
		i3(indexarr,this->sourcei-1,this->sourcej,this->sourcek).metric = 0;
		i3(indexarr,this->sourcei+1,this->sourcej,this->sourcek).metric = 0;
		i3(indexarr,this->sourcei,this->sourcej-1,this->sourcek).metric = 0;
		i3(indexarr,this->sourcei,this->sourcej+1,this->sourcek).metric = 0;
		i3(indexarr,this->sourcei,this->sourcej,this->sourcek-1).metric = 0;
		i3(indexarr,this->sourcei,this->sourcej,this->sourcek+1).metric = 0;
	}


	//SORT candidates
	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,this->candidates.size());
	std::nth_element(this->candidates.begin(),this->candidates.begin()+numberToExpand,this->candidates.end(),FDTD_1E_KBEST::cmpLT_indexmetrictuple);



	//may empty out part of the candidate list here
	//this->candidates.erase(this->candidates.begin()+numberToExpand,this->candidates.end());


	//clear neighbor counts.
	//you may be able to remove this later and instead clear neighbor counters only when they go over 4.
	//also, this can be TBB'd
	//TODO
	//OPTIMIZE
	for (size_t i = 0; i < this->iSize*this->jSize*this->kSize; i++)
	{
		this->indexarr[i].neighborsUpdated = 0;
	}

	//UPDATE IMPORTANT CELLS
	for (size_t i = 0; i < numberToExpand; i++)
	{
		IndexMetricTuple* calcthis;
		calcthis = this->candidates[i];

		if (calcthis->metric >= INFINITECOST)
		{
			calcthese[i] = NULL;
			continue;
		}
		else
		{
			calcthese[i] = calcthis;
		}

#ifdef DEBUG
		if (calcthis->computedAt == this->count)
			cout << "ehhh" << endl;
		//Mark that this cell has been selected once already this frame
		calcthis->computedAt = this->count;
#endif

		//neighbor counters
		//ever time an expansion is made, we have to indicate to its neighbors that a neighbor was expanded
		if (calcthis->i > 0)
		{
			i3(this->indexarr,calcthis->i-1,calcthis->j,calcthis->k).neighborsUpdated++;
		}
		if (calcthis->i < this->iSize-1)
		{
			i3(this->indexarr,calcthis->i+1,calcthis->j,calcthis->k).neighborsUpdated++;
		}
		if (calcthis->j > 0)
		{
			i3(this->indexarr,calcthis->i,calcthis->j-1,calcthis->k).neighborsUpdated++;
		}
		if (calcthis->j < this->jSize-1)
		{
			i3(this->indexarr,calcthis->i,calcthis->j+1,calcthis->k).neighborsUpdated++;
		}
		if (calcthis->k > 0)
		{
			i3(this->indexarr,calcthis->i,calcthis->j,calcthis->k-1).neighborsUpdated++;
		}
		if (calcthis->k < this->kSize-1)
		{
			i3(this->indexarr,calcthis->i,calcthis->j,calcthis->k+1).neighborsUpdated++;
		}

#ifdef DEBUG
		//highlight updated cells
		//if (this->count % 10 == 0)
		if (calcthis->k == this->mick)
			i2(this->view->highlightThese,calcthis->i,calcthis->j) = true;
#endif

		this->updateCellPhase1(calcthis->i,calcthis->j,calcthis->k);
		//this->updateObstacleMaskAndLaplacian(calcthis->i,calcthis->j,calcthis->k);
	}


	//how to treat neighbors
	//The only need to be considered if it's on the wave frontier
	//While adding above, a counter could be incremented in each IndexMetricTuple structure each time one of its neigbors is chosen
	//Then, if the counter is not 4, add neighbors here. This will create some duplicates but not many

	//Loop over expanded nodes, adding them and their neighbors to the updateList
	for (size_t i = 0; i < numberToExpand; i++)
	{
		if (calcthese[i] == NULL)
			continue;

		//add updated cell
		this->updateList[this->updateListCount++] = calcthese[i];

		//add the important neighbors too
		IndexMetricTuple* neighbor;

		const int lim = 6;
		if (calcthese[i]->i > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i-1,calcthese[i]->j,calcthese[i]->k);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->i < this->iSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i+1,calcthese[i]->j,calcthese[i]->k);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}

		if (calcthese[i]->j > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j-1,calcthese[i]->k);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->j < this->jSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j+1,calcthese[i]->k);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}

		if (calcthese[i]->k > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j,calcthese[i]->k-1);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->k < this->kSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j,calcthese[i]->k+1);
			if (neighbor->neighborsUpdated < lim)
				this->updateList[this->updateListCount++] = neighbor;
		}
	}


	//D2 Update
	for (size_t i = 0; i < numberToExpand; i++)
	{
		IndexMetricTuple* calcthis = calcthese[i];
		if (calcthis == NULL)
			continue;

		this->updateCellPhase2(calcthis->i,calcthis->j,calcthis->k);
	}
	

	//ADD TO CANDIDATES FROM UPDATELIST?




}



#endif