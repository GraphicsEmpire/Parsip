#pragma once
#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "CVector.h"
#include "mathHelper.h"

namespace PS{
class CBoundingBox
{
public:
	typedef enum BOXSIDE {sideX, sideY, sideZ};

	CBoundingBox() { reset();}
	CBoundingBox(vec3 minCorner, vec3 maxCorner)
	{
		m_minCorner = minCorner;
		m_maxCorner = maxCorner;
	}

	void expand(vec3 p)
	{
		m_minCorner.x = MATHMIN(m_minCorner.x, p.x);
		m_minCorner.y = MATHMIN(m_minCorner.y, p.y);
		m_minCorner.z = MATHMIN(m_minCorner.z, p.z);

		m_maxCorner.x = MATHMAX(m_maxCorner.x, p.x);
		m_maxCorner.y = MATHMAX(m_maxCorner.y, p.y);
		m_maxCorner.z = MATHMAX(m_maxCorner.z, p.z);
	}

	BOXSIDE getMinSide()
	{
		vec3 sides = m_maxCorner - m_minCorner;
		float f = MATHMIN(MATHMIN(sides.x, sides.y), sides.z);
		if(f == sides.x)
			return sideX;
		else if(f == sides.y)
			return sideY;
		else
			return sideZ;
	}

	BOXSIDE getMaxSide()
	{
		vec3 sides = m_maxCorner - m_minCorner;
		float f = MATHMAX(MATHMAX(sides.x, sides.y), sides.z);
		if(f == sides.x)
			return sideX;
		else if(f == sides.y)
			return sideY;
		else
			return sideZ;
	}

	vec3 middle()
	{
		return (m_minCorner + m_maxCorner) * 0.5f;
	}

	vec3 getClosestCorner(vec3 v) const
	{
		//XYZ 000 to 111
		const int maskX = 1;
		const int maskY = 2;
		const int maskZ = 4;
		vec3 pos, closestPos; 
		float dist = FLT_MAX;
		float len;
		for(int i=0; i<8; i++)
		{
			if((i & maskX) == 0)
				pos.x = m_minCorner.x;
			else 
				pos.x = m_maxCorner.x;
			if((i & maskY) == 0)
				pos.y = m_minCorner.y;
			else 
				pos.y = m_maxCorner.y;
			if((i & maskZ) == 0)
				pos.z = m_minCorner.z;
			else 
				pos.z = m_maxCorner.z;

			len = pos.distance(v);
			if(len < dist)
			{
				dist = len;
				closestPos = pos;
			}			
		}

		return closestPos;
	}

	//Corners Getter and Setters
	void setMinCorner(vec3 minCorner) { m_minCorner = minCorner;}
	vec3 getMinCorner() const { return m_minCorner;}

	void setMaxCorner(vec3 maxCorner) { m_maxCorner = maxCorner;}
	vec3 getMaxCorner() const { return m_maxCorner;}
	
	void reset()
	{
		m_minCorner = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		m_maxCorner = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}

	bool correct()
	{
		if((m_minCorner.x <= m_maxCorner.x)&&
		   (m_minCorner.y <= m_maxCorner.y)&&
		   (m_minCorner.z <= m_maxCorner.z))
		   return true;
		return false;
	}
public:
	vec3 m_minCorner;
	vec3 m_maxCorner;

	//const int L =	0;  /* left direction:	-x, -i */
	//const int R =	1;  /* right direction:	+x, +i */
	//const int B =	2;  /* bottom direction: -y, -j */
	//const int T =	3;  /* top direction:	+y, +j */
	//const int N =	4;  /* near direction:	-z, -k */
	//const int F =	5;  /* far direction:	+z, +k */
	//const int LBN =	0;  /* left bottom near corner  */
	//const int LBF =	1;  /* left bottom far corner   */
	//const int LTN =	2;  /* left top near corner     */
	//const int LTF =	3;  /* left top far corner      */
	//const int RBN =	4;  /* right bottom near corner */
	//const int RBF =	5;  /* right bottom far corner  */
	//const int RTN =	6;  /* right top near corner    */
	//const int RTF =	7;  /* right top far corner     */

};

}

#endif