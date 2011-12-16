#ifndef _GLOBALFUNCTIONS_H
#define _GLOBALFUNCTIONS_H

#include <QString>
#include <stdio.h>
#include <stdlib.h>

#include "PS_FrameWork/include/PS_String.h"
#include "PS_BlobTree/include/CBlobTree.h"
#include "PS_FrameWork/include/PS_Material.h"

using namespace PS;
using namespace PS::BLOBTREE;

//Solution from:
//http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
float Brightness(vec4f c);
vec4f TextColor(vec4f bgColor);

CMaterial ColorToMaterial(const vec4f& color);

DAnsiStr AxisToString(MajorAxices axis);
MajorAxices StringToAxis(DAnsiStr strAxis);

vec3f StringToVec3(DAnsiStr strVal);
DAnsiStr Vec3ToString(vec3f v);

vec4f StringToVec4(DAnsiStr strVal);
DAnsiStr Vec4ToString(vec4f v);

bool FindSeedPoint(CBlobNode* lpNode, bool bFindHot, float iso_value, int ctTries, vec3f& p, float& fp, size_t& fieldEvaluations);
bool FindSeedPoint(CBlobNode* lpNode, bool bFindHot, float iso_value, int ctTries, float search_step, vec3f search_dir, vec3f& p, float& fp, size_t& fieldEvaluations);

inline bool intersects(const vec4f& lo1, const vec4f& hi1, const vec4f& lo2, const vec4f& hi2)
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
QString printToQStr(const char* pFmt, ...);


#endif
