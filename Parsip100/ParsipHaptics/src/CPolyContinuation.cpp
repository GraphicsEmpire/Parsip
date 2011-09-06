#include <QTime>
#include "CPolyContinuation.h"
#include "PS_FrameWork/include/_parsProfile.h"
#include "PS_FrameWork/include/PS_DateTime.h"

using namespace PS::DATETIMEUTILS;

CVoxelTracker::CVoxelTracker(CLayer * aLayer)
{
	//Set Mesh to clear the mesh and make sure instance is created	
	m_layer       = aLayer;
	m_root		  = aLayer->getBlob();
	m_mode        = aLayer->getCellShape();
	m_ptSearchStart = aLayer->getPolySeedPoint();
	m_size		  = aLayer->getCellSize();
	m_normalDelta = aLayer->getCellSize() * 0.01f;		 
	m_vBounds     = aLayer->getPolyBounds();	

	if(aLayer->getMesh() == NULL) 
		aLayer->setMesh();
	m_lpMesh = aLayer->getMesh();
	
	m_centers.resize(HASHSIZE);
	m_corners.resize(HASHSIZE);
}

// setCorner: return corner with the given lattice location
// set (and cache) its function value 
CORNER* CVoxelTracker::setCorner (int i, int j, int k) 
{
	//Cache corner value
	//First we pushback an empty one
	m_cornerCache.push_back(CORNER());

	//Take a reference to the empty corner just pushedback
	CORNER *c = &m_cornerCache.back();

	//Fill corner with index in lattice
	c->i = i; 
	c->j = j; 
	c->k = k; 

	//position of the corner in 3D
	c->pos.x = static_cast<float>(m_ptSeed.x + ((float)i- 0.5) * m_size);
	c->pos.y = static_cast<float>(m_ptSeed.y + ((float)j- 0.5) * m_size);
	c->pos.z = static_cast<float>(m_ptSeed.z + ((float)k- 0.5) * m_size);

	//Compute Hash Signature
	int index = HASH(i, j, k);

	//Field Value calculation is costy. So look for it in the corners HashTable first
	size_t ctCorners = m_corners[index].size();
	CORNERELEMENT corner;
	for(size_t iCorner = 0; iCorner < ctCorners; iCorner++)
	{
		corner = m_corners[index][iCorner];
		if(corner.i == i && corner.j == j && corner.k == k)
		{
			c->value = corner.value;
			return c;
		}
	}

	
	//Not found in Hash Table so compute it here. and then add it to hash table for future use
	//Compute Field Value
	//Increment counter
	m_ctFieldEvalPhase1++;
	c->value = m_root->fieldValue(c->pos);

	//Setup a corner element 
	CORNERELEMENT elem;
	elem.i = i;
	elem.j = j;
	elem.k = k;
	elem.value = c->value;

	//Put corner element in HashTable
	m_corners[index].push_back(elem);
	return c;
}

size_t CVoxelTracker::getNumFieldEvals(size_t& phase1, size_t& phase2)
{
	phase1 = m_ctFieldEvalPhase1;
	phase2 = m_ctFieldEvalPhase2;
	return getNumFieldEvals();
}

size_t CVoxelTracker::getNumFieldEvals()
{
	return m_ctFieldEvalPhase1 + m_ctFieldEvalPhase2;
}

// setCenter: set (i,j,k) entry of table[]
// return 1 if already set; otherwise, set and return 0 
bool CVoxelTracker::setCenter(int i, int j, int k)
{
	int index = HASH(i,j,k);
	size_t ctCenters = m_centers[index].size();
	CENTERELEMENT element;

	for (size_t iCenter=0; iCenter < ctCenters; iCenter++)
	{
		element = m_centers[index][iCenter];
		if(element.i == i && element.j == j && element.k == k)
			return true;
	}

	element.i = i;
	element.j = j;
	element.k = k;
		 
	m_centers[index].push_back(element);
	return false;
}

