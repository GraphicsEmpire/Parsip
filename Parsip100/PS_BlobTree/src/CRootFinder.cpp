//#include "_GlobalSettings.h"
//#include "mathHelper.h"
#include "PS_FrameWork/include/PS_Interval.h"
#include "CRootFinder.h"

namespace PS{
	namespace BLOBTREE{


		int CRootFinder::solveQuadratic(float a, float b, float c, float eq, float res[2])
		{
			c = c - eq;
			float del = b*b - 4*a*c;
			if(del < 0.0f) 
				return 0;
			else if(del == 0.0f)
			{
				res[0] = -b / (2*a);
				res[1] = -b / (2*a);
				return 1;
			}
			else
			{
				float delroot2 = sqrtf(del);
				res[0] = (-b + delroot2) / (2*a);
				res[1] = (-b - delroot2) / (2*a);
				return 2;
			}
		}

		size_t CRootFinder::findRoot(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput)
		{
			if(m_method == rfmParabola)
				return rootFitParabola(p1, p2, fp1, fp2, output, fieldoutput);
			else if(m_method == rfmBisection)
				return rootBisection(p1, p2, fp1, fp2, output, fieldoutput);
			else if(m_method == rfmNewtonRaphson)
				return rootNewtonRaphson(p1, p2, fp1, fp2, output, fieldoutput);
			else
				return rootLinearInterpolation(p1, p2, fp1, fp2, output, fieldoutput);
		}

		//Newton Raphson uses gradient and is very fast on converging to the root
		size_t CRootFinder::rootNewtonRaphson(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput)
		{		
			vec3 grad, x;
			float f,d;

			if(fabsf(fp1 - m_isoValue) < fabsf(fp2 - m_isoValue))
				x = p1;
			else
				x = p2;

			for(int i=0; i<m_nIterations; i++)
			{
				//Get gradient for direction of displacement
				//grad = m_root->gradient(x, FIELD_VALUE_EPSILON);			
				//f = m_root->fieldValue(x);
				//use faster method fieldvalue and gradient
				m_root->fieldValueAndGradient(x, FIELD_VALUE_EPSILON, grad, f);
		
				d = (m_isoValue - f);

				//Used shrinkwrap method to converge to surface
				x = x + ((d*grad)/grad.dot(grad));

				fieldoutput = m_root->fieldValue(x);
				output = x;
				d = fabsf(fieldoutput - m_isoValue); 
				if(d < FIELD_VALUE_EPSILON)				
					return i;				
			}

			return m_nIterations;
		}


		size_t CRootFinder::rootFitParabola(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput) 
		{	
			size_t ctFieldEval = 0;


			//Grab another sample point
			float t = (m_isoValue - fp1) / (fp2 - fp1);				
			vec3 p3 = p1 + t *(p2 - p1);
			float a,b,c;

			CInterval interval(0.0f, 1.0f);

			int i = 0;
			do
			{
				//We need one additional fieldValue sample at p3			
				ctFieldEval++;
				float fp3 = m_root->fieldValue(p3);
				float delta = fabsf(fp3 - m_isoValue);
				if(delta < FIELD_VALUE_EPSILON)
				{
					output = p3;
					fieldOutput = fp3;
					return i;
				}

				//Find new point
				c = fp1;
				a = ((fp3 - fp1) - (fp2 - fp1)*t) / (t*t - t);
				b = (fp2 - fp1 - a);
				float res[2];
				int roots = solveQuadratic(a,b,c, m_isoValue, res);
				if(roots > 0)
				{
					if(interval.isInside(res[0]))
						t = res[0];				
					else 
						t = res[1];
					p3 = p1 + t*(p2 - p1);
				}
				else
				{
					output = p3;
					fieldOutput = fp3;
					return i;
				}

			}while(++i < m_nIterations);	

			ctFieldEval++;
			fieldOutput = m_root->fieldValue(output);
			return i;		
		}

		size_t CRootFinder::rootLinearInterpolation(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput) 
		{
			float t = (m_isoValue - fp1) / (fp2 - fp1);
			output = p1 + t*(p2 - p1);
			fieldOutput = m_root->fieldValue(output);

			return 1;
		}

		//Root Secant Method
		size_t CRootFinder::rootBisection(vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldOutput) 
		{		
			vec3 pos, neg;
			size_t ctFieldEval = 0;

			//fp1 < 0 means p1 returned negative field value
			if (fp1 < m_isoValue) {
				pos = p2;
				neg = p1;
			}
			else 
			{
				pos = p1;
				neg = p2;
			}

			float delta = 0.0f;
			//While not satisfied the resolution
			for(int i = 0; i <m_nIterations; i++)
			{
				//update output using bisection method
				output = 0.5f * (pos + neg);

				//Increment number of field evaluations
				ctFieldEval++;

				//update field output
				fieldOutput = m_root->fieldValue(output);
				delta = fabsf(fieldOutput - m_isoValue);
				if(delta < FIELD_VALUE_EPSILON)
					break;

				if (fieldOutput > m_isoValue)
					pos = output;				
				else 
					neg = output;	  
			}

			return ctFieldEval;
		}

		//Returns number of field evaluations
		int ComputeRootNewtonRaphson(CBlobTree* root, vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput,
		 							 float target_field, int iterations)
		{
			vec3 grad, x;
			float f,d;

			if(iterations <= 0) return -1;
			if(root == NULL) return -1;

			if(fabsf(fp1 - target_field) < fabsf(fp2 - target_field))
				x = p1;
			else
				x = p2;

			for(int i=0; i<iterations; i++)
			{
				//Get gradient for direction of displacement
				//grad = m_root->gradient(x, FIELD_VALUE_EPSILON);			
				//f = m_root->fieldValue(x);
				//Use faster method to compute fieldvalue and gradient at once
				root->fieldValueAndGradient(x, FIELD_VALUE_EPSILON, grad, f);

				d = (target_field - f);

				//Uses shrink-wrap method to converge to surface
				x = x + ((d*grad)/grad.dot(grad));

				fieldoutput = root->fieldValue(x);
				output = x;
				d = fabsf(fieldoutput - target_field); 
				if(d < FIELD_VALUE_EPSILON)				
					return (i+1)*4;				
			}

			return iterations*4;
		}

		int ComputeRootBiSection( CBlobTree* root, vec3 p1, vec3 p2, float fp1, float fp2, vec3 &output, float &fieldoutput, float target_field /*= ISO_VALUE*/, int iterations /*= DEFAULT_ITERATIONS*/ )
		{
			vec3 pos, neg;
			size_t ctFieldEval = 0;

			//fp1 < 0 means p1 returned negative field value
			if (fp1 < target_field) {
				pos = p2;
				neg = p1;
			}
			else 
			{
				pos = p1;
				neg = p2;
			}

			float delta = 0.0f;
			//While not satisfied the resolution
			for(int i = 0; i < iterations; i++)
			{
				//update output using bisection method
				output = 0.5f * (pos + neg);

				//Increment number of field evaluations
				ctFieldEval++;

				//update field output
				fieldoutput = root->fieldValue(output);
				delta = fabsf(fieldoutput - target_field);
				if(delta < FIELD_VALUE_EPSILON)
					break;

				if (fieldoutput > target_field)
					pos = output;				
				else 
					neg = output;	  
			}

			return ctFieldEval;
		}

	}
}