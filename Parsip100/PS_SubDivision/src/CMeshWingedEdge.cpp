//#include "stdafx.h"
#include "CMeshWingedEdge.h"
#include "PS_FrameWork/include/_parsProfile.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"

#include <windows.h>
#include <wtypes.h>

void CMeshQuery::operator()(const tbb::blocked_range<unsigned int>& range )
{
	if(m_bFound) return;

	CWEMesh::EDGE e;
	for(size_t iEdge = range.begin(); iEdge != range.end(); iEdge++)
	{
		e = m_input->edgeAt(iEdge);

		if((e.start == m_queryEdge.start && e.end == m_queryEdge.end)||
		   (e.start == m_queryEdge.end && e.end == m_queryEdge.start))
		{
			m_bFound = true;
			m_foundIdx = iEdge;
			return;
		}
	}
}

//====================================================================================
__inline bool CEdgeCheckerBody::checkWindowed(size_t iEdge, CWEMesh::EDGE& query, int ctSides, size_t startFace, size_t endFace)
{
	CWEMesh::FACE f;
	CWEMesh::EDGE e;

	size_t idxPrev, idxNext;
	
	//Find missing face and add its info
	for(size_t iFace=startFace; iFace < endFace; iFace++)
	{
		f = m_inputMesh->faceAt(iFace);

		//Check to avoid setting both sides to the same face
		if((query.rface == NOT_SET_YET)&&(query.lface == iFace))
			continue;
		else if((query.lface == NOT_SET_YET)&&(query.rface == iFace))
			continue;

		for(int i=0; i < ctSides; i++)
		{
			e = m_inputMesh->edgeAt(f.edges[i]);

			if(((query.start == e.start)&&(query.end == e.end))||
				((query.start == e.end)&&(query.end == e.start)))
			{
				if(i == 0)
					idxPrev = f.edges[ctSides-1];
				else
					idxPrev = f.edges[i-1];

				if(i == ctSides-1)
					idxNext = f.edges[0];
				else
					idxNext = f.edges[i+1];

				if(query.rface == NOT_SET_YET)
				{
					query.rface = iFace;
					query.rprev = idxPrev;
					query.rnext = idxNext;
				}
				else
				{
					query.lface = iFace;
					query.lprev = idxPrev;
					query.lnext = idxNext;
				}

				m_inputMesh->setEdge(iEdge, query);
				
				return true;
			}

		}//End For Side
	}//End For Faces

	return false;
}


void CEdgeCheckerBody::operator() (const tbb::blocked_range<unsigned int>& range )
{
	CWEMesh::EDGE query;
	

	size_t ctEdges = m_inputMesh->countEdges();
	size_t ctFaces = m_inputMesh->countFaces();
	size_t ctVertices = m_inputMesh->countVertices();	
	int ctSides		  = m_inputMesh->getFaceSides();
	bool bEdgeCorrected = false;

		
	size_t window_size = 5;
	vec2ui window;

	for(size_t iEdge = range.begin(); iEdge != range.end(); iEdge++)
	{
		query = m_inputMesh->edgeAt(iEdge);

		if((query.lface == NOT_SET_YET)||(query.rface == NOT_SET_YET))
		{
			m_ctErrors++;
			
			if(m_inputMesh->isFaceIndex(query.rface))
			{				
				//Set the smallest window possible around the incident 
				window.x = query.rface > window_size?query.rface - window_size:0;
				window.y = query.rface < (ctFaces - window_size)? query.rface + window_size:ctFaces;
			}
			else
				window = vec2ui(0, ctFaces);

			bEdgeCorrected = checkWindowed(iEdge, query, ctSides, window.x, window.y);
			window = vec2ui(0, ctFaces);
			if(!bEdgeCorrected)
				bEdgeCorrected = checkWindowed(iEdge, query, ctSides, window.x, window.y);			
				
			if(bEdgeCorrected)
				m_ctCorrections++;
		}//End if 
	}//End for
}


CWEMesh::CWEMesh(int ctFaceSides)
{
	m_ctFaceSides = ctFaceSides;
	m_pOnOverallProgress = NULL;
	m_pOnStageProgress = NULL;
}

CWEMesh::~CWEMesh()
{
	cleanup();
}

CWEMesh::FACE CWEMesh::faceAt(size_t idx) const
{ 
	if(isFaceIndex(idx)) 
		return m_lstFaces[idx];
	else
	{
		ReportError("Invalid face index.");			
		return m_lstFaces[0];
	}
}

CWEMesh::EDGE CWEMesh::edgeAt(size_t idx) const 
{ 
	if(isEdgeIndex(idx)) 
		return m_lstEdges[idx];
	else
	{			
		ReportError("Invalid Edge index");
		return m_lstEdges[0];
	}
}

CWEMesh::VERTEX CWEMesh::vertexAt(size_t idx) const 
{ 
	if(isVertexIndex(idx)) 
		return m_lstVertices[idx];
	else
	{
		ReportError("Invalid vertex index.");
		return m_lstVertices[0];
	}
}


