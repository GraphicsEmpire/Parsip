#ifndef CPCM_H
#define CPCM_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

//***********************************************************
//PCM: Precise Contact Modeling
class  CPcm : public CBlobTree
{
private:
	CBlobTree* m_otherObj;
	COctree* m_otherOct;

	float m_w;
	float m_alpha;


public:
	CPcm() {;}
	CPcm(CBlobTree * child)
	{
		addChild(child);
	}

	CPcm(CBlobTree * child, CBlobTree* otherBlob, COctree* otherOct, float w, float alpha)
	{
		addChild(child);
		m_otherObj = otherBlob;
		m_otherOct = otherOct;
		m_w		= w;
		m_alpha = alpha;
	}

		
	//FieldValue computation
	float fieldValue(vec3f p)
	{		
		float f1 = m_children[0]->fieldValue(p);
		float f2 = m_otherObj->fieldValue(p);

		float g1;
		//Check if we are in inter-penetration region
		if((f1 >= ISO_VALUE)&&(f2 >= ISO_VALUE))
		{
			g1 = -1.0f * f2;

			return f1 + g1;
		}
		else
			return f1;
	}

	float curvature(vec3f p)
	{
		return m_children[0]->curvature(p);
	}

	vec4f baseColor(vec3f p)
	{
		return m_children[0]->baseColor(p);
	}

	CMaterial baseMaterial(vec3f p)
	{
		return m_children[0]->baseMaterial(p);
	}


	void getName(char * chrName)
	{
            strncpy(chrName, "PCM", MAX_NAME_LEN);
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();		
		return m_octree;
	}


	bool isOperator() { return true;}
	
	BlobNodeType getNodeType() { return bntOpPCM;}
};

}
}


#endif
