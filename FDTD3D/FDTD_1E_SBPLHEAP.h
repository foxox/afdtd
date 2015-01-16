#ifndef FDTD_1E_SBPLHEAPH
#define FDTD_1E_SBPLHEAPH

#include "FDTD_TBB.h"

#include <list>
#include <vector>
#include <queue>

#include "sbpl/heap.h"

//struct FDTD1EPQUEUEParams
//{
//	bool ABSOLUTE_LIMIT;
//	int ABSOLUTE_NUMBER;
//	bool WAVEVOLUME_LIMIT;
//	float WAVEVOLUME_LIMIT_FACTOR;
//};

class FDTD_1E_SBPLHEAP : public FDTD
{

public:
	struct IndexTuple : AbstractSearchState
	{
		//float metric;
		unsigned int i;
		unsigned int j;
		unsigned int k;
		uint_fast16_t neighborsUpdated;
		size_t metricMeasuredAt;
#ifdef DEBUG
		size_t computedAt;
#endif
	};

	typedef CKey Metric;

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

	CHeap heap;
	IndexTuple* indexarr;
	size_t indexarrJSIZE;
	size_t indexarrKSIZE;
	
	IndexTuple** updateList;
	size_t updateListCount;

	IndexTuple** calcthese;

public:

	FDTD_1E_SBPLHEAP() : FDTD()
	{
		cout << "Default FDTD_1E_SBPLHEAP constructor called. You might not want this!" << endl;
	}

	FDTD_1E_SBPLHEAP(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1eheapparams) : FDTD(modelparams, fdtdparams)
	{
		this->ABSOLUTE_LIMIT = fdtd1eheapparams.ABSOLUTE_LIMIT;
		this->ABSOLUTE_NUMBER = fdtd1eheapparams.ABSOLUTE_NUMBER;
		this->WAVEVOLUME_LIMIT = fdtd1eheapparams.WAVEVOLUME_LIMIT;
		this->WAVEVOLUME_LIMIT_FACTOR = fdtd1eheapparams.WAVEVOLUME_LIMIT_FACTOR;

		//allocate index state array so it's not always rebuilt
		indexarr = new IndexTuple[this->iSize * this->jSize * this->kSize];
		this->indexarrJSIZE = this->pnJSIZE;
		this->indexarrKSIZE = this->pnKSIZE;
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					IndexTuple* thisindtup = &i3(this->indexarr, i,j,k);
					thisindtup->i = i;
					thisindtup->j = j;
					thisindtup->k = k;
					thisindtup->heapindex = 0;
					thisindtup->metricMeasuredAt = -1;
#ifdef DEBUG
					thisindtup->computedAt = -1;	//needed because init value could happen to be 0 by random chance
#endif
					//thisindtup->neighborsUpdated = 0; //Not needed because it isn't used until after it's cleared in the frame update anyway

					//place in heap
					Metric met;
					met.key[0] = INFINITECOST; //initialize high so they are unimportant by default
					this->heap.insertheap(thisindtup,met);
				}
			}
		}

		//Put them all in the update list so they all get processed once
		//TODO:
		//OPTIMIZE
		//alternatively put only the neighbors of the source cell in here to start
		this->updateList = new IndexTuple*[this->iSize*this->jSize*this->kSize * 10]; 

		this->updateListCount = 0;
		for (size_t i = 0; i < this->iSize*this->jSize*this->kSize; i++)
		{
			this->updateList[i] = this->indexarr+i;
		}

		this->calcthese = new IndexTuple*[this->iSize*this->jSize*this->kSize];
	}

	virtual ~FDTD_1E_SBPLHEAP(void)
	{
		delete[] this->indexarr;
		delete[] this->updateList;
		delete[] this->calcthese;
	}

};



float FDTD_1E_SBPLHEAP::metric(unsigned int i, unsigned int j, unsigned int k)
{
	//return (float)rand() / (float)RAND_MAX;

	//return -((float)i-(float)this->sourcei)*((float)i-(float)this->sourcei) -
	//	   ((float)j-(float)this->sourcej)*((float)j-(float)this->sourcej) -
	//	   ((float)k-(float)this->sourcek)*((float)k-(float)this->sourcek);


	//Laplacian
	//return i3(this->laplacian,i,j,k);

	//Sum of absolute values of neighbors
	float returnme = 0.0f;
	returnme += abs(i3(this->pn,i,j,k));
	
	returnme += i>0?abs(i3(this->pn,i-1,j,k)):0.0f;
	returnme += j>0?abs(i3(this->pn,i,j-1,k)):0.0f;
	returnme += k>0?abs(i3(this->pn,i,j,k-1)):0.0f;
	returnme += i+1<iSize-1?abs(i3(this->pn,i+1,j,k)):0.0f;
	returnme += j+1<jSize-1?abs(i3(this->pn,i,j+1,k)):0.0f;
	returnme += k+1<kSize-1?abs(i3(this->pn,i,j,k+1)):0.0f;
	return returnme;

	//Absolute value of largest difference
	//float min = FLT_MAX;
	//float max = FLT_MIN;
	//float center = i3(this->pn,i,j,k)/6.0f;
	//float neighbors[] = {center,
	//i>0?i3(this->pn,i-1,j,k):center,
	//j>0?i3(this->pn,i,j-1,k):center,
	//k>0?i3(this->pn,i,j,k-1):center,
	//i+1<iSize-1?i3(this->pn,i+1,j,k):center,
	//j+1<jSize-1?i3(this->pn,i,j+1,k):center,
	//k+1<kSize-1?i3(this->pn,i,j,k+1):center
	//};
	//for (int i = 0; i < 7; i++)
	//{
	//	if (neighbors[i] < min)
	//		min = neighbors[i];
	//	if (neighbors[i] > max)
	//		max = neighbors[i];
	//}
	//return max-min;
}