void CWEMesh::copyFrom(const CWEMesh& rhs)
{
	this->cleanup();

	this->m_ctFaceSides = rhs.m_ctFaceSides;
	this->m_pOnOverallProgress = rhs.m_pOnOverallProgress;
	this->m_pOnStageProgress   = rhs.m_pOnStageProgress;

	for (size_t iEdge=0; iEdge < rhs.m_lstEdges.size(); iEdge++)
		this->m_lstEdges.push_back(rhs.m_lstEdges[iEdge]);
	
	for (size_t iFace=0; iFace < rhs.m_lstFaces.size(); iFace++)
		this->m_lstFaces.push_back(rhs.m_lstFaces[iFace]);

	for (size_t iVertex=0; iVertex < rhs.m_lstVertices.size(); iVertex++)
		this->m_lstVertices.push_back(rhs.m_lstVertices[iVertex]);

}

void CWEMesh::cleanup()
{
	m_lstEdges.resize(0);
	m_lstVertices.resize(0);
	m_lstFaces.resize(0);
	m_pOnOverallProgress = NULL;
	m_pOnStageProgress = NULL;
}

//================================================================================
// Returns vertex indices for all faces of mesh
// passing NULL for buffer or 0 for size of buffer will force this function to return the 
// required buffer size
size_t CWEMesh::getAllFaceIndices(size_t* pVertexIndices, size_t szVertexIndices)  const
{
	size_t ctFaces = m_lstFaces.size();

	//If input is null return the memory size required to allocate
	if((pVertexIndices == NULL)||(szVertexIndices < ctFaces*MAX_FACE_SIDES))
		return MAX_FACE_SIDES*ctFaces;

	size_t oneset[MAX_FACE_SIDES];
	int ctRead = 0;
	
	//Get All Faces and add them
	for (size_t iFace=0; iFace < ctFaces; iFace++)
	{
		ctRead = getFaceVertices(iFace, oneset, MAX_FACE_SIDES);
		for (int iv=0; iv < ctRead; iv++)
			pVertexIndices[iv] = oneset[iv];

		pVertexIndices += MAX_FACE_SIDES;				 
	}

	return ctFaces;	

}

int CWEMesh::getOneRingNeighborV(size_t idxFace, size_t * pIndices, size_t szBufferSize) const
{
	if(!isFaceIndex(idxFace))
		return NOT_AN_INDEX;

	//Number of vertices in the ring is: K = 2N + 8
	//N is valence of the vertex
	if((pIndices == NULL)||(szBufferSize < 12))
		return BUFFER_TOO_SMALL;

	size_t faceVertexIndices[MAX_FACE_SIDES];	
	size_t ctVertices = getFaceVertices(idxFace, faceVertexIndices, MAX_FACE_SIDES);
	FACE& curFace     = faceAt(idxFace);

	int ctAdded = 0;
	int ctSharedEdges = 0;
	DVec<size_t> lstFaces;	
	for(size_t i=0; i < ctVertices; i++)
	{
		//Faces  around a vertex
		lstFaces.resize(0);
		size_t idxCornerVertex = faceVertexIndices[i];
		getVertexIncidentFaces(idxCornerVertex, lstFaces);
				
		//Iterating on faces		
		for(size_t iFace = 0; iFace < lstFaces.size(); iFace++)
		{		
			if(lstFaces[iFace] != idxFace)
			{
				//Find all shared edges
				ctSharedEdges = 0;
				for(int j=0; j < MAX_FACE_SIDES; j++)
				{
					if((curFace.edges[j] != NOT_SET_YET))
					{
						if(faceHasEdge(faceAt(lstFaces[iFace]), curFace.edges[j]))
						{
							ctSharedEdges++;														
						}
					}
				}	

				//We are only interested in corner faces with no shared edges
				if(ctSharedEdges == 0)
				{
					size_t vIndices[MAX_FACE_SIDES];
					int ret = getFaceVertices(lstFaces[iFace], vIndices, MAX_FACE_SIDES);

					//To sort ring vertices in the proper order we need to find shared vertex
					int idxStart = -1;
					for(int j=0; j < ret; j++)
					{
						if(vIndices[j] == idxCornerVertex)
						{
							idxStart = j;
							break;
						}
					}

					//Loop over vertices in the ring
					int rem = ret;
					while(rem > 0)
					{
						idxStart = (idxStart+1) % ret;
						if(vIndices[idxStart] != idxCornerVertex)					
						{						
							if(ctAdded < (int)szBufferSize)
							{
								pIndices[ctAdded] = vIndices[idxStart];
								ctAdded++;										
							}
							else
								return BUFFER_TOO_SMALL;
						}						
						rem--;
					}

					//Since we added one won't find another quad for this corner
					break;
				}				
				//-------------------------------
			}
		}
	}

	//Return number of vertices found for the ring
	return ctAdded;
}

