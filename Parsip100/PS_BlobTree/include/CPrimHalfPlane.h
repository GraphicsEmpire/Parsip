#ifndef CHALFPLANE_H
#define CHALFPLANE_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
	namespace BLOBTREE{

		class  CHalfPlane : public CBlobTree
		{
		private:
			vec3f m_position;
			vec3f m_normal;
		public:
			CHalfPlane() {;}
			CHalfPlane(vec3f p, vec3f n)
			{
				m_position = p;
				m_normal = n;
				m_normal.normalize();
			}

			void setPosition(vec3f p)
			{
				m_position = p;
			}

			void setNormal(vec3f n)
			{
				m_normal = n;
				m_normal.normalize();
			}

			float fieldValue(vec3f p)
			{
				vec3f d = p - m_position;
				d.normalize();
				
				if(Absolutef(d.getAngleDeg(m_normal)) <= 90.0f)
					return ISO_VALUE;
				else
					return 0.0f;				
			}

			float curvature(vec3f p)
			{
				return 0.0f;
			}

			vec4f baseColor(vec3f p)
			{		
				return m_color;
			}

			CMaterial baseMaterial(vec3f p)
			{
				return m_material;
			}

			void getName(char * chrName)
			{
				strcpy_s(chrName, MAX_NAME_LEN, "HALFPLANE");		
			}

			//Octree manipulation
			COctree computeOctree()
			{
				m_octree = m_children[0]->getOctree();
				for(size_t i = 1; i < m_children.size(); i++)
					m_octree.csgUnion(m_children[i]->getOctree());		

				return m_octree;
			}


			bool isOperator() { return false;}
			BlobNodeType getNodeType() {return bntPrimHalfPlane;}
		};

	}
}

#endif

