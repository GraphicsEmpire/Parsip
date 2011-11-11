//***************************************************************************
// Author: Pourya Shirazian 
// Quaternion math for efficient rotations. Use quaternions to avoid 
// gymbal lock problem.
//***************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "PS_Quaternion.h"


#define DELTA 1e-6     // error tolerance

namespace PS{
	namespace MATH{

static float ranf(void)
{
	float v = (float) rand() *(1.0f/32767.0f);
	return v;
}


/*SDOC***********************************************************************

	Name:		gluQuatSlerp_EXT

	Action:	Smoothly (spherically, shortest path on a quaternion sphere)
			interpolates between two UNIT quaternion positions

	Params:   GLQUAT (first and second quaternion), float (interpolation
			parameter [0..1]), GL_QUAT (resulting quaternion; inbetween)

	Returns:  nothing

	Comments: Most of this code is optimized for speed and not for readability

			As t goes from 0 to 1, qt goes from p to q.
		slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega)

***********************************************************************EDOC*/
void  CQuaternion::Slerp(const CQuaternion &from,const CQuaternion &to,float t)  // smooth interpolation.
{
	float           to1[4];
	float           omega, cosom, sinom;
	float           scale0, scale1;

	// calc cosine
	cosom = from.q.x * to.q.x + from.q.y * to.q.y + from.q.z * to.q.z + from.w * to.w;

	// adjust signs (if necessary)
	if ( cosom < 0.0f )
	{
		cosom  = -cosom;
		to1[0] = -to.q.x;
		to1[1] = -to.q.y;
		to1[2] = -to.q.z;
		to1[3] = -to.w;
	}
	else
	{
		to1[0] = to.q.x;
		to1[1] = to.q.y;
		to1[2] = to.q.z;
		to1[3] = to.w;
	}

	// calculate coefficients
	if ( (1.0f - cosom) > DELTA )
	{
		// standard case (slerp)
		omega = (float)acos(cosom);
		sinom = (float)sin(omega);
		scale0 = (float)sin((1.0f - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	}
	else
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	q.x = scale0 * from.q.x + scale1 * to1[0];
	q.y = scale0 * from.q.y + scale1 * to1[1];
	q.z = scale0 * from.q.z + scale1 * to1[2];
	w   = scale0 * from.w + scale1 * to1[3];

}



/*SDOC***********************************************************************

	Name:		gluQuatLerp_EXT

	Action:   Linearly interpolates between two quaternion positions

	Params:   GLQUAT (first and second quaternion), float (interpolation
			parameter [0..1]), GL_QUAT (resulting quaternion; inbetween)

	Returns:  nothing

	Comments: fast but not as nearly as smooth as Slerp

***********************************************************************EDOC*/
void  CQuaternion::Lerp(const CQuaternion &from,const CQuaternion &to,float t)   // fast interpolation, not as smooth.
{
	float           to1[4];
	float           cosom;
	float           scale0, scale1;

	// calc cosine
	cosom = from.q.x * to.q.x + from.q.y * to.q.y + from.q.z * to.q.z + from.w * to.w;

	// adjust signs (if necessary)
	if ( cosom < 0.0f )
	{
		to1[0] = -to.q.x;
		to1[1] = -to.q.y;
		to1[2] = -to.q.z;
		to1[3] = -to.w;
	}
	else
	{
		to1[0] = to.q.x;
		to1[1] = to.q.y;
		to1[2] = to.q.z;
		to1[3] = to.w;
	}
	// interpolate linearly
	scale0 = 1.0f - t;
	scale1 = t;

	// calculate final values
	q.x = scale0 * from.q.x + scale1 * to1[0];
	q.y = scale0 * from.q.y + scale1 * to1[1];
	q.z = scale0 * from.q.z + scale1 * to1[2];
	w   = scale0 * from.w +   scale1 * to1[3];

}


void CQuaternion::RandomRotation(bool x,bool y,bool z)
{
	float ex = 0;
	float ey = 0;
	float ez = 0;

	if ( x )
	{
		ex = ranf()*Pi*2;
	}
	if ( y )
	{
		ey = ranf()*Pi*2;
	}
	if ( z )
	{
		ez = ranf()*Pi*2;
	}

	fromEuler(ex,ey,ez);

}

}
}
