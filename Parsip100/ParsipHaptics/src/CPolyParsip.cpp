#include "CPolyParsip.h"
#include "PS_FrameWork/include/_parsProfile.h"
#include "CPolyContinuation.h"

#include "PS_FrameWork/include/PS_DateTime.h"

using namespace PS::DATETIMEUTILS;

//Copy Constructor
CParsip::CParsip(CLayer* alayer, vec3i totalSubVolumes)
{
	if(alayer == NULL)
		throw "Invalid Layer passed to ParMC.";

	this->m_layer			= alayer;
	this->m_root			= alayer->getBlob();
	this->m_mode			= alayer->getCellShape();
	this->m_cellSize		= alayer->getCellSize();
	this->m_normalDelta		= alayer->getCellSize() * 0.01f;
	this->m_totalSubVolumes = totalSubVolumes;


	//Set this to a proper root finding method
	this->m_lpFinder = new CRootFinder(this->m_root, rfmBisection);
	//this->m_lpVVMesh = new CMeshVV();	
	this->m_ctHashTable = 0;
	this->m_idxThread = 0;
	this->m_ctThreads = 1;
	this->m_ctFieldEval = 0;
}

CParsip::CParsip(CParsip &x, tbb::split)
{
	//Objects Read from
	this->m_layer = x.m_layer;
	this->m_root = x.m_root;

	//Scalar Settings
	this->m_mode			= x.m_mode;
	this->m_cellSize		= x.m_cellSize;
	this->m_normalDelta		= x.m_normalDelta;
	this->m_totalSubVolumes = x.m_totalSubVolumes;	

	//Independent objects
	this->m_lpFinder = new CRootFinder(this->m_root, rfmNewtonRaphson);
	//this->m_lpVVMesh = new CMeshVV();
	this->m_ctHashTable = 0;	
	this->m_idxThread	= x.m_idxThread + 1;
	this->m_ctThreads   = x.m_ctThreads;

	this->m_ctFieldEval = 0;
}

CParsip::~CParsip()
{
	clearAll();
}

void CParsip::clearAll()
{
	CORNER_ELEMENT_LIST cornerList;
	for(vector<CORNER_ELEMENT_LIST>::iterator it1 = m_tableCornerElements.begin(); it1 != m_tableCornerElements.end(); ++it1)
	{
		cornerList = static_cast<CORNER_ELEMENT_LIST>(*it1);				
		cornerList.clear();
	}		
	m_tableCornerElements.clear();	
	//************************************************************
	CENTERLIST centerList;
	for(vector<CENTERLIST>::iterator it2 = m_tableProcessed.begin(); it2 != m_tableProcessed.end(); ++it2)
	{
		centerList = static_cast<CENTERLIST>(*it2);		
		centerList.clear();
	}		
	m_tableProcessed.clear();
	//************************************************************
	m_edges.clearAll();
	delete m_lpFinder;
	m_lpFinder = NULL;

	m_layer = NULL;
	m_root = NULL;
	m_mesh.removeAll();
}

void CParsip::join(const CParsip &y)
{
	this->m_mesh.appendFrom(y.m_mesh);

	//Accumulate Field Evaluations
	this->m_ctThreads++;
	this->m_ctFieldEval += y.m_ctFieldEval;	
}

//Note:blocked_range3d will divide ranges and sort them in format of: Pages, Rows, Cols
//Example:
//for( tbb::blocked_range<int>::const_iterator i=r.pages().begin();
//                i!=r.pages().end(); ++i )
//            for( tbb::blocked_range<int>::const_iterator j=r.rows().begin();
//                     j!=r.rows().end(); ++j )
//                for( tbb::blocked_range<int>::const_iterator
//                        k=r.cols().begin(); k!=r.cols().end(); ++k )
//                    ++Array[i][j][k];
void CParsip::operator() (const blocked_range3d<int>& range)
{	
	for( tbb::blocked_range<int>::const_iterator i= range.pages().begin(); i!= range.pages().end(); ++i )
		for( tbb::blocked_range<int>::const_iterator j= range.rows().begin(); j!= range.rows().end(); ++j )
			for( tbb::blocked_range<int>::const_iterator k= range.cols().begin(); k!= range.cols().end(); ++k )			
				process(i, j, k);
}