//int getOneRingNeighborV(size_t idxFace, vec3 * pVertices, size_t szBufferSize) const;
//int getOneRingNeighborF(size_t idxFace, size_t * pFaces, size_t szBufferSize) const;

size_t CWEMesh::addEdge(EDGE& e)
{
	m_lstEdges.push_back(e);
	return m_lstEdges.size() - 1;	
}

bool CWEMesh::setEdge(unsigned int idxEdge, EDGE& value)
{
	if(!isEdgeIndex(idxEdge)) return false;

	m_lstEdges[idxEdge] = value;
	return true;

}

bool CWEMesh::setVertex(unsigned int idxVertex, vec3 pos)
{
	if(!isVertexIndex(idxVertex)) return false;
	m_lstVertices[idxVertex].pos = pos;
	return true;
}

bool CWEMesh::performCompleteTest(int* pErrorCount)
{	
	unsigned int  ctEdges = m_lstEdges.size();
	if(ctEdges == 0)
		return false;

	parsProfilerStart(1);

	CEdgeCheckerBody body(this);
	tbb::parallel_reduce(tbb::blocked_range<unsigned int>(0, ctEdges), body, tbb::auto_partitioner());

	parsProfilerEnd(1);

	double t = parsProfilerTime(1);
	parsProfilerPrint("seconds to correct mesh", t);

	if(pErrorCount)
		*pErrorCount = body.getErrorsCount();
	return body.correctedAll();
}


void CWEMesh::copy(EDGE &dst, const EDGE &src)
{
	dst.end = src.end;
	dst.start = src.start;
	dst.lprev = src.lprev;
	dst.lnext = src.lnext;
	dst.rprev = src.rprev;
	dst.rnext = src.rnext;
	dst.lface = src.lface;
	dst.rface = src.rface;
}

void CWEMesh::copy(VERTEX &dst, const VERTEX &src)
{
	dst.pos = src.pos;
	dst.edge = src.edge;
}

void CWEMesh::copy(FACE &dst, const FACE &src)
{
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		dst.edges[i] = src.edges[i];		
	}
	//dst.edges = src.edges;
}

bool CWEMesh::hasVertex(vec3 v, unsigned int& foundIdx, unsigned int start, unsigned int end)
{
	unsigned int temp;
	
	if(start == -1)
		start = 0;
	if(end == -1)
		end = m_lstVertices.size();

	if(end < start)
	{
		temp = start;
		start = end;
		end = temp;
	}

	static const unsigned int window_size = 10;

	if(end - start < window_size)
	{
		vec3 p;
		for(unsigned int i = start; i < end; i++)
		{
			p = m_lstVertices[i].pos;
			if(p == v)
			{
				foundIdx = i;
				return true;
			}
		}
		return false;
	}
	else
	{
		vec2ui part1 = vec2ui(start, (start+end)/2);
		vec2ui part2 = vec2ui((start+end)/2 + 1, end);

		bool bres = hasVertex(v, foundIdx, part1.x, part1.y);
		if(!bres)
			bres = hasVertex(v, foundIdx, part2.x, part2.y);
		return bres;
	}
}

//Search
//return -1 is not found OTW returns vertex index
int CWEMesh::hasVertex(vec3 v)
{
	//DVec<VERTEX>::const_iterator itor;
	//for(itor = m_lstVertices.begin(); itor != m_lstVertices.end(); itor++)
	size_t sz = m_lstVertices.size();
	vec3 u;
	for(size_t i=0; i < sz; i++)
	{
		u = m_lstVertices[i].pos;
		
		if(u == v)
			return i;
	}
	return NOT_FOUND;
}

//return -1 is not found OTW returns Edge index
bool CWEMesh::hasEdge(vec3 sv, vec3 ev, unsigned int& foundEdge)
{
	vec2ui query;	
	if(!hasVertex(sv,  query.x))
		return false;
	if(!hasVertex(ev,  query.y))
		return false;

	if(hasEdge(query.x, query.y, foundEdge))
		return true;
	else
	{
		static const int window_size = 5;
		unsigned int ctEdges = m_lstEdges.size();
		EDGE e;

		if(ctEdges > 2*window_size)
		{			
			vec2ui window;
			VERTEX v = vertexAt(query.x);			
			if(isEdgeIndex(v.edge))
			{
				window.x = v.edge > window_size? v.edge - window_size:0;
				window.y = v.edge < (ctEdges - window_size)? v.edge + window_size:ctEdges;
				for(size_t iEdge=window.x; iEdge < window.y; iEdge++)
				{
					e  = m_lstEdges[iEdge];
					if(( e.start == query.x && e.end == query.y)||
						( e.start == query.y && e.end == query.x))
					{
						foundEdge = iEdge;
						return true;
					}
				}
			}
			
			//======================================================
			VERTEX u = vertexAt(query.y);
			if(isEdgeIndex(u.edge))
			{
				window.x = u.edge > window_size? u.edge - window_size:0;
				window.y = u.edge < (ctEdges - window_size)? u.edge + window_size:ctEdges;
				for(size_t iEdge=window.x; iEdge < window.y; iEdge++)
				{
					e  = m_lstEdges[iEdge];
					if(( e.start == query.x && e.end == query.y)||
						( e.start == query.y && e.end == query.x))
					{
						foundEdge = iEdge;
						return true;
					}
				}
			}			
			

			//Search all edges
			e.start = query.x;	
			e.end	= query.y;
			CMeshQuery body(qtEdge, this, e);
			tbb::parallel_reduce(tbb::blocked_range<unsigned int>(0, ctEdges), body, tbb::auto_partitioner());
			foundEdge = body.getFoundIndex();
			bool bFound = body.isFound();
			if(bFound)
				bFound = bFound;
			return bFound;
			
		}
		else
		{		
			for(size_t iEdge=0; iEdge < m_lstEdges.size(); iEdge++)
			{
				e  = m_lstEdges[iEdge];
				if(( e.start == query.x && e.end == query.y)||
					( e.start == query.y && e.end == query.x))
				{
					foundEdge = iEdge;
					return true;
				}
			}
		}
	}

	return false;		
}


