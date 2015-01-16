#ifndef FDTDH
#define FDTDH

#include "common.h"

#include "Model.h"

struct FDTDParams
{
	bool PML_OBSTACLE_STRETCH;
	float maxfreq;
	float m;
	float R0;
	unsigned int na;
	float c;
	float Cmax;
	float ax;
	float n2;
	float epsilon0;

	float micx;
	float micy;
	float micz;
};

class FDTD :
	public Model
{

public:
	FDTD(void);
	FDTD(ModelParams modelparams, FDTDParams fdtdparams);
	virtual ~FDTD(void);


	//PARAMETERS

	bool PML_OBSTACLE_STRETCH;
	
	float maxfreq;

	float m;
	float R0;
	unsigned int na;

	float c;
	float h;
	
	unsigned int iSizeSim;
	unsigned int jSizeSim;
	unsigned int kSizeSim;

	unsigned int mici;
	unsigned int micj;
	unsigned int mick;

	unsigned int iSize;
	unsigned int jSize;
	unsigned int kSize;

	unsigned int sourcei;
	unsigned int sourcej;
	unsigned int sourcek;

	float Cmax;

	float lambda;
	float lambda2;


	//PML PARAMETERS
	//This being a single value depends on the dimensions having the same scale.
	float deltadim;
	float pmlupdated2factor;

	float sigmamax;

	float ax;
	float n2;
	float epsilon0;



	//MEMBERS ON THE HEAP

	//PML stuff
	//You must delete[] these; these always represent the space allocated for pml d1/d2 calculations
	float* d1n1;
	float* d1n2;
	float* d2n1;
	float* d2n2;
	//You don't have to delete[] these (they just alias the ones above)
	//TODO: I think the memory use here can be halved. If it gets tight, optimize this
	float* d1nphalf;
	float* d2nphalf;
	float* d1nmhalf;
	float* d2nmhalf;
	//Size values for those
	//TODO: make these defines or const or something. might help runtimes, optimization of the index macros
	size_t d1nphalfJSIZE;
	size_t d1nphalfKSIZE;
	size_t d1nphalfLSIZE;
	size_t d2nphalfJSIZE;
	size_t d2nphalfKSIZE;
	size_t d2nphalfLSIZE;

	size_t d1nmhalfJSIZE;
	size_t d1nmhalfKSIZE;
	size_t d1nmhalfLSIZE;
	size_t d2nmhalfJSIZE;
	size_t d2nmhalfKSIZE;
	size_t d2nmhalfLSIZE;


	//delete[] these in destructor
	float* pn1;
	float* pn2;
	float* pn3;
	//Don't delete[] these; they only alias the pn1 pn2 pn3 ones above
	float* pn;
	float* pnp1;
	float* pnm1;

	unsigned int pn1JSIZE;
	unsigned int pn2JSIZE;
	unsigned int pn3JSIZE;

	unsigned int pn1KSIZE;
	unsigned int pn2KSIZE;
	unsigned int pn3KSIZE;
	
	unsigned int pnJSIZE;
	unsigned int pnp1JSIZE;
	unsigned int pnm1JSIZE;

	unsigned int pnKSIZE;
	unsigned int pnp1KSIZE;
	unsigned int pnm1KSIZE;

	//delete[] this
	float* laplacian;
	unsigned int laplacianJSIZE;
	unsigned int laplacianKSIZE;



	//METHODS

	inline virtual void updateState(float time);
	inline virtual void timeshiftState();

	virtual void runSim();

protected:
	inline virtual void updateCellPhase1(unsigned int i, unsigned int j, unsigned int k);
	inline virtual void updateCellPhase2(unsigned int i, unsigned int j, unsigned int k);

	inline virtual float calcLaplacian(unsigned int i, unsigned int j, unsigned int k, float obstaclemask[7]);
	inline virtual void updateLaplacian(unsigned int i, unsigned int j, unsigned int k, float obstaclemask[7]);
	inline virtual void updateObstacleMaskAndLaplacian(unsigned int i, unsigned int j, unsigned int k);
	inline virtual float calcObstacleMaskAndLaplacian(unsigned int i, unsigned int j, unsigned int k);
};