void CParsip::process()
{
	for(int i=0; i < m_totalSubVolumes.x; i++)
		for(int j=0; j < m_totalSubVolumes.y; j++)
			for(int k=0; k < m_totalSubVolumes.z; k++)
				process(i, j, k);			
}

//Process will take a subvolume and starts applying MC on it
bool CParsip::process(int iSubVol, int jSubVol, int kSubVol)
{	
	if((m_totalSubVolumes.x == 0)||(m_totalSubVolumes.y == 0)||(m_totalSubVolumes.z == 0))
	{
		//throw "Error in Total Number of SubVolumes";
		return false;
	}

	if(((iSubVol < 0)||(iSubVol >= m_totalSubVolumes.x))||
	   ((jSubVol < 0)||(jSubVol >= m_totalSubVolumes.y))||
	   ((kSubVol < 0)||(kSubVol >= m_totalSubVolumes.z)))
	{
		//throw "Passed is subVolume indices are out of range!";
		return false;
	}

	vec3 lo = m_root->getOctree().lower;
	vec3 hi = m_root->getOctree().upper;	
	vec3 eachSide;
	//compute the length of eachside 
	eachSide.x = (hi - lo).x / static_cast<float>(m_totalSubVolumes.x);
	eachSide.y = (hi - lo).y / static_cast<float>(m_totalSubVolumes.y);
	eachSide.z = (hi - lo).z / static_cast<float>(m_totalSubVolumes.z);

	vec3 curLo;
	curLo.x = lo.x + static_cast<float>(iSubVol) * eachSide.x;
	curLo.y = lo.y + static_cast<float>(jSubVol) * eachSide.y;
	curLo.z = lo.z + static_cast<float>(kSubVol) * eachSide.z;
	vec3 curHi = curLo + eachSide;

	//Now we know where we are standing in the model
	COctree oct(curLo, curHi);
	oct.correct();

	
	size_t ctTotalSeeds = m_layer->countSeedPoints();
	
	//Check if there is any of seed points inside
	vec3 seed;
	bool bFoundSeed = false;	
	for(size_t i=0; i < ctTotalSeeds; i++)
	{
		seed = m_layer->getSeed(i);
		if(oct.isInside(seed))
		{
			bFoundSeed = true;
			break;
		}
	}
	//Now lets make sure we have a seed point
	if(!bFoundSeed) 
		seed = oct.center();

	//Find the surface point. The Origin
	srand(1);
	SUBVOL_RAY_TEST_RESULT testHot, testCold;	
	testHot  = shootRayTest(vsHot, seed, curLo, curHi, true);
	testCold = shootRayTest(vsCold, seed, curLo, curHi, true);		
	//Redo the tests
	if(!testHot.bSuccess)	
		testHot = shootRayTest(vsHot, seed, curLo, curHi, false);
	if(!testCold.bSuccess)	
		testCold = shootRayTest(vsCold, seed, curLo, curHi, false);
		
	//Now instead of Marching Cubes and complete Subdivision we do a continuation pass
	bool bInitComplete = false;
	vec3 origin;
	float fp;
	
	if((testHot.bSuccess)&&(testCold.bSuccess))	
	{
		m_ctFieldEval += m_lpFinder->findRoot(testHot.pos, testCold.pos, testHot.fieldValue, testCold.fieldValue, origin, fp);
		bInitComplete = oct.isInside(origin);
	}

	//If computed origin isnot inside ThreadVolume it is useless
	if(!bInitComplete)
	{
		for(size_t i=0; i < ctTotalSeeds; i++)
		{
			seed = m_layer->getSeed(i);
			if(oct.isInside(seed))
			{				
				testHot = shootRayTest(vsHot, seed, curLo, curHi, false);
				testCold = shootRayTest(vsCold, seed, curLo, curHi, false);
				if((testHot.bSuccess)&&(testCold.bSuccess))						
				{
					m_ctFieldEval += m_lpFinder->findRoot(testHot.pos, testCold.pos, testHot.fieldValue, testCold.fieldValue, origin, fp);
					if(oct.isInside(origin))
					{
						bInitComplete = true;	
						break;
					}						
				}
			}
		}

		if(!bInitComplete)	
			return false;
	}
	//*************************************************************	
	vec3i vStartIndex;
	vStartIndex.x = static_cast<int>((origin.x - curLo.x) / m_cellSize);
	vStartIndex.y = static_cast<int>((origin.y - curLo.y) / m_cellSize);
	vStartIndex.z = static_cast<int>((origin.z - curLo.z) / m_cellSize);

	//SubDivide Completely
	vec3i ctCells;
	ctCells.x = static_cast<int>(ceil((curHi.x - curLo.x) / m_cellSize));
	ctCells.y = static_cast<int>(ceil((curHi.y - curLo.y) / m_cellSize));
	ctCells.z = static_cast<int>(ceil((curHi.z - curLo.z) / m_cellSize));

	
	//Clear hash table before doing anything else
	//Reset Hash Table
	if(m_ctHashTable > 0)
	{
		CENTERLIST centerList;
		for(vector<CENTERLIST>::iterator it1 = m_tableProcessed.begin(); it1 != m_tableProcessed.end(); ++it1)
		{
			centerList = static_cast<CENTERLIST>(*it1);		
			centerList.clear();
		}		
		m_tableProcessed.clear();
		//***********************************
		CORNER_ELEMENT_LIST cornerList;
		for(vector<CORNER_ELEMENT_LIST>::iterator it2 = m_tableCornerElements.begin(); it2 != m_tableCornerElements.end(); ++it2)
		{
			cornerList = static_cast<CORNER_ELEMENT_LIST>(*it2);		
			cornerList.clear();
		}		
		m_tableCornerElements.clear();
		m_ctHashTable = 0;
	}
	m_edges.reset();
	m_tableCornerElements.resize(HASHSIZE);
	m_tableProcessed.resize(HASHSIZE);
	
	//push initial cube on stack:
	CELL * c = new CELL();
	c->i = vStartIndex.x; 
	c->j = vStartIndex.y; 
	c->k = vStartIndex.z;								
	for (int n = 0; n < 8; n++)	
	{
		c->corners[n] = createCorner(curLo, vStartIndex.x + BIT(n,2), vStartIndex.y + BIT(n,1), vStartIndex.z + BIT(n,0));
	}
	m_stkCubes.push(c);

	bool bDone;
	std::vector<CELL*> lstProcessedCubes;

	//PASS 1: Tracking all the cubes on the surface
	while (m_stkCubes.size() > 0) 
	{
		//Process Cube Faces and find nearby cubes
		c = m_stkCubes.top();

		//Remove front item from stack
		m_stkCubes.pop();	

		//CHANGE TO PARALLEL Triangulation		
		//Process Cube Right Now		
		if(m_mode == csCube)
			bDone = docube(c);
		else
		{
			//Break into Tetrahedra and Polygonize
			bDone = dotet(c, LBN, LTN, RBN, LBF) &&
				dotet(c, RTN, LTN, LBF, RBN) &&
				dotet(c, RTN, LTN, LTF, LBF) &&
				dotet(c, RTN, RBN, LBF, RBF) &&
				dotet(c, RTN, LBF, LTF, RBF) &&
				dotet(c, RTN, LTF, RTF, RBF);
		}
		
		//test six faces then add to stack
		//Left face
		testFace(curLo, c->i-1, c->j, c->k, ctCells, c, L, LBN, LBF, LTN, LTF);

		//Right face
		testFace(curLo, c->i+1, c->j, c->k, ctCells, c, R, RBN, RBF, RTN, RTF);

		//Bottom face
		testFace(curLo, c->i, c->j-1, c->k, ctCells, c, B, LBN, LBF, RBN, RBF);

		//Top face
		testFace(curLo, c->i, c->j+1, c->k, ctCells, c, T, LTN, LTF, RTN, RTF);

		//Near face
		testFace(curLo, c->i, c->j, c->k-1, ctCells, c, N, LBN, LTN, RBN, RTN);

		//Far face
		testFace(curLo, c->i, c->j, c->k+1, ctCells, c, F, LBF, LTF, RBF, RTF);

		//Add to processed list
		lstProcessedCubes.push_back(c);
	}

	//CHANGE TO PARALLEL Triangulation
	//PASS 2: Triangulating each individual Cube
	/*
	CVoxelTriangulationBody body(&lstProcessedCubes, 								 
							     m_root,
								 m_lpFinder, 
								 m_normalDelta, 
								 m_layer->getPolyMethod());		
	parallel_reduce(blocked_range<size_t>(0, lstProcessedCubes.size()), body, auto_partitioner());
	m_mesh.copyFrom(body.getMesh());
	body.clearAll();
	//*******************************************************************
	*/
	int ctProcessed = lstProcessedCubes.size();
	for(int iCube = 0; iCube < ctProcessed ; iCube++)
	{
		c = lstProcessedCubes[iCube];
		//Free Cube
		for (int n = 0; n < 8; n++)										
		{
			delete c->corners[n];
			c->corners[n] = NULL;
		}
		delete c;
		c = NULL;
	}
	lstProcessedCubes.clear();


	return true;
//End of process
}

