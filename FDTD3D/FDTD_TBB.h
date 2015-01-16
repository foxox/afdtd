#ifndef FDTD_TBBH
#define FDTD_TBBH

#include "FDTD.h"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#pragma warning (disable: 4355)

class FDTD_TBB : public FDTD
{
public:

	FDTD_TBB(void) : tbbTask_UpdateD1(this), tbbTask_UpdateD2(this) {}
	FDTD_TBB(ModelParams modelparams, FDTDParams fdtdparams) : FDTD(modelparams, fdtdparams), tbbTask_UpdateD1(this), tbbTask_UpdateD2(this) {}
	virtual ~FDTD_TBB(void) {}

	inline virtual void updateState(float time);

protected:

	//friend class FDTDTBBTask_UpdateD1;
	//friend class FDTDTBBTask_UpdateD2;
	//friend class TBBTask_Update_D1_1E;

	//Functor for TBB loop
	class FDTDTBBTask_UpdateD1
	{
	protected:
		FDTD_TBB* fdtd;
	public:
		FDTDTBBTask_UpdateD1(FDTD_TBB* _fdtd) : fdtd(_fdtd) {};
		inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	};

	//Functor for TBB loop
	class FDTDTBBTask_UpdateD2
	{
	protected:
		FDTD_TBB* fdtd;
	public:
		FDTDTBBTask_UpdateD2(FDTD_TBB* _fdtd) : fdtd(_fdtd) {};
		inline virtual void operator() (const tbb::blocked_range<unsigned int>& range ) const;
	};

	//TBB Task for updating D1. It must be all done before D2
	FDTDTBBTask_UpdateD1 tbbTask_UpdateD1;
	//TBB Task for updating D2 and ultimately PNP1
	FDTDTBBTask_UpdateD2 tbbTask_UpdateD2;
};




void FDTD_TBB::updateState(float time)
{
	//Run parallel for loop over cells to update
	//The i dimension, no matter 1D, 2D, or 3D, will always be divisible across threads

	parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD1);
	parallel_for(tbb::blocked_range<unsigned int> (0, this->iSize, 1), this->tbbTask_UpdateD2);
}




void FDTD_TBB::FDTDTBBTask_UpdateD1::operator ()(const tbb::blocked_range<unsigned int> &range) const
{
	//cout << "Range: " << range.begin() << " to " << range.end() << endl;
	for (unsigned int i = range.begin(); i < range.end(); i++)
	{
		for (unsigned int j = 0; j < this->fdtd->jSize; j++)
		{
			for (unsigned int k = 0; k < this->fdtd->kSize; k++)
			{
				if ((i3(this->fdtd->obstaclemap,i,j,k)&1u) == 0)
					continue;

				this->fdtd->updateCellPhase1(i,j,k);

				this->fdtd->updateObstacleMaskAndLaplacian(i,j,k);
			}//k loop
		}//j loop
	}//i loop
}//update state task


void FDTD_TBB::FDTDTBBTask_UpdateD2::operator ()(const tbb::blocked_range<unsigned int> &range) const
{
	//cout << "Range: " << range.begin() << " to " << range.end() << endl;
	for (unsigned int i = range.begin(); i < range.end(); i++)
	{
		for (unsigned int j = 0; j < this->fdtd->jSize; j++)
		{
			for (unsigned int k = 0; k < this->fdtd->kSize; k++)
			{
				if ((i3(this->fdtd->obstaclemap,i,j,k)&1u) == 0)
					continue;

				this->fdtd->updateCellPhase2(i,j,k);
			}//k loop
		}//j loop
	}//i loop
}//update state task




#endif