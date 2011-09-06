#include <algorithm>

#include <cmath>
#include <float.h>

#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "CMeshQuality.h"

using namespace std;

namespace PS{


//========================================================================
CMeshQuality::CMeshQuality(void)
{
	create();
}

//========================================================================
CMeshQuality::~CMeshQuality(void)
{
}

//========================================================================
bool CMeshQuality::load(int vertexCount, 
						int vertexSize, 
						int vertexOffset, 
						const float *vertexArray, 
						int elementCount, 
						const unsigned int *elementArray, 
						CMeshVV::POLYGON_MODE polygonMode)
{
	int i;

	if (!vertexCount || !vertexArray || !elementCount || !elementArray)
	{
		MarkError();

		return false;
	}

	m_lpMesh = new CMeshVV();

	if (m_lpMesh == NULL)
	{
		MarkError();

		return false;
	}

	m_lpMesh->SetMode(polygonMode);

	m_lpMesh->m_vertexSize = vertexSize;

	if (vertexOffset == 0)
		vertexOffset = vertexSize;

	for (i=0; i<vertexCount; i++)
	{
		m_lpMesh->AddVertex(vertexArray, vertexArray + vertexSize);

		vertexArray += vertexOffset;
	}

	m_lpMesh->AddElementArray(elementArray, elementArray + elementCount);

	if (!m_lpMesh->IsValid())
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
bool CMeshQuality::SetMeasure(TriangleQualityEnum qualityMeasure)
{
	m_qualityMeasure = qualityMeasure;

	return true;
}

/**
*/
bool CMeshQuality::MeasureQuality(void)
{
	if (m_lpMesh == NULL)
	{
		MarkError();

		return false;
	}

	switch (m_lpMesh->m_mode)
	{
	case CMeshVV::TRIANGLES:
		MeasureQualityOfTriangleMesh();
		break;
	case CMeshVV::QUADS:
		MeasureQualityOfQuadMesh();
		break;
	default:
		MarkError();

		return false;
	};

	return true;
}

/**
*/
float CMeshQuality::GetVertexQuality(int vertexName) const
{
	if (!m_lpMesh)
	{
		MarkError();

		return 0.0f;
	}

	if (vertexName >= (int)m_qualityArray.size())
	{
		MarkError();

		return 0.0f;
	}

	return m_qualityArray[vertexName];
}

/**
*/
float CMeshQuality::GetMinQuality(void) const
{
	return m_minQuality;
}

/**
*/
float CMeshQuality::GetMaxQuality(void) const
{
	return m_maxQuality;
}

/**
*/
float CMeshQuality::GetAverageQuality(void) const
{
	return m_averageQuality;
}

/**
*/
int CMeshQuality::GetDegenerateTriangleCount(void) const
{
	return m_degenerateTriangleCount;
}

/**
*/
float CMeshQuality::GetTriangleQuality(const float *A, const float *B, const float *C)
{
	float a,
		b,
		c,
		lmin,
		lmax,
		semiperimeter,
		area,
		inradius,
		circumradius,
		quality;
	int i;
		
	a = 0.0f;
	b = 0.0f;
	c = 0.0f;
		
	for (i=0; i<3; i++)
	{
		a += (C[i] - B[i])*(C[i] - B[i]);
		b += (A[i] - C[i])*(A[i] - C[i]);
		c += (B[i] - A[i])*(B[i] - A[i]);
	}
		
    a = sqrtf(a);
    b = sqrtf(b);
	c = sqrtf(c);

#ifndef _MSC_VER
    lmin = std::min(a, std::min(b, c));
	lmax = std::max(a, std::max(b, c));
#else // !_MSC_VER
    lmin = min(a, min(b, c));
	lmax = max(a, max(b, c));
#endif // _MSC_VER

	semiperimeter = (a + b + c)*0.5f;

    area = sqrtf(semiperimeter*(semiperimeter - a)*(semiperimeter - b)*(semiperimeter - c));

	if (area < MyMath::GetMinimumPositiveValue<float>())
		return 0.0f;

    inradius = 2.0f*area/(a + b + c);

	circumradius = (a*b*c)/(4.0f*inradius*semiperimeter);

	switch (m_qualityMeasure) {
		case RADII_RATIO:
			quality = inradius/(0.5f*circumradius);
			break;
		case EDGE_RATIO:
			quality = lmin/lmax;
			break;
		case ASPECT_RATIO:
			quality = inradius/(0.2886751f*lmax);
			break;
		case CIRCUMRADIUS_TO_MAX_EDGE_RATIO:
			quality = lmax/(2.0f*circumradius);
			break;
		case CIRCUMRADIUS_TO_SEMIPERIMETER_RATIO:
			quality = semiperimeter/(2.5980762f*circumradius);
			break;
		case NORMALIZED_AREA:
			quality = area;
			break;
		case MINIMUM_INTERNAL_ANGLE:
			quality = GetMinimumTriangleAngle(A, B, C);
			break;
		case MAXIMUM_INTERNAL_ANGLE:
			quality = GetMaximumTriangleAngle(A, B, C);
			break;
	}

	return quality;
}

/**
*/
float CMeshQuality::GetMinimumTriangleAngle(const float *A, const float *B, const float *C)
{
	float u[3],
		v[3],
		minimumAngle;
	int i;

	for (i=0; i<3; ++i)
	{
		u[i] = B[i] - A[i];
		v[i] = C[i] - A[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	minimumAngle = acosf(MyMath::DotProduct<float, 3>(u, v));

	for (i=0; i<3; ++i)
	{
		u[i] = A[i] - B[i];
		v[i] = C[i] - B[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	minimumAngle = min(minimumAngle, acosf(MyMath::DotProduct<float, 3>(u, v)));

	for (i=0; i<3; ++i)
	{
		u[i] = A[i] - C[i];
		v[i] = B[i] - C[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	minimumAngle = min(minimumAngle, acosf(MyMath::DotProduct<float, 3>(u, v)));

	return MyMath::RadiansToDegrees<float>(minimumAngle);
}

/**
*/
float CMeshQuality::GetMaximumTriangleAngle(const float *A, const float *B, const float *C)
{
	float u[3],
		v[3],
		maximumAngle;
	int i;

	for (i=0; i<3; ++i)
	{
		u[i] = B[i] - A[i];
		v[i] = C[i] - A[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	maximumAngle = acosf(MyMath::DotProduct<float, 3>(u, v));

	for (i=0; i<3; ++i)
	{
		u[i] = A[i] - B[i];
		v[i] = C[i] - B[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	maximumAngle = max(maximumAngle, acosf(MyMath::DotProduct<float, 3>(u, v)));

	for (i=0; i<3; ++i)
	{
		u[i] = A[i] - C[i];
		v[i] = B[i] - C[i];
	}

	MyMath::Normalize<float, 3>(u);
	MyMath::Normalize<float, 3>(v);

	maximumAngle = max(maximumAngle, acosf(MyMath::DotProduct<float, 3>(u, v)));

	return MyMath::RadiansToDegrees<float>(maximumAngle);
}

/**
*/
void CMeshQuality::Create(void)
{
	m_lpMesh = NULL;

	// Adequate default value?
	m_qualityMeasure = RADII_RATIO;

	m_qualityArray.clear();

	m_minQuality = 0.0f;
	m_maxQuality = 1.0f;

	m_averageQuality = 1.0f;

	m_degenerateTriangleCount = 0;
}

/**
*/
bool CMeshQuality::MeasureQualityOfTriangleMesh(void)
{
	int vertexCount,
		triangleCount,
		degenerateTriangleCount,
		triangleElementArray[3],
		i,
		j;
    float *vertexArray,
		*A,
        *B,
        *C,
		quality,
		minQuality,
		maxQuality;
	double qualitySum;
	unsigned int *element;
	vector<float>::iterator normalizedQuality;

	if (m_lpMesh == NULL)
	{
		MarkError();

		return false;
	}

	vertexCount = (int)m_lpMesh->m_vertexArray.size()/m_lpMesh->m_vertexSize;

	triangleCount = (int)m_lpMesh->m_elementArray.size()/3;
		
	m_qualityArray.assign(vertexCount, 1.0f);

	minQuality = MyMath::GetMaximumPositiveValue<float>();
	maxQuality = -MyMath::GetMaximumPositiveValue<float>();

	qualitySum = 0.0;

	degenerateTriangleCount = 0;

	element = &m_lpMesh->m_elementArray[0];

	vertexArray = &m_lpMesh->m_vertexArray[0];

	for (i=triangleCount; i--; )
    {
		// Triangle quality measures are orientation insensitive.
		for (j=3; j--; )
		{
			triangleElementArray[j] = *element++;

			if (triangleElementArray[j] >= vertexCount)
			{
				MarkError();

				return false;
			}
		}
			
		A = vertexArray + 3*triangleElementArray[0];
		B = vertexArray + 3*triangleElementArray[1];
		C = vertexArray + 3*triangleElementArray[2];

		quality = GetTriangleQuality(A, B, C);

		// This function returns -1 in case or error.
		if (quality > 0.0f)
		{
			minQuality = min(minQuality, quality);
			maxQuality = max(maxQuality, quality);

			qualitySum += (double)quality;

			// Associates to the vertex the quality of the worst triangle 
			// incident to it.
			for (j=3; j--; )
				m_qualityArray[triangleElementArray[j]] = min(m_qualityArray[triangleElementArray[j]], quality);
		}
		else
			degenerateTriangleCount++;
	}

	m_minQuality = minQuality;
	m_maxQuality = maxQuality;

	m_averageQuality = (float)(qualitySum/(triangleCount - degenerateTriangleCount));

	if (m_qualityMeasure == NORMALIZED_AREA)
	{
		for (normalizedQuality=m_qualityArray.begin(); normalizedQuality!=m_qualityArray.end(); normalizedQuality++)
			*normalizedQuality /= maxQuality;

		m_minQuality /= maxQuality;
		m_maxQuality /= maxQuality;
		
		m_averageQuality /= maxQuality;
	}

	m_degenerateTriangleCount = degenerateTriangleCount;

	return true;
}

/**
*/
bool CMeshQuality::MeasureQualityOfQuadMesh(void)
{
	int vertexCount,
		quadCount,
		degenerateTriangleCount,
		quadElementArray[4],
		i,
		j,
		k;
    float *vertexArray,
		*A,
        *B,
        *C,
		*D,
		// Treats each quad as two triangles.
		quality[2],
		minQuality,
		maxQuality;
	double qualitySum;
	unsigned int *element;
	vector<float>::iterator normalizedQuality;

	if (m_lpMesh == NULL)
	{
		MarkError();

		return false;
	}

	vertexCount = (int)m_lpMesh->m_vertexArray.size()/m_lpMesh->m_vertexSize;

	quadCount = (int)m_lpMesh->m_elementArray.size()/4;
		
	m_qualityArray.assign(vertexCount, 1.0f);

	minQuality = MyMath::GetMaximumPositiveValue<float>();
	maxQuality = -MyMath::GetMaximumPositiveValue<float>();

	qualitySum = 0.0;

	degenerateTriangleCount = 0;

	element = &m_lpMesh->m_elementArray[0];

	vertexArray = &m_lpMesh->m_vertexArray[0];

	for (i=quadCount; i--; )
    {
		// Triangle quality measures are orientation insensitive.
		for (j=4; j--; )
		{
			quadElementArray[j] = *element++;

			if (quadElementArray[j] >= vertexCount)
			{
				MarkError();

				return false;
			}
		}
			
		A = vertexArray + 3*quadElementArray[0];
		B = vertexArray + 3*quadElementArray[1];
		C = vertexArray + 3*quadElementArray[2];
		D = vertexArray + 3*quadElementArray[3];

		// CCW orientation.
		quality[0] = GetTriangleQuality(A, B, C);
		quality[1] = GetTriangleQuality(A, C, D);

		for (j=2; j--; )
		{
			// This function returns -1 in case or error.
			if (quality[j] >= 0.0f)
			{
				minQuality = min(minQuality, quality[j]);
				maxQuality = max(maxQuality, quality[j]);

				qualitySum += (double)quality[j];

				// Associates to the vertex the quality of the worst triangle 
				// incident to it.
				for (k=3; k--; )
					m_qualityArray[quadElementArray[k]] = min(m_qualityArray[quadElementArray[k]], quality[j]);
			}
			else
				degenerateTriangleCount++;
		}
	}

	m_minQuality = minQuality;
	m_maxQuality = maxQuality;

	m_averageQuality = (float)(qualitySum/(2*quadCount - degenerateTriangleCount));

	if (m_qualityMeasure == NORMALIZED_AREA)
	{
		for (normalizedQuality=m_qualityArray.begin(); normalizedQuality!=m_qualityArray.end(); normalizedQuality++)
			*normalizedQuality /= maxQuality;

		m_minQuality /= maxQuality;
		m_maxQuality /= maxQuality;
		
		m_averageQuality /= maxQuality;
	}

	m_degenerateTriangleCount = degenerateTriangleCount;

	return true;
}

}