//return -1 is not found OTW returns Edge index
bool CWEMesh::hasEdge(unsigned int start, unsigned int end, unsigned int& foundEdge)
{
	EDGE want(start, end);
	DVec<unsigned int> lstIncidents;
	getVertexIncidentEdges(start, lstIncidents);
	getVertexIncidentEdges(end, lstIncidents);

	EDGE s;
	for(size_t i=0; i < lstIncidents.size(); i++)
	{
		s = edgeAt(lstIncidents[i]); 
		if((s.start == want.start && s.end == want.end)||
		   (s.start == want.end && s.end == want.start))
		{
			foundEdge = lstIncidents[i];
			return true;
		}
	}
	return false;

	/*
	EDGE query(start, end);
	for(size_t iEdge=0; iEdge < m_lstEdges.size(); iEdge++)
	{
		if(query == m_lstEdges[iEdge])
		{
			foundEdge = iEdge;
			return true;
		}
	}
	return false;
	*/

	

	/* try1
	foundEdge = NOT_SET_YET;
	if(m_lstEdges.size() == 0)
		return false;
	if((!isVertexIndex(start))||(!isVertexIndex(end)))
		return false;
	
	size_t curEdge = NOT_SET_YET;
	if(isEdgeIndex(m_lstVertices[start].edge))
		curEdge = m_lstVertices[start].edge;
	else if(isEdgeIndex(m_lstVertices[end].edge))
		curEdge = m_lstVertices[end].edge;
	else
		return hasEdge(vertexAt(start).pos, vertexAt(end).pos, foundEdge);

	EDGE want(start, end);
	EDGE e1 = edgeAt(curEdge);
	EDGE s = e1;	
	
	do{
		if(s == want)
		{
			foundEdge = curEdge;
			return true;
		}

		if(s.start == start)		
			curEdge = s.lnext;
		else
			curEdge = s.rnext;
		if(isEdgeIndex(curEdge))
			s = edgeAt(curEdge);			
		else 
			return false;
	}
	while(!( s==e1 ));

	return false;
	*/
}

