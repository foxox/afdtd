#ifndef FDTD_1E_SBPLHEAP3H
#define FDTD_1E_SBPLHEAP3H

#include "FDTD_TBB.h"

#include <list>
#include <vector>
#include <queue>

#include "sbpl/heap.h"

class FDTD_1E_SBPLHEAP3 : public FDTD
{

public:
	struct IndexTuple : AbstractSearchState
	{
		//float metric;
		unsigned int i;
		unsigned int j;
		unsigned int k;
		uint_fast16_t neighborsUpdated;
		//size_t metricMeasuredAt;
		size_t computedAt;
	};

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


	//priority_queue<IndexTuple, vector<IndexTuple>, less<vector<IndexTuple>::value_type> > heap;
	CHeap heap;
	CHeap heapfull;
	IndexTuple* indexarr;
	size_t indexarrJSIZE;
	size_t indexarrKSIZE;
	IndexTuple* indexarr2;
	size_t indexarr2JSIZE;
	size_t indexarr2KSIZE;
	
	IndexTuple** updateList;
	size_t updateListCount;

	IndexTuple** calcthese;
	//size_t calctheseCount;	//should always match numberToExpand

public:

	FDTD_1E_SBPLHEAP3() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD_1E_SBPLHEAP3 constructor called. You might not want this!" << endl;
	}

	FDTD_1E_SBPLHEAP3(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1eheapparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
	{
		this->ABSOLUTE_LIMIT = fdtd1eheapparams.ABSOLUTE_LIMIT;
		this->ABSOLUTE_NUMBER = fdtd1eheapparams.ABSOLUTE_NUMBER;
		this->WAVEVOLUME_LIMIT = fdtd1eheapparams.WAVEVOLUME_LIMIT;
		this->WAVEVOLUME_LIMIT_FACTOR = fdtd1eheapparams.WAVEVOLUME_LIMIT_FACTOR;

		//allocate index state array so it's not always rebuilt
		indexarr = new IndexTuple[this->iSize * this->jSize * this->kSize];
		this->indexarrJSIZE = this->pnJSIZE;
		this->indexarrKSIZE = this->pnKSIZE;
		indexarr2 = new IndexTuple[this->iSize * this->jSize * this->kSize];
		this->indexarr2JSIZE = this->pnJSIZE;
		this->indexarr2KSIZE = this->pnKSIZE;
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					if (i == 4 && j == 28 && k == 10)
						cout << "weird one" << endl;

					IndexTuple* thisindtup = &i3(this->indexarr, i,j,k);
					thisindtup->i = i;
					thisindtup->j = j;
					thisindtup->k = k;
					thisindtup->heapindex = 0;
					//thisindtup->metricMeasuredAt = -1;
					//thisindtup->computedAt = -1;

					//place in heap
					Metric met;
					met.key[0] = INFINITECOST; //initialize high so the source cells are more important

					this->heap.insertheap(thisindtup,met);

					//and for heapfull
					thisindtup = &i3(this->indexarr2, i,j,k);
					thisindtup->i = i;
					thisindtup->j = j;
					thisindtup->k = k;
					thisindtup->heapindex = 0;
					this->heapfull.insertheap(thisindtup,met);
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

	virtual ~FDTD_1E_SBPLHEAP3(void)
	{
		delete[] this->indexarr;
		delete[] this->indexarr2;
		delete[] this->updateList;
		delete[] this->calcthese;
	}

};



float FDTD_1E_SBPLHEAP3::metric(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_1E_SBPLHEAP3::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase1(i, j, k);
	this->updateObstacleMaskAndLaplacian(i,j,k);
}

void FDTD_1E_SBPLHEAP3::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase2(i, j, k);
}

