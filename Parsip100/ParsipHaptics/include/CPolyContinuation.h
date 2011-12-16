#ifndef CPOLYGONIZERPROCESS_H
#define CPOLYGONIZERPROCESS_H

#include "_PolygonizerStructs.h"
#include "CEdgeTable.h"
#include "CCubeTable.h"

#include "CLayerManager.h"
#include "PS_BlobTree/include/CRootFinder.h"

//Implementing Parallel While
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
#include "tbb/atomic.h"

using namespace tbb;
using namespace PS;
using namespace PS::BLOBTREE;
using namespace PS::MATH;
// ----------------------------------------------------------------------
//Voxel Tracker will find all the cubes across the surface boundarys
//Send these cubes to VoxelTriangulation to convert to triangles

class CVoxelTracker
{	

private:	
	//Pointer to layer
	CLayer * m_layer;

	//Pointer to mesh
	CMeshVV * m_lpMesh;

	//Blobtree root node
	CBlobNode* m_root;

	//Polygonization mode
	CellShape m_mode;

	//Root Finder
	CRootFinder * m_lpFinder;

	//Cube Side
	float m_size;		   

	//Delta to calculate Normal
	float m_normalDelta;

	//Cube Range within Lattice	
	vec3i m_vBounds;

	//Count number of field evaluations
	size_t m_ctFieldEvalPhase1;
	size_t m_ctFieldEvalPhase2;

	//Count number of threads
	int	 m_ctThreads;

	//Seed Point on Surface
	vec3f m_ptSeed;

	//Search start from here
	vec3f m_ptSearchStart;

	//Timers
	int m_timerPass1;
	int m_timerPass2;

	// Global list of corners (keeps track of memory)	
	list<CELL*> m_stkTempCubes;	
	std::vector<CELL*> m_lstFinalCubes;
	list<CORNER> m_cornerCache;	
	//vector of a linked list means 2D table
	// cube center hash table
	std::vector<CENTERLIST> m_centers;	   

	// corner value hash table 
	std::vector<CORNER_ELEMENT_LIST> m_corners;	   
	CEdgeTable m_edges;

private:
	TEST find (int sign, vec3f start);

	//Set Center
	bool setCenter(int i, int j, int k);

	//Set Corner
	CORNER *setCorner (int i, int j, int k);

	bool testface (int i, int j, int k, CELL* old, 
				   int face, int c1, int c2, int c3, int c4); 

public:
	CVoxelTracker(CLayer * aLayer);
	
	~CVoxelTracker() 
	{
		clearAll();
	}

	void clearAll();

	bool march(bool bParallel);

	void getTimerValues(int &pass1, int &pass2);
	int getNumThreads() {return m_ctThreads;}

	size_t getNumFieldEvals(size_t& phase1, size_t& phase2);
	size_t getNumFieldEvals();

};


//A body is needed for Parallel For structure.
//Triangulating 
class CVoxelTriangulationBody
{
private:
	//Input Settings
	std::vector<CELL*> * m_plstCubes;
	CRootFinder * m_lpFinder;
	CBlobNode* m_root;
	float m_normalDelta;	
	size_t m_ctFieldEval;
	CellShape m_cellShape;
	CEdgeTable m_edges;
	size_t m_start;
	size_t m_end;

	int m_ctThreads;
	int m_idxThread;
	//Output
	CMeshVV m_mesh;
private:
	//Create a Triangle and set each vertex to the proper index
	bool triangle (int i1, int i2, int i3);

	// vertid: return index for vertex on edge:
	// c1->value and c2->value are presumed of different sign
    // return saved index if any; else compute vertex and save 
	int vertid (CORNER* c1, CORNER* c2);

	//**** Tetrahedral Polygonization ****/
	// dotet: triangulate the tetrahedron
	// b, c, d should appear clockwise when viewed from a
	// return 0 if client aborts, 1 otherwise 
	// docube: triangulate the cube directly, without decomposition 	
	bool docube (CELL* cube);

	//Decompose each cube to 4 tetrahedras
	bool dotet (CELL* cube, int c1, int c2, int c3, int c4);

public:
	CVoxelTriangulationBody(std::vector<CELL*> * plstCubes,					   
		CBlobNode * root,
		CRootFinder * finder,
		float delta,
		CellShape mode);

	CVoxelTriangulationBody(CVoxelTriangulationBody &x, split);

	~CVoxelTriangulationBody();

	void join(const CVoxelTriangulationBody &y);

	void clearAll();

	CMeshVV& getMesh();
	size_t getNumFieldEval() const {return m_ctFieldEval;}
	int getNumThreads() {return m_ctThreads;}

	void process(size_t start, size_t end);

	void operator ()(const blocked_range<size_t> &range);
};

bool runParallelReduceContinuation(CLayer * alayer);

#endif