//Add Face by its ids
int CWEMesh::addFace(const size_t* pFaceIndices)
{
	if((m_ctFaceSides <= 0)||(m_ctFaceSides > MAX_FACE_SIDES))
		return NOT_FOUND;

	struct EDGE_QUEUE_ENTRY{
		int idx;		
		bool bRightFace;
	};

	DVec<EDGE_QUEUE_ENTRY> qEdges;
	EDGE_QUEUE_ENTRY curEdge;
	FACE futureFace;	
	size_t start, end;

	//Initialize new face
	for(int i=0; i < MAX_FACE_SIDES; i++)
		futureFace.edges[i] = NOT_SET_YET;

	//Future index that will is reserved for this face
	int idxFutureFace = m_lstFaces.size();
	size_t idxFutureEdge;

	//Assume all vertices have already been added	
	//Adding all required edges	
	//Add Face finally
	for(int i=0; i < m_ctFaceSides; i++)
	{
		//We are closed polygons. So last vertex is connected to first.
		if(i == m_ctFaceSides - 1)
		{
			start = pFaceIndices[m_ctFaceSides-1];
			end	  = pFaceIndices[0];
		}
		else
		{
			start = pFaceIndices[i];
			end	  = pFaceIndices[i+1];
		}

		//**********************************		
		//If edge found update current record
		//if(hasEdge(vertexAt(start).pos, vertexAt(end).pos, idxFutureEdge))
		if(hasEdge(start, end, idxFutureEdge))
		{			
			futureFace.edges[i] = idxFutureEdge;			
			
			//create entry in the queue
			curEdge.idx		   = idxFutureEdge;
			//Is found edge a right edge for this face?
			curEdge.bRightFace = (edgeAt(idxFutureEdge).start == start);
			qEdges.push_back(curEdge);
		}
		else
		//If not found the edge then add a new one
		{	
			EDGE futureEdge;

			idxFutureEdge    = m_lstEdges.size();
			
			//Update Vertex for this edge
			if(m_lstVertices[start].edge == NOT_SET_YET)
				m_lstVertices[start].edge = idxFutureEdge;
			if(m_lstVertices[end].edge == NOT_SET_YET)
				m_lstVertices[end].edge   = idxFutureEdge;

			futureEdge.start = start;
			futureEdge.end   = end;

			futureEdge.rface = idxFutureFace;
			futureEdge.lface = NOT_SET_YET;	

			futureEdge.rprev = NOT_SET_YET;
			futureEdge.rnext = NOT_SET_YET;
			futureEdge.lprev = NOT_SET_YET;
			futureEdge.lnext = NOT_SET_YET;
			m_lstEdges.push_back(futureEdge);
			futureFace.edges[i] = idxFutureEdge;

			//create entry in the queue
			curEdge.idx	   = idxFutureEdge;			
			//It is right face because we added it for this face for the first time.
			curEdge.bRightFace = true;
			qEdges.push_back(curEdge);
		}			
	}		
	//**************************		
	//Add this face to the list of faces
	m_lstFaces.push_back(futureFace);

	//Now lets update the other fields for each edge added. Chaining
	if(qEdges.size() != m_ctFaceSides) 
		ReportError("An error occured in chaining edges");

	int prev, next;
	EDGE edge;
	for(int iEdge=0; iEdge < m_ctFaceSides; iEdge++)
	{		
		curEdge = qEdges[iEdge];

		if(iEdge == 0)
			prev = qEdges[m_ctFaceSides-1].idx;			
		else
			prev = qEdges[iEdge-1].idx;
			

		if(iEdge == m_ctFaceSides - 1)
			next = qEdges[0].idx;			
		else
			next = qEdges[iEdge+1].idx;
			

		//Update entry in edgetable
		if(curEdge.bRightFace)
		{
			m_lstEdges[curEdge.idx].rface = idxFutureFace;
			m_lstEdges[curEdge.idx].rprev = prev;
			m_lstEdges[curEdge.idx].rnext = next;
		}
		else
		{
			m_lstEdges[curEdge.idx].lface = idxFutureFace;
			m_lstEdges[curEdge.idx].lprev = prev;
			m_lstEdges[curEdge.idx].lnext = next;
		}		
	}
	

	qEdges.resize(0);
	return 1;
}

//Add face in a Wedged-Edge Construction mesh
//very slow because of searching for dups.
int CWEMesh::addFace(int sides, const vec3* arrV)
{
	if((sides <= 0)||(sides > MAX_FACE_SIDES))
		return NOT_FOUND;

	struct EDGE_QUEUE_ENTRY{
		int idx;		
		bool bRightFace;
	};

	DVec<EDGE_QUEUE_ENTRY> qEdges;
	EDGE_QUEUE_ENTRY curEdge;
	FACE futureFace;	
	vec3 start, end;	

	//Initialize new face
	for(int i=0; i < MAX_FACE_SIDES; i++)
		futureFace.edges[i] = NOT_SET_YET;

	//Future index that will is reserved for this face
	int idxFutureFace = m_lstFaces.size();

	//Adding all required edges
	//Adding all vertices and 
	//Add Face finally
	for(int i=0; i < sides; i++)
	{
		//We are closed polygons. So last vertex is connected to first.
		if(i == sides - 1)
		{
			start = arrV[sides-1];
			end	  = arrV[0];
		}
		else
		{
			start = arrV[i];
			end = arrV[i+1];
		}

		//**************************		
		size_t idxFutureEdge;
				
		//If edge found update current record
		if(hasEdge(start, end, idxFutureEdge))
		{			
			futureFace.edges[i] = idxFutureEdge;			
			
			//create entry in the queue
			curEdge.idx	   = idxFutureEdge;

			EDGE etemp = edgeAt(idxFutureEdge);
			VERTEX vtemp = vertexAt(etemp.start);
			//Is found edge a right edge for this face?
			curEdge.bRightFace = (vertexAt(edgeAt(idxFutureEdge).start).pos == start);
			if(curEdge.bRightFace)
				vtemp.pos = vtemp.pos;
				
			qEdges.push_back(curEdge);
		}
		else
		//If not found the edge then add a new one
		{	
			EDGE futureEdge;

			idxFutureEdge = m_lstEdges.size();
			futureEdge.start = addVertexUnique(start, idxFutureEdge);
			futureEdge.end   = addVertexUnique(end, idxFutureEdge);

			futureEdge.rface = idxFutureFace;
			futureEdge.lface = NOT_SET_YET;	

			futureEdge.rprev = NOT_SET_YET;
			futureEdge.rnext = NOT_SET_YET;
			futureEdge.lprev = NOT_SET_YET;
			futureEdge.lnext = NOT_SET_YET;
			m_lstEdges.push_back(futureEdge);
			futureFace.edges[i] = idxFutureEdge;


			//create entry in the queue
			curEdge.idx	   = idxFutureEdge;			
			//It is right face because we added it for this face for the first time.
			curEdge.bRightFace = true;
			qEdges.push_back(curEdge);
		}			
	}		
	//**************************		
	//Add this face to the list of faces
	m_lstFaces.push_back(futureFace);

	//Now lets update the other fields for each edge added. Chaining
	if(qEdges.size() != sides) 
		ReportError("An error occured in chaining edges");

	int prev, next;
	for(int iEdge=0; iEdge < sides; iEdge++)
	{		
		curEdge = qEdges[iEdge];

		if(iEdge > 0)
			prev = qEdges[iEdge-1].idx;
		else
			prev = qEdges[sides-1].idx;

		if(iEdge < sides - 1)
			next = qEdges[iEdge+1].idx;
		else
			next = qEdges[0].idx;

		//Update entry in edgetable
		if(curEdge.bRightFace)
		{
			m_lstEdges[curEdge.idx].rface = idxFutureFace;
			m_lstEdges[curEdge.idx].rprev = prev;
			m_lstEdges[curEdge.idx].rnext = next;

			EDGE etemp = m_lstEdges[curEdge.idx];
			etemp = etemp;

		}
		else
		{
			m_lstEdges[curEdge.idx].lface = idxFutureFace;
			m_lstEdges[curEdge.idx].lprev = prev;
			m_lstEdges[curEdge.idx].lnext = next;

			EDGE etemp = m_lstEdges[curEdge.idx];
			etemp = etemp;

		}
	}


	qEdges.resize(0);
	
	return 1;
}

