#ifndef CSKELETONCATMULLROMCURVE_H
#define CSKELETONCATMULLROMCURVE_H

#include "CSkeleton.h"

namespace PS{
	namespace BLOBTREE{

		//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
		class  CSkeletonCatmullRomCurve: public CSkeleton
		{
		private:
			vec3f m_center;
			float m_side;
		public:
			CSkeletonCatmullRomCurve() 
			{ 
				m_center.set(0.0f, 0.0f, 0.0f);
				m_side = 1.0f;
			}

			CSkeletonCatmullRomCurve(vec3f position, float side)
			{
				m_center = position;
				m_side = side;				
			}

			CSkeletonCatmullRomCurve(CSkeleton* other)
			{
				setParamFrom(other);
			}

			void setParamFrom(CSkeleton* input)
			{
				CSkeletonCatmullRomCurve* cubeN = dynamic_cast<CSkeletonCatmullRomCurve*>(input);
				this->m_center = cubeN->m_center;
				this->m_side = cubeN->m_side;
			}

			vec3f getPosition() const { return m_center;}
			void setPosition(vec3f pos) { m_center = pos;}

			float getSide() const {return m_side;}
			void setSide(float side) {m_side = side;}


			float distance(vec3f p)
			{				
				return sqrt(squareDistance(p));
			}

			float squareDistance(vec3f p)
			{
				vec3f dif = p - m_center;
				float dist2 = 0.0f;
				float delta;

				float projected;

				//Along X
				projected = dif.dot(vec3f(1.0f, 0.0f, 0.0f));
				if(projected < -1.0f * m_side)
				{
					delta = projected + m_side;
					dist2 += delta*delta;
				}
				else if (projected > m_side)
				{
					delta = projected - m_side;
					dist2 += delta*delta;					
				}

				//Along Y
				projected = dif.dot(vec3f(0.0f, 1.0f, 0.0f));
				if(projected < -1.0f * m_side)
				{
					delta = projected + m_side;
					dist2 += delta*delta;					
				}
				else if (projected > m_side)
				{
					delta = projected - m_side;
					dist2 += delta*delta;					
				}

				//Along Z
				projected = dif.dot(vec3f(0.0f, 0.0f, 1.0f));
				if(projected  < -1.0f * m_side)
				{
					delta = projected + m_side;
					dist2 += delta*delta;					
				}
				else if (projected > m_side)
				{
					delta = projected - m_side;
					dist2 += delta*delta;					
				}

				return dist2;
			}

			float squareDistance(vec3f p, vec3f& outClosestCubePoint)
			{
				vec3f dif = p - m_center;
				float dist2 = 0.0f;
				float delta;

				vec3f closest;

				closest.x = dif.dot(vec3f(1.0f, 0.0f, 0.0f));
				if(closest.x  < -1.0f * m_side)
				{
					delta = closest.x + m_side;
					dist2 += delta*delta;
					closest.x = -1.0f*m_side;
				}
				else if (closest.x > m_side)
				{
					delta = closest.x - m_side;
					dist2 += delta*delta;
					closest.x = m_side;
				}

				closest.y = dif.dot(vec3f(0.0f, 1.0f, 0.0f));
				if(closest.y  < -1.0f * m_side)
				{
					delta = closest.y + m_side;
					dist2 += delta*delta;
					closest.y = -1.0f*m_side;
				}
				else if (closest.y > m_side)
				{
					delta = closest.y - m_side;
					dist2 += delta*delta;
					closest.y = m_side;
				}

				closest.z = dif.dot(vec3f(0.0f, 0.0f, 1.0f));
				if(closest.z  < -1.0f * m_side)
				{
					delta = closest.z + m_side;
					dist2 += delta*delta;
					closest.z = -1.0f*m_side;
				}
				else if (closest.z > m_side)
				{
					delta = closest.z - m_side;
					dist2 += delta*delta;
					closest.z = m_side;
				}

				//Closest point on the cube
				outClosestCubePoint = m_center + closest;
				return dist2;
			}

			vec3f normal(vec3f p)
			{
				vec3f onCube;
				squareDistance(p, onCube);
				vec3f n = onCube - m_center;
				n.normalize();
				return n;
			}

			float getDistanceAndNormal(vec3f p, vec3f& normal)
			{
				vec3f onCube;
				float dist2 = squareDistance(p, onCube);
				normal = onCube - m_center;
				normal.normalize();
				return sqrt(dist2);
			}

			void getName(char * chrName)
			{
				strcpy_s(chrName, MAX_NAME_LEN, "CATMULLROM CURVE");
			}

			bool getExtremes(vec3f& lower, vec3f& upper)
			{
				lower = m_center;
				upper = m_center;
				return false;
			}

			Vol::CVolume* getBoundingVolume(float range)
			{
				Vol::CVolumeBox * b = new Vol::CVolumeBox(m_center - m_side, m_center + m_side);
				return b;
			}

			vec3 getPolySeedPoint()
			{					
				return m_center;		
			}

			void translate(vec3f d)
			{
				m_center += d;
			}

			SkeletonType getType()		{return sktCube;}
		};

	}
}

#endif