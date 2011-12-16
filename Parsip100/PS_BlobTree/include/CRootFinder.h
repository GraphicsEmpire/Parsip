#ifndef CROOTFINDER_H
#define CROOTFINDER_H

#include "CBlobTree.h"


namespace PS{
namespace BLOBTREE{

enum RootFindingMethod {rfmParabola, rfmBisection, rfmLinearInterpolation, rfmNewtonRaphson};

class CRootFinder 
{
private:
	CBlobNode * m_root;
	RootFindingMethod m_method;	
	int				  m_nIterations;
	float			  m_isoValue;	

	//returns number of roots found
	static int solveQuadratic(float a, float b, float c, float eq, float res[2]);

public:
	CRootFinder(const CBlobNode * root, RootFindingMethod method = rfmParabola, int nConvergeIterations = DEFAULT_ITERATIONS, float target_field = ISO_VALUE)
	{
		m_root        = const_cast<CBlobNode*>(root);
		m_method	  = method;
		m_nIterations = nConvergeIterations;
		m_isoValue	  = target_field;
	}

	size_t findRoot(vec3f p1, vec3f p2, float fp1, float fp2, vec3f &output, float &fieldoutput);

	//Newton Raphson uses gradient and is very fast on converging to the root
	__inline size_t rootNewtonRaphson(vec3f p1, vec3f p2, float fp1, float fp2, vec3f &output, float &fieldoutput);

	//Fitting a parabola to get closer to the root
	__inline size_t rootFitParabola(vec3f p1, vec3f p2, float fp1, float fp2, vec3f &output, float &fieldOutput);

	//Lowest Quality for root finding
	__inline size_t rootLinearInterpolation(vec3f p1, vec3f p2, float fp1, float fp2, vec3f &output, float &fieldOutput);

	//Bisection Method
	__inline size_t rootBisection(vec3f p1, vec3f p2, float fp1, float fp2, vec3f &output, float &fieldOutput);
	
};

	//Function outside of the class scope
	int ComputeRootNewtonRaphson(CBlobNode* root, vec3f p1, vec3f p2, float fp1, float fp2, 
  								 vec3f &output, float &fieldoutput,
								 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);


	int ComputeRootBiSection(CBlobNode* root, vec3f p1, vec3f p2, float fp1, float fp2, 
						 	 vec3f &output, float &fieldoutput,
							 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);

}
}
#endif