size_t CWEMesh::addVertexArray(DVec<vec3>& lstVertices)
{
	size_t ctVertices = lstVertices.size();

	m_lstVertices.resize(lstVertices.size());

	for(size_t i=0; i < ctVertices; i++)
	{
		m_lstVertices[i].pos  = lstVertices[i];
		m_lstVertices[i].edge = NOT_SET_YET;
	}	
	return ctVertices;
}

size_t CWEMesh::addVertexArray(size_t ctVertices, int elementSize, const float* pBuffer)
{
	m_lstVertices.resize(ctVertices);
	for(size_t i=0; i < ctVertices; i++)
	{
		m_lstVertices[i].pos  = vec3(&pBuffer[i*elementSize]);
		m_lstVertices[i].edge = NOT_SET_YET;
	}	
	return ctVertices;
}

size_t CWEMesh::addVertex(vec3 pos, int edge)
{
	VERTEX v;
	v.pos  = pos;
	v.edge = edge;
	m_lstVertices.push_back(v);
	return m_lstVertices.size() - 1;
}

size_t CWEMesh::addVertexUnique(vec3 pos, int edge)
{
	int idxVertex = hasVertex(pos);
	if(isVertexIndex(idxVertex))
		return idxVertex;

	VERTEX v;
	v.pos = pos;
	v.edge = edge;
	m_lstVertices.push_back(v);
	return m_lstVertices.size() - 1;
}

//Add
int CWEMesh::addQuadFace(float* v1, float* v2, float* v3, float* v4)
{
	return addQuadFace(vec3(v1), vec3(v2), vec3(v3), vec3(v4));
}

int CWEMesh::addQuadFace(vec3 v1, vec3 v2, vec3 v3, vec3 v4)
{
	vec3 arr[4];
	arr[0] = v1;
	arr[1] = v2;
	arr[2] = v3;
	arr[3] = v4;

	return addFace(4, arr);
}

int CWEMesh::addTriangleFace(float* v1, float* v2, float* v3)
{
	return addTriangleFace(vec3(v1), vec3(v2), vec3(v3));
}

int CWEMesh::addTriangleFace(vec3 v1, vec3 v2, vec3 v3)
{
	vec3 arr[3];
	arr[0] = v1;
	arr[1] = v2;
	arr[2] = v3;	

	return addFace(3, arr);	
}
//============================================================================
int CWEMesh::getFaceVertices(int idxFace, vec3* pOutVertices, size_t szOutVertices) const
{
	FACE& curFace = faceAt(idxFace);

	if((pOutVertices == NULL)||(szOutVertices < MAX_FACE_SIDES))
		return BUFFER_TOO_SMALL;

	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;

		EDGE& e		= edgeAt(curFace.edges[i]);
		pOutVertices[ctAdded] = vertexAt(e.start).pos;		
		ctAdded++;
	}

	return ctAdded;
}
//============================================================================
//Get all face vertices in one call
int CWEMesh::getFaceVertices(int idxFace, DVec<vec3> &outVertices) const
{
	FACE& curFace = faceAt(idxFace);
				
	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;
		
		EDGE& e		= edgeAt(curFace.edges[i]);
		vec3& start = vertexAt(e.start).pos;
		outVertices.push_back(start);
		ctAdded++;
	}

	return ctAdded;
}
//============================================================================
//Get all face vertices in one call
int CWEMesh::getFaceVertices(int idxFace, size_t* outIndices, size_t outIndicesSize) const
{
	if((outIndicesSize < MAX_FACE_SIDES)||(outIndices == NULL))
		return BUFFER_TOO_SMALL;

	FACE curFace = faceAt(idxFace);
	EDGE e;
	vec3 start, end;

	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;		
		e	  = edgeAt(curFace.edges[i]);
		if(e.rface == idxFace)
			outIndices[ctAdded] = e.start;
		else
			outIndices[ctAdded] = e.end;
		ctAdded++;
	}

	return ctAdded;
}

