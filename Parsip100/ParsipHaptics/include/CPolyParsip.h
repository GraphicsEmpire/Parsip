#ifndef CPOLYPARSIP_H
#define CPOLYPARSIP_H

#include <math.h>
#include <stack>
#include "_PolygonizerStructs.h"
#include "CEdgeTable.h"
#include "CCubeTable.h"

#include "CLayerManager.h"
#include "PS_BlobTree/include/CRootFinder.h"

//Implementing Parallel While
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
#include "tbb/blocked_range3d.h"
#include "tbb/atomic.h"


using namespace tbb;
using namespace PS;
using namespace PS::BLOBTREE;
using namespace PS::MATH;

class CParsip
{
private:
	//Pointer to the layer object
	CLayer * m_layer;

	//Root BlobTree
	CBlobTree* m_root;

	//Root Finder
	CRootFinder * m_lpFinder;

	//Edge Table
	CEdgeTable m_edges;

	//Current Task mesh
	//CMeshVV * m_lpVVMesh;
	CMeshVV m_mesh;

	//Polygonization mode
	CellShape m_mode;

	//Cube Side
	float m_cellSize;		   

	//Delta to calculate Normal
	float m_normalDelta;

	//Total SubVolumes
	vec3i m_totalSubVolumes;	

	//Cache Corners to fetch them in the future
	//std::vector<CORNER>  m_cornerCache;
	std::vector<CORNER_ELEMENT_LIST> m_tableCornerElements;

	//Hold the list of all centers that has been processed
	std::vector<CENTERLIST> m_tableProcessed;

	//Stack of cubes to be processed
	stack<CELL*> m_stkCubes;

	//Count number of field evaluations
	size_t m_ctFieldEval;	

	//Count number of corner elementes pushed in HashTable
	size_t m_ctHashTable;

	//Thread Index to Color
	int m_idxThread;
	int m_ctThreads;

private:

	bool isProcessed(int i, int j, int k);

	//Shoot Ray will find a point either inside or outside the surface
	SUBVOL_RAY_TEST_RESULT shootRayTest(VertexState desired, vec3 start, vec3 boundLo, vec3 boundHi, bool bRandomWalk = true);

	//Determine a vertex state. Either Hot or Cold
	VertexState getVertexState(float fieldValue);

	//Fetch fieldValue from a HashTable
	__inline bool    fetchFieldValueFromHashTable(int i, int j, int k, float &value);

	//Create a corner based on the origin and i,j,k indices passed
	CORNER* createCorner(vec3 threadVolumeLo, int i, int j, int k);
	CORNER* createCorner(CORNER* src);
	bool testFace(vec3 threadVolumeLo, int i, int j, int k, vec3i bounds, 
		       	  CELL* old, int face, int c1, int c2, int c3, int c4);


	bool addTriangle (int i1, int i2, int i3);
	int vertid (CORNER* c1, CORNER* c2);
	
	//Outputs triangles from a Cube
	bool docube(CELL* cube);

	//Output triangles from a Tetrahedra
	bool dotet (CELL* cube, int c1, int c2, int c3, int c4);	
public:	
	CParsip(CLayer * aLayer, vec3i totalSubVolumes);
	CParsip(CParsip &x, split);
	~CParsip();

	CMeshVV& getMesh() { return m_mesh;}
	//CMeshVV* getMeshVV() {return m_lpVVMesh;}

	void join(const CParsip &y);

	void clearAll();
	void process();
	bool process(int iSubVol, int jSubVol, int kSubVol);

	void operator ()(const blocked_range3d<int>& range);
};

bool runParallelReduceParsip(CLayer* aLayer, vec3i subVols, const char * strDesc = NULL);
//bool runParallelReduceMC(CLayer* aLayer);


#endif