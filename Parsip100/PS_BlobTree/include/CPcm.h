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
	CBlobTree* m_lpLeftChild;
	CBlobTree* m_lpRightChild;


public:
	CPcm() {;}
	CPcm(CBlobTree* lpLeftChild, CBlobTree* lpRightChild)
	{
		m_lpLeftChild = lpLeftChild;
		m_lpRightChild = lpRightChild;
	}
		
	//FieldValue computation
	float fieldValue(vec3f p)
	{		
		//1. 
		return 0.0f;
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