//============================================================================
//Get all face vertices in one call
int CWEMesh::getFaceVertices(int idxFace, DVec<unsigned int> &lstIndices) const
{
	FACE curFace = faceAt(idxFace);
	EDGE e;
	vec3 start, end;
		
	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;		
		e	  = edgeAt(curFace.edges[i]);
		if(e.rface == idxFace)
			lstIndices.push_back(e.start);
		else
			lstIndices.push_back(e.end);
		ctAdded++;
	}

	return ctAdded;
}

//============================================================================
//Get all face vertices in one call
int CWEMesh::getFaceEdges(int idxFace, size_t* pOutIndices, size_t outIndicesSize) const
{
	if((outIndicesSize < MAX_FACE_SIDES)||(pOutIndices == NULL))
		return BUFFER_TOO_SMALL;

	FACE curFace = faceAt(idxFace);
	EDGE e;
	vec3 start, end;

	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;				
		pOutIndices[ctAdded] = (curFace.edges[i]);
		ctAdded++;
	}
	return ctAdded;
}
//============================================================================
//Get all face vertices in one call
int CWEMesh::getFaceEdges(int idxFace, DVec<unsigned int> &lstIndices) const
{
	FACE curFace = faceAt(idxFace);
	EDGE e;
	vec3 start, end;
		
	int ctAdded = 0;
	for(int i=0; i < MAX_FACE_SIDES; i++)
	{
		if(curFace.edges[i] == NOT_SET_YET)
			continue;				
		lstIndices.push_back(curFace.edges[i]);
		ctAdded++;
	}

	return ctAdded;
}

//============================================================================
int CWEMesh::getEdgeEndPoints(int idxEdge, vec3& start, vec3& end) const
{
	if(!isEdgeIndex(idxEdge))
		return NOT_AN_INDEX;

	EDGE curEdge = edgeAt(idxEdge);

	start = vertexAt(curEdge.start).pos;
	end = vertexAt(curEdge.end).pos;

	return 2;
}

//============================================================================
bool CWEMesh::getEdgeMidPoint(int idxEdge, vec3& midpoint) const
{
	vec3 start, end;
	if(getEdgeEndPoints(idxEdge, start, end) > 0)
	{
		midpoint = 0.5f * (start + end);
		return true;	
	}
	return false;
}

//============================================================================
int CWEMesh::getVertexIncidentFaces(int idxQuery, DVec<unsigned int>& lstFaces) const
{
	if(!isVertexIndex(idxQuery))
		return NOT_FOUND;

	VERTEX query = vertexAt(idxQuery);

	//Get First Edge
	EDGE e1 = edgeAt(query.edge);
	EDGE s = e1;	
	
	do{
		if(s.start == idxQuery)
		{
			lstFaces.push_back(s.lface);
			s = edgeAt(s.lnext);					
		}
		else if(s.end == idxQuery)
		{
			lstFaces.push_back(s.rface);
			s = edgeAt(s.rnext);
		}
		else
			break;
	}
	while(!( s==e1 ));

	return (int)lstFaces.size();
}


//============================================================================
int CWEMesh::getVertexIncidentFaces(int idxQuery, DVec<FACE>& lstFaces) const
{
	if(!isVertexIndex(idxQuery))
		return NOT_FOUND;

	VERTEX query = vertexAt(idxQuery);

	//Get First Edge
	EDGE e1 = edgeAt(query.edge);
	EDGE s = e1;	
	
	do{
		if(s.start == idxQuery)
		{
			lstFaces.push_back(faceAt(s.lface));
			s = edgeAt(s.lnext);					
		}
		else
		{
			lstFaces.push_back(faceAt(s.rface));
			s = edgeAt(s.rnext);
		}
	}
	while(!( s==e1 ));

	return (int)lstFaces.size();
}

//============================================================================
int CWEMesh::getVertexIncidentEdges(int idxQuery, DVec<unsigned int>& lstEdges) const
{
	if(!isVertexIndex(idxQuery))
		return NOT_FOUND;
	
	size_t curEdge  = vertexAt(idxQuery).edge;
	if(!isEdgeIndex(curEdge))
		return 0;

	//Get First Edge
	EDGE e1 = edgeAt(curEdge);	
	EDGE s = e1;		
	do{		
		lstEdges.push_back(curEdge);
		if(s.start == idxQuery)				
			curEdge = s.lnext;														
		else		
			curEdge = s.rnext;

		if(isEdgeIndex(curEdge))
			s = edgeAt(curEdge);	
		else
			break;
	}
	while(!( s==e1 ));

	return (int)lstEdges.size();
}

