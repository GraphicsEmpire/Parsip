#include "CParallelAdaptiveSubdivision.h"

//////////////////////////////////////////////////////////////////////////
void SubdivideAnalyzeBody::operator()( const tbb::blocked_range<int>& range )
{		
	vec3i tri;		
	vec3f n[3];
	float alpha[3];
	int decisionbits;

	size_t ctCoarseFaces = 0;		

	int ctOutVertices;
	

	//OpenMP parallel
	//#pragma omp parallel for
	for(int iFace=range.begin(); iFace < range.end(); iFace++)
	{
		//loadControlPoints
		tri.x = m_lpInMesh->m_lstFaces[iFace*3+0];
		tri.y = m_lpInMesh->m_lstFaces[iFace*3+1];
		tri.z = m_lpInMesh->m_lstFaces[iFace*3+2];

		n[0] = m_lpInMesh->getNormal3(tri.x);
		n[1] = m_lpInMesh->getNormal3(tri.y);
		n[2] = m_lpInMesh->getNormal3(tri.z);

		//Compute Normal Angles
		alpha[0] = VAngleDegree(n[0], n[1]);
		alpha[1] = VAngleDegree(n[1], n[2]);
		alpha[2] = VAngleDegree(n[2], n[0]);

		if(n[0] == n[1])	alpha[0] = 0.0f;
		if(n[1] == n[2])	alpha[1] = 0.0f;
		if(n[2] == n[0])	alpha[2] = 0.0f;

		ctOutVertices = 3;
		decisionbits = 0;			
		for(int j=0;j<3;j++)
		{
			if(alpha[j] > m_adaptiveParam)
			{
				ctOutVertices++;
				decisionbits = decisionbits | (1 << j);
			}				
		}

		if(decisionbits > 0)
			ctCoarseFaces++;

		m_lpOutVertexCount[iFace]  = ctOutVertices;
		m_lpOutDecisionBits[iFace] = decisionbits;
	}

	//return number of faces to be subdivided
	m_ctOutCoarseFaces = ctCoarseFaces;

}