void FDTD::updateState(float time)
{
	//D1 update

	for (unsigned int i = 0; i < this->iSize; i++)
	{
		for (unsigned int j = 0; j < this->jSize; j++)
		{
			for (unsigned int k = 0; k < this->kSize; k++)
			{
				if ((i3(this->obstaclemap,i,j,k)&1u) == 0)
					continue;

				this->updateCellPhase1(i,j,k);

				this->updateObstacleMaskAndLaplacian(i,j,k);
			}//k loop
		}//j loop
	}//i loop


	//D2 Update

	for (unsigned int i = 0; i < this->iSize; i++)
	{
		for (unsigned int j = 0; j < this->jSize; j++)
		{
			for (unsigned int k = 0; k < this->kSize; k++)
			{
				if ((i3(this->obstaclemap,i,j,k)&1u) == 0)
					continue;

				this->updateCellPhase2(i,j,k);
			}//k loop
		}//j loop
	}//i loop

	//debug code was here; see bottom of file.

}//updateState

void FDTD::timeshiftState()
{
	//Shift the pressure arrays back in time one step in preparation for the next round

	float* temp = pnm1;
	pnm1 = pn;
	pn = pnp1;
	pnp1 = temp;

	//Shift back the PML arrays too

	temp = this->d1nmhalf;
	this->d1nmhalf = this->d1nphalf;
	this->d1nphalf = temp;

	temp = this->d2nmhalf;
	this->d2nmhalf = this->d2nphalf;
	this->d2nphalf = temp;
}

void FDTD::updateCellPhase1(unsigned int i, unsigned int j, unsigned int k)
{
	//Perform PML operations on all dimensions. 0=x, 1=y, 2=z
	//This is made possible by having all dimensions stored in the same array
	unsigned char inside_pml_zone[] = {0,0,0};	//0 = not, 1 = is
	unsigned int idx[] = {i,j,k};
	unsigned int dimSize[] = {this->iSize,this->jSize,this->kSize};
	
	//sigmamax calculation was originally inside the loop (for readability) moved outside for efficiency
	float sigmamax = this->n2 * this->epsilon0 * this->c * ((float)this->m+1.0f) * log(this->R0) / (2.0f * (float)this->na * this->h);
	
	for (unsigned int dim = 0; dim < 3; dim++)
	{
		//check if we're within the PML zone for this dimension
		//if (idx[dim] < this->na + 1 || idx[dim] >= (dimSize[dim] - this->na - 1))
		//if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na))
		{
			if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na))
			{
				inside_pml_zone[dim] = 1;	//mark that this dimension's index is within the pml zone
			}

			//Calculate values which depend on the side of the sim such as kvalue
			float kvalue = 0.0f;
			if (idx[dim] < this->na)
			{
				kvalue = (float)(this->na-idx[dim]);
			}
			else if (idx[dim] >= (dimSize[dim] - this->na))
			{
				kvalue = (float)(idx[dim] - (dimSize[dim] - this->na) + 1);
			}

			//sigmamax calculation was here (see above loop ... moved for efficiency)

			//TODO: test if these operations are faster done before the whole sim and stored in variables or if that extra memory use is slower than simply recalculating
			float sigdim = -sigmamax * pow(kvalue / this->na, (float)this->m);
			float tempdim = sigdim * this->deltaTime / (2.0f * this->epsilon0 * this->n2);
			float d1xnphalf_term1_factor = ((this->ax - tempdim) / (this->ax + tempdim));
			float d1xnphalf_term2_factor = 1.0f / (this->ax + tempdim);

			//These may run off the edges of the sim, so take that into account.
			//TODO: flip it all around in the regions they run off the edges so there's always good stuff in the calculations
			float pndim_diff = 0.0f;

			//D1
			//Calculate pndim_diff
			if (idx[dim] == dimSize[dim]-1)
			{
				pndim_diff = - i3(this->pn,i,j,k);
			}
			else
			{
				switch(dim)
				{
				case 0: pndim_diff = i3(this->pn,i+1,j,k) - i3(this->pn,i,j,k); break;
				case 1: pndim_diff = i3(this->pn,i,j+1,k) - i3(this->pn,i,j,k); break;
				case 2: pndim_diff = i3(this->pn,i,j,k+1) - i3(this->pn,i,j,k); break;
				}
				//TODO: test speed of using dim to calculate an index offset (like, +1 for dim=k, +kSize for dim=j, +jSize*kSize for dim=i
			}
			float d1nphalf_current = (d1xnphalf_term1_factor * i4(this->d1nmhalf,dim,i,j,k)) + 
				pndim_diff * d1xnphalf_term2_factor;
			i4(this->d1nphalf,dim,i,j,k) = d1nphalf_current;
		}
	}
}

