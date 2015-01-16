#ifndef FDTD_TBB_1E_THRESHOLDH
#define FDTD_TBB_1E_THRESHOLDH

#include "FDTD_TBB.h"

#include <list>
#include <vector>

class FDTD_TBB_1E_THRESHOLD : public FDTD_TBB
{

protected:

	inline virtual void updateCellPhase1(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateCellPhase2(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateState(float time);

	float threshold;
	inline virtual float metric(unsigned int i, unsigned int j, unsigned int k);

	//Give the TBB loop functor access to threshold
	//friend class TBBTask_ThresholdFind;

	//Functor for TBB loop
	class TBBTask_ThresholdFind
	{
	protected:
		FDTD_TBB_1E_THRESHOLD* fdtd;

	public:
		TBBTask_ThresholdFind(FDTD_TBB_1E_THRESHOLD* _fdtd) : fdtd(_fdtd) {};
		~TBBTask_ThresholdFind(void) {};

		inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	};

	TBBTask_ThresholdFind tbbTask_ThresholdFind;

public:

	FDTD_TBB_1E_THRESHOLD() : FDTD_TBB(), tbbTask_ThresholdFind(this)
	{
		cout << "Default FDTD TBB 1E THRESHOLD constructor called. You might not want this!" << endl;
	}

	FDTD_TBB_1E_THRESHOLD(ModelParams modelparams, FDTDParams fdtdparams) : FDTD_TBB(modelparams, fdtdparams), tbbTask_ThresholdFind(this)
	{
		//size_t pnSize = this->iSize * this->jSize * this->kSize;
		//this->combinedNeighbors = new float[pnSize];
	}

	virtual ~FDTD_TBB_1E_THRESHOLD(void) {}

};


float FDTD_TBB_1E_THRESHOLD::metric(unsigned int i, unsigned int j, unsigned int k)
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

void FDTD_TBB_1E_THRESHOLD::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	//if (i < this->na || j < this->na || k < this->na || i >= this->iSize - this->na || j >= this->jSize - this->na || k >= this->kSize - this->na
	//||(i3(pn,i,j,k) >= this->threshold)
	//)
	{
		FDTD::updateCellPhase1(i, j, k);
		this->updateObstacleMaskAndLaplacian(i,j,k);
	}
}

void FDTD_TBB_1E_THRESHOLD::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	//if (i < this->na || j < this->na || k < this->na || i >= this->iSize - this->na || j >= this->jSize - this->na || k >= this->kSize - this->na
	// ||(i3(pn,i,j,k) >= this->threshold)
	//||
	if ( this->metric(i,j,k) >= this->threshold	)
	{
		FDTD::updateCellPhase2(i, j, k);
	}
	else
	{
		//cout << 's';
		//this->expansionCount++;
	}
}

void FDTD_TBB_1E_THRESHOLD::updateState(float time)
{

	parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD1);

	//Find threshold
	this->threshold = 0.0f;
	parallel_for(tbb::blocked_range<unsigned int> (this->na, this->iSize - this->na, 1), this->tbbTask_ThresholdFind);
	//if (this->threshold < 0.001)
		//return;
	this->threshold *= 0.07f;
	
	parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD2);
	//this->FDTD_TBB::updateState(time);



}

void FDTD_TBB_1E_THRESHOLD::TBBTask_ThresholdFind::operator ()(const tbb::blocked_range<unsigned int> &range) const
{
	for (unsigned int i = range.begin(); i < range.end(); i++)
	{
		for (unsigned int j = 0; j < this->fdtd->jSize; j++)
		{
			for (unsigned int k = 0; k < this->fdtd->kSize; k++)
			{
				float metricv = this->fdtd->metric(i,j,k);
				if (metricv > this->fdtd->threshold)
				{
					this->fdtd->threshold = metricv;
				}
			}//k
		}//j
	}//i
}


#endif