// testface: given cube at lattice (i, j, k), and four corners of face,
// if surface crosses face, compute other four corners of adjacent cube
// and add new cube to cube stack 
bool CVoxelTracker::testface (int i, int j, int k, CELL* old, 
							  int face, int c1, int c2, int c3, int c4)
{	
	static int facebit[6] = {2, 2, 1, 1, 0, 0};
	int n;
	int pos = old->corners[c1]->value > ISO_VALUE ? 1 : 0;
	int bit = facebit[face];

	//test if no surface crossing, cube out of bounds, or already visited: 
	if ((old->corners[c2]->value > ISO_VALUE) == pos &&
		(old->corners[c3]->value > ISO_VALUE) == pos &&
		(old->corners[c4]->value > ISO_VALUE) == pos) return false;
	//Pourya
	if (abs(i) > m_vBounds[0] || abs(j) > m_vBounds[1] || abs(k) > m_vBounds[2]) return false;
	//if (abs(i) > m_bounds || abs(j) > m_bounds || abs(k) > m_bounds) return;

	if (setCenter(i, j, k)) return false;

	// create new_obj cube:
	CELL* new_obj = new CELL;
	new_obj->i = i;
	new_obj->j = j;
	new_obj->k = k;

	for (n = 0; n < 8; n++) 
		new_obj->corners[n] = 0;

	new_obj->corners[FLIP(c1, bit)] = old->corners[c1];
	new_obj->corners[FLIP(c2, bit)] = old->corners[c2];
	new_obj->corners[FLIP(c3, bit)] = old->corners[c3];
	new_obj->corners[FLIP(c4, bit)] = old->corners[c4];

	for (n = 0; n < 8; n++)
		if (new_obj->corners[n] == 0)
			new_obj->corners[n] = setCorner(i+BIT(n,2), j+BIT(n,1), k+BIT(n,0));

	m_stkTempCubes.push_front(new_obj);	
	return true;
}


// find: search for point with value of given sign (0: neg, 1: pos)
TEST CVoxelTracker::find (int sign, vec3 start)
{
	int i;
	TEST test;
	float range = m_size;
	test.ok = 1;
	for (i = 0; i < 10000; i++) 
	{
		test.pos.x = static_cast<float>(start.x + range*(RAND()-0.5));
		test.pos.y = static_cast<float>(start.y + range*(RAND()-0.5));
		test.pos.z = static_cast<float>(start.z + range*(RAND()-0.5));

		//Increment counter
		m_ctFieldEvalPhase1++;
		test.value = m_root->fieldValue(test.pos);
		if (sign == (test.value > ISO_VALUE)) 		
			return test;	

		//slowly expand search outwards
		range = range * 1.0005f; 
	}
	test.ok = 0;
	return test;
}


void CVoxelTracker::clearAll()
{
	delete m_lpFinder;
	m_lpFinder = NULL;

	//Clear own things
	m_cornerCache.clear();

	//Clear list of Temp cubes
	size_t count = m_stkTempCubes.size();
	for(list<CELL*>::iterator it1 = m_stkTempCubes.begin(); it1 != m_stkTempCubes.end(); ++it1)
	{
		delete * it1;
	}
	//m_stkTempCubes.clear();	
	m_stkTempCubes.resize(0);

	count = m_lstFinalCubes.size();
	for(vector<CELL*>::iterator it2 = m_lstFinalCubes.begin(); it2 != m_lstFinalCubes.end(); ++it2)
	{
		delete * it2;
	}	
	//m_lstFinalCubes.clear();
	m_lstFinalCubes.resize(0);
	//*************************************************************************
	CENTERLIST centerlist;
	for(vector<CENTERLIST>::iterator it3 = m_centers.begin(); it3 != m_centers.end(); ++it3)
	{
		centerlist = static_cast<CENTERLIST>(*it3);
		//centerlist.clear();		
		centerlist.resize(0);		
	}
	//m_centers.clear();
	m_centers.resize(0);

	CORNER_ELEMENT_LIST cornerList;
	for(vector<CORNER_ELEMENT_LIST>::iterator it4 = m_corners.begin(); it4 != m_corners.end(); ++it4)
	{
		cornerList = static_cast<CORNER_ELEMENT_LIST>(*it4);
		//cornerList.clear();
		cornerList.resize(0);
	}	
	//m_corners.clear();
	m_corners.resize(0);

	m_edges.clearAll();
}