void FDTD::updateCellPhase2(unsigned int i, unsigned int j, unsigned int k)
{
	this->expansionCount++;

	//Perform PML operations on all dimensions. 0=x, 1=y, 2=z
	//This is made possible by having all dimensions stored in the same array
	unsigned char inside_pml_zone[] = {0,0,0};	//0 = not, 1 = is
	unsigned int idx[] = {i,j,k};
	unsigned int dimSize[] = {this->iSize,this->jSize,this->kSize};
	for (unsigned int dim = 0; dim < 3; dim++)
	{
		//check if we're within the PML zone for this dimension
		//if (idx[dim] < this->na + 1 || idx[dim] >= (dimSize[dim] - this->na - 1))
		//if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na))
		{
			if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na))
			{
				inside_pml_zone[dim] = 1;	//mark that this dimension's index is within the pml zone
			}

			//Calculate values which depend on the side of the sim such as kvalue
			float kvalue = 0.0f;
			if (idx[dim] < this->na)
			{
				kvalue = (float)(this->na-idx[dim]);
			}
			else if (idx[dim] >= (dimSize[dim] - this->na))
			{
				kvalue = (float)(idx[dim] - (dimSize[dim] - this->na) + 1);
			}

			//TODO: test if these operations are faster done before the whole sim and stored in variables or if that extra memory use is slower than simply recalculating
			float sigdim = -this->sigmamax * pow(kvalue / this->na, (float)this->m);
			float tempdim = sigdim * this->deltaTime / (2.0f * this->epsilon0 * this->n2);
			float d2dimnphalf_term1_factor = ((this->ax - tempdim) / (this->ax + tempdim));
			float d2dimnphalf_term2_factor = 1.0f / (this->ax + tempdim);


			float d1dimnphalf_diff = 0.0f;
			float d1dimnmhalf_diff = 0.0f;

			//D2
			//Calculate d1dimnphalf_diff.  0 is 0+1/2, 1 is 1+1/2. So here the -1/2 terms fall off the low end
			if (idx[dim] == 0)
			{
				d1dimnphalf_diff = i4(this->d1nphalf,dim,i,j,k);
			}
			else
			{
				switch(dim)
				{
				case 0:	d1dimnphalf_diff = i4(this->d1nphalf,0,i,j,k) - i4(this->d1nphalf,0,i-1,j,k); break;
				case 1:	d1dimnphalf_diff = i4(this->d1nphalf,1,i,j,k) - i4(this->d1nphalf,1,i,j-1,k); break;
				case 2:	d1dimnphalf_diff = i4(this->d1nphalf,2,i,j,k) - i4(this->d1nphalf,2,i,j,k-1); break;
				}
			}
			//Calculate d1dimnmhalf_diff.	0 is 0+1/2, 1 is 1+1/2. So here the -1/2 terms fall off the low end
			if (idx[dim] == 0)
			{
				d1dimnmhalf_diff = i4(this->d1nmhalf,dim,i,j,k);
			}
			else
			{
				switch(dim)
				{
				case 0: d1dimnmhalf_diff = i4(this->d1nmhalf,0,i,j,k) - i4(this->d1nmhalf,0,i-1,j,k); break;
				case 1: d1dimnmhalf_diff = i4(this->d1nmhalf,1,i,j,k) - i4(this->d1nmhalf,1,i,j-1,k); break;
				case 2: d1dimnmhalf_diff = i4(this->d1nmhalf,2,i,j,k) - i4(this->d1nmhalf,2,i,j,k-1); break;
				}
			}

			//D2
			float d2nphalf_curent = (d2dimnphalf_term1_factor * i4(this->d2nmhalf,dim,i,j,k)) + 
				d2dimnphalf_term2_factor * (d1dimnphalf_diff - d1dimnmhalf_diff);
			i4(this->d2nphalf,dim,i,j,k) = d2nphalf_curent;
		}//if within pml zone or along boundary
	}//loop over dims





	////other cells in the middle of the space
	//float absorptivity = i3(this->absorbmap,i,j,k);
	//if (absorptivity != 0)
	//{
	//	//Calculate values which depend on the side of the sim such as kvalue
	//	//float kvalue = 0.0f;
	//	//kvalue = absorptivity * (float)na;

	//	//float sigdim = -this->sigmamax * pow(kvalue / this->na, (float)this->m);
	//	float sigdim = -this->sigmamax * pow(absorptivity, (float)this->m);
	//	float tempdim = sigdim * this->deltaTime / (2.0f * this->epsilon0 * this->n2);
	//	float d2dimnphalf_term1_factor = ((this->ax - tempdim) / (this->ax + tempdim));
	//	float d2dimnphalf_term2_factor = 1.0f / (this->ax + tempdim);


	//	float d1dimnphalf_diff = 0.0f;
	//	float d1dimnmhalf_diff = 0.0f;

	//	//above stuff is same for all 3 dims
	//	//loop over dims
	//	for (unsigned int dim = 0; dim < 3; dim++)
	//	{
	//		//D2
	//		//Calculate d1dimnphalf_diff.  0 is 0+1/2, 1 is 1+1/2. So here the -1/2 terms fall off the low end
	//		//if (idx[dim] == 0)
	//		//{
	//			//d1dimnphalf_diff = i4(this->d1nphalf,dim,i,j,k);
	//		//}
	//		//else
	//		//{
	//			switch(dim)
	//			{
	//			case 0:	d1dimnphalf_diff = i4(this->d1nphalf,0,i,j,k) - i4(this->d1nphalf,0,i-1,j,k); break;
	//			case 1:	d1dimnphalf_diff = i4(this->d1nphalf,1,i,j,k) - i4(this->d1nphalf,1,i,j-1,k); break;
	//			case 2:	d1dimnphalf_diff = i4(this->d1nphalf,2,i,j,k) - i4(this->d1nphalf,2,i,j,k-1); break;
	//			}
	//		//}
	//		//Calculate d1dimnmhalf_diff.	0 is 0+1/2, 1 is 1+1/2. So here the -1/2 terms fall off the low end
	//		//if (idx[dim] == 0)
	//		//{
	//			d1dimnmhalf_diff = i4(this->d1nmhalf,dim,i,j,k);
	//		//}
	//		//else
	//		//{
	//			switch(dim)
	//			{
	//			case 0: d1dimnmhalf_diff = i4(this->d1nmhalf,0,i,j,k) - i4(this->d1nmhalf,0,i-1,j,k); break;
	//			case 1: d1dimnmhalf_diff = i4(this->d1nmhalf,1,i,j,k) - i4(this->d1nmhalf,1,i,j-1,k); break;
	//			case 2: d1dimnmhalf_diff = i4(this->d1nmhalf,2,i,j,k) - i4(this->d1nmhalf,2,i,j,k-1); break;
	//			}
	//		//}

	//		//D2
	//		float d2nphalf_curent = (d2dimnphalf_term1_factor * i4(this->d2nmhalf,dim,i,j,k)) + 
	//			d2dimnphalf_term2_factor * (d1dimnphalf_diff - d1dimnmhalf_diff);
	//		i4(this->d2nphalf,dim,i,j,k) = d2nphalf_curent;
	//	}
	//}





	

	//if (obstaclemask[0] == 0)
	//{
		//i3(this->pnp1,i,j,k) = 0;
	//}
	//else
		if (inside_pml_zone[0] || inside_pml_zone[1] || inside_pml_zone[2]
		//|| absorptivity != 0
		)
			//TODO: OPTIMIZE: change to addition? any faster?
	{
		//Generate this pnp1 value with the PML calculation rather than the regular calculation

		float pnp1_current = 2.0f * i3(this->pn,i,j,k) - i3(this->pnm1,i,j,k) +
			this->pmlupdated2factor * (
			i4(this->d2nphalf,0,i,j,k) - i4(this->d2nmhalf,0,i,j,k) +
			i4(this->d2nphalf,1,i,j,k) - i4(this->d2nmhalf,1,i,j,k) +
			i4(this->d2nphalf,2,i,j,k) - i4(this->d2nmhalf,2,i,j,k)
								);

		//float pnp1_current = -1.0f * i3(this->pnm1,i,j,k) + 2.0f * i3(this->pn,i,j,k);
		//if (inside_pml_zone[0])
		//	pnp1_current += this->pmlupdated2factor * (i4(this->d2nphalf,0,i,j,k) - i4(this->d2nmhalf,0,i,j,k));
		//else
		//	pnp1_current += this->pmlupdated2factor * ( i>0?i3(this->pn,i-1,j,k):0.0f + i<iSize-1?i3(this->pn,i+1,j,k):0.0f - 2.0f * i3(this->pn,i,j,k));

		//if (inside_pml_zone[1])
		//	pnp1_current += this->pmlupdated2factor * (i4(this->d2nphalf,1,i,j,k) - i4(this->d2nmhalf,1,i,j,k));
		//else
		//	pnp1_current += this->pmlupdated2factor * ( j>0?i3(this->pn,i,j-1,k):0.0f + j<jSize-1?i3(this->pn,i,j+1,k):0.0f - 2.0f * i3(this->pn,i,j,k));

		//if (inside_pml_zone[2])
		//	pnp1_current += this->pmlupdated2factor * (i4(this->d2nphalf,2,i,j,k) - i4(this->d2nmhalf,2,i,j,k));
		//else
		//	pnp1_current += this->pmlupdated2factor * ( k>0?i3(this->pn,i,j,k-1):0.0f + k<kSize-1?i3(this->pn,i,j,k+1):0.0f - 2.0f * i3(this->pn,i,j,k));

		i3(this->pnp1,i,j,k) = pnp1_current;
	}
	else // not in PML zone
	{
		//Apply regular update for this cell
		
		float laplacian = i3(this->laplacian,i,j,k);

		float pnp1_current = 2.0f * i3(this->pn,i,j,k) - i3(this->pnm1,i,j,k) + this->lambda2 * (float)laplacian;
		i3(this->pnp1,i,j,k) = pnp1_current;
	}
}