bool CParsip::isProcessed(int i, int j, int k)
{
	int index = HASH(i,j,k);
	size_t ctProcessed = m_tableProcessed[index].size();
	
	CENTERELEMENT element;
	for (size_t iCenter=0; iCenter < ctProcessed; iCenter++)
	{
		element = m_tableProcessed[index][iCenter];
		if(element.i == i && element.j == j && element.k == k)
			return true;
	}
	element.i = i;
	element.j = j;
	element.k = k;
		 
	m_tableProcessed[index].push_back(element);
	return false;
}

//Decompose each cube to 4 tetrahedras
bool CParsip::docube (CELL* cube)
{
	int index = 0;
	for (int i = 0; i < 8; i++) 
	{
		if (getVertexState(cube->corners[i]->value) == vsHot ) 
			index += (1<<i);
	}
	if((index == 0)||(index == 255)) 
		return false;

	
	INTLISTS lstPolygons = CMarchCubeTable::getInstance()->getCubeCase(index);
	size_t ctTotalPolygons = lstPolygons.size();

	int a, b, ctVertices;
	size_t ctEdges = 0;
	INTLIST polygon; 
	for (size_t iPoly = 0; iPoly < ctTotalPolygons; iPoly++)
	{
		a = -1;
		b = -1;
		ctVertices = 0;
		polygon = lstPolygons[iPoly];
		ctEdges = polygon.size();

	
		int iEdge = static_cast<int>(ctEdges);
		while(--iEdge >= 0)
		{
			CORNER *c1 = cube->corners[corner1[polygon[iEdge]]];
			CORNER *c2 = cube->corners[corner2[polygon[iEdge]]];
			int c = vertid(c1, c2);
			ctVertices++;
			if (ctVertices > 2) 				
				addTriangle(a, b, c);			
			if (ctVertices < 3) 
				a = b;
			b = c;
		}
	}
	return (ctTotalPolygons > 0);
}


