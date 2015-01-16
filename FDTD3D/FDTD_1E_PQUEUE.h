#ifndef FDTD_1E_PQUEUEH
#define FDTD_1E_PQUEUEH

#include "FDTD_TBB.h"

#include <list>
#include <vector>
#include <queue>

struct FDTD1EPQUEUEParams
{
	bool ABSOLUTE_LIMIT;
	int ABSOLUTE_NUMBER;
	bool WAVEVOLUME_LIMIT;
	float WAVEVOLUME_LIMIT_FACTOR;
};

class FDTD_1E_PQUEUE : public FDTD
{

public:
	struct MetricIndexTuple
	{
		float metric;
		unsigned int i;
		unsigned int j;
		unsigned int k;
	};
	class MetricIndexTuplePointerCompare
	{
	public:
		inline bool operator() (MetricIndexTuple* a, MetricIndexTuple* b)
		{
			return FDTD_1E_PQUEUE::MetricIndexTuplePointerCompare::compare(a,b);
		}
		static inline bool compare(MetricIndexTuple* a, MetricIndexTuple* b)
		{
			return a->metric < b->metric;
		}
	};

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
	//	FDTD_1E_PQUEUE* fdtd;

	//public:
	//	tbbTask_PQueueGen(FDTD_1E_PQUEUE* _fdtd) : fdtd(_fdtd) {};
	//	~tbbTask_PQueueGen(void) {};

	//	inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	//};

	//tbbTask_PQueueGen tbbTask_PQueueGen;


	//priority_queue<MetricIndexTuple*, vector<MetricIndexTuple*>, MetricIndexTuplePointerCompare> pqueue2;
	vector<MetricIndexTuple*> pqueue;
	FDTD_1E_PQUEUE::MetricIndexTuple* indexarr;
	size_t indexarrJSIZE;
	size_t indexarrKSIZE;


public:

	FDTD_1E_PQUEUE() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD TBB 1E PQUEUE constructor called. You might not want this!" << endl;
	}

	FDTD_1E_PQUEUE(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1epqueueparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
	{
		this->ABSOLUTE_LIMIT = fdtd1epqueueparams.ABSOLUTE_LIMIT;
		this->ABSOLUTE_NUMBER = fdtd1epqueueparams.ABSOLUTE_NUMBER;
		this->WAVEVOLUME_LIMIT = fdtd1epqueueparams.WAVEVOLUME_LIMIT;
		this->WAVEVOLUME_LIMIT_FACTOR = fdtd1epqueueparams.WAVEVOLUME_LIMIT_FACTOR;

		//size_t pnSize = this->iSize * this->jSize * this->kSize;
		//this->combinedNeighbors = new float[pnSize];

		//TODO: see about pre-allocating space for the pqueue... may need to supply my own custom implementation?


		//allocate index state array so it's not always rebuilt
		indexarr = new MetricIndexTuple[this->iSize * this->jSize * this->kSize];
		this->indexarrJSIZE = this->pnJSIZE;
		this->indexarrKSIZE = this->pnKSIZE;
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					MetricIndexTuple* thisindtup = &i3(this->indexarr, i,j,k);
					thisindtup->i = i;
					thisindtup->j = j;
					thisindtup->k = k;
					
					thisindtup->metric = 0.01f;

					//this->pqueue2.push(thisindtup);
					//this->pqueue.push_back(thisindtup);
					//push_heap(this->pqueue.begin(), this->pqueue.end(), MetricIndexTuplePointerCompare::compare);
					//this->heap.insertheap(thisindtup,met);
				}
			}
		}
	}

	virtual ~FDTD_1E_PQUEUE(void)
	{
		delete[] this->indexarr;
	}

};

namespace std
{
inline bool operator< (const FDTD_1E_PQUEUE::MetricIndexTuple& a, const FDTD_1E_PQUEUE::MetricIndexTuple& b)
{
	return a.metric < b.metric;
}
inline bool operator> (const FDTD_1E_PQUEUE::MetricIndexTuple& a, const FDTD_1E_PQUEUE::MetricIndexTuple& b)
{
	return a.metric > b.metric;
}
}


float FDTD_1E_PQUEUE::metric(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_1E_PQUEUE::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	//if (i < this->na || j < this->na || k < this->na || i >= this->iSize - this->na || j >= this->jSize - this->na || k >= this->kSize - this->na
	//||(i3(pn,i,j,k) >= this->threshold)
	//)
	{
		FDTD::updateCellPhase1(i, j, k);
		this->updateObstacleMaskAndLaplacian(i,j,k);
	}
}