void FDTD_1E_SBPLHEAP::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase1(i, j, k);
	this->updateObstacleMaskAndLaplacian(i,j,k);
}

void FDTD_1E_SBPLHEAP::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase2(i, j, k);
}

void FDTD_1E_SBPLHEAP::updateState(float time)
{
	//REFILL HEAP METHOD
	//this->heap.makeemptyheap();
	//for (i = 0; i < this->iSize * this->jSize * this->kSize; i++)
	//{
	//	float metricv = this->metric(this->indexarr[i].i,this->indexarr[i].j,this->indexarr[i].k);
	//	Metric metric;
	//	metric.key[0] = static_cast<long>(INFINITECOST - (metricv * INFINITECOST));
	//	this->heap.insertheap(&this->indexarr[i],metric);
	//}



	//Go through updatelist and update them in the heap so the correct things are pulled from the min spot in the coming calculations
	//for (list<IndexTuple*>::iterator it = this->updateList.begin(); it != this->updateList.end(); it++)
	for (size_t i = 0; i < this->updateListCount; i++)
	{
		//avoid recalculating things unnecessarily
		if (updateList[i]->metricMeasuredAt == this->count)
			continue;
		updateList[i]->metricMeasuredAt = this->count;

		//calculate new metric
		float metricv = this->metric(this->updateList[i]->i,this->updateList[i]->j,this->updateList[i]->k);
		Metric metric; metric.key[0] = static_cast<long>(1 + INFINITECOST - (metricv * 0.1f * INFINITECOST));

#ifdef DEBUG
		if (metric.key[0] < 0)
			cout << metricv << "  " << metric.key[0] << endl;
#endif

		//attempt to make cells important if they have been computed recently
		//if (this->count - updateList[i]->computedAt < 2)
		//	metric.key[0] = 1;

		if (updateList[i]->heapindex == 0)
			this->heap.insertheap(updateList[i],metric);
		else
			this->heap.updateheap(updateList[i],metric);
	}
	//reset update list now that it's done being processed
	this->updateListCount = 0;


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
	//debug highlighting
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));
#endif

	//TODO: HACK FIX LATER
	//If numberToExpand is <7, bump it up to 7
	if (numberToExpand < 7) numberToExpand = 7;

#ifdef DEBUG
	if (this->heap.currentsize < 7)
		cout << "heap too small" << endl;
#endif

	//Also, force the source & neighbor cells to be important through the whole wave period
	if (this->count > 0 && this->simtime < 2.0f * WAVEOFFSET)
	{
		Metric newkeymet;
		//newkeymet.key[0] = 0;
		newkeymet.key[0] = INFINITECOST - 1000000000;

		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek),newkeymet);

		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei-1,this->sourcej,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei-1,this->sourcej,this->sourcek),newkeymet);
		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei+1,this->sourcej,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei+1,this->sourcej,this->sourcek),newkeymet);

		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej-1,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej-1,this->sourcek),newkeymet);
		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej+1,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej+1,this->sourcek),newkeymet);

		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej,this->sourcek-1).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek-1),newkeymet);
		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej,this->sourcek+1).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek+1),newkeymet);
	}

	//clear neighbor counts.
	//you may be able to remove this later and instead clea r neighbor counters only when they go over 4.
	//also, this can be TBB'd
	//TODO
	//OPTIMIZE
	for (size_t i = 0; i < this->iSize*this->jSize*this->kSize; i++)
	{
		this->indexarr[i].neighborsUpdated = 0;
	}

	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,heap.currentsize);
	//cout << numberToExpand << endl;

	//Update important cells. EXPANSIONS
	for (size_t i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis;
		Metric met;
		calcthis = (IndexTuple*)this->heap.getminheap(met);
		this->heap.deleteminheap();
		calcthese[i] = calcthis;

#ifdef DEBUG
		if (calcthis->computedAt == this->count)
			cout << "ehhh" << endl;
		//Mark that this cell has been selected once already this frame
		calcthis->computedAt = this->count;
#endif

		//neighbor counters
		//every time an expansion is made, we have to indicate to its neighbors that a neighbor was expanded
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
		if (calcthis->k == this->mick)
			i2(this->view->highlightThese,calcthis->i,calcthis->j) = true;
#endif

		this->updateCellPhase1(calcthis->i,calcthis->j,calcthis->k);

		this->updateObstacleMaskAndLaplacian(calcthis->i,calcthis->j,calcthis->k);
	}


	//how to treat neighbors
	//The only need to be considered if it's on the wave frontier
	//While adding above, a counter could be incremented in each IndexTuple structure each time one of its neigbors is chosen
	//Then, if the counter is not 4, add neighbors here. This will create some duplicates but not many

	//Loop over expanded nodes, adding them and their neighbors to the updateList
	for (size_t i = 0; i < numberToExpand; i++)
	{
		//add updated cell
		this->updateList[this->updateListCount++] = calcthese[i];

		//add the important neighbors too
		IndexTuple* neighbor;

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
	for (int i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis = calcthese[i];
		this->updateCellPhase2(calcthis->i,calcthis->j,calcthis->k);
	}
}


#endif