//**** Tetrahedral Polygonization ****/
// dotet: triangulate the tetrahedron
// b, c, d should appear clockwise when viewed from a
// return 0 if client aborts, 1 otherwise 
// docube: triangulate the cube directly, without decomposition 	
bool CParsip::dotet (CELL* cube, int c1, int c2, int c3, int c4) 
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
	case 1:	 return addTriangle(e5, e6, e3);
	case 2:	 return addTriangle(e2, e6, e4);
	case 3:	 return addTriangle(e3, e5, e4) && addTriangle(e3, e4, e2);
	case 4:	 return addTriangle(e1, e4, e5);
	case 5:	 return addTriangle(e3, e1, e4) && addTriangle(e3, e4, e6);
	case 6:	 return addTriangle(e1, e2, e6) && addTriangle(e1, e6, e5);
	case 7:	 return addTriangle(e1, e2, e3);
	case 8:	 return addTriangle(e1, e3, e2);
	case 9:	 return addTriangle(e1, e5, e6) && addTriangle(e1, e6, e2);
	case 10: return addTriangle(e1, e3, e6) && addTriangle(e1, e6, e4);
	case 11: return addTriangle(e1, e5, e4);
	case 12: return addTriangle(e3, e2, e4) && addTriangle(e3, e4, e5);
	case 13: return addTriangle(e6, e2, e4);
	case 14: return addTriangle(e5, e3, e6);
	}
	return true;
}


