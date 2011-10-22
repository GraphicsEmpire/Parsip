//-------------------------------------------------------------------------------------------
//  University of Victoria Computer Science Department 
//	Assignment02
//  Course title: Computer Animation CSC578B
//	Please report any comments or feedback to: pouryash@uvic.ca
//
//	Author: Pourya Shirazian 
//  Student# V00681446
//	Date  : January 2009
//-------------------------------------------------------------------------------------------


#ifndef CEASEINEASEOUT_H
#define CEASEINEASEOUT_H

#include "PS_FrameWork/include/mathHelper.h"
#include <math.h>


class CEaseInEaseOut
{
public:
	static float linearEase(float t)
	{
		return (2 - t) * t;

	}

	static float sineEase(float t)
	{
                return ((sin(t * Pi - Pi/2) + 1)/2);
	}

	static float sineEase(float t, float k1,float k2)
	{
                float f = k1 * 2/Pi + k2 - k1 + (1.0 - k2)*2/Pi;
		float s = 0.0f;

		if (t < k1)
		{
                        s = k1 * (2/Pi) * (sin((t/k1) * (Pi/2) - Pi/2) + 1);
		}
		else if(t < k2)
		{
                        s = (2*k1/Pi + t - k1);

		}
		else
		{
                        s = 2*k1/Pi + k2 - k1 + ((1 - k2) * (2 / Pi)) * sin(((t - k2)/(1.0 - k2))*Pi/2);

		}
		
		return (s/f);
		//return s;
	}
};


#endif
