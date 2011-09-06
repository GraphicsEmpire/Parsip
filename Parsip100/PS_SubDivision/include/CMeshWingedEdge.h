#pragma once
#ifndef WINGED_EDGEMESH
#define WINGED_EDGEMESH

#include "DSystem/include/DContainers.h"
#include "PS_FrameWork/include/PS_Vector.h"
#include "PS_FrameWork/include/_dataTypes.h"

#include "tbb/blocked_range.h"
#include "tbb/parallel_reduce.h"

#define MAX_FACE_SIDES 4

//Error codes
#define NOT_FOUND		 -1
#define NOT_SET_YET		 -2
#define NOT_AN_INDEX	 -3
#define BUFFER_TOO_SMALL -4

using namespace PS;
using namespace PS::MATH;
//Winged Edge Mesh Data-Structure
//Useful for Subdivision algorithms
class CWEMesh
{
public:
	struct FACE{		
		unsigned int edges[MAX_FACE_SIDES];			
	};

	struct VERTEX{
		vec3 pos;
		unsigned int edge;
	};

	class EDGE{
	public:
		unsigned int start, end;
		unsigned int lface, rface;
		unsigned int lprev, lnext;
		unsigned int rprev, rnext;

		EDGE() {;}
		EDGE(unsigned int startIdx, unsigned int endIdx)
		{
			this->start = startIdx;
			this->end = endIdx;
		};
		
		EDGE& operator = (const EDGE& A)          // ASSIGNMENT (=)
		{ 
			start = A.start; end = A.end; 
			lface = A.lface; rface = A.rface;
			lprev = A.lprev; lnext = A.lnext;
			rprev = A.rprev; rnext = A.rnext;
			return(*this);  
		};

		__inline bool operator==(const EDGE& A) const
		{
			return (((this->start == A.start)&&(this->end == A.end))||
					((this->start == A.end)&&(this->end == A.start)));					
		};
	};

private:
	int	m_ctFaceSides;
	DVec<FACE> m_lstFaces;
	DVec<VERTEX> m_lstVertices;
	DVec<EDGE> m_lstEdges;

	__inline bool isSameFace(FACE& f1, FACE& f2) const
	{
		for(int i=0; i < m_ctFaceSides; i++)
			if(f1.edges[i] != f2.edges[i])
				return false;
		return true;
	}

	__inline bool faceHasEdge(FACE& f1, size_t idxEdge) const
	{
		for(int i=0; i < MAX_FACE_SIDES; i++)
			if(f1.edges[i] == idxEdge)
				return true;
		return false;
	}

	__inline bool getPrevNextEdge(FACE& f, size_t idxEdge, size_t& idxPrev, size_t& idxNext) const
	{
		idxPrev = NOT_SET_YET;
		idxNext = NOT_SET_YET;
		int current = NOT_SET_YET;
		for(int i=0; i < m_ctFaceSides; i++)
		{
			if(f.edges[i] == idxEdge)
			{
				current = i;
				break;
			}
		}

		if(current == NOT_SET_YET)
			return false;

		if(current == 0)
			idxPrev = f.edges[m_ctFaceSides-1];
		else
			idxPrev = f.edges[current-1];

		if(current == m_ctFaceSides-1)
			idxNext = f.edges[0];
		else
			idxNext = f.edges[current+1];


		return true;
	}

public:
	OnOverallProgress m_pOnOverallProgress;
	OnStageProgress   m_pOnStageProgress;


	CWEMesh(int ctFaceSides = 4);
	
	~CWEMesh();

	void cleanup();

	//Copy
	void copyFrom(const CWEMesh& rhs);

	//
	bool performCompleteTest(int* pErrorCount = NULL);

	//Check passed indices
	bool isFaceIndex(size_t idx) const { return (idx >= 0 && idx < m_lstFaces.size());}
	bool isEdgeIndex(size_t idx) const { return (idx >= 0 && idx < m_lstEdges.size());}
	bool isVertexIndex(size_t idx) const { return (idx >= 0 && idx < m_lstVertices.size());}

	void removeFace(size_t idxFace)
	{
		if(isFaceIndex(idxFace))
		{
			m_lstFaces.remove(idxFace);
		}
	}

	//Setters
	void setFaceSides(int sides)  {m_ctFaceSides = sides;}

	//Getters
	size_t countFaces() const {return m_lstFaces.size();}
	size_t countEdges() const {return m_lstEdges.size();}
	size_t countVertices() const {return m_lstVertices.size();}

	int getFaceSides() const {return  m_ctFaceSides;}

	FACE faceAt(size_t idx) const;

	EDGE edgeAt(size_t idx) const;

	VERTEX vertexAt(size_t idx) const;

	//Diagnose Extraordinary vertices and fix them
	size_t findAllExtraOrdinaryVertices(int *pOutValences = NULL, size_t szOutValences = 0) const;
	size_t findAllExtraOrdinaryFaces(int *pOutFaceErrors= NULL, size_t szOutFaceErrors = 0) const;

	//Set
	size_t addEdge(EDGE& e);
	bool setEdge(unsigned int idxEdge, EDGE& value);
	bool setVertex(unsigned int idxVertex, vec3 pos);

	//Copy
	void copy(EDGE &dst, const EDGE &src);
	void copy(VERTEX &dst, const VERTEX &src);
	void copy(FACE &dst, const FACE &src);


	//Search
	bool hasVertex(vec3 v, unsigned int& foundIdx, unsigned int start = 0, unsigned int end = -1);
	int  hasVertex(vec3 v); //return -1 is not found OTW returns vertex index	
	bool hasEdge(vec3 sv, vec3 ev, unsigned int& foundEdge);
	bool hasEdge(unsigned int start, unsigned int end, unsigned int& foundEdge);//return false if not found OTW returns Edge index	