//All the marching cube Algorithm is done here. 
bool CVoxelTracker::march(bool bParallel)
{
	//Timer is used for performance measurements
	QTime timer;
	timer.start();

	//Count field value evaluations for first pass
	m_ctFieldEvalPhase1 = 0;		
	m_lpFinder = new CRootFinder(m_root, rfmNewtonRaphson);
	int NoAbort = 1;
	TEST in, out;

	// find point on surface, beginning search at (x, y, z): 
	srand(1);
	in = find(1, m_ptSearchStart);
	out = find(0, m_ptSearchStart);
	if (!in.ok || !out.ok) 
		return false;
			
	//Compute first intersection point aka Seed Point
	float field = 0.0f;
	m_ptSeed = vec3(0.0f, 0.0f, 0.0f);
	m_ctFieldEvalPhase1 += m_lpFinder->findRoot(in.pos, out.pos, in.value, out.value, m_ptSeed, field);

	//push initial cube on stack:
	CELL *cube = new CELL();
	cube->i = 0;
	cube->j = 0;
	cube->k = 0;
	for (int n = 0; n < 8; n++)
		cube->corners[n] = setCorner(BIT(n,2), BIT(n,1), BIT(n,0));
	m_stkTempCubes.push_front(cube);

	// set corners of initial cube:	
	setCenter(0, 0, 0);

	int maxi = 0;
	int maxj = 0;
	int maxk = 0;
	
	//PASS 1: Tracking all the cubes on the surface
	while (m_stkTempCubes.size() != 0) 
	{
		//Process Cube Faces and find nearby cubes
		CELL* c = m_stkTempCubes.front();

		//Remove front item from stack
		m_stkTempCubes.pop_front();	

		//test six faces then add to stack
		//Left face
		testface(c->i-1, c->j, c->k, c, L, LBN, LBF, LTN, LTF);

		//Right face
		testface(c->i+1, c->j, c->k, c, R, RBN, RBF, RTN, RTF);

		//Bottom face
		testface(c->i, c->j-1, c->k, c, B, LBN, LBF, RBN, RBF);

		//Top face
		testface(c->i, c->j+1, c->k, c, T, LTN, LTF, RTN, RTF);

		//Near face
		testface(c->i, c->j, c->k-1, c, N, LBN, LTN, RBN, RTN);

		//Far face
		testface(c->i, c->j, c->k+1, c, F, LBF, LTF, RBF, RTF);

		maxi = max(maxi, c->i);
		maxj = max(maxj, c->j);
		maxk = max(maxk, c->k);

		//Add to final Cubes
		m_lstFinalCubes.push_back(c);
	}
	
	//Compute time spent for tracking cubes
	m_timerPass1 = timer.restart();

	//Set to zero
	m_ctFieldEvalPhase2 = 0;

	//PASS 2: Triangulating each individual Cube
	CVoxelTriangulationBody body(&m_lstFinalCubes, 								 
		m_root,
		m_lpFinder, 
		m_normalDelta, 
		m_mode);
	
	//Number of cubes Found
	size_t ctFoundCubes = m_lstFinalCubes.size();
	
	if(bParallel)
	{
		//Parallel Triangulation. Triangulate all the cubes			
		parallel_reduce(blocked_range<size_t>(0, ctFoundCubes), body, auto_partitioner());
	}
	else
	{
		//Serial Triangulation
		body.process(0, ctFoundCubes);
	}

	//Get result mesh back from Voxel Triangulator
	m_ctFieldEvalPhase2 = body.getNumFieldEval();
	m_ctThreads = body.getNumThreads();

	//Write result to Layer's Mesh
	m_lpMesh->copyFrom(body.getMesh());

	body.clearAll();

	//Clear Self data-structures.
	clearAll();

	m_timerPass2 = timer.restart();
	return true;
}

void CVoxelTracker::getTimerValues(int &pass1, int &pass2)
{
	pass1 = m_timerPass1;
	pass2 = m_timerPass2;
}
//***********************************************************************************************************************
//Class VoxelTriangulation Body
//Create a Triangle and set each vertex to the proper index
CVoxelTriangulationBody::CVoxelTriangulationBody(std::vector<CELL*> * plstCubes,					   
												 CBlobTree * root,
												 CRootFinder * finder,
												 float delta,
												 CellShape mode)
{
	m_plstCubes = plstCubes;		
	m_root		= root;
	m_normalDelta = delta;
	m_lpFinder	  = finder;
	m_cellShape	  = mode;
	m_ctFieldEval = 0;
	m_ctThreads = 0;
	m_idxThread = 0;

	//Face, Vertex, Color, TexChannels, TexCoords
	m_mesh.initMesh(CMeshVV::TRIANGLES, 3, 3, 4, 4);
}

