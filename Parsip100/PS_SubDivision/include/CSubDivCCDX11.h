#pragma once
#ifndef SUBDIV_CATMULLCLARK_DX11
#define SUBDIV_CATMULLCLARK_DX11

#include "CMeshWingedEdge.h"
#include "PS_FrameWork/include/PS_MeshVV.h"
#include "PS_FrameWork/include/_dataTypes.h"
#include "PS_FrameWork/include/DX_ComputeShader11.h"
#include "PS_FrameWork/include/TaskManager.h"

#include <d3dx9math.h>
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"


using namespace std;

#define INVALID_INPUT_PARAMS -1
#define UNABLE_TO_COMPUTE	 -2

namespace PS
{

bool convertToWingedEdgeMesh(const CMeshVV* input, CWEMesh* output);
bool convertToVVMesh(const CWEMesh* input, CMeshVV* output);

//=============================================================================
typedef enum SCANVERTEXSTATE {svsNotVisited, svsOK, svsDuplicate};

struct SCANVERTEX{
	vec3 pos;
	SCANVERTEXSTATE state;
	size_t nLoc;
};

struct FindDupsLoopContext{
	SCANVERTEX* pVertices;	
	size_t uStart;
	size_t uEnd;
	vec3 pos;
};

class FindDupsVertexWithTBBTaskManager
{
private:
	SCANVERTEX* m_pVertices;
	size_t m_ctVertices;
	size_t m_window_size;
public:

	FindDupsVertexWithTBBTaskManager(SCANVERTEX* pVertices, size_t ctVertices, size_t window_size = 0) :
	  m_pVertices(pVertices),
	  m_ctVertices(ctVertices),
	  m_window_size(window_size)
	  {}


  static void taskMarkDups(FindDupsLoopContext *aContext);

  void processAll();

};


//Implements our parallel_for algorithm for fixing face indices for a list of faces
//and vertices

class FindDupsFaceBody
{
private:
	//std::pair<vec4ui, vec4ui> CacheEntry;
	static const int CACHE_SIZE = 4;

	SCANVERTEX* m_lstVertices;
	size_t		m_ctVertices;
	vec4ui*		m_lstFaces;
	size_t		m_ctFaces;

	//int			m_idxCache;
	//CacheEntry  m_cache[CACHE_SIZE];



	__inline vec4ui countGaps(vec4ui upToExclusive) const;
public:
	FindDupsFaceBody(SCANVERTEX* lstVertices, vec4ui* lstFaces, size_t ctVertices, size_t ctFaces)
	{
		m_lstVertices = lstVertices;
		m_lstFaces = lstFaces;
		m_ctVertices = ctVertices;
		m_ctFaces    = ctFaces;
	}

	void operator()(const tbb::blocked_range<size_t>& range) const;

};

//handle Subdivision 
class CSubDivDX11
{
public:
	typedef vec4ui DEBUGFACE;

	struct VERTEX
	{
		D3DXVECTOR3 pos;
		UINT edge;
	};

	struct GPUEDGE
	{
		UINT selr[4];
		UINT lnrn[2];
	};

	struct QUADFACE
	{
		D3DXVECTOR3 p1;
		D3DXVECTOR3 p2;
		D3DXVECTOR3 p3;
		D3DXVECTOR3 p4;
	};


	struct FPEPVP
	{
		D3DXVECTOR3 fp;
		D3DXVECTOR3 ep;
		D3DXVECTOR3 vp;	
	};

	struct CBCS
	{
		UINT g_ctFaceSides;
		UINT g_ctInputFaces;
		UINT g_ctInputEdges;
		UINT g_ctInputVertices;
	};


	class CProcessFaceBody
	{
	private:
		CBCS		 inConstBuf;
		DEBUGFACE*	 inputFaces;
		GPUEDGE*	 inputEdges;
		VERTEX*		 inputVertices;

		FPEPVP*		 outputAll;
	public:
		CProcessFaceBody(const CBCS& aConstBuf, DEBUGFACE* lpFaces, GPUEDGE* lpEdges, VERTEX* lpVertices, FPEPVP* lpOutputAll)
		{
			this->inConstBuf = aConstBuf;
			this->inputFaces = lpFaces;
			this->inputEdges = lpEdges;
			this->inputVertices = lpVertices;
			this->outputAll = lpOutputAll;
		}

		void operator() (const tbb::blocked_range<UINT>& range ) const
		{
			for (UINT i=range.begin(); i!=range.end(); i++)			
				process(i);			
		}

		void processAll(UINT ctOutput)
		{
			for (UINT i=0; i < ctOutput; i++)			
				process(i);			
		}

		//void debugShader(UINT iface, const CBCS& inConstBuf, UINT* inputFaces, GPUEDGE* inputEdges, VERTEX* inputVertices, FPEPVP* outputAll);
		void process(UINT iface) const;
	};

public:
	OnOverallProgress m_pOnOverallProgress;
	OnStageProgress   m_pOnStageProgress;

	CSubDivDX11();
	CSubDivDX11(const CMeshVV * input);
	CSubDivDX11(const CWEMesh * input);

	~CSubDivDX11();

	//Get and Set
	CWEMesh* getMesh() const {return m_weMesh;}
	void setMesh(const CWEMesh* input);


	//Find all duplicate elements
	size_t countDuplicateVertices(const CWEMesh* lpInMesh);


	void getResult(CWEMesh *output) 
	{
		output->copyFrom(*m_weMesh);		
	}

	bool getResultAsMeshVV(CMeshVV *output)
	{
		return convertToVVMesh(m_weMesh, output);
	}
	
	void cleanup();
	bool run();
	bool runStageByStage();

	bool runOneShot();
	bool runOneShotTBB();
	
	bool compileAllShaders();
	bool isCompiled() const { return m_bCompiled;}

	int getLevel() const {return m_level;}
	void setLevel(int level) { m_level = level;}

	size_t computeFacePoints();
	size_t computeEdgePoints();
	size_t computeVertexPoints();

	void drawComputedPoints(float pointSize = 4.0f, 
							bool bFacePoints = true, 
							bool bEdgePoints = true, 
							bool bVertexPoints = true);

	public:
		//Indices based on faces
		DVec<vec3> m_lstAllFacePoints;

		//Indices based on edges
		DVec<vec3> m_lstAllEdgePoints;

		//Indices based on vertices
		DVec<vec3> m_lstAllVertexPoints;


private:
	bool m_bCompiled;

	int m_level;
//	CVVMesh*	m_input;

	//Converted to winged edge mesh
	CWEMesh*	m_weMesh;

	//Shaders
	DXShader11*		m_shaderFP;
	DXShader11*		m_shaderEP;
	DXShader11*		m_shaderVP;
	DXShader11*		m_shaderFPEPVP;


	void init();

	bool createFinalMeshInPlace();
	bool createFinalMesh();

	int readFromGPU(PS::DXShader11* shader, ID3D11Buffer* gpuOutputBuffer, UINT count, DVec<vec3>& lstToStore);

};




}
#endif 