#ifndef _GLOBALFUNCTIONS_H
#define _GLOBALFUNCTIONS_H

#include <QString>
#include <stdio.h>
#include <stdlib.h>

#include "PS_FrameWork/include/PS_String.h"
#include "PS_BlobTree/include/CBlobTree.h"

using namespace PS::BLOBTREE;

//Solution from:
//http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
float Brightness(vec4f c);
vec4f TextColor(vec4f bgColor);

DAnsiStr AxisToString(MajorAxices axis);
MajorAxices StringToAxis(DAnsiStr strAxis);

vec3f StringToVec3(DAnsiStr strVal);
DAnsiStr Vec3ToString(vec3f v);

vec4f StringToVec4(DAnsiStr strVal);
DAnsiStr Vec4ToString(vec4f v);

bool FindSeedPoint(CBlobTree* lpNode, bool bFindHot, float iso_value, int ctTries, vec3f& p, float& fp, size_t& fieldEvaluations);
bool FindSeedPoint(CBlobTree* lpNode, bool bFindHot, float iso_value, int ctTries, float search_step, vec3f search_dir, vec3f& p, float& fp, size_t& fieldEvaluations);

CBlobTree* cloneNode(CBlobTree* node, int id = 0);
CBlobTree* cloneBlobTree(CBlobTree* input, int rootID = 0, int* lpCtClonned = NULL);


__inline bool intersects(const vec4f& lo1, const vec4f& hi1, const vec4f& lo2, const vec4f& hi2)
{
	if ((lo1.x >= hi2.x) || (hi1.x <= lo2.x))
		return false;
	if ((lo1.y >= hi2.y) || (hi1.y <= lo2.y))
		return false;
	if ((lo1.z >= hi2.z) || (hi1.z <= lo2.z))
		return false;

	return true;
}
//////////////////////////////////////////////////////////////////////////
bool isEquivalentOp( CBlobTree* a, CBlobTree* b );
bool isCompactableOp( BlobNodeType bnt );
CBlobTree* CompactBlobTree(CBlobTree* input);

//////////////////////////////////////////////////////////////////////////
QString printToQStr(const char* pFmt, ...);

/* align_size has to be a power of two !! */
//void* aligned_malloc_ps(size_t size, size_t align_size);

//void aligned_free_ps(void *ptr);

/*
int main( void )
{
	// declare the type of data pointer that
	// we want to allocate...
	char* pData = 0;

	pData = aligned_malloc( 64, 4 ); // let's get 64 bytes aligned on a 4-byte boundry

	if( pData != NULL )
	{
		; // do something with our allocated memory

		// free it...
		aligned_free( pData );
		pData = 0;
	}

	return 0;
}
*/
#endif
