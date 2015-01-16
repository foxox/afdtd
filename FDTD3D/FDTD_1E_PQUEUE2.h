#ifndef FDTD_1E_PQUEUE2H
#define FDTD_1E_PQUEUE2H

#include "FDTD_TBB.h"

#include <list>
#include <vector>
#include <queue>

//struct FDTD1EPQUEUE2Params
//{
//	bool ABSOLUTE_LIMIT;
//	int ABSOLUTE_NUMBER;
//	bool WAVEVOLUME_LIMIT;
//	float WAVEVOLUME_LIMIT_FACTOR;
//};

class FDTD_1E_PQUEUE2 : public FDTD
{

public:
	struct MetricIndexTuple
	{
		float metric;
		unsigned int i;
		unsigned int j;
		unsigned int k;
	};
	static inline bool CompareMetricsForDescendingSort(const MetricIndexTuple& a, const MetricIndexTuple& b)
	{ return a.metric > b.metric; }

	bool ABSOLUTE_LIMIT;
	int ABSOLUTE_NUMBER;
	bool WAVEVOLUME_LIMIT;
	float WAVEVOLUME_LIMIT_FACTOR;
	//const int numToCalc;

protected:

	inline virtual void updateCellPhase1(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateCellPhase2(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateState(float time);

	//float threshold;
	inline virtual float metric(unsigned int i, unsigned int j, unsigned int k);

	//Functor for TBB loop
	//class tbbTask_PQueueGen
	//{
	//protected:
	//	FDTD_1E_PQUEUE2* fdtd;

	//public:
	//	tbbTask_PQueueGen(FDTD_1E_PQUEUE2* _fdtd) : fdtd(_fdtd) {};
	//	~tbbTask_PQueueGen(void) {};

	//	inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	//};

	//tbbTask_PQueueGen tbbTask_PQueueGen;


	//priority_queue<MetricIndexTuple, vector<MetricIndexTuple>, less<vector<MetricIndexTuple>::value_type> > pqueue;
	list<MetricIndexTuple> sortedMetrics;
	list<MetricIndexTuple>::iterator interestingZoneMarker;


public:

	FDTD_1E_PQUEUE2() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD TBB 1E PQUEUE constructor called. You might not want this!" << endl;
	}

	FDTD_1E_PQUEUE2(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1epqueueparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
	{
		this->ABSOLUTE_LIMIT = fdtd1epqueueparams.ABSOLUTE_LIMIT;
		this->ABSOLUTE_NUMBER = fdtd1epqueueparams.ABSOLUTE_NUMBER;
		this->WAVEVOLUME_LIMIT = fdtd1epqueueparams.WAVEVOLUME_LIMIT;
		this->WAVEVOLUME_LIMIT_FACTOR = fdtd1epqueueparams.WAVEVOLUME_LIMIT_FACTOR;

		//size_t pnSize = this->iSize * this->jSize * this->kSize;
		//this->combinedNeighbors = new float[pnSize];

		//Fill sortedMetrics list
		for (unsigned int i = 0; i < this->iSize; i++)
		{
			for (unsigned int j = 0; j < this->jSize; j++)
			{
				for (unsigned int k = 0; k < this->kSize; k++)
				{
					float metricv = this->metric(i,j,k);
					FDTD_1E_PQUEUE2::MetricIndexTuple addme;
					addme.i = i;
					addme.j = j;
					addme.k = k;
					addme.metric = metricv;
					this->sortedMetrics.push_front(addme);
				}
			}
		}
	}

	virtual ~FDTD_1E_PQUEUE2(void) {}

};

namespace std
{
inline bool operator< (const FDTD_1E_PQUEUE2::MetricIndexTuple& a, const FDTD_1E_PQUEUE2::MetricIndexTuple& b)
{
	return a.metric < b.metric;
}

inline bool operator> (const FDTD_1E_PQUEUE2::MetricIndexTuple& a, const FDTD_1E_PQUEUE2::MetricIndexTuple& b)
{
	return a.metric > b.metric;
}
}


float FDTD_1E_PQUEUE2::metric(unsigned int i, unsigned int j, unsigned int k)
{
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

void FDTD_1E_PQUEUE2::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase1(i, j, k);
	this->updateObstacleMaskAndLaplacian(i,j,k);
}

void FDTD_1E_PQUEUE2::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	//if ( this->metric(i,j,k) >= this->threshold	)
	//{
		FDTD::updateCellPhase2(i, j, k);
	//}
	//else
	//{
		//cout << 's';
		//this->expansionCount++;
	//}
}

//bool operator< (const FDTD_1E_PQUEUE::MetricIndexTuple &a, const FDTD_1E_PQUEUE::MetricIndexTuple &b)
//{
//	return a.metric < b.metric;
//}

void FDTD_1E_PQUEUE2::updateState(float time)
{
	size_t numberToExpand = 0;
	if (this->ABSOLUTE_LIMIT)
	{
		numberToExpand = ABSOLUTE_NUMBER;
	}
	else if (this->WAVEVOLUME_LIMIT)
	{
		//updateState, so sort the old list with new metrics

		//Update metrics in (whole thing? interesting zone? update zone(larger?)?)
		for (list<MetricIndexTuple>::iterator i = sortedMetrics.begin(); i != sortedMetrics.end(); i++)
		{
			MetricIndexTuple newmetric(*i);	//copy constructor?
			newmetric.metric = this->metric(newmetric.i, newmetric.j, newmetric.k);
			*i = newmetric;	//can I do this to an iterator?
		}
		
		//Split list into interesting zone and other part
		//list<MetricIndexTuple> sortedMetricsInteresting;
		//sortedMetricsInteresting.splice(sortedMetricsInteresting.begin(),sortedMetrics.begin(),interestingZoneMarker);

		//Sort interesting zone
		sortedMetrics.sort(&CompareMetricsForDescendingSort);
		for (int a = 0; a < 10; a++)
		{
			//bubble
			for (list<MetricIndexTuple>::iterator it = sortedMetrics.begin(); it != sortedMetrics.end()--;)
			{
				list<MetricIndexTuple>::iterator curr = it;
				list<MetricIndexTuple>::iterator next = it++;
				if (curr->metric < next->metric)
				{
					sortedMetrics.splice(next,sortedMetrics,curr);
				}
			}
		}

		//stick it back into the other list

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

		//if (this->simtime > (pulseCenterTime-pulseHalfWidthTime) && this->simtime < (pulseCenterTime+pulseHalfWidthTime))
			//cout << "hmm" << endl;
	}
	else
	{
		cout << "Something is wrong with the configuration." << endl;
		this->shouldStop = true;
	}

	//debug highlighting
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));

	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,sortedMetrics.size());
	//TODO: could save memory by changing this to just store the indexes and not the metric too here (new struct)
	MetricIndexTuple* calcthese = new MetricIndexTuple[numberToExpand];
	list<MetricIndexTuple>::iterator sortedMetricIter = this->sortedMetrics.begin();
	for (int i = 0; i < numberToExpand; i++)
	{
		MetricIndexTuple calcthis;
		calcthis = *sortedMetricIter;
		calcthese[i] = calcthis;

		//highlight updated cells
		if (calcthis.k == this->mick)
			i2(this->view->highlightThese,calcthis.i,calcthis.j) = true;

		this->updateCellPhase1(calcthis.i,calcthis.j,calcthis.k);

		//this->updateObstacleMaskAndLaplacian(calcthis.i,calcthis.j,calcthis.k);

		sortedMetricIter++;
	}


	//D2 Update
	for (int i = 0; i < numberToExpand; i++)
	{
		MetricIndexTuple calcthis = calcthese[i];
		this->updateCellPhase2(calcthis.i,calcthis.j,calcthis.k);
	}
	
	delete[] calcthese;

	//parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD1);
	//parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD2);
	//this->FDTD_TBB::updateState(time);

}

//void FDTD_1E_PQUEUE2::tbbTask_PQueueGen::operator ()(const tbb::blocked_range<unsigned int> &range) const
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
//				FDTD_1E_PQUEUE2::MetricIndexTuple addme;
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