void FDTD_1E_PQUEUE::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_1E_PQUEUE::updateState(float time)
{

	//clear queue
	//this->pqueue2 = priority_queue<MetricIndexTuple*, vector<MetricIndexTuple*>, MetricIndexTuplePointerCompare>();
	this->pqueue = vector<MetricIndexTuple*>();
	//this->pqueue.clear();

	//Fill pqueue
	for (unsigned int i = 0; i < this->iSize; i++)
	{
		for (unsigned int j = 0; j < this->jSize; j++)
		{
			for (unsigned int k = 0; k < this->kSize; k++)
			{
				float metricv = this->metric(i,j,k);
				
				//if (this->simtime > 0.02f && metricv > 0.007f)
				//	cout << metricv << endl;

				//if (metricv > this->fdtd->threshold)
				//{
				//	this->fdtd->threshold = metricv;
				//}
				FDTD_1E_PQUEUE::MetricIndexTuple* addme = &i3(indexarr,i,j,k);
				addme->i = i;
				addme->j = j;
				addme->k = k;
				addme->metric = metricv;
				
				//this->pqueue2.push(addme);
				this->pqueue.push_back(addme);
				push_heap(this->pqueue.begin(), this->pqueue.end(), MetricIndexTuplePointerCompare::compare);
			}
		}
	}


	//this->threshold = 0.0f;
	//parallel_for(tbb::blocked_range<unsigned int> (this->na, this->iSize - this->na, 1), this->tbbTask_PQueueGen);
	

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

		//if (this->simtime > (pulseCenterTime-pulseHalfWidthTime))// && this->simtime < (pulseCenterTime+pulseHalfWidthTime))
		//	cout << "hmm" << endl;
	}
	else
	{
		cout << "Something is wrong with the configuration." << endl;
		this->shouldStop = true;
	}

	//debug highlighting
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));


	//TODO: 
	//BAD HACK FIX LATER
	//If numberToExpand is <7, bump it up to 7
	if (numberToExpand < 7) numberToExpand = 7;

	////attempt to force source cells to be important
	//if (this->simtime < 1.0f * WAVEOFFSET)
	//{
	//	i3(indexarr,this->sourcei,this->sourcej,this->sourcek).metric = 0;

	//	i3(indexarr,this->sourcei-1,this->sourcej,this->sourcek).metric = 0;
	//	i3(indexarr,this->sourcei+1,this->sourcej,this->sourcek).metric = 0;
	//	i3(indexarr,this->sourcei,this->sourcej-1,this->sourcek).metric = 0;
	//	i3(indexarr,this->sourcei,this->sourcej+1,this->sourcek).metric = 0;
	//	i3(indexarr,this->sourcei,this->sourcej,this->sourcek-1).metric = 0;
	//	i3(indexarr,this->sourcei,this->sourcej,this->sourcek+1).metric = 0;

	//	make_heap(this->pqueue.begin(), this->pqueue.end(), MetricIndexTuplePointerCompare::compare);
	//}



	//limit numberToExpand to the maximum number of cells

	numberToExpand = min(numberToExpand,pqueue.size());
	MetricIndexTuple** calcthese = new MetricIndexTuple*[numberToExpand];
	//cout << numberToExpand << endl;
	for (int i = 0; i < numberToExpand; i++)
	{
		MetricIndexTuple* calcthis;
		
		//calcthis = this->pqueue.top();
		calcthis = this->pqueue.front();
		pop_heap(this->pqueue.begin(), this->pqueue.end(), MetricIndexTuplePointerCompare::compare);
		this->pqueue.pop_back();
		//this->pqueue.pop();

		calcthese[i] = calcthis;

		//highlight updated cells
		if (calcthis->k == this->mick)
			i2(this->view->highlightThese,calcthis->i,calcthis->j) = true;

		this->updateCellPhase1(calcthis->i,calcthis->j,calcthis->k);

		//this->updateObstacleMaskAndLaplacian(calcthis->i,calcthis->j,calcthis->k);
	}


	//D2 Update
	for (int i = 0; i < numberToExpand; i++)
	{
		MetricIndexTuple* calcthis = calcthese[i];
		this->updateCellPhase2(calcthis->i,calcthis->j,calcthis->k);
	}
	
	delete[] calcthese;

	//parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD1);
	//parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD2);
	//this->FDTD_TBB::updateState(time);

}

//void FDTD_1E_PQUEUE::tbbTask_PQueueGen::operator ()(const tbb::blocked_range<unsigned int> &range) const
//{
//	for (unsigned int i = range.begin(); i < range.end(); i++)
//	{
//		for (unsigned int j = 0; j < this->fdtd->jSize; j++)
//		{
//			for (unsigned int k = 0; k < this->fdtd->kSize; k++)
//			{
//				float metricv = this->fdtd->metric(i,j,k);
//				//if (metricv > this->fdtd->threshold)
//				//{
//				//	this->fdtd->threshold = metricv;
//				//}
//				FDTD_1E_PQUEUE::MetricIndexTuple addme;
//				addme.i = i;
//				addme.j = j;
//				addme.k = k;
//				addme.metric = metricv;
//				this->fdtd->pqueue.push(addme);
//			}//k
//		}//j
//	}//i
//}


#endif