CVoxelTriangulationBody::CVoxelTriangulationBody(CVoxelTriangulationBody &x, split)
{
	m_plstCubes = x.m_plstCubes;		
	m_lpFinder  = x.m_lpFinder;
	m_root	    = x.m_root;
	m_normalDelta = x.m_normalDelta;
	m_cellShape	 = x.m_cellShape;	
	m_ctFieldEval = 0;
	m_idxThread = x.m_idxThread + 1;

	//Face, Vertex, Color, TexChannels, TexCoords
	m_mesh.initMesh(CMeshVV::TRIANGLES, 3, 3, 4, 4);
}

CVoxelTriangulationBody::~CVoxelTriangulationBody()
{
	m_mesh.removeAll();
}

bool CVoxelTriangulationBody::triangle (int i1, int i2, int i3) 
{
	m_mesh.addTriangle(i1, i2, i3);
	return true;
}	

// vertid: return index for vertex on edge:
// c1->value and c2->value are presumed of different sign
// return saved index if any; else compute vertex and save 
int CVoxelTriangulationBody::vertid (CORNER* c1, CORNER* c2) 
{			
	int vid = m_edges.getEdge(c1->i, c1->j, c1->k, c2->i, c2->j, c2->k);
	if (vid != -1) 
		return vid;			     

	vec3f a, b;
	a = c1->pos;
	b = c2->pos;

	vec3f output;
	float outputField;

	//Find Root		
	m_ctFieldEval += m_lpFinder->findRoot(a, b, c1->value, c2->value, output, outputField);

	//For Normal Calculation we have 3 more field calculations
	m_ctFieldEval += 3;

	CMaterial mtrl = m_root->baseMaterial(output);
	//Send material of each vertex as texture coordinates
	//Last entry is shine power + Unit Size of Ambient, Diffused, Speculer
	vec4f shine = vec4f(mtrl.shininess, 4.0f, 4.0f, 4.0f);
	m_mesh.addTexCoord(0, mtrl.ambient);
	m_mesh.addTexCoord(1, mtrl.diffused);
	m_mesh.addTexCoord(2, mtrl.specular);
	m_mesh.addTexCoord(3, shine);
	m_mesh.addColor(mtrl.getColorAsVector(ctDiffused));
	m_mesh.addNormal(m_root->normal(output));
	m_mesh.addVertex(output);

	//Save PolyVertex
	//Get vertex v index from list. It is the last one
	vid = m_mesh.countVertices() - 1;		
	m_edges.setEdge(c1->i, c1->j, c1->k, c2->i, c2->j, c2->k, vid);
	return vid;
}

//**** Tetrahedral Polygonization ****/
// dotet: triangulate the tetrahedron
// b, c, d should appear clockwise when viewed from a
// return 0 if client aborts, 1 otherwise 
// docube: triangulate the cube directly, without decomposition 	
bool CVoxelTriangulationBody::docube (CELL* cube)
{
	int index = 0;
	for (int i = 0; i < 8; i++) 
	{
		if (cube->corners[i]->value > ISO_VALUE) 
			index += (1<<i);
	}

	
	INTLISTS lstPolygons = CMarchCubeTable::getInstance()->getCubeCase(index);
	size_t ctTotalPolygons = lstPolygons.size();

	int a, b, ctVertices;	
	INTLIST polygon; 
	for (size_t iPoly = 0; iPoly < ctTotalPolygons; iPoly++)
	{
		a = -1;
		b = -1;
		ctVertices = 0;
		polygon = lstPolygons[iPoly];
		
	
		int iEdge = static_cast<int>(polygon.size());
		while(--iEdge >= 0)
		{
			CORNER *c1 = cube->corners[corner1[polygon[iEdge]]];
			CORNER *c2 = cube->corners[corner2[polygon[iEdge]]];
			int c = vertid(c1, c2);
			ctVertices++;
			if (ctVertices > 2) 				
				triangle(a, b, c);			
			if (ctVertices < 3) 
				a = b;
			b = c;
		}
	}
	return true;
}


