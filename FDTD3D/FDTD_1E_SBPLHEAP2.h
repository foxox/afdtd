#ifndef FDTD_1E_SBPLHEAP2H
#define FDTD_1E_SBPLHEAP2H

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

class FDTD_1E_SBPLHEAP2 : public FDTD
{

public:
	struct IndexTuple : AbstractSearchState
	{
		//float metric;
		unsigned int i;
		unsigned int j;
		unsigned int k;
		//uint_fast16_t neighborsUpdated;
	};

	//struct Metric : CKey
	//{
	//	float metric;
	//};
	typedef CKey Metric;

	bool ABSOLUTE_LIMIT;
	int ABSOLUTE_NUMBER;
	bool WAVEVOLUME_LIMIT;
	float WAVEVOLUME_LIMIT_FACTOR;
	//const int numToCalc;

protected:

	inline virtual void updateCellPhase1(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateCellPhase2(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateState(float time);

	float threshold;
	inline virtual float metric(unsigned int i, unsigned int j, unsigned int k);

	//Give the TBB loop functor access to threshold
	//friend class tbbTask_PQueueGen;

	//Functor for TBB loop
	//class tbbTask_PQueueGen
	//{
	//protected:
	//	FDTD_1E_SBPLHEAP2* fdtd;

	//public:
	//	tbbTask_PQueueGen(FDTD_1E_SBPLHEAP2* _fdtd) : fdtd(_fdtd) {};
	//	~tbbTask_PQueueGen(void) {};

	//	inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	//};

	//tbbTask_PQueueGen tbbTask_PQueueGen;


	//priority_queue<IndexTuple, vector<IndexTuple>, less<vector<IndexTuple>::value_type> > heap;
	CHeap heap;
	IndexTuple* indexarr;
	size_t indexarrJSIZE;
	size_t indexarrKSIZE;
	
	IndexTuple** calcthese;
	size_t calctheseCount;

public:

	FDTD_1E_SBPLHEAP2() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD_1E_SBPLHEAP2 constructor called. You might not want this!" << endl;
	}

	FDTD_1E_SBPLHEAP2(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1eheapparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
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

					//place in heap
					Metric met;
					met.key[0] = 0;
					this->heap.insertheap(thisindtup,met);
				}
			}
		}

		this->calcthese = new IndexTuple*[this->iSize*this->jSize*this->kSize];
		calctheseCount = 0;
	}

	virtual ~FDTD_1E_SBPLHEAP2(void)
	{
		delete[] this->indexarr;
		delete[] this->calcthese;
	}

};



float FDTD_1E_SBPLHEAP2::metric(unsigned int i, unsigned int j, unsigned int k)
{
	//return (float)rand() / (float)RAND_MAX;

	//return -((float)i-(float)this->sourcei)*((float)i-(float)this->sourcei) -
	//	   ((float)j-(float)this->sourcej)*((float)j-(float)this->sourcej) -
	//	   ((float)k-(float)this->sourcek)*((float)k-(float)this->sourcek);


	//Laplacian
	//return i3(this->laplacian,i,j,k);

	//Sum of absolute values of neighbors
	//float returnme = 0.0f;
	//returnme += abs(i3(this->pn,i,j,k));
	//
	//returnme += i>0?abs(i3(this->pn,i-1,j,k)):0.0f;
	//returnme += j>0?abs(i3(this->pn,i,j-1,k)):0.0f;
	//returnme += k>0?abs(i3(this->pn,i,j,k-1)):0.0f;
	//returnme += i+1<iSize-1?abs(i3(this->pn,i+1,j,k)):0.0f;
	//returnme += j+1<jSize-1?abs(i3(this->pn,i,j+1,k)):0.0f;
	//returnme += k+1<kSize-1?abs(i3(this->pn,i,j,k+1)):0.0f;
	//return returnme;

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
	for (int i = 0; i < 7; i++)
	{
		if (neighbors[i] < min)
			min = neighbors[i];
		if (neighbors[i] > max)
			max = neighbors[i];
	}
	return max-min;
}

void FDTD_1E_SBPLHEAP2::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	//if (i < this->na || j < this->na || k < this->na || i >= this->iSize - this->na || j >= this->jSize - this->na || k >= this->kSize - this->na
	//||(i3(pn,i,j,k) >= this->threshold)
	//)
	{
		FDTD::updateCellPhase1(i, j, k);
		this->updateObstacleMaskAndLaplacian(i,j,k);
	}
}

void FDTD_1E_SBPLHEAP2::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	//if (i < this->na || j < this->na || k < this->na || i >= this->iSize - this->na || j >= this->jSize - this->na || k >= this->kSize - this->na
	// ||(i3(pn,i,j,k) >= this->threshold)
	//||
	//if ( this->metric(i,j,k) >= this->threshold	)
	{
		FDTD::updateCellPhase2(i, j, k);
	}
	//else
	//{
		//cout << 's';
		//this->expansionCount++;
	//}
}

void FDTD_1E_SBPLHEAP2::updateState(float time)
{
	//REFILL HEAP METHOD
	this->heap.makeemptyheap();
	for (size_t i = 0; i < this->iSize * this->jSize * this->kSize; i++)
	{
		float metricv = this->metric(this->indexarr[i].i,this->indexarr[i].j,this->indexarr[i].k);
		Metric metric;
		metric.key[0] = static_cast<long>(INFINITECOST - (metricv * INFINITECOST));
		this->heap.insertheap(&this->indexarr[i],metric);
	}

	size_t numberToExpand = 0;
	if (this->ABSOLUTE_LIMIT)
	{
		numberToExpand = ABSOLUTE_NUMBER;
	}
	else if (this->WAVEVOLUME_LIMIT)
	{
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
	}
	else
	{
		cout << "Something is wrong with the configuration." << endl;
		this->shouldStop = true;
	}

	//debug highlighting
#ifdef DEBUG
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));
#endif

	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,heap.currentsize);
	//IndexTuple** calcthese = new IndexTuple*[numberToExpand];
	this->calctheseCount = 0;
	for (size_t i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis;
		Metric met;
		calcthis = (IndexTuple*)this->heap.getminheap(met);
		this->heap.deleteminheap();
		calcthese[i] = calcthis;

#ifdef DEBUG
		//highlight updated cells
		if (calcthis->k == this->mick)
			i2(this->view->highlightThese,calcthis->i,calcthis->j) = true;
#endif

		this->updateCellPhase1(calcthis->i,calcthis->j,calcthis->k);

		//this->updateObstacleMaskAndLaplacian(calcthis->i,calcthis->j,calcthis->k);
	}

	//D2 Update
	for (int i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis = calcthese[i];
		this->updateCellPhase2(calcthis->i,calcthis->j,calcthis->k);
	}
}

#endif