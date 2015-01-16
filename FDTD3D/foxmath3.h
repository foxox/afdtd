#ifndef FOXMATH3H
#define FOXMATH3H

#include <math.h>

static const float PI = (float)3.1415926535897932384626433832795;
static const float DEG2RAD = (float)0.01745329251994329576923690768489;
static const float RAD2DEG = (float)57.295779513082320876798154814105;

typedef unsigned int uint;

#ifdef __cplusplus
namespace FM
{
#endif

//#ifndef __cplusplus

	typedef struct Vec2 Vec2;

	typedef struct Vec3 Vec3;

	typedef struct Mat4 Mat4;

	typedef struct Vec2
	{
		float u;
		float v;
	} Vec2;

	typedef struct Vec3
	{
		float x;
		float y;
		float z;
	} Vec3;

	typedef struct Vec4
	{
		float r;
		float g;
		float b;
		float a;
	} Vec4;

	typedef struct Mat4
	{
		float mat[4][4];
	} Mat4;

/*
#else

	struct Vec3
	{
	public:
		float x;
		float y;
		float z;

		//Vec3();
		//Vec3(float x, float y, float z);
	};

	struct Vec4TODO
	{
	public:
		float r;
		float g;
		float b;
		float a;
	};

	class Mat4
	{
	public:
		float mat[4][4];

		Mat4();
	};

#endif*/


//VEC2 FUNCTIONS

Vec2 Vec2GenVec2(float u, float v);


//VEC3 FUNCTIONS

Vec3 Vec3GenVec3(float x, float y, float z);

void Vec3ZeroOut(Vec3*);
void Vec3NormalizeOut(Vec3*);

float Vec3DotProduct(Vec3, Vec3);
Vec3 Vec3CrossProduct(Vec3, Vec3);
Vec3 Vec3Add(Vec3, Vec3);
Vec3 Vec3Sub(Vec3, Vec3);
float Vec3Length(Vec3);
float Vec3LengthSquared(Vec3);

//MAT4 FUNCTIONS

void Mat4ZeroOut(Mat4*);
void Mat4IdentityOut(Mat4*);
Mat4 Mat4GenZero();
Mat4 Mat4GenIdentity();

Mat4 Mat4GenTranslate(float, float, float);

Mat4 Mat4Transpose(Mat4);

Mat4 Mat4Mat4Multiply(Mat4, Mat4);

//COMBO FUNCTIONS

Vec3 Vec3Mat4Transform(Vec3, Mat4);
Vec3 Vec3Mat4TransformNormal(Vec3, Mat4);

//GRAPHICS SUPPORT

Mat4 Mat4GenPerspectiveProjection(float fovx, float aspect, float near, float far);
Mat4 Mat4GenLookAtTransform(Vec3 pos, Vec3 target, Vec3 up);


//CPP stuff

/*
#ifdef __cplusplus

//Operator overloads

Vec3 operator+(Vec3 a, Vec3 b);
Vec3 operator-(Vec3 a, Vec3 b);
float operator*(Vec3 a, Vec3 b);
Vec3 operator%(Vec3 a, Vec3 b);

#endif
*/

#ifdef __cplusplus
}
#endif


#endif