bool CParsip::addTriangle (int i1, int i2, int i3) 
{
	m_mesh.addTriangle(i1, i2, i3);
	//m_lpVVMesh->AddTriangle(i1, i2, i3);
	return true;
}	

// vertid: return index for vertex on edge:
// c1->value and c2->value are presumed of different sign
// return saved index if any; else compute vertex and save 
int CParsip::vertid (CORNER* c1, CORNER* c2) 
{			
	int vid = m_edges.getEdge(c1->i, c1->j, c1->k, c2->i, c2->j, c2->k);
	if (vid != -1) 
		return vid;			     

	vec3 a, b;
	a = c1->pos;
	b = c2->pos;

	vec3 output;
	float outputField;

	//Find Root		
	m_ctFieldEval += m_lpFinder->findRoot(a, b, c1->value, c2->value, output, outputField);

	//Compute new 
	CMaterial mtrl = m_root->baseMaterial(output);
	vec4f diffused = mtrl.getColorAsVector(ctDiffused);
	m_mesh.addColor(diffused);
	m_mesh.addNormal(m_root->normal(output));
	m_mesh.addVertex(output);
	m_ctFieldEval += 3;
	
	vid = m_mesh.countVertices() - 1;
	m_edges.setEdge(c1->i, c1->j, c1->k, c2->i, c2->j, c2->k, vid);
	return vid;
}

CORNER* CParsip::createCorner(CORNER* src)
{
	if(src == NULL) return NULL;
	CORNER* myCorner = new CORNER;
	myCorner->i = src->i;
	myCorner->j = src->j;
	myCorner->k = src->k;
	myCorner->pos = src->pos;
	myCorner->value = src->value;
	return myCorner;
}

CORNER* CParsip::createCorner(vec3 threadVolumeLo, int i, int j, int k)
{	
	vec3 pos;
	pos.x = threadVolumeLo.x + i*m_cellSize;
	pos.y = threadVolumeLo.y + j*m_cellSize;
	pos.z = threadVolumeLo.z + k*m_cellSize;

	CORNER* myCorner = new CORNER;
	myCorner->i = i;
	myCorner->j = j;
	myCorner->k = k;
	myCorner->pos = pos;
	float fv;

	//Check if we have it from Cache
	if(fetchFieldValueFromHashTable(i, j, k, fv))
		myCorner->value = fv;
	else
	{
		fv = m_root->fieldValue(pos);
		m_ctFieldEval++;
		myCorner->value = fv;

		//Cache Corner for Future Use
		int signature = HASH(i, j, k);
	
		//Compute fieldvalue
		CORNERELEMENT cacheCorner;
		cacheCorner.i = i;
		cacheCorner.j = j;
		cacheCorner.k = k;
		cacheCorner.value = fv;
		
		m_tableCornerElements[signature].push_back(cacheCorner);
		m_ctHashTable++;
	}

	return myCorner;
}

__inline bool CParsip::fetchFieldValueFromHashTable(int i, int j, int k, float &value)
{
	int signature = HASH(i, j, k);
	
	size_t ctCorners = m_tableCornerElements[signature].size();
	CORNERELEMENT element;
	for(size_t iCorner = 0; iCorner < ctCorners; iCorner++)
	{
		element = m_tableCornerElements[signature][iCorner];
		if((element.i == i)&& (element.j == j) && (element.k == k))
		{
			value = element.value;
			return true;
		}
	}
	return false;
}


