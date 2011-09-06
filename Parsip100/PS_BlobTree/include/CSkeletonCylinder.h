#ifndef CSKELETONCYLINDER_H
#define CSKELETONCYLINDER_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonCylinder: public CSkeleton
{
private:
	vec3f m_position;
	vec3f m_direction;
	float m_radius;
	float m_height;
public:
	CSkeletonCylinder() 
	{ 
		m_position.set(0.0f, 0.0f, 0.0f);
		m_direction.set(0.0f, 1.0f, 0.0f);
		m_radius = 1.0f;
		m_height = 1.0f;
	}
	
	CSkeletonCylinder(vec3f position, vec3f direction, float radius, float height)
	{
		m_position = position;
		m_direction = direction;
		m_radius = radius;
		m_height = height;

		m_direction.normalize();
	}

	CSkeletonCylinder(CSkeleton* other)
	{
		setParamFrom(other);
	}

	void setParamFrom(CSkeleton* input)
	{
		CSkeletonCylinder* cylN = dynamic_cast<CSkeletonCylinder*>(input);
		this->m_position = cylN->m_position;
		this->m_direction = cylN->m_direction;
		this->m_radius    = cylN->m_radius;
		this->m_height    = cylN->m_height;
	}

	vec3f getPosition() const { return m_position;}
	void setPosition(vec3f pos) { m_position = pos;}

	vec3f getDirection() const { return m_direction;}
	void setDirection(vec3f dir) { m_direction = dir;}

	float getHeight() const { return m_height;}
	void setHeight(float height) { m_height = height;}

	float getRadius() const { return m_radius;}
	void setRadius(float radius) { m_radius = radius;}

	float distance(vec3f p)
	{
		return sqrt(squareDistance(p));
	}

	float squareDistance(vec3f p)
	{
		vec3f pos = p - m_position;
		float y = pos.dot(m_direction);
		float x = maxf(0.0f, sqrt(pos.length2() - y*y) - m_radius);

		//Make y 0.0 if it is positive and less than height 
		// For Hemispherical caps
		if(y > 0)
			y = maxf(0.0f, y - m_height);

		return x*x + y*y;
	}
	
	vec3f normal(vec3f p)
	{
		vec3f n = p - m_position;
		n.normalize();
		return n;
	}
	
	float getDistanceAndNormal(vec3f p, vec3f& normal)
	{
		vec3f pos = p - m_position;
		normal = pos;
		normal.normalize();

		float y = pos.dot(m_direction);
		float x = maxf(0.0f, sqrt(pos.length2() - y*y) - m_radius);
		if(y > 0)
			y = maxf(0.0f, y - m_height);

		return sqrt(x*x + y*y);

	}

	void getName(char * chrName)
	{
		strcpy_s(chrName, MAX_NAME_LEN, "CYLINDER");
	}

	bool getExtremes(vec3f& lower, vec3f& upper)
	{
		lower = m_position - m_radius;
		upper = m_position + (m_height + m_radius) * m_direction;
		return true;
	}

	Vol::CVolume* getBoundingVolume(float range)
	{
		m_direction.normalize();
		float r = sqrtf(m_radius * m_radius + 0.25f * m_height * m_height);
		Vol::CVolumeSphere * s = new Vol::CVolumeSphere(vec3(), r + range);
		
		vec3 d  = (0.5f * m_height) * m_direction;					
		vec3 da = d.vectorAbs();
		vec3 bv = da + vec3(cos(m_direction.x) + range, cos(m_direction.y) + range, cos(m_direction.z) + range).vectorAbs();

		vec3 p = m_position + da;
		Vol::CVolumeBox * b = new Vol::CVolumeBox(p - bv, p + bv);
		if (s->size() > b->size())
		{
			s->setCenter(m_position + d);
			delete b;
			b = NULL;

			return s;
		}
		else
		{
			delete s; 
			s = NULL;
			return b;
		}

	}

	vec3 getPolySeedPoint()
	{		
		//Obtain a perpendicular vector to the direction vector
		vec3f perp = m_direction.findArbitaryNormal();
		return m_position + (m_radius + 0.001f)*perp;		
	}

	void translate(vec3f d)
	{
		m_position += d;
	}

	SkeletonType getType()		{return sktCylinder;}
};

}
}

#endif