#ifndef CRicciBlend_H
#define CRicciBlend_H

#include <math.h>
#include "CBlobTree.h"

#define DEFAULT_RICCI_POWER 2.0f
namespace PS{
namespace BLOBTREE{

	class  CRicciBlend : public CBlobTree
	{
	private:
		float m_n;
		float m_nDiv;

	public:
		CRicciBlend()
		{
			setN(DEFAULT_RICCI_POWER);
		}
		
		CRicciBlend(float n)
		{
			setN(n);
		}
		CRicciBlend(CBlobTree * child, float n = DEFAULT_RICCI_POWER)
		{
			addChild(child);
			setN(n);
		}

		CRicciBlend(CBlobTree * child1, CBlobTree * child2, float n = DEFAULT_RICCI_POWER)
		{
			addChild(child1);
			addChild(child2);
			setN(n);
		}

		CRicciBlend(CBlobTree * child1, CBlobTree * child2, CBlobTree * child3, float n = DEFAULT_RICCI_POWER)
		{
			addChild(child1);
			addChild(child2);
			addChild(child3);
			setN(n);
		}

		CRicciBlend(BLOBTREECHILDREN children)
		{
			addChild(children);
		}

		void setParamFrom(CBlobTree* input)
		{
			CRicciBlend* inputRicci = dynamic_cast<CRicciBlend*>(input);
			this->m_n = inputRicci->m_n;
			this->m_nDiv = inputRicci->m_nDiv;
		}

		float getN()
		{
			return m_n;				
		}
			
		float getNDiv()
		{
			return m_nDiv;
		}

		void setN(float n)
		{
			if(n != 0.0f)
			{
				m_n = n;
				m_nDiv = 1.0f / n;
			}
		}

		float fieldValue(vec3f p)
		{		
			float result = 0.0f;

			for(size_t i=0; i < m_children.size(); i++)
			{
				result += pow(m_children[i]->fieldValue(p), m_n);
			}		

			return pow(result, m_nDiv);
		}

		float curvature(vec3f p)
		{
			float sum = 0.0f;
			float c = 0.0f;
			float f = 0.0f;
			float curvature;

			for(size_t i=0; i < m_children.size(); i++)
			{
				f = m_children[i]->fieldValue(p);
				if( f > 0.0f)
				{
					curvature = m_children[i]->curvature(p);
					c += curvature * curvature * f;
					sum += f;
				}
			}

			if( sum == 0.0f)
				return m_children[0]->curvature(p);
			else 
				return sqrt(c  / sum);
		}

		vec4f baseColor(vec3f p)
		{		
			vec4f color;
			float f = 0.0f;
			float sum = 0.0f;
			for(size_t i = 0; i < m_children.size(); i++)
			{
				f = m_children[i]->fieldValue(p);
				if(f > 0.0f)
				{
					color += m_children[i]->baseColor(p) * f;
					sum += f;
				}
			}

			if(sum == 0.0f)
				return m_children[0]->baseColor(p);
			else
				return color*static_cast<float>(1.0f /(float)sum);

		}

		CMaterial baseMaterial(vec3f p)
		{
			float sum = 0.0f;
			CMaterial m;
			float f = 0.0f;

			for(size_t i = 0; i < m_children.size(); i++)
			{
				f = m_children[i]->fieldValue(p);
				if(f > 0.0f)
				{
					m += m_children[i]->baseMaterial(p) * f;
					sum += f;
				}
			}

			if(sum == 0.0f)
				return m_children[0]->baseMaterial(p);
			else
				return m*static_cast<float>(1.0f /(float)sum);
		}

		CBlobTree * getChildMax(vec3f p)
		{
			if(countChildren() == 0)
				return NULL;

			float maxField = m_children[0]->fieldValue(p);
			size_t iMax = 0;
			float curField;
			for(size_t i = 1; i < m_children.size(); i++)
			{
				curField = m_children[i]->fieldValue(p);
				if(curField > maxField)
				{
					maxField = curField;
					iMax = i;
				}
			}
			return m_children[iMax];
		}

		CBlobTree* operator +(CBlobTree* other)
		{		
			CRicciBlend * blend = new CRicciBlend(this, other);
			return blend;
		}

		void getName(char * chrName)
		{
			strcpy_s(chrName, MAX_NAME_LEN, "RICCI BLEND");		
		}


		COctree computeOctree()
		{
			m_octree = m_children[0]->getOctree();
			for(size_t i = 1; i < m_children.size(); i++)
				m_octree.csgUnion(m_children[i]->getOctree());			
			return m_octree;
		}


		bool isOperator() { return true;}
		BlobNodeType getNodeType() {return bntOpRicciBlend;}
	};
}
}

#endif