void FDTD_1E_SBPLHEAP3::updateState(float time)
{
	//UPDATELIST PROCESSING. UPDATE ALL METRICS WHICH MAY HAVE CHANGED
	for (size_t i = 0; i < this->updateListCount; i++)
	{
		float metricv = this->metric(this->updateList[i]->i,this->updateList[i]->j,this->updateList[i]->k);
		Metric metric; metric.key[0] = static_cast<long>(1 + INFINITECOST - (metricv * 0.1f * INFINITECOST));

		if (this->updateList[i]->i == 4 && this->updateList[i]->j == 28 && this->updateList[i]->k == 10)
			cout << "weird one" << endl;

		if (metric.key[0] < 0)
			cout << metricv << "  " << metric.key[0] << endl;

		if (updateList[i]->heapindex == 0)
			this->heap.insertheap(updateList[i],metric);
		else
			this->heap.updateheap(updateList[i],metric);
	}

	//clear updatelist for next round!
	this->updateListCount = 0;



	//ALTERNATIVELY, UPDATE EVERY SINGLE ONE
	for (size_t i = 0; i < this->iSize*this->jSize*this->kSize; i++)
	{
		if (&indexarr2[i] == &i3(indexarr2,4,28,10))
			cout << "weird one" << endl;

		float metricv = this->metric(this->indexarr2[i].i,this->indexarr2[i].j,this->indexarr2[i].k);
		Metric metric; metric.key[0] = static_cast<long>(1 + INFINITECOST - (metricv * 1.0f * INFINITECOST));
		if (this->indexarr2[i].heapindex == 0)
			this->heapfull.insertheap(&indexarr2[i],metric);
		else
			this->heapfull.updateheap(&indexarr2[i],metric);
	}

	

	size_t numberToExpand = 0;
	if (this->WAVEVOLUME_LIMIT)
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

	//If numberToExpand is <7, bump it up to 7
	if (numberToExpand < 7) numberToExpand = 7;

	//Also, force the source & neighbor cells to be important through the whole wave period
	if (this->count > 0 && this->simtime < 2.0f * WAVEOFFSET)
	{
		Metric newkeymet;
		//newkeymet.key[0] = 0;
		newkeymet.key[0] = INFINITECOST - 1000000000;
		//newkeymet.key[0] = this->heap.heap[i3(indexarr,this->sourcei,this->sourcej,this->sourcek).heapindex].key.key[0]-1;
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek),newkeymet);

		this->heap.updateheap(&i3(indexarr,this->sourcei-1,this->sourcej,this->sourcek),newkeymet);
		this->heap.updateheap(&i3(indexarr,this->sourcei+1,this->sourcej,this->sourcek),newkeymet);

		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej-1,this->sourcek),newkeymet);
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej+1,this->sourcek),newkeymet);

		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek-1),newkeymet);
		this->heap.updateheap(&i3(indexarr,this->sourcei,this->sourcej,this->sourcek+1),newkeymet);

		//and for heapfull
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei,this->sourcej,this->sourcek),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei-1,this->sourcej,this->sourcek),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei+1,this->sourcej,this->sourcek),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei,this->sourcej-1,this->sourcek),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei,this->sourcej+1,this->sourcek),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei,this->sourcej,this->sourcek-1),newkeymet);
		this->heapfull.updateheap(&i3(indexarr2,this->sourcei,this->sourcej,this->sourcek+1),newkeymet);
	}

	if (this->heap.currentsize != this->heapfull.currentsize)
		cout << "sizes don't match!" << endl;

	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,heap.currentsize);
	//cout << numberToExpand << endl;

	//EXPANSIONS
	for (size_t i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis;
		Metric met;

		//Get most important cell to update next. Use this loop iteration.
		calcthis = (IndexTuple*)this->heap.getminheap(met);
		this->heap.deleteminheap();

		//Heapfull versions
		IndexTuple* calcthis2;
		Metric met2;
		calcthis2 = (IndexTuple*)this->heapfull.getminheap(met2);
		this->heapfull.deleteminheap();

		//if (calcthis2->i != calcthis->i || calcthis2->j != calcthis->j || calcthis2->k != calcthis->k)
		//{
		//	cout << "heap mins don't match! uh oh" << endl;

		//	//Look at the top few things in the heaps
		//	Metric met1s[5];
		//	IndexTuple* idx1s[5];
		//	Metric met2s[5];
		//	IndexTuple* idx2s[5];

		//	for(size_t i = 0; i < 5; i++)
		//	{
		//		idx1s[i] = (IndexTuple*)this->heap.getminheap(met1s[i]);
		//		this->heap.deleteminheap();
		//		idx2s[i] = (IndexTuple*)this->heapfull.getminheap(met2s[i]);
		//		this->heapfull.deleteminheap();
		//	}
		//	for(size_t i = 0; i < 5; i++)
		//	{
		//		cout << idx1s[i]->i << " " << idx1s[i]->j << " " << idx1s[i]->k << " " << met1s[i].key[0] << endl;
		//	}
		//	for(size_t i = 0; i < 5; i++)
		//	{
		//		cout << idx2s[i]->i << " " << idx2s[i]->j << " " << idx2s[i]->k << " " << met2s[i].key[0] << endl;
		//	}
		//	for(size_t i = 0; i < 5; i++)
		//	{
		//		this->heap.insertheap(idx1s[i],met1s[i]);
		//		this->heapfull.insertheap(idx2s[i],met2s[i]);
		//	}
		//}
		
		//if (calcthis->computedAt == this->count)
			//cout << "ehhh" << endl;

		calcthese[i] = calcthis;

		//Mark that this cell has been selected once already this frame
		calcthis->computedAt = this->count;

		this->updateCellPhase1(calcthis->i,calcthis->j,calcthis->k);

		this->updateObstacleMaskAndLaplacian(calcthis->i,calcthis->j,calcthis->k);
	}

	//D2 Update
	for (int i = 0; i < numberToExpand; i++)
	{
		IndexTuple* calcthis = calcthese[i];
		this->updateCellPhase2(calcthis->i,calcthis->j,calcthis->k);
	}


		//Loop over expanded nodes, adding them and their neighbors to the updateList
	for (size_t i = 0; i < numberToExpand; i++)
	{
		//add updated cell
		this->updateList[this->updateListCount++] = calcthese[i];

		//add the important neighbors too. 
		IndexTuple* neighbor;

		if (calcthese[i]->i > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i-1,calcthese[i]->j,calcthese[i]->k);
			this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->i < this->iSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i+1,calcthese[i]->j,calcthese[i]->k);
			this->updateList[this->updateListCount++] = neighbor;
		}

		if (calcthese[i]->j > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j-1,calcthese[i]->k);
			this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->j < this->jSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j+1,calcthese[i]->k);
			this->updateList[this->updateListCount++] = neighbor;
		}

		if (calcthese[i]->k > 0)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j,calcthese[i]->k-1);
			this->updateList[this->updateListCount++] = neighbor;
		}
		if (calcthese[i]->k < this->kSize-1)
		{
			neighbor = &i3(indexarr,calcthese[i]->i,calcthese[i]->j,calcthese[i]->k+1);
			this->updateList[this->updateListCount++] = neighbor;
		}
	}//end updatelist creation for next round's beginning
}//end of update frame method


#endif