#ifndef CROOTFINDER_H
#define CROOTFINDER_H

#include "CBlobTree.h"


namespace PS{
namespace BLOBTREE{

typedef enum RootFindingMethod {rfmParabola, rfmBisection, rfmLinearInterpolation, rfmNewtonRaphson};

class CRootFinder 
{
private:
	CBlobTree * m_root;
	RootFindingMethod m_method;	
	int				  m_nIterations;
	float			  m_isoValue;	

	//returns number of roots found
	static int solveQuadratic(float a, float b, float c, float eq, float res[2]);

public:
	CRootFinder(const CBlobTree * root, RootFindingMethod method = rfmParabola, int nConvergeIterations = DEFAULT_ITERATIONS, float target_field = ISO_VALUE)
	{
		m_root        = const_cast<CBlobTree*>(root);
		m_method	  = method;
		m_nIterations = nConvergeIterations;
		m_isoValue	  = target_field;
	}

	size_t findRoot(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput);

	//Newton Raphson uses gradient and is very fast on converging to the root
	__inline size_t rootNewtonRaphson(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput);

	//Fitting a parabola to get closer to the root
	__inline size_t rootFitParabola(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput);

	//Lowest Quality for root finding
	__inline size_t rootLinearInterpolation(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput);

	//Bisection Method
	__inline size_t rootBisection(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput);
	
};

	//Function outside of the class scope
	int ComputeRootNewtonRaphson(CBlobTree* root, vec3 p1, vec3 p2, float fp1, float fp2, 
  								 vec3 &output, float &fieldoutput,
								 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);


	int ComputeRootBiSection(CBlobTree* root, vec3 p1, vec3 p2, float fp1, float fp2, 
						 	 vec3 &output, float &fieldoutput,
							 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);

}
}
#endif