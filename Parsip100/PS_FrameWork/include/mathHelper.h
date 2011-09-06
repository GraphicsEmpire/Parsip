#pragma once
#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <cmath>
#include <limits>
#include<math.h>

//Constants for conversion
const float DEG_TO_RAD = ((2.0f * 3.14152654f) / 360.0f);
const float RAD_TO_DEG = (360.0f / (2.0f * 3.141592654f));

//PI
const float Pi       =  3.14159265358979323846f;

//PI over 2
const float PiOver2  = Pi / 2.0f;

//2PI
const float TwoPi    = Pi * 2.0f;

// error tolerance for check
const float EPSILON  = 0.0001f;

//////////////////////////////////////////////////////////////////////////
// Angle Conversions
#define	DEGTORAD(x)	( ((x) * Pi) / 180.0f )
#define	RADTODEG(x)	( ((x) * 180.0f) / Pi )

#define	SQR(x)		( (x) * (x) )

//Min and Max
#define MATHMIN(_A_,_B_) ((_A_) < (_B_))?(_A_):(_B_)
#define MATHMAX(_A_,_B_) ((_A_) > (_B_))?(_A_):(_B_)

//Angle between Vectors
#define VAngleDegree(p,q) ((acos((p).x * (q).x + (p).y * (q).y + (p).z * (q).z) * 180.0f) / Pi)

// limits a value to low and high
#define LIMIT_RANGE(low, value, high)	{	if (value < low)	value = low;	else if(value > high)	value = high;	}

// set float to 0 if within tolerance
#define ZERO_CLAMP(x)	((((x) > 0 && ((x) < EPSILON)) || ((x) < 0 && ((x) > -EPSILON)))?0.0f:(x) )

//////////////////////////////////////////////////////////////////////////
__inline int log2f(float x)
{
	unsigned int ix = (unsigned int&)x;
	unsigned int exp = (ix >> 23) & 0xFF;
	int log2 = int(exp) - 127;
	return log2;
}

__inline int log2i(unsigned int x)
{
	return log2f((float)x);
}

__inline float lerp(float t, float s1, float s2)
{
	return (1 - t)*s1 + t*s2;
}

__inline void clampf(float &v, float min, float max)
{
    if(v < min)
        v = min;
    else if(v > max)
        v = max;
}

__inline void clampd(double &v, double min, double max)
{
    if(v < min)
        v = min;
    else if(v > max)
        v = max;
}

__inline bool FLOAT_EQ(float x, float v)
{
	return ( ((v - EPSILON) < x) && (x < (v + EPSILON)) );
}

__inline bool FLOAT_EQ(float x, float v, float epsi)
{
	return ( ((v - epsi) < x) && (x < (v + epsi)) );	
}

//Swap 2 floating point numbers
__inline void SWAP(float &x, float &y)
{	
	float temp;	
	temp = x;	
	x = y;	
	y = temp;	
}

__inline void SWAP(int &x, int &y)
{
	int temp; 
	temp = x; 
	x = y;
	y = temp;
}

//Round for Accuracy. The same method we used by AMin
// round a float to a specified degree of accuracy
__inline float ROUND(const float value, const int accuracy)
{
	double integer, fraction;

	fraction = modf(value, &integer);		// get fraction and int components

	return (float(integer + (float(int(fraction*pow(float(10), float(accuracy))))) / pow(float(10), float(accuracy)) ) );
}

// If num is less than zero, we want to return the absolute value of num.
// This is simple, either we times num by -1 or subtract it from 0.
__inline double Absoluted(double num)
{	
	if(num < 0)
		return (0 - num);

	// Return the original number because it was already positive
	return num;
}

__inline float Absolutef(float num)
{	
	if(num < 0)
		return (0 - num);

	// Return the original number because it was already positive
	return num;
}

__inline float maxf(float a, float b)
{
	return (a>b)?a:b;
}

__inline float minf(float a, float b)
{
	return (a<b)?a:b;
}

template <class T>
__inline T gmax(T a, T b)
{
	return (a > b)?a:b;
}

template <class T>
__inline T gmin(T a, T b)
{
	return (a < b)?a:b;
}

#ifdef max
#undef max
#endif // #ifdef min
template <typename T>
T getMaxLimit(void)
{
	return std::numeric_limits<T>::max();
}

#ifdef min
#undef min
#endif // #ifdef min
template <typename T>
T getMinLimit(void)
{
	return std::numeric_limits<T>::min();
}

//Fast SquareRoot Found on internet http://www.codemaestro.com/reviews/9
__inline float FastSqrt(float number) 
{
	long i;
	float x, y;
	const float f = 1.5F;

	x = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = * ( float * ) &i;
	y  = y * ( f - ( x * y * y ) );
	y  = y * ( f - ( x * y * y ) );
	return number * y;
}

__inline float FastInvSqrt(float x)
{
	float xhalf = 0.5f*x;
	int i = *(int*)&x; // get bits for floating value
	i = 0x5f375a86- (i>>1); // gives initial guess y0
	x = *(float*)&i; // convert bits back to float
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
	return x;
}


#endif