//Decompose each cube to 4 tetrahedras
bool CVoxelTriangulationBody::dotet (CELL* cube, int c1, int c2, int c3, int c4) 
{
	CORNER *a = cube->corners[c1];		
	CORNER *b = cube->corners[c2];
	CORNER *c = cube->corners[c3];
	CORNER *d = cube->corners[c4];
	int index = 0;
	int apos, bpos, cpos, dpos;

	if (apos = (a->value > ISO_VALUE)) index += 8;
	if (bpos = (b->value > ISO_VALUE)) index += 4;
	if (cpos = (c->value > ISO_VALUE)) index += 2;
	if (dpos = (d->value > ISO_VALUE)) index += 1;

	int e1, e2, e3, e4, e5, e6;
	//index is now 4-bit number representing one of the 16 possible cases 
	if (apos != bpos) e1 = vertid(a, b);
	if (apos != cpos) e2 = vertid(a, c);
	if (apos != dpos) e3 = vertid(a, d);
	if (bpos != cpos) e4 = vertid(b, c);
	if (bpos != dpos) e5 = vertid(b, d);
	if (cpos != dpos) e6 = vertid(c, d);

	//14 productive tetrahedral cases (0000 and 1111 do not yield polygons
	switch (index) 
	{
	case 1:	 return triangle(e5, e6, e3);
	case 2:	 return triangle(e2, e6, e4);
	case 3:	 return triangle(e3, e5, e4) && triangle(e3, e4, e2);
	case 4:	 return triangle(e1, e4, e5);
	case 5:	 return triangle(e3, e1, e4) && triangle(e3, e4, e6);
	case 6:	 return triangle(e1, e2, e6) && triangle(e1, e6, e5);
	case 7:	 return triangle(e1, e2, e3);
	case 8:	 return triangle(e1, e3, e2);
	case 9:	 return triangle(e1, e5, e6) && triangle(e1, e6, e2);
	case 10: return triangle(e1, e3, e6) && triangle(e1, e6, e4);
	case 11: return triangle(e1, e5, e4);
	case 12: return triangle(e3, e2, e4) && triangle(e3, e4, e5);
	case 13: return triangle(e6, e2, e4);
	case 14: return triangle(e5, e3, e6);
	}
	return true;
}

void CVoxelTriangulationBody::join(const CVoxelTriangulationBody &y)
{
	this->m_mesh.appendFrom(y.m_mesh);			
	this->m_ctFieldEval += y.m_ctFieldEval;
	this->m_ctThreads = max(this->m_idxThread, y.m_idxThread);
}

void CVoxelTriangulationBody::clearAll()
{
	m_mesh.removeAll();
}

CMeshVV& CVoxelTriangulationBody::getMesh() 
{
	return m_mesh;
}

void CVoxelTriangulationBody::process(size_t start, size_t end)
{
	bool bDone = false;

	//Hold Start and end
	m_start = start;
	m_end   = end;

	std::vector<CELL*> lstCubes = *m_plstCubes;
	if(m_cellShape == csTetrahedra)
	{
		for(size_t i = start ; i != end; ++i)
		{
			CELL * c = lstCubes[i];
			//Break into Tetrahedra and Polygonize
			bDone = dotet(c, LBN, LTN, RBN, LBF) &&
				dotet(c, RTN, LTN, LBF, RBN) &&
				dotet(c, RTN, LTN, LTF, LBF) &&
				dotet(c, RTN, RBN, LBF, RBF) &&
				dotet(c, RTN, LBF, LTF, RBF) &&
				dotet(c, RTN, LTF, RTF, RBF);
		}
	}
	else
	{
		for(size_t i = start ; i != end; ++i)
		{
			CELL * c = lstCubes[i];
			bDone = docube(c);
		}
	}
}

void CVoxelTriangulationBody::operator ()(const blocked_range<size_t> &range) 
{
	process(range.begin(), range.end());
}


bool runParallelReduceContinuation(CLayer * alayer)
{	
	parsProfilerStart(1);

	CVoxelTracker * tracker = new CVoxelTracker(alayer);
	bool bPar = alayer->getParallelMode();	
	bool bRes = tracker->march(bPar);

	parsProfilerEnd(1);	

	//House Keeping
	SAFE_DELETE(tracker);
	
	return true;
}

