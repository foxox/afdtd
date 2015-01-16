#ifndef MESHH
#define MESHH

#include <vector>
using namespace std;

#include "foxmath3.h"
using namespace FM;

class Mesh
{
public:
	Mesh(void);
	virtual ~Mesh(void);

	Vec3 pos;
	//Vec3 rotEuler;
	//Mat4 rotMat;

	vector<Vec3> verts;
	vector<Vec2> uvs;

	Vec3 color;
};

#endif
