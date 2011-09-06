#ifndef CQUARICPOINT_H
#define CQUARICPOINT_H

#include "math.h"
#include "CBlobTree.h"

namespace PS{
	namespace BLOBTREE{

		class  CQuadricPoint : public CBlobTree
		{
		protected:
			vec3f	m_ptPosition;
			float m_fScale;
			float m_fRadius;

		public:
			CQuadricPoint()	
			{		
				m_fScale = 1.0f;
				m_fRadius = 1.0f;
			}

			CQuadricPoint(vec3f pos, float fieldRadius, float fieldScale)
			{
				m_ptPosition = pos;
				m_fRadius = fieldRadius;
				m_fScale = fieldScale;				
			}

			//Accessor Functions
			vec3 getPosition() const {return m_ptPosition;}
			float getFieldScale() const {return m_fScale;}
			float getFieldRadius() const {return m_fRadius;}


			float fieldValue(vec3f p)
			{
				float fDist2 = m_ptPosition.dist2(p);

				float fValue = (1.0f - (fDist2 / (m_fRadius * m_fRadius)));
				if(fValue <= 0.0f)
					return 0.0f;

				return m_fScale  * fValue * fValue;
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
				strcpy_s(chrName, MAX_NAME_LEN, "QUADRICPOINT");				
			}

			COctree computeOctree()
			{
				m_octree.set(m_ptPosition - m_fRadius, m_ptPosition + m_fRadius);
				return m_octree;
			}


			bool isOperator() { return false;}
			BlobNodeType getNodeType() {return bntPrimQuadricPoint;}
		};

	}
}
#endif