//////////////////////////////////////////////////////////////////////////
void SubDividePerformBody::operator()( const tbb::blocked_range<int>& range ) const
{
	//For Loading Control Points
	vec3i tri;
	vec3f v[6];
	vec3f n[6];
	vec4f c[3];

	int bits;
	size_t offsetV, offsetT;

	//Process face by face
	//ToDo: Make it Parallel_For
	//#pragma omp parallel for
	for(int iFace=range.begin(); iFace < range.end(); iFace++)
	{
		//loadControlPoints
		tri.x = m_lpInMesh->m_lstFaces[iFace*3+0];
		tri.y = m_lpInMesh->m_lstFaces[iFace*3+1];
		tri.z = m_lpInMesh->m_lstFaces[iFace*3+2];

		v[0] = m_lpInMesh->getVertex3(tri.x);
		v[1] = m_lpInMesh->getVertex3(tri.y);
		v[2] = m_lpInMesh->getVertex3(tri.z);

		n[0] = m_lpInMesh->getNormal3(tri.x);
		n[1] = m_lpInMesh->getNormal3(tri.y);
		n[2] = m_lpInMesh->getNormal3(tri.z);

		c[0] = vec4f(m_lpInMesh->getColor(tri.x));
		c[1] = vec4f(m_lpInMesh->getColor(tri.y));
		c[2] = vec4f(m_lpInMesh->getColor(tri.z));

		//Vertices:
		//First output original control points
		offsetV = m_lpInVertexCountScanned[iFace];
		for(int i=0; i<3; i++)
		{
			m_lpOutMesh->m_lstVertices[(offsetV + i)*3+0] = v[i].x;
			m_lpOutMesh->m_lstVertices[(offsetV + i)*3+1] = v[i].y;
			m_lpOutMesh->m_lstVertices[(offsetV + i)*3+2] = v[i].z;

			m_lpOutMesh->m_lstNormals[(offsetV + i)*3+0] = n[i].x;
			m_lpOutMesh->m_lstNormals[(offsetV + i)*3+1] = n[i].y;
			m_lpOutMesh->m_lstNormals[(offsetV + i)*3+2] = n[i].z;

			m_lpOutMesh->m_lstColors[(offsetV + i)*4+0] = c[i].x;
			m_lpOutMesh->m_lstColors[(offsetV + i)*4+1] = c[i].y;
			m_lpOutMesh->m_lstColors[(offsetV + i)*4+2] = c[i].z;
			m_lpOutMesh->m_lstColors[(offsetV + i)*4+3] = c[i].w;
		}

		//Add MidPoints to list of Points
		offsetV += 3;
		bits = m_lpInDecisionBits[iFace];

		vec3f mid, midNormal;
		vec4f midColor;
		int incr = 0;
		if(bits != 0)
		{
			for(int i=0; i<3; i++)								
			{
				if((bits & (1 << i)) != 0)
				{
					mid = (v[i] + v[(i+1)%3]) * 0.5f;
					SubDivide_MoveMidPointToSurface(m_lpInRoot, mid, midNormal, midColor);

					m_lpOutMesh->m_lstVertices[offsetV*3 + 0] = mid.x;
					m_lpOutMesh->m_lstVertices[offsetV*3 + 1] = mid.y;
					m_lpOutMesh->m_lstVertices[offsetV*3 + 2] = mid.z;

					m_lpOutMesh->m_lstNormals[offsetV*3 + 0] = midNormal.x;
					m_lpOutMesh->m_lstNormals[offsetV*3 + 1] = midNormal.y;
					m_lpOutMesh->m_lstNormals[offsetV*3 + 2] = midNormal.z;								

					m_lpOutMesh->m_lstColors[offsetV*4 + 0] = midColor.x;
					m_lpOutMesh->m_lstColors[offsetV*4 + 1] = midColor.y;
					m_lpOutMesh->m_lstColors[offsetV*4 + 2] = midColor.z;								
					m_lpOutMesh->m_lstColors[offsetV*4 + 3] = midColor.w;

					v[3 + incr] = mid;
					n[3 + incr] = midNormal;

					incr++;
					offsetV++;
				}
			}
		}


		//Faces:			
		int nTris = m_lpInVertexCount[iFace] - 2;
		offsetV = m_lpInVertexCountScanned[iFace];
		offsetT = m_lpInVertexCountScanned[iFace] - (iFace * 2);
		if(nTris == 1)
		{
			for(int i=0; i<3; i++)								
				m_lpOutMesh->m_lstFaces[offsetT*3 + i] = offsetV + i;
		}
		else if(nTris == 2)
		{
			if(bits == 1)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
				offsetT++;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
			}
			else if(bits == 2)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 0;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 2;
				offsetT++;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
			}
			else if(bits == 4)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 0;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 1;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
				offsetT++;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
			}
		}
		else if(nTris == 3)
		{
			if(bits == 3)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
				offsetT++;
				if(v[4].dist2(v[0]) < v[3].dist2(v[2]))
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 0;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 2;
				}
				else
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 2;
				}
			}
			else if(bits == 5)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 0;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 4;
				offsetT++;
				if(v[3].dist2(v[2]) < v[4].dist2(v[1]))
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 4;
				}
				else
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 3;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 2;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 4;
				}

			}
			else if(bits == 6)
			{
				m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 4;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
				m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 2;
				offsetT++;
				if(v[0].dist2(v[3]) < v[1].dist2(v[4]))
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
				}
				else
				{
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 0;
					offsetT++;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 1;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
					m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 4;
				}
			}
		}
		else if(nTris == 4)
		{
			m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 0;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 3;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 5;
			offsetT++;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 1;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 4;
			offsetT++;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 3;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 5;
			offsetT++;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 0] = offsetV + 5;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 1] = offsetV + 4;
			m_lpOutMesh->m_lstFaces[offsetT*3 + 2] = offsetV + 2;
		}
	}
}

int SubDivide_MoveMidPointToSurface( CBlobTree* root,
									 vec3f& m, 
									 vec3f& outputNormal, 
									 vec4f& outputColor, 
									 float target_field /*= ISO_VALUE*/, 
									 int nIterations /*= DEFAULT_ITERATIONS */ )
{
	vec3f grad;
	float d;

	//Compute Initial FieldValue
	float f = root->fieldValue(m);	

	int i;
	for(i=0; i<nIterations; i++)
	{
		//Use faster method to compute fieldvalue and gradient at once
		//m_root->fieldValueAndGradient(m, FIELD_VALUE_EPSILON, grad, f);

		//New Method
		grad = root->gradient(m, FIELD_VALUE_EPSILON, f);

		//Compute Distance
		d = (target_field - f);

		//Move Point to new position. Uses shrink-wrap method to converge to surface
		m = m + ((d*grad)/grad.dot(grad));

		//New Field
		f = root->fieldValue(m);			
		d = fabsf(target_field - f); 
		if(d < FIELD_VALUE_EPSILON)				
			break;
	}

	outputNormal = root->normal(m, f, NORMAL_DELTA);
	outputColor = root->baseColor(m);

	return (i+2)*4;				
}

