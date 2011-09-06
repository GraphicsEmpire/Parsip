#ifndef MESHQUALITY_INCLUDED
#define MESHQUALITY_INCLUDED

#include "PS_FrameWork/include/PS_MeshVV.h"

namespace PS{

class CMeshQuality
{
public:
	typedef enum TriangleQualityEnum 
	{
		RADII_RATIO,
		EDGE_RATIO,
		ASPECT_RATIO,
		CIRCUMRADIUS_TO_MAX_EDGE_RATIO,
		CIRCUMRADIUS_TO_SEMIPERIMETER_RATIO,
		NORMALIZED_AREA,
		MINIMUM_INTERNAL_ANGLE,
		MAXIMUM_INTERNAL_ANGLE,
	};

	CMeshQuality();
	~CMeshQuality();
	
	bool load(int vertexCount, 
			  int vertexSize, 
			  int vertexOffset, 
			  const float *vertexArray, 
			  int elementCount, 
			  const unsigned int *elementArray, 
			  PS::CMeshVV::POLYGON_MODE polygonMode);

	bool setMeasure(TriangleQualityEnum qualityMeasure);
	bool measureQuality();
	float getVertexQuality(int vertexName) const;
	float getMinQuality() const;
	float getMaxQuality() const;
	float getAverageQuality() const;
	int getDegenerateTriangleCount() const;
	static float getTriangleQuality(const float *A, const float *B, const float *C);
	static float getMinimumTriangleAngle(const float *A, const float *B, const float *C);
	static float getMaximumTriangleAngle(const float *A, const float *B, const float *C);
protected:
	void create();
	bool measureQualityOfTriangleMesh();
	bool measureQualityOfQuadMesh();
private:
	CMeshVV* m_lpMesh;
	TriangleQualityEnum m_qualityMeasure;
	std::vector<float> m_qualityArray;
	float m_minQuality;
	float m_maxQuality;
	float m_averageQuality;
	int m_degenerateTriangleCount;
};

}

#endif // MESHQUALITY_INCLUDED