float FDTD::calcLaplacian(unsigned int i, unsigned int j, unsigned int k, float obstaclemask[7])
{
	//TODO: For L6, is the mask better or would logic to determine what not to even bother mult/adding be better?

	//mask just the blocked parts
	/*return i3(this->pn,i-1,j,k) * obstaclemask[1]
		+ i3(this->pn,i+1,j,k) * obstaclemask[2]
		+ i3(this->pn,i,j-1,k) * obstaclemask[3]
		+ i3(this->pn,i,j+1,k) * obstaclemask[4]
		+ i3(this->pn,i,j,k-1) * obstaclemask[5]
		+ i3(this->pn,i,j,k+1) * obstaclemask[6]
		- 6.0f * i3(this->pn,i,j,k) * obstaclemask[0];*/

	//mask it all
	return (i3(this->pn,i-1,j,k)
		+ i3(this->pn,i+1,j,k)
		+ i3(this->pn,i,j-1,k)
		+ i3(this->pn,i,j+1,k)
		+ i3(this->pn,i,j,k-1)
		+ i3(this->pn,i,j,k+1)
		- 6.0f * i3(this->pn,i,j,k)) * obstaclemask[0];

	// 2 -27 270 -490 270 -27 2
	//return (
	//	2.0f * i3(this->pn,i-3,j,k) + 
	//	-27.0f * i3(this->pn,i-2,j,k) + 
	//	270.0f * i3(this->pn,i-1,j,k) + 
	//	270.0f * i3(this->pn,i+1,j,k) + 
	//	-27.0f * i3(this->pn,i+2,j,k) + 
	//	2.0f * i3(this->pn,i+3,j,k) +

	//	2.0f * i3(this->pn,i,j-3,k) + 
	//	-27.0f * i3(this->pn,i,j-2,k) + 
	//	270.0f * i3(this->pn,i,j-1,k) + 
	//	270.0f * i3(this->pn,i,j+1,k) + 
	//	-27.0f * i3(this->pn,i,j+2,k) + 
	//	2.0f * i3(this->pn,i,j+3,k) +

	//	2.0f * i3(this->pn,i,j,k-3) + 
	//	-27.0f * i3(this->pn,i,j,k-2) + 
	//	270.0f * i3(this->pn,i,j,k-1) + 
	//	270.0f * i3(this->pn,i,j,k+1) + 
	//	-27.0f * i3(this->pn,i,j,k+2) + 
	//	2.0f * i3(this->pn,i,j,k+3) +
	//	
	//	-1470.0f * i3(this->pn,i,j,k)
	//	)
	//	/ 180.0f * obstaclemask[0];


}

