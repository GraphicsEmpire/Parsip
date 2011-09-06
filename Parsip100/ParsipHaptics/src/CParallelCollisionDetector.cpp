#include "CParallelCollisionDetector.h"
#include "CPolyParsipOptimized.h"
#include "CompactBlobTree.h"

namespace PS{
	namespace BLOBTREEANIMATION{

int CParCollisionDetector::detect()
{
	if(m_vModels.size() < 2) return 0;

	size_t ctModels = m_vModels.size();

	COctree oct1;
	COctree oct2;

	CParsipOptimized* poly1;
	CParsipOptimized* poly2;
	//COMPACTBLOBTREE cptBlob1;
	//COMPACTBLOBTREE cptBlob2;

	int res = 0;
	for(size_t i=0; i<ctModels; i++)
	{
		for(size_t j=i+1; j<ctModels; j++)
		{
			oct1 = m_vModels[i]->getOctree();
			oct2 = m_vModels[j]->getOctree();

			//There is an octree intersection so there is a chance of interpenetration
			if(oct1.intersect(oct2.lower, oct2.upper))
			{
				res++;

				//Convert to compact BlobTrees
				/*
				cptBlob1.reset();
				cptBlob1.convert(m_vModels[i]);
				cptBlob2.reset();
				cptBlob2.convert(m_vModels[j]);
				*/
				poly1 = Run_Polygonizer(m_vModels[i], 0.16f);
				poly2 = Run_Polygonizer(m_vModels[j], 0.16f);


			}
		}
	}


	//Delete both polygonizers
	SAFE_DELETE(poly1);
	SAFE_DELETE(poly2);
	return res;
}


	}
}