	//Add
	int addFace(int sides, const vec3* arrV);
	int addFace(const size_t* pFaceIndices);

	int addQuadFace(float* v1, float* v2, float* v3, float* v4);
	int addQuadFace(vec3 v1, vec3 v2, vec3 v3, vec3 v4);	

	int addTriangleFace(float* v1, float* v2, float* v3);
	int addTriangleFace(vec3 v1, vec3 v2, vec3 v3);


	size_t addVertex(vec3 pos, int edge = NOT_SET_YET);
	size_t addVertexUnique(vec3 pos, int edge);
	size_t addVertexArray(DVec<vec3>& lstVertices);
	size_t addVertexArray(size_t ctVertices, int elementSize, const float* pBuffer);

	//Getters
	size_t getAllFaceIndices(size_t* pVertexIndices, size_t szVertexIndices)  const;
	int getFaceVertices(int idxFace, size_t* outIndices, size_t outIndicesSize) const;
	int getFaceVertices(int idxFace, vec3* pOutVertices, size_t szOutVertices) const;
	int getFaceVertices(int idxFace, DVec<unsigned int> &lstIndices) const;
	int getFaceVertices(int idxFace, DVec<vec3> &outVertices) const;


	int getFaceEdges(int idxFace, size_t* pOutIndices, size_t outIndicesSize) const;
	int getFaceEdges(int idxFace, DVec<unsigned int> &lstIndices) const;

	int getEdgeEndPoints(int idxEdge, vec3& start, vec3& end) const;
	bool getEdgeMidPoint(int idxEdge, vec3& midpoint) const;

	//Conversion from SubD to Bezier Patch need:
	int getOneRingNeighborV(size_t idxFace, size_t * pIndices, size_t szBufferSize) const;
	//int getOneRingNeighborV(size_t idxFace, vec3 * pVertices, size_t szBufferSize) const;
	//int getOneRingNeighborF(size_t idxFace, size_t * pFaces, size_t szBufferSize) const;
		

	//Get Face Edge
	bool getFaceEdge(int idxFace, int idxEdge, vec3& start, vec3& end) const;

	//int getAllIncidentEdges(int idxQuery, DVec<int>& lstEdges);
	//int getAllIncidentFaces(int idxQuery, DVec<int>& lstFaces);

	int getVertexIncidentFaces(int idxQuery, DVec<FACE>& lstFaces) const;
	int getVertexIncidentFaces(int idxQuery, DVec<unsigned int>& lstFaces) const;
	
	int getVertexIncidentEdges(int idxQuery, DVec<EDGE>& lstEdges) const;
	int getVertexIncidentEdges(int idxQuery, DVec<unsigned int>& lstEdges) const;
};

//Find vertex indices and edge indices in a winged-mesh structure

typedef enum QUERY_TYPE {qtVertex, qtEdge, qtFace};
class CMeshQuery
{
public:
	

private:
	QUERY_TYPE  m_qtype;
	CWEMesh*	m_input;
	CWEMesh::EDGE m_queryEdge;

	bool		m_bFound;
	unsigned int m_foundIdx;


public:

	//CMeshQuery(QUERY_TYPE qtype):m_qtype(qtype), m_input(NULL) {;}
	CMeshQuery(QUERY_TYPE qtype, CWEMesh* input, CWEMesh::EDGE queryEdge)
	{
		m_qtype = qtype;
		m_input = input;
		m_bFound = false;
		m_foundIdx = NOT_SET_YET;
		m_queryEdge = queryEdge;
	}

	CMeshQuery(CMeshQuery& parent, tbb::split)
	{
		m_qtype = parent.m_qtype;
		m_input = parent.m_input;		
		m_bFound = parent.m_bFound;
		m_foundIdx = parent.m_foundIdx;
		m_queryEdge = parent.m_queryEdge;
	}

	bool isFound() const { return m_bFound;}
	unsigned int getFoundIndex() const {return m_foundIdx;}

	void join(CMeshQuery& rhs)
	{
		m_bFound = rhs.m_bFound;
		m_foundIdx = rhs.m_foundIdx;
	}


	void operator() (const tbb::blocked_range<unsigned int>& range );		

};


//Checks all edges for their missing left or right faces
class CEdgeCheckerBody
{
	CWEMesh* m_inputMesh;
	unsigned int m_ctErrors;
	unsigned int m_ctCorrections;

	__inline bool checkWindowed(size_t iEdge, CWEMesh::EDGE& query, int ctSides, size_t startFace, size_t endFace);

public:
	CEdgeCheckerBody()
	{
		m_inputMesh = NULL;
		m_ctErrors = 0;
		m_ctCorrections = 0;
	}

	CEdgeCheckerBody(CWEMesh*  input)
	{
		m_inputMesh = input;
		m_ctErrors = 0;
		m_ctCorrections = 0;
	}

	CEdgeCheckerBody(CEdgeCheckerBody& ec, tbb::split )
	{
		m_inputMesh = ec.m_inputMesh;
		m_ctErrors = 0;
		m_ctCorrections = 0;
	}

	~CEdgeCheckerBody()
	{
		m_inputMesh = NULL;
		m_ctErrors = 0;
		m_ctCorrections = 0;
	}

	void join(CEdgeCheckerBody& rhs)
	{
		m_ctCorrections += rhs.m_ctCorrections;
		m_ctErrors += rhs.m_ctErrors;
	}

	unsigned int getErrorsCount() const {return m_ctErrors;}
	unsigned int getCorrectionsCount() const { return m_ctCorrections; }
	bool correctedAll() const { return (m_ctCorrections == m_ctErrors);}
	void operator() (const tbb::blocked_range<unsigned int>& range );		
};

#endif