//============================================================================
int CWEMesh::getVertexIncidentEdges(int idxQuery, DVec<EDGE>& lstEdges) const
{
	if(!isVertexIndex(idxQuery))
		return NOT_FOUND;

	VERTEX query = vertexAt(idxQuery);

	//Get First Edge
	EDGE e1 = edgeAt(query.edge);
	EDGE s = e1;	
	
	do{
		lstEdges.push_back(s);

		if(s.start == idxQuery)
			s = edgeAt(s.lnext);					
		else
			s = edgeAt(s.rnext);
	}
	while(!( s==e1 ));

	return (int)lstEdges.size();
}

bool CWEMesh::getFaceEdge(int idxFace, int idxEdge, vec3& start, vec3& end) const
{
	DVec<vec3> lstVertices;
	int ctSides = getFaceVertices(idxFace, lstVertices);
	
	if(idxEdge < 0 || idxEdge >= ctSides)
		return false;

	start = lstVertices[idxEdge];
	if(idxEdge == ctSides - 1)
		end = lstVertices[0];
	else
		end = lstVertices[idxEdge + 1];

	return true;
}

size_t CWEMesh::findAllExtraOrdinaryVertices(int *pOutValences, size_t szOutValences) const
{
	size_t ctFaces = m_lstFaces.size();
	size_t ctEdges = m_lstEdges.size();
	size_t ctVertices = m_lstVertices.size();
	
	int	* valences = new int[ctVertices];
	ZeroMemory(valences, ctVertices*sizeof(int));

	//Count all vertices usage
	EDGE e;
	for(size_t iEdge = 0; iEdge < ctEdges; iEdge++)
	{	
		e = edgeAt(iEdge);
		valences[e.start] += 1;
		valences[e.end] += 1;
	}
	/*
	size_t faceVertexIndices[MAX_FACE_SIDES];
	int ctFaceCorners = 0;
	for(size_t iFace=0; iFace < ctFaces; iFace++)
	{
		ctFaceCorners = getFaceVertices(iFace, faceVertexIndices, MAX_FACE_SIDES);
		for(int i=0; i < ctFaceCorners; i++)
		{
			valences[faceVertexIndices[i]] += 1;
		}
	}
	*/


	//Analyse counted data to find extra-ordinary vertices
	size_t ctExtraOridinary = 0;
	if((pOutValences)&&(szOutValences >= ctVertices))
	{	
		for (size_t iVertex=0; iVertex < ctVertices; iVertex++)
		{
			pOutValences[iVertex] = valences[iVertex];
			if(valences[iVertex] != m_ctFaceSides)
				ctExtraOridinary++;
		}
	}
	else
	{
		for (size_t iVertex=0; iVertex < ctVertices; iVertex++)
		{		
			if(valences[iVertex] != m_ctFaceSides)
				ctExtraOridinary++;
		}
	}

	SAFE_DELETE_ARRAY(valences);
	return ctExtraOridinary;
}

size_t CWEMesh::findAllExtraOrdinaryFaces(int *pOutFaceErrors, size_t szOutFaceErrors) const
{
	size_t ctFaces = m_lstFaces.size();
	size_t ctVertices = m_lstVertices.size();

	int	* valences = new int[ctVertices];
	ZeroMemory(valences, ctVertices*sizeof(int));
	bool bOutput = (pOutFaceErrors)&&(szOutFaceErrors >= ctFaces);
	if(bOutput)
		ZeroMemory(pOutFaceErrors, szOutFaceErrors*sizeof(int));

	size_t ctVertexErrors = findAllExtraOrdinaryVertices(valences, ctVertices);
	if(ctVertexErrors == 0)
		return 0;	

	size_t faceVertexIndices[MAX_FACE_SIDES];
	int ctFaceCorners = 0;
	int ctFaceErrors = 0;	
	size_t ctAllExtraOridinaryFaces = 0;
	for(size_t iFace=0; iFace < ctFaces; iFace++)
	{
		ctFaceErrors = 0;
		ctFaceCorners = getFaceVertices(iFace, faceVertexIndices, MAX_FACE_SIDES);		
		for(int i=0; i < ctFaceCorners; i++)
		{
			if(valences[faceVertexIndices[i]] != m_ctFaceSides)
				ctFaceErrors++;
		}

		if(ctFaceErrors > 0)
			ctAllExtraOridinaryFaces++;
		if(bOutput)
			pOutFaceErrors[iFace] = ctFaceErrors;
	}

	SAFE_DELETE_ARRAY(valences);
	return ctAllExtraOridinaryFaces;
}