//////////////////////////////////////////////////////////////////////////
size_t SubDivide_ParallelPerform( CMeshVV& inMesh, CBlobTree* lpInBlob, float adaptiveParam )
{
	int ctFaces;	
	DVec<int> arrVertexCount;	
	DVec<int> arrVertexCountScanned;
	DVec<int> arrDecisionBits;		

	size_t result = 0;
	size_t ctFieldEvaluations = 0;


	//Attempts
	for(int i=0; i<3; i++)
	{	
		ctFaces = inMesh.countFaces();
		arrVertexCount.resize(ctFaces);
		arrDecisionBits.resize(ctFaces);
		
		/*
		SubdivideAnalyzeBody AnalyzeBody(adaptiveParam, 
										 &inMesh,
										 &arrVertexCount[0],
										 &arrDecisionBits[0]);
		tbb::parallel_reduce(tbb::blocked_range<int>(0, ctFaces), AnalyzeBody, tbb::auto_partitioner());
		result = AnalyzeBody.getCoarseFacesCount();
		*/
		


		result = SubDivide_Analyze(inMesh, adaptiveParam, arrVertexCount, arrDecisionBits);

		if(result > 0) 
		{		
			size_t ctTotalTriangles = 0;
			size_t ctTotalVertices = 0;

			arrVertexCountScanned.resize(ctFaces + 1);
			arrVertexCountScanned[0] = 0;
			for(int iFace=0; iFace < ctFaces; iFace++)
			{
				ctTotalVertices += arrVertexCount[iFace];
				arrVertexCountScanned[iFace+1] = ctTotalVertices;
			}
			ctTotalTriangles = ctTotalVertices - ctFaces*2;
			
			//Estimated field evaluations is 8 per each new mid point computation
			ctFieldEvaluations += (ctTotalVertices - ctFaces*3)*8;


			CMeshVV outMesh;
			outMesh.initMesh(CMeshVV::TRIANGLES, 3, 4, 0, 0);
			//Reserve memory
			outMesh.m_lstVertices.resize(ctTotalVertices*3);
			outMesh.m_lstNormals.resize(ctTotalVertices*3);
			outMesh.m_lstColors.resize(ctTotalVertices*4);
			outMesh.m_lstFaces.resize(ctTotalTriangles*3);

			SubDividePerformBody PerformBody(lpInBlob, 
											&arrVertexCount[0], 
											&arrVertexCountScanned[0],
											&arrDecisionBits[0],
											&inMesh, 
											&outMesh);
			tbb::parallel_for(tbb::blocked_range<int>(0, ctFaces), PerformBody, tbb::auto_partitioner());

			inMesh.m_lstFaces.copyFrom(outMesh.m_lstFaces);
			inMesh.m_lstVertices.copyFrom(outMesh.m_lstVertices);
			inMesh.m_lstNormals.copyFrom(outMesh.m_lstNormals);
			inMesh.m_lstColors.copyFrom(outMesh.m_lstColors);
			outMesh.removeAll();
		}
		else
			break;
	}

	
	arrVertexCount.clear();
	arrVertexCountScanned.clear();
	arrDecisionBits.clear();

	return ctFieldEvaluations;
}

//////////////////////////////////////////////////////////////////////////
size_t SubDivide_Analyze(CMeshVV& inMesh, float adaptiveParam, DVec<int>& arrVertexCount, DVec<int>& arrDecisionBits)
{
	size_t ctFaceElements = inMesh.m_lstFaces.size();
	int ctFaces = ctFaceElements / 3;

	vec3i tri;		
	vec3f n[3];
	float alpha[3];
	int decisionbits;

	size_t ctCoarseFaces = 0;		
	arrVertexCount.resize(ctFaces);
	arrDecisionBits.resize(ctFaces);

	int ctOutVertices;
	int iFace;

	//OpenMP parallel
	//#pragma omp parallel for
	for(iFace=0; iFace<ctFaces; iFace++)
	{
		//loadControlPoints
		tri.x = inMesh.m_lstFaces[iFace*3+0];
		tri.y = inMesh.m_lstFaces[iFace*3+1];
		tri.z = inMesh.m_lstFaces[iFace*3+2];

		n[0] = inMesh.getNormal3(tri.x);
		n[1] = inMesh.getNormal3(tri.y);
		n[2] = inMesh.getNormal3(tri.z);

		//Compute Normal Angles
		alpha[0] = VAngleDegree(n[0], n[1]);
		alpha[1] = VAngleDegree(n[1], n[2]);
		alpha[2] = VAngleDegree(n[2], n[0]);

		if(n[0] == n[1])	alpha[0] = 0.0f;
		if(n[1] == n[2])	alpha[1] = 0.0f;
		if(n[2] == n[0])	alpha[2] = 0.0f;

		ctOutVertices = 3;
		decisionbits = 0;			
		for(int j=0;j<3;j++)
		{
			if(alpha[j] > adaptiveParam)
			{
				ctOutVertices++;
				decisionbits = decisionbits | (1 << j);
			}				
		}

		if(decisionbits > 0)
			ctCoarseFaces++;

		arrVertexCount[iFace]  = ctOutVertices;
		arrDecisionBits[iFace] = decisionbits;
	}

	//return number of faces to be subdivided
	return ctCoarseFaces;
}