SUBVOL_RAY_TEST_RESULT CParsip::shootRayTest(VertexState desired, vec3 start, vec3 boundLo, vec3 boundHi, bool bRandomWalk)
{		
	vec3 range = boundHi - boundLo;

	SUBVOL_RAY_TEST_RESULT result;
	result.bSuccess = false;
	result.ctIterations = 0;

	if(bRandomWalk)
	{
		for (int i = 0; i < 10000; i++) 
		{		
			result.ctIterations++;

			result.pos.x = static_cast<float>(start.x + range.x *(RAND()-0.5f));
			result.pos.y = static_cast<float>(start.y + range.y *(RAND()-0.5f));
			result.pos.z = static_cast<float>(start.z + range.z *(RAND()-0.5f));

			//Increment counter		
			result.fieldValue = m_root->fieldValue(result.pos);
			if (desired == getVertexState(result.fieldValue)) 		
			{
				result.bSuccess = true;
				return result;	
			}

			//slowly expand search outwards
			range = range * 1.0005f; 
		}
	}
	else
	{
		COctree oct(boundLo, boundHi);		

		//Test for all corners of current Thread Volume
		for(int n=0; n < 8; n++)
		{		
			result.ctIterations++;
			vec3 corner  = oct.getCorner(n);
			float step   = m_layer->getCellSize();
			vec3 dir	 = corner - start;			
			int ctSteps  = static_cast<int>(ceilf(dir.length() / step));
			dir.normalize();
			for (int i = 0; i < ctSteps; i++) 
			{	
				//Increment counter		
				result.ctIterations++;
				result.pos = start + (static_cast<float>(i) * step) * dir;				
				result.fieldValue = m_root->fieldValue(result.pos);
				if((desired == getVertexState(result.fieldValue))&&(oct.isInside(result.pos)))
				{
					result.bSuccess = true;
					return result;	
				}
			}

		}
	}
	
	return result;
}

// testface: given cube at lattice (i, j, k), and four corners of face,
// if surface crosses face, compute other four corners of adjacent cube
// and add new cube to cube stack 
bool CParsip::testFace (vec3 threadVolumeLo, int i, int j, int k, vec3i bounds, 
						CELL* old, int face, int c1, int c2, int c3, int c4)
{	
	static int facebit[6] = {2, 2, 1, 1, 0, 0};
	int n;
	int pos = old->corners[c1]->value > ISO_VALUE ? 1 : 0;
	int bit = facebit[face];

	//test if no surface crossing, cube out of bounds, or already visited: 
	if ((old->corners[c2]->value > ISO_VALUE) == pos &&
		(old->corners[c3]->value > ISO_VALUE) == pos &&
		(old->corners[c4]->value > ISO_VALUE) == pos) 
		return false;

	//Check if getting out of bounds
	if (abs(i) > bounds.x || abs(j) > bounds.y || abs(k) > bounds.z) 
		return false;

	//If it has been processed before
	if (isProcessed(i, j, k)) 
		return false;

	// create new_obj cube:
	CELL* new_obj = new CELL;
	new_obj->i = i;
	new_obj->j = j;
	new_obj->k = k;

	for (n = 0; n < 8; n++) 
		new_obj->corners[n] = NULL;

	new_obj->corners[FLIP(c1, bit)] = createCorner(old->corners[c1]);
	new_obj->corners[FLIP(c2, bit)] = createCorner(old->corners[c2]);
	new_obj->corners[FLIP(c3, bit)] = createCorner(old->corners[c3]);
	new_obj->corners[FLIP(c4, bit)] = createCorner(old->corners[c4]);

	//new_obj->corners[FLIP(c1, bit)] = old->corners[c1];
	//new_obj->corners[FLIP(c2, bit)] = old->corners[c2];
	//new_obj->corners[FLIP(c3, bit)] = old->corners[c3];
	//new_obj->corners[FLIP(c4, bit)] = old->corners[c4];

	for (n = 0; n < 8; n++)
	{
		if (new_obj->corners[n] == NULL)
			new_obj->corners[n] = createCorner(threadVolumeLo, i+BIT(n,2), j+BIT(n,1), k+BIT(n,0));
	}

	m_stkCubes.push(new_obj);
	return true;
}


VertexState CParsip::getVertexState(float fieldValue)
{
	if(fieldValue > ISO_VALUE)
		return vsHot;
	else
		return vsCold;
}

//StandAlone Functions
bool runParallelReduceParsip(CLayer* aLayer, vec3i subVols, const char * strDesc)
{
	//Start Timer
	parsProfilerStart(0);

	//vec3i subVols(1, 1, 2);
	CParsip body(aLayer, subVols);

	if(aLayer->getParallelMode())
	{
		blocked_range3d<int> range(0, subVols.x, 1, 0, subVols.y, 1, 0, subVols.z, 1);
		parallel_reduce(range, body, auto_partitioner());
	}
	else
		body.process();
	aLayer->setMesh(body.getMesh());
	
	//Stop Timer
	parsProfilerEnd(0);
	

	//Cleanup memory
	body.clearAll();

	return true;
}


