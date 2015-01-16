#include "common.h"

#include "view.h"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

int sim2D(void);
int sim2DTBB(void);

class FDTDSimTask
{
public:
	float* pn;
	float* pnp1;
	float* pnm1;

	GLubyte* texture_2Dfielddata;

	unsigned int jSize;
	float lambda2;

	FDTDSimTask(float *_pn, float *_pnp1, float *_pnm1, GLubyte *_texture_2Dfielddata, unsigned int _jSize, float _lambda2);

	void operator() (const tbb::blocked_range<unsigned int>& range ) const;
};
