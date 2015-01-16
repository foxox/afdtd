#ifndef COMMONH
#define COMMONH

#include <limits>
#include <iostream>
#include <cmath>

#include <string>
#include <sstream>

//TODO: Swap these on VC10+
//#include <cstdint>
typedef unsigned int uint_fast32_t;
typedef unsigned int uint_fast16_t;

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../wavio/wavio.h"
#include "../ImageIO/ImageIO.h"

//#define math_pi 3.14159265358979f

#define i2(arr,i,j) arr[((i) * (arr ## JSIZE)) + (j)]

//TODO: Have these use precomputed JSIZE * KSIZE, JSIZE * KSIZE * LSIZE, KSIZE*LSIZE

#define i3(arr,i,j,k) arr[((i) * ((arr ## JSIZE) * (arr ## KSIZE))) + (j) * (arr ## KSIZE) + (k)]
//#define i3s(arr,i,j,k,jSize,kSize) arr[( (i) * (jSize) * (kSize) ) + (j) * (kSize) + (k)]

#define i4(arr,i,j,k,l) arr[(  (i) * ( (arr ## JSIZE) * (arr ## KSIZE) * (arr ## LSIZE) )  ) + (j) * (  (arr ## KSIZE) * (arr ## LSIZE)  ) + (k) * (arr ## LSIZE) + (l)]
//#define i4s(arr,i,j,k,l,jSize,kSize,lSize) arr[( (i) * (jSize) * (kSize) * (lSize) ) + (j) * ((kSize) * (lSize)) + (k) * (lSize) + (l)]

#define strprintf_s(str, const_strlen, ...) {char buf[const_strlen]; sprintf_s<const_strlen>(buf, __VA_ARGS__); str = buf;}

#define strstreamed(arg) ((ostringstream st()) << arg).str()

struct IndexTuple
{
	unsigned int i;
	unsigned int j;
	unsigned int k;
};

struct IndexMetricTuple
{
	unsigned int i;
	unsigned int j;
	unsigned int k;
	long metric;
	uint_fast16_t neighborsUpdated;
	size_t metricMeasuredAt;
#ifdef DEBUG
	size_t computedAt;
#endif
};

#define WAVESPREAD 0.001f
#define WAVEOFFSET 0.0025f

//too small I think.
//#define WAVESPREAD 0.0003f
//#define WAVEOFFSET 0.001f



#endif