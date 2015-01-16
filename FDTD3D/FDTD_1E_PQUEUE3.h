#ifndef FDTD_1E_PQUEUE3H
#define FDTD_1E_PQUEUE3H

#include "FDTD_TBB.h"

#include "PQueueList.h"

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

class FDTD_1E_PQUEUE3 : public FDTD
{

public:
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
	//	FDTD_1E_PQUEUE3* fdtd;

	//public:
	//	tbbTask_PQueueGen(FDTD_1E_PQUEUE3* _fdtd) : fdtd(_fdtd) {};
	//	~tbbTask_PQueueGen(void) {};

	//	inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	//};

	//tbbTask_PQueueGen tbbTask_PQueueGen;


	//priority_queue<MetricIndexTuple, vector<MetricIndexTuple>, less<vector<MetricIndexTuple>::value_type> > pqueue;
	//list<MetricIndexTuple> sortedMetrics;
	//list<MetricIndexTuple>::iterator interestingZoneMarker;
	PQList sortedMetrics;
	list<PQListNode*> resortList;

public:

	FDTD_1E_PQUEUE3() : FDTD()//, tbbTask_PQueueGen(this)
	{
		cout << "Default FDTD TBB 1E PQUEUE constructor called. You might not want this!" << endl;
	}

	FDTD_1E_PQUEUE3(ModelParams modelparams, FDTDParams fdtdparams, FDTD1EPQUEUEParams fdtd1epqueueparams) : FDTD(modelparams, fdtdparams)//, tbbTask_PQueueGen(this)
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
					MetricIndexTuple addme;
					addme.i = i;
					addme.j = j;
					addme.k = k;
					addme.metric = metricv;
					this->sortedMetrics.push_front(addme);
				}
			}
		}
	}

	virtual ~FDTD_1E_PQUEUE3(void) {}

};

//namespace std
//{
//inline bool operator< (const MetricIndexTuple& a, const MetricIndexTuple& b)
//{
//	return a.metric < b.metric;
//}
//
//inline bool operator> (const MetricIndexTuple& a, const MetricIndexTuple& b)
//{
//	return a.metric > b.metric;
//}
//}


float FDTD_1E_PQUEUE3::metric(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_1E_PQUEUE3::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	FDTD::updateCellPhase1(i, j, k);
	this->updateObstacleMaskAndLaplacian(i,j,k);
}

void FDTD_1E_PQUEUE3::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_1E_PQUEUE3::updateState(float time)
{
	size_t numberToExpand = 0;
	if (this->ABSOLUTE_LIMIT)
	{
		numberToExpand = ABSOLUTE_NUMBER;
	}
	else if (this->WAVEVOLUME_LIMIT)
	{
		//Calculate expansion count based on current wave volume
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


	//Refresh all metrics in sortedMetrics
	PQListNode* node = this->sortedMetrics.head;
	while (node != NULL)
	{
		node->mit.metric = this->metric(node->mit.i,node->mit.j,node->mit.k);
		node = node->next;
	}

	////Prepare pqlist by sorting
	//this->sortedMetrics.updateZoneSize = numberToExpand;
	//for (list<PQListNode*>::iterator it = this->resortList.begin(); it != this->resortList.end(); it++)
	//{
	//	(*it)->mit.metric = this->metric((*it)->mit.i,(*it)->mit.j,(*it)->mit.k);
	//	PQList::sortNode(*it);
	//}


	this->resortList.clear();
	//this->sortedMetrics.pruneBelowUpdateZone();

	cout << this->sortedMetrics.size << endl;


	//debug highlighting
	memset(this->view->highlightThese,0,this->iSize*this->jSize*sizeof(bool));

	//limit numberToExpand to the maximum number of cells
	numberToExpand = min(numberToExpand,sortedMetrics.updateZoneSize);
	numberToExpand = min(numberToExpand,sortedMetrics.size);




	//Go through list, updating all cells until bottomOfUpdateZone is reached
		//add updated cell to re-sort list
		//add updated cell neighbors to re-sort list
	node = this->sortedMetrics.head;
	MetricIndexTuple* calcthese = new MetricIndexTuple[numberToExpand];
	size_t i = 0;
	while(node != NULL)
	{
		MetricIndexTuple calcthis;
		calcthis = node->mit;
		calcthese[i] = calcthis;
		i++;

		this->resortList.push_back(node);
		if (node->mit.i > 0)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i-1;
			newmit.j = node->mit.j;
			newmit.k = node->mit.k;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}
		if (node->mit.i < this->iSize-1)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i+1;
			newmit.j = node->mit.j;
			newmit.k = node->mit.k;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}
		if (node->mit.j > 0)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i;
			newmit.j = node->mit.j-1;
			newmit.k = node->mit.k;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}
		if (node->mit.j < this->jSize-1)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i;
			newmit.j = node->mit.j+1;
			newmit.k = node->mit.k;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}
		if (node->mit.k > 0)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i;
			newmit.j = node->mit.j;
			newmit.k = node->mit.k-1;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}
		if (node->mit.k < this->kSize-1)
		{
			//TODO: make this better. much better. this sucks.
			MetricIndexTuple newmit;
			newmit.i = node->mit.i;
			newmit.j = node->mit.j;
			newmit.k = node->mit.k+1;
			PQListNode* newnode = new PQListNode(newmit);
			this->resortList.push_back(newnode);
			this->sortedMetrics.insertBelowUpdateZone(newnode);
		}

		//highlight updated cells
		if (calcthis.k == this->mick)
			i2(this->view->highlightThese,calcthis.i,calcthis.j) = true;

		this->updateCellPhase1(calcthis.i,calcthis.j,calcthis.k);

		//this->updateObstacleMaskAndLaplacian(calcthis.i,calcthis.j,calcthis.k);

		//Advance to the next thing, or leave the loop
		node = node->next;
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

//void FDTD_1E_PQUEUE3::tbbTask_PQueueGen::operator ()(const tbb::blocked_range<unsigned int> &range) const
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
//				MetricIndexTuple addme;
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