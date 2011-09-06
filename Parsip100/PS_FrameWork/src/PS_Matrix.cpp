//-------------------------------------------------------------------------------------------
//  University of Victoria Computer Science Department 
//	Assignment04
//  Course title: Computer Animation CSC578B
//	Please report any comments or feedback to: pouryash@uvic.ca
//
//	Author: Pourya Shirazian 
//  Student# V00681446
//	Date  : January 2009
//-------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "PS_Matrix.h"
#include "PS_Quaternion.h"

namespace PS{
	namespace MATH{


CMatrix gIdentity;

static void ComputeEulerMatrix( CMatrix &matrix,
								float cx, float cy, float cz,
								float sx, float sy, float sz )
{
	// After heavily refactoring the matrix math, this is the
	// final distilled product...

	matrix.mElement[0][0] = cz*cy;
	matrix.mElement[0][1] = sz*cy;
	matrix.mElement[0][2] = -sy;
	matrix.mElement[0][3] = 0.0f;

	matrix.mElement[1][0] = cz*sy*sx - sz*cx;
	matrix.mElement[1][1] = sz*sy*sx + cz*cx;
	matrix.mElement[1][2] = cy*sx;
	matrix.mElement[1][3] = 0.0f;

	matrix.mElement[2][0] = cz*sy*cx + sz*sx;
	matrix.mElement[2][1] = sz*sy*cx - cz*sx;
	matrix.mElement[2][2] = cy*cx;
	matrix.mElement[2][3] = 0.0f;

	matrix.mElement[3][0] = 0.0f;
	matrix.mElement[3][1] = 0.0f;
	matrix.mElement[3][2] = 0.0f;
	matrix.mElement[3][3] = 1.0f;
}


void CMatrix::rotate(float x,float y,float z)
{
	ComputeEulerMatrix( *this,
						cosf(x), cosf(y), cosf(z),
						sinf(x), sinf(y), sinf(z) );
}

void CMatrix::rotate(float angle, float x, float y, float z)
{
	CQuaternion q;
	vec3f axis(x,y,z);
	q.fromAngleAxis(angle, axis);
	q.toMatrix(*this);
}

//***************************************************************************
void CMatrix::set(const vec3f &facing,const vec3f &N)
{
	//n cross ((h cross n) /|h cross n|)
	vec3f r0;
	r0.cross(facing,N);
	r0.normalize();
	vec3f v0;
	v0.cross(N,r0);

	vec3f v1;

	v1.cross(N,v0);

	v1.normalize();

	mElement[0][0] = v0.x;
	mElement[0][1] = v0.y;
	mElement[0][2] = v0.z;
	mElement[0][3] = 0;

	mElement[1][0] = v1.x;
	mElement[1][1] = v1.y;
	mElement[1][2] = v1.z;
	mElement[1][3] = 0;

	mElement[2][0] = N.x;
	mElement[2][1] = N.y;
	mElement[2][2] = N.z;
	mElement[2][3] = 0;

	mElement[3][0] = 0;
	mElement[3][1] = 0;
	mElement[3][2] = 0;
	mElement[3][3] = 1;
}


void CMatrix::GetPose(const vec3f &pos,
							const CQuaternion            &rot,
							vec3f       &tpos,
							CQuaternion                  &trot) const
{
        CMatrix child;
        CMatrix combine;

	rot.toMatrix(child);
	child.setTranslate(pos);

	combine.multiply(child,*this);

	combine.GetTranslation(tpos);
	trot.fromMatrix(combine);

}

void CMatrix::Report(const char *header) const
{
#if USE_LOG
	printf("===============================================\n");
	printf("Matrix Report (%s)\n", header);
	printf("===============================================\n");
	printf("[0][0]%0.2f [1][0]%0.2f [2][0]%0.2f [3][0]%0.2f\n",mElement[0][0],mElement[1][0],mElement[2][0],mElement[3][0]);
	printf("[0][1]%0.2f [1][1]%0.2f [2][1]%0.2f [3][1]%0.2f\n",mElement[0][1],mElement[1][1],mElement[2][1],mElement[3][1]);
	printf("[0][2]%0.2f [1][2]%0.2f [2][2]%0.2f [3][2]%0.2f\n",mElement[0][2],mElement[1][2],mElement[2][2],mElement[3][2]);
	printf("[0][3]%0.2f [1][3]%0.2f [2][3]%0.2f [3][3]%0.2f\n",mElement[0][3],mElement[1][3],mElement[2][3],mElement[3][3]);
	printf("===============================================\n");
#endif
}

void CMatrix::set(const char *data)
{
	int index = 0;

	identity();

	while ( *data && *data == 32 ) data++;

	while ( *data )
	{
		float v = (float)atof( data );

		int iy = index / 4;
		int ix = index % 4;

		mElement[ix][iy] = v;

		while ( *data && *data != 32 ) data++;
		while ( *data && *data == 32 ) data++;

		index++;
	}
}

CMatrix::CMatrix(const float *quat,const float *pos)
{
	if ( quat )
	{
		CQuaternion q(quat);
		q.toMatrix(*this);
	}
	else
	{
		identity();
	}
	if ( pos ) setTranslate(pos);
}

void CMatrix::LookAtMatrix( const vec3f &from,const vec3f &to,const vec3f &up )
{
	identity();

	vec3f row2 = to - from;
	row2.normalize();

	vec3f row0;
	vec3f row1;

	row0.cross( up, row2 );
	row1.cross( row2, row0 );

	row0.normalize();
	row1.normalize();

	mElement[0][0] = row0.x;
	mElement[0][1] = row0.y;
	mElement[0][2] = row0.z;

	mElement[1][0] = row1.x;
	mElement[1][1] = row1.y;
	mElement[1][2] = row1.z;

	mElement[2][0] = row2.x;
	mElement[2][1] = row2.y;
	mElement[2][2] = row2.z;

	mElement[3][0] = from.x;
	mElement[3][1] = from.y;
	mElement[3][2] = from.z;
}
}
}