void FDTD::updateLaplacian(unsigned int i, unsigned int j, unsigned int k, float obstaclemask[7])
{
	i3(this->laplacian,i,j,k) = this->calcLaplacian(i,j,k,obstaclemask);
}

void FDTD::updateObstacleMaskAndLaplacian(unsigned int i, unsigned int j, unsigned int k)
{
	//make sure it isn't in PML
	if (i >= na && j >= na && k >= na && i < iSize-na && j < jSize-na && k < kSize-na)
	{
		//Read local obstacle data
		uint_fast32_t obstacle = i3(this->obstaclemap,i,j,k);
		//increase this when 7x7x7 kernel is used
		float obstaclemask[7];
		for (unsigned int a = 0; a < 7; a++)
		{
			obstaclemask[a] = (float)((obstacle >> a) & 1u);
			if (a == 0 && obstaclemask[a] == 0)
				break;
		}
		//update laplacian using that
		this->updateLaplacian(i,j,k,obstaclemask);
	}
}

float FDTD::calcObstacleMaskAndLaplacian(unsigned int i, unsigned int j, unsigned int k)
{
	//make sure it isn't in PML
	if (i >= na && j >= na && k >= na && i < iSize-na && j < jSize-na && k < kSize-na)
	{
		//Read local obstacle data
		uint_fast32_t obstacle = i3(this->obstaclemap,i,j,k);
		//increase this when 7x7x7 kernel is used
		float obstaclemask[7];
		for (unsigned int a = 0; a < 7; a++)
		{
			obstaclemask[a] = (float)((obstacle >> a) & 1u);
			if (a == 0 && obstaclemask[a] == 0)
				break;
		}
		//update laplacian using that
		return this->calcLaplacian(i,j,k,obstaclemask);
	}
	return 0.0f;
}

