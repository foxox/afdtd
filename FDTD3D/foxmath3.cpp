#include "foxmath3.h"

using namespace FM;

//VEC2 FUNCTIONS

Vec2 FM::Vec2GenVec2(float u, float v)
{
	Vec2 returnme = {u, v};
	return returnme;
}

//VEC3 FUNCTIONS

Vec3 FM::Vec3GenVec3(float x, float y, float z)
{
	//#ifdef __cplusplus
		//Vec3 returnme(x,y,z);
	//#else
		Vec3 returnme = {x,y,z};
	//#endif
	return returnme;
}

void FM::Vec3ZeroOut(Vec3* vec)
{
	vec->x = 0;
	vec->y = 0;
	vec->z = 0;
}

void FM::Vec3NormalizeOut(Vec3* vec)
{
	float len = Vec3Length(*vec);
	vec->x /= len;
	vec->y /= len;
	vec->z /= len;
}

float FM::Vec3DotProduct(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 FM::Vec3CrossProduct(Vec3 a, Vec3 b)
{
	Vec3 returnme;
	returnme.x = a.y * b.z - a.z * b.y;
	returnme.y = a.z * b.x - a.x * b.z;
	returnme.z = a.x * b.y - a.y * b.x;
	return returnme;
}

Vec3 FM::Vec3Add(Vec3 a, Vec3 b)
{
	Vec3 returnme;
	returnme.x = a.x + b.x;
	returnme.y = a.y + b.y;
	returnme.z = a.z + b.z;
	return returnme;
}

Vec3 FM::Vec3Sub(Vec3 a, Vec3 b)
{
	Vec3 returnme;
	returnme.x = a.x - b.x;
	returnme.y = a.y - b.y;
	returnme.z = a.z - b.z;
	return returnme;
}

float FM::Vec3Length(Vec3 vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float FM::Vec3LengthSquared(Vec3 vec)
{
	return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

//MAT4 FUNCTIONS

void FM::Mat4ZeroOut(Mat4* mat)
{
//	uint i = 0;
//	uint j = 0;
//	for (i = 0; i < 16; i++)
		//mat->mat[i][j] = 0;
	mat->mat[0][0] = mat->mat[0][1] = mat->mat[0][2] = mat->mat[0][3] = 0;
	mat->mat[1][0] = mat->mat[1][1] = mat->mat[1][2] = mat->mat[1][3] = 0;
	mat->mat[2][0] = mat->mat[2][1] = mat->mat[2][2] = mat->mat[2][3] = 0;
	mat->mat[3][0] = mat->mat[3][1] = mat->mat[3][2] = mat->mat[3][3] = 0;
}

void FM::Mat4IdentityOut(Mat4* mat)
{
	Mat4ZeroOut(mat);
	mat->mat[0][0] = 1.0f;
	mat->mat[1][1] = 1.0f;
	mat->mat[2][2] = 1.0f;
	mat->mat[3][3] = 1.0f;
}

Mat4 FM::Mat4GenZero()
{
	Mat4 returnme;
	Mat4ZeroOut(&returnme);
	return returnme;
}

Mat4 FM::Mat4GenIdentity()
{
	Mat4 returnme;
	Mat4IdentityOut(&returnme);
	return returnme;
}

Mat4 FM::Mat4GenTranslate(float x, float y, float z)
{
	Mat4 returnme = Mat4GenIdentity();
	returnme.mat[0][3] = x;
	returnme.mat[1][3] = y;
	returnme.mat[2][3] = z;
	return returnme;
}

Mat4 FM::Mat4Transpose(Mat4 mat)
{
	Mat4 returnme;
	//returnme.mat[0][0] = mat.mat[0][0];
	returnme.mat[0][1] = mat.mat[1][0];
	returnme.mat[0][2] = mat.mat[2][0];
	returnme.mat[0][3] = mat.mat[3][0];
	returnme.mat[1][0] = mat.mat[0][1];
	//returnme.mat[1][1] = mat.mat[1][1];
	returnme.mat[1][2] = mat.mat[2][1];
	returnme.mat[1][3] = mat.mat[3][1];
	returnme.mat[2][0] = mat.mat[0][2];
	returnme.mat[2][1] = mat.mat[1][2];
	//returnme.mat[2][2] = mat.mat[2][2];
	returnme.mat[2][3] = mat.mat[3][2];
	returnme.mat[3][0] = mat.mat[0][3];
	returnme.mat[3][1] = mat.mat[1][3];
	returnme.mat[3][2] = mat.mat[2][3];
	//returnme.mat[3][3] = mat.mat[3][3];
	return returnme;
}

Mat4 FM::Mat4Mat4Multiply(Mat4 a, Mat4 b)
{
	Mat4 returnme;
	uint i, j, k;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			returnme.mat[i][j] = 0;
			for (k = 0; k < 4; k++)
			{
				returnme.mat[i][j] += a.mat[i][k] * b.mat[k][j];
			}
		}
	}
	return returnme;
}

//COMBO FUNCTIONS

Vec3 FM::Vec3Mat4Transform(Vec3 vec, Mat4 mat)
{
	Vec3 returnme;
	returnme.x = mat.mat[0][0]*vec.x+mat.mat[0][1]*vec.y+mat.mat[0][2]*vec.z+mat.mat[0][3]*1.0f;
	returnme.y = mat.mat[1][0]*vec.x+mat.mat[1][1]*vec.y+mat.mat[1][2]*vec.z+mat.mat[1][3]*1.0f;
	returnme.z = mat.mat[2][0]*vec.x+mat.mat[2][1]*vec.y+mat.mat[2][2]*vec.z+mat.mat[2][3]*1.0f;
	return returnme;
}

Vec3 FM::Vec3Mat4TransformNormal(Vec3 vec, Mat4 mat)
{
	Vec3 returnme;
	returnme.x = mat.mat[0][0]*vec.x+mat.mat[0][1]*vec.y+mat.mat[0][2]*vec.z;
	returnme.y = mat.mat[1][0]*vec.x+mat.mat[1][1]*vec.y+mat.mat[1][2]*vec.z;
	returnme.z = mat.mat[2][0]*vec.x+mat.mat[2][1]*vec.y+mat.mat[2][2]*vec.z;
	return returnme;
}

//GRAPHICS SUPPORT
Mat4 FM::Mat4GenPerspectiveProjection(float fovx, float aspect, float near, float far)
{
	Mat4 returnme = Mat4GenZero();
	float xScale = 1.0f / tanf((fovx * DEG2RAD) / 2.0f);
	float yScale = xScale * aspect;
	float frustLen = near - far;
	returnme.mat[0][0] = xScale;
	returnme.mat[1][1] = yScale;
	returnme.mat[2][2] = (far+near)/frustLen;
	returnme.mat[2][3] = (2*near*far)/frustLen;
	returnme.mat[3][2] = -1;
	return returnme;
}

//Mat4 FM::Mat4GenLookAtTransform(Vec3 pos, Vec3 target, Vec3 up)
//{
//	Mat4 returnme = Mat4GenIdentity();
//	Vec3 right;
//	Vec3 forward;
//	
//	forward = Vec3Sub(target, pos);
//	Vec3NormalizeOut(&forward);
//
//	//Vec3NormalizeOut(&up);
//	right = Vec3CrossProduct(forward, up);
//	Vec3NormalizeOut(&right);
//
//	up = Vec3CrossProduct(right, forward);
//	Vec3NormalizeOut(&up);
//
//	returnme.mat[0][0] = right.x;
//	returnme.mat[1][0] = right.y;
//	returnme.mat[2][0] = right.z;
//
//	returnme.mat[3][0] = -Vec3DotProduct(right, pos);
//	
//	returnme.mat[0][1] = up.x;
//	returnme.mat[1][1] = up.y;
//	returnme.mat[2][1] = up.z;
//	
//	returnme.mat[3][1] = -Vec3DotProduct(up, pos);
//	
//	returnme.mat[0][2] = -forward.x;
//	returnme.mat[1][2] = -forward.y;
//	returnme.mat[2][2] = -forward.z;
//	
//	returnme.mat[3][2] = Vec3DotProduct(forward, pos);
//
//	//returnme.mat[0][3] = 0.0f;
//	//returnme.mat[1][3] = 0.0f;
//	//returnme.mat[2][3] = 0.0f;
//
//	//returnme.mat[3][3] = 1.0f;
//	
//	return returnme;
//}

Mat4 FM::Mat4GenLookAtTransform(Vec3 pos, Vec3 target, Vec3 up)
{
	Mat4 returnme = Mat4GenIdentity();
	Vec3 z = Vec3Sub(pos, target);
		Vec3NormalizeOut(&z);
	Vec3 x = Vec3CrossProduct(up, z);
		Vec3NormalizeOut(&x);
	Vec3 y = Vec3CrossProduct(z,x);
	
	returnme.mat[0][0] = x.x;
	returnme.mat[1][0] = x.y;
	returnme.mat[2][0] = x.z;

	returnme.mat[3][0] = -Vec3DotProduct(x, pos);
	
	returnme.mat[0][1] = y.x;
	returnme.mat[1][1] = y.y;
	returnme.mat[2][1] = y.z;
	
	returnme.mat[3][1] = -Vec3DotProduct(y, pos);
	
	returnme.mat[0][2] = z.x;
	returnme.mat[1][2] = z.y;
	returnme.mat[2][2] = z.z;
	
	returnme.mat[3][2] = -Vec3DotProduct(z, pos);

	returnme.mat[0][3] = 0.0f;
	returnme.mat[1][3] = 0.0f;
	returnme.mat[2][3] = 0.0f;

	returnme.mat[3][3] = 1.0f;
	
	return returnme;
}


//CPP additions
/*

Vec3::Vec3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vec3::Vec3()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

Mat4::Mat4()
{
	Mat4ZeroOut(this);
}
*/

//Operators
/*
#ifdef __cplusplus

Vec3 operator+(Vec3 a, Vec3 b)
{
	return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(Vec3 a, Vec3 b)
{
	return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

float operator*(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 operator%(Vec3 a, Vec3 b)
{
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

#endif

*/
