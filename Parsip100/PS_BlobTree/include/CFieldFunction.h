#ifndef CFIELDFUNCTION_H
#define CFIELDFUNCTION_H

#include <math.h>
#include <complex>

using namespace std;

namespace PS{
namespace BLOBTREE{

typedef enum FieldFunctionType {fftWyvill, fftSoftObjects};

//Field Function Parent Class
class  CFieldFunction {
public:
	virtual float fieldValue(float d) = 0;
	
	virtual float fieldValueSquare(float dd) 
	{
		return fieldValue(sqrt(dd));
	}

	virtual void setRange(float roi) = 0;
	virtual float getRange() = 0;

	virtual float inverse(float y) = 0;
	virtual FieldFunctionType getType() = 0;

};

//Wyvill Function implements: (1-d^2/R^2)^3 / R^4
// (R^2 - d^2)^3 / R^6
class  CFunctionWyvill : public CFieldFunction
{
private:
	float m_r;
	float m_rr;	

public:
	CFunctionWyvill()
	{
		setRange(1.0f);
	}

	
	CFunctionWyvill(float roi)
	{
		setRange(roi);
	}

	CFunctionWyvill(CFieldFunction* other)
	{
		setParamFrom(other);
	}	

	void setParamFrom(CFieldFunction* input)
	{
		CFunctionWyvill* wyvillN = dynamic_cast<CFunctionWyvill*>(input);
		this->m_r = wyvillN->m_r;
		this->m_rr = wyvillN->m_rr;
	}

	//Pass Range of Effect
	void setRange(float roi)
	{
		m_r = roi;
		m_rr = m_r * m_r;
	}

	float getRange() { return m_r;}

	//Returns the field value of the given distance from the skeleton	
	__inline float fieldValue(float d)
	{		
		return fieldValueSquare(d*d);
	}

	__inline float fieldValueSquare(float dd)
	{
		if(dd >= m_rr)
			return 0.0f;
		else
		{
			float t = (1.0f - dd / m_rr);
			return t*t*t;
		}
	}

	//Returns the distance to the skeleton
	float inverse(float fv)
	{
		float oneThird = 1.0f / 3.0f;
		return m_r * sqrt(1.0f - pow(fv, oneThird));
	}

	FieldFunctionType getType()
	{
		return fftWyvill;
	}
};

//Change to simple functions
__inline float ComputeWyvillFieldValueSquare(float dd)
{
	if(dd >= 1.0f)
		return 0.0f;
	else
	{
		//float t = (1.0f - dd / rr);
		float t = (1.0f - dd);
		return t*t*t;
	}
}


//Soft object function is: g(d) = (1 - (4d^6)/(9r^6) + (17d^4)/(9r^4) - (22d^2)/(9r^2))
class  CFunctionSoftObject : public CFieldFunction
{
private:
	float m_r;
	float m_rr;
	float m_r2;
	float m_r4;
	float m_r6;

public:
	CFunctionSoftObject()
	{
		setRange(1.0f);
	}
	
	CFunctionSoftObject(float roi)
	{
		setRange(roi);
	}

	CFunctionSoftObject(CFieldFunction* other)
	{
		setParamFrom(other);
	}

	void setParamFrom(CFieldFunction* input)
	{
		CFunctionSoftObject* softN = dynamic_cast<CFunctionSoftObject*>(input);
		this->m_r  = softN->m_r;
		this->m_rr = softN->m_rr;
		this->m_r2 = softN->m_r2;
		this->m_r4 = softN->m_r4;
		this->m_r6 = softN->m_r6;
	}

	void setRange(float roi)
	{
		m_r = roi;
		m_rr = m_r * m_r;
		m_r2 = 22.0f / (9.0f * m_rr);
		m_r4 = 17.0f / (9.0f * m_rr * m_rr);
		m_r6 = 4.0f / (9.0f * m_rr * m_rr * m_rr);
	}
	
	float getRange() { return m_r;}

	__inline float fieldValue(float d)
	{
		float dd = d*d;
		if(d >= m_r)
			return 0.0f;
		else
		{			
			float d4 = dd * dd;
			return(1.0f - m_r6 * dd * d4 + m_r4 * d4 - m_r2 * dd);
		}
	}

	__inline float fieldValueSquare(float dd)
	{
		if(dd >= m_rr)
			return 0.0f;
		else
		{
			float d4 = dd * dd;
			return(1.0f - m_r6 * dd * d4 + m_r4 * d4 - m_r2 * dd);
		}
	}
		
	float inverse(float y)
	{
		const float ca = -36.0f * sqrt(3.0f);
		const float cb = 1.0f / 12.0f;
		const float cc = 1.0f / 3.0f;
		
		float r4 = m_rr * m_rr;
		float r6 = r4 * m_rr;
		float d = y * (y * 972.0f - 125.0f);
		if (d >= 0)
		{
			float t = pow(static_cast<float>(ca * r6 * sqrt(d) + r6 * (y * 1944.0f - 125.0f)), static_cast<float>(cc));
			return sqrt((17.0f * m_rr - 25.0f * r4 / t - t) * cb);
		}
		else
		{		
			
			std::complex<float> t(r6 * (y * 1944.0f - 125.0f), ca * r6 * sqrt(-d));
			
			t = complexPow(t, cc);
			t = (-25.0f * r4) / t - t;
			return sqrt((17.0f * m_rr + t.real()) * cb);
		}		
	}

	complex<float> complexPow(complex<float> comp, float exponent)
	{
		float x = comp.real();
		float y = comp.imag();

		float modulous = pow(static_cast<float>(x*x + y*y), static_cast<float>(exponent * 0.5));
		float argument = atan2(y, x) *  exponent;

		comp.real(modulous * cos(argument));
		comp.imag(modulous * sin(argument));
		
		return comp;
	}

	FieldFunctionType getType()
	{
		return fftSoftObjects;
	}

};

}
}
#endif