#endif












	//Debug outputs


	//cout << "pnp1" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i3(this->pnp1,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "pn" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i3(this->pn,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "pnm1" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i3(this->pnm1,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "d1xnphalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d1nphalf,0,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "d1xnmhalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d1nmhalf,0,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	
	/*cout << "d2xnphalf" << endl;
	for (unsigned int i = 0; i < this->iSize; i++)
	{
		for (unsigned int j = 0; j < this->jSize; j++)
		{
			for (unsigned int k = 0; k < this->kSize; k++)
			{
				cout << i4(this->d2nphalf,0,i,j,k) << " ";
			}
			cout << endl;
		}
		cout << endl;
	}*/

	/*cout << "d2xnmhalf" << endl;
	for (unsigned int i = 0; i < this->iSize; i++)
	{
		for (unsigned int j = 0; j < this->jSize; j++)
		{
			for (unsigned int k = 0; k < this->kSize; k++)
			{
				cout << i4(this->d2nmhalf,0,i,j,k) << " ";
			}
			cout << endl;
		}
		cout << endl;
	}*/

	//cout << "pnxdiff" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			float pnxdiff = 0.0f;
	//			if (i == iSize-1)
	//			{
	//				pnxdiff = - i3(pn,i,j,k);
	//			}
	//			else
	//			{
	//				pnxdiff = i3(pn,i+1,j,k) - i3(pn,i,j,k);
	//			}
	//			cout << pnxdiff << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}






	//cout << "d1znphalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d1nphalf,2,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "d1znmhalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d1nmhalf,2,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//
	//cout << "d2znphalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d2nphalf,2,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "d2znmhalf" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			cout << i4(this->d2nmhalf,2,i,j,k) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "pnzdiff" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			float pnzdiff = 0.0f;
	//			if (k == kSize-1)
	//			{
	//				pnzdiff = - i3(pn,i,j,k);
	//			}
	//			else
	//			{
	//				pnzdiff = i3(pn,i,j,k) - i3(pn,i,j,k+1);
	//			}
	//			cout << pnzdiff << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//cout << "kvalue" << endl;
	//for (unsigned int i = 0; i < this->iSize; i++)
	//{
	//	for (unsigned int j = 0; j < this->jSize; j++)
	//	{
	//		for (unsigned int k = 0; k < this->kSize; k++)
	//		{
	//			unsigned int idx[] = {i,j,k};
	//			unsigned int dimSize[] = {this->iSize,this->jSize,this->kSize};
	//			float kvalue = 0.0f;
	//			for (unsigned int dim = 0; dim < 3; dim++)
	//			{
	//				//if (idx[dim] < this->na + 1 || idx[dim] >= (dimSize[dim] - this->na - 1))
	//				//if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na - 1))
	//				{
	//					if (idx[dim] < this->na || idx[dim] >= (dimSize[dim] - this->na))
	//					{
	//						//inside_pml_zone[dim] = 1;	//mark that this dimension's index is within the pml zone
	//						//Calculate values which depend on the side of the sim such as kvalue
	//						if (idx[dim] < this->na)
	//						{
	//							kvalue += (float)(this->na-idx[dim]);
	//						}
	//						else if (idx[dim] >= (dimSize[dim] - this->na))
	//						{
	//							kvalue += (float)(idx[dim] - (dimSize[dim] - this->na) + 1);
	//						}
	//					}
	//				}
	//			}
	//			cout << (kvalue/1.0f) << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}


	//cout << "----------------------------------------------------" << endl;