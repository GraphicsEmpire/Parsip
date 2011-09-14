//#include "stdafx.h"
#include "CSubDivCCDX11.h"
//#include <GL/glut.h>
#include "PS_FrameWork/include/_parsProfile.h"
#include "PS_FrameWork/include/PS_PerfLogger.h"
#include "PS_FrameWork/include/DX_ShaderManager.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"
#include "PS_FrameWork/include/mathHelper.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"

#define MAXUI(x,y) gmax<size_t>((x), (y))

#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif


#define PERFORM_CPU_GPU_COMPARISON  1


#define START	0
#define END		1
#define LEFTFACE	2
#define RIGHTFACE	3

#define LEFTNEXT	0
#define RIGHTNEXT	1



namespace PS{

	//Convert OpenGL VVMesh to WingedEdge Mesh Structure
	bool convertToWingedEdgeMesh(const CMeshVV* input, CWEMesh* output)
	{
		if((input == NULL)||(output == NULL))
			return false;

		//Number of faces
		unsigned int ctFaces = input->countFaces();
		int ctSides = input->getUnitFaceSize();

		//1.First we will try to create winged-edge mesh by copying all vertices and adding edges ids
		output->cleanup();
		output->setFaceSides(ctSides);

		parsProfilerStart(3);

		//Add All vertices at once	
		output->addVertexArray(input->countVertices(), input->getVertexSize(), input->getVertexArray());

		size_t* ids = NULL;
		unsigned int iFace = 0;

		for(iFace=0; iFace < ctFaces; iFace++)
		{
			if(output->m_pOnStageProgress)
				output->m_pOnStageProgress(0, ctFaces, iFace);

			ids = const_cast<unsigned int*>(input->getFace(iFace));
			output->addFace(ids);
		}

		parsProfilerEnd(3);
		double t = parsProfilerTime(3);
		parsProfilerPrint("time to create mesh with face indices", t);

		/*
		bool bres = output->performCompleteTest();
		if(!bres)
		{
			//2.Since we failed for ids now we try adding faces directly by their vertices
			//Reset everything
			output->cleanup();
			output->setFaceSides(ctSides);

			parsProfilerStart(2);

			vec3* arrVertices = new vec3[ctSides];
			for(iFace=0; iFace < ctFaces; iFace++)
			{
				if(output->m_pOnStageProgress)
					output->m_pOnStageProgress(0, ctFaces, iFace);

				ids = const_cast<unsigned int*>(input->GetPrimitive(iFace));
				for(int iVertex=0; iVertex < ctSides; iVertex++)
					arrVertices[iVertex] = input->GetVertex3(ids[iVertex]);
				output->addFace(ctSides, arrVertices); 
			}

			//Check timings
			parsProfilerEnd(2);
			double t = parsProfilerTime(2);
			parsProfilerPrint("time to create mesh with direct face vertices", t);


			SAFE_DELETE_ARRAY(arrVertices);
			int errors;
			bool bres = output->performCompleteTest(&errors);
			if(!bres)	
			{
				DAnsiStr strMsg = PS::FILESTRINGUTILS::printToAStr("Error in Convert to WEMesh: Output mesh has %d errors [Faces with no relevant adjacency.]", errors);
				ReportError(strMsg.ptr());
				return false;
			}
		}
		*/
		return true;			
	}

	//Convert WingedEdge Mesh Structure to OpenGL VVMesh
	bool convertToVVMesh(const CWEMesh* input, CMeshVV* output)
	{
		if((input == NULL)||(output == NULL))
			return false;

		output->removeAll();	
		output->setModeByFaceSides(input->getFaceSides());
		output->setUnitVertexSize(3);

		int ctVertices = input->countVertices();
		for(int iVertex=0; iVertex < ctVertices; iVertex++)
		{
			output->addVertex(input->vertexAt(iVertex).pos);
		}

		int ctFaces = input->countFaces();

		DVec<unsigned int> lstFaceVertices;
		//unsigned int quadIDs[MAX_FACE_SIDES];
		for(int iFace=0; iFace < ctFaces; iFace++)
		{
			lstFaceVertices.resize(0);
			input->getFaceVertices(iFace, lstFaceVertices);				
			output->addFaceArray(lstFaceVertices);
		}
		lstFaceVertices.resize(0);

		return (output->countFaces() == ctFaces);
	}

	//==========================================================================================
	void FindDupsVertexWithTBBTaskManager::taskMarkDups(FindDupsLoopContext *aContext)
	{			
		SCANVERTEX* arrVertices = aContext->pVertices;
		for(size_t i = aContext->uStart; i < aContext->uEnd; i++)
		{
			if(arrVertices[i].state == svsNotVisited)
			{
				if(arrVertices[i].pos == aContext->pos)
				{
					arrVertices[i].state = svsDuplicate;
					arrVertices[i].nLoc  = aContext->uStart-1;
				}
			}
		}
	}

	void FindDupsVertexWithTBBTaskManager::processAll()
	{		
		TaskManager *pTaskManager = TaskManager::getTaskManager();		
		if(m_window_size == 0)
			m_window_size = pTaskManager->getThreadCount();


		FindDupsLoopContext* arrTaskParams = new FindDupsLoopContext[m_window_size];
		TaskManager::JobResult* arrJobResult = new TaskManager::JobResult[m_window_size];

		vec3* arrWindowBuf		 = new vec3[m_window_size];
		int* arrIsDispatched     = new int[m_window_size];


		//Variables for the two while loops
		size_t idxVertex = 0;		
		size_t idxTask = 0;
		
		bool bInWindow = false;
		//While loop on vertices list
		while(idxVertex < m_ctVertices)
		{				
			//Init task index
			idxTask = 0;
			//Init Dispatched
			for(size_t i=0; i < m_window_size; i++)
				arrIsDispatched[i] = FALSE;

			//Loop on tasks
			while(idxTask < m_window_size)
			{
				if(idxVertex >= m_ctVertices)
					break;

				//Check in current window if we already have such a vertex
				bInWindow = false;				
				for(size_t j=0; j < idxTask; j++)
				{
					if(arrWindowBuf[j] == m_pVertices[idxVertex].pos)
					{
						bInWindow = true;
						break;
					}
				}


				//Dispatch the task if it is safe to do 
				if((!bInWindow)&&(m_pVertices[idxVertex].state == svsNotVisited))
				{
					//Set status of the vertex to ok
					m_pVertices[idxVertex].state = svsOK;

					//Init params for task
					arrTaskParams[idxTask].pos	     = m_pVertices[idxVertex].pos;
					arrTaskParams[idxTask].pVertices = m_pVertices;
					arrTaskParams[idxTask].uStart    = idxVertex+1;
					arrTaskParams[idxTask].uEnd      = m_ctVertices;
					pTaskManager->dispatch(&arrJobResult[idxTask], TaskManager::JobFunction(taskMarkDups) ,&arrTaskParams[idxTask]);
					//TaskManager::sleep(50);
					
					//keep it for checking
					arrWindowBuf[idxTask] = m_pVertices[idxVertex].pos;
					arrIsDispatched[idxTask] = TRUE;
					idxTask++;
				}				

				idxVertex++;
			}//End While idxTask

			//Wait for all launched tasks
			for(size_t i=0; i < m_window_size; i++)
			{
				if(arrIsDispatched[i])
					pTaskManager->wait(&arrJobResult[i]);
			}			
		}//End while idxVertex

		//Cleanup		
		SAFE_DELETE_ARRAY(arrTaskParams);
		SAFE_DELETE_ARRAY(arrWindowBuf);
		SAFE_DELETE_ARRAY(arrIsDispatched);
		SAFE_DELETE_ARRAY(arrJobResult);
	}
	//==========================================================================================	
	__inline vec4ui FindDupsFaceBody::countGaps(vec4ui upToExclusive) const
	{
		vec4ui ctGaps = vec4ui(0, 0, 0, 0);
		size_t maxUpto = MAXUI(MAXUI(MAXUI(upToExclusive.x, upToExclusive.y), upToExclusive.z), upToExclusive.w);

		for(size_t i=0; i < maxUpto; i++)
		{
			if(m_lstVertices[i].state != svsOK) 
			{				
				if(i < upToExclusive.x)
					ctGaps.x++;
				if(i < upToExclusive.y)
					ctGaps.y++;
				if(i < upToExclusive.z)
					ctGaps.z++;
				if(i < upToExclusive.w)
					ctGaps.w++;
			}
		}
		return ctGaps;
	}

	void FindDupsFaceBody::operator ()(const tbb::blocked_range<size_t>& range) const
	{
		SCANVERTEX faceVertices[4];
		for (size_t i = range.begin(); i != range.end(); i++)
		{
			vec4ui& current = m_lstFaces[i];
			faceVertices[0] = m_lstVertices[current.x];
			faceVertices[1] = m_lstVertices[current.y];
			faceVertices[2] = m_lstVertices[current.z];
			faceVertices[3] = m_lstVertices[current.w];

			//Make corrections on Face indices

			//1.Set unique face id
			current.x	= (faceVertices[0].state == svsOK)?current.x:faceVertices[0].nLoc;
			current.y	= (faceVertices[1].state == svsOK)?current.y:faceVertices[1].nLoc;
			current.z	= (faceVertices[2].state == svsOK)?current.z:faceVertices[2].nLoc;
			current.w	= (faceVertices[3].state == svsOK)?current.w:faceVertices[3].nLoc;

			//2.Now decrease gaps in the series
			current -= countGaps(current);
		}

	}

//========================================================================================
CSubDivDX11::CSubDivDX11()
{
	init();
}

CSubDivDX11::CSubDivDX11(const CMeshVV * input)
{
	init();
	convertToWingedEdgeMesh(input, m_weMesh);			
}

CSubDivDX11::CSubDivDX11(const CWEMesh * input)
{
	init();
	setMesh(input);
}

void CSubDivDX11::setMesh(const CWEMesh* input)
{	
	//m_weMesh->cleanup();
	m_weMesh->copyFrom(*input);
}


void CSubDivDX11::init()
{
	m_shaderFP = NULL;
	m_shaderEP = NULL;
	m_shaderVP = NULL;	
	m_shaderFPEPVP = NULL;
	m_level = 0;	
	m_bCompiled = false;
	m_pOnOverallProgress = NULL;
	m_pOnStageProgress   = NULL;	

	//Create WEMesh
	m_weMesh = new CWEMesh();
}




CSubDivDX11::~CSubDivDX11()
{
	//cleanup();
	SAFE_DELETE(m_weMesh);
}

void CSubDivDX11::cleanup()
{
	m_level = 0;
	
	SAFE_DELETE(m_shaderFP);
	SAFE_DELETE(m_shaderVP);
	SAFE_DELETE(m_shaderEP);
	SAFE_DELETE(m_shaderFPEPVP);

	SAFE_DELETE(m_weMesh);	
	m_lstAllFacePoints.resize(0);
	m_lstAllEdgePoints.resize(0);
	m_lstAllVertexPoints.resize(0);
}

//Computes facepoints using Compute Shaders
size_t CSubDivDX11::computeFacePoints()
{
	if(m_weMesh == NULL)
		return INVALID_INPUT_PARAMS;
	//Clear 
	m_lstAllFacePoints.resize(0);

	//Start shader
	DXShader11* shaderFP = new DXShader11(L"catmullclarkStages.hlsl", "CSFacePoints", NULL);
	ID3D11Buffer* gpuInputVertices;
	ID3D11Buffer* gpuOutputFacePoints;
	ID3D11ShaderResourceView* srvInputVertices;
	ID3D11UnorderedAccessView* uavOutputFacePoints;

	UINT ctFaces = m_weMesh->countFaces();
	QUADFACE* vertices = new QUADFACE[ctFaces];

	DVec<vec3> lstFaceVertices;
	vec3 zero(0.0f, 0.0f, 0.0f);
	vec3 temp;

	int ctSides = m_weMesh->getFaceSides();
	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		lstFaceVertices.resize(0);
		m_weMesh->getFaceVertices(iFace, lstFaceVertices);
		
		vertices[iFace].p1 = D3DXVECTOR3(lstFaceVertices[0].x, lstFaceVertices[0].y, lstFaceVertices[0].z);
		vertices[iFace].p2 = D3DXVECTOR3(lstFaceVertices[1].x, lstFaceVertices[1].y, lstFaceVertices[1].z);
		vertices[iFace].p3 = D3DXVECTOR3(lstFaceVertices[2].x, lstFaceVertices[2].y, lstFaceVertices[2].z);
		if(ctSides == 4)			
			vertices[iFace].p4 = D3DXVECTOR3(lstFaceVertices[3].x, lstFaceVertices[3].y, lstFaceVertices[3].z);
		else
			vertices[iFace].p4 = D3DXVECTOR3(zero.x, zero.y, zero.z);

#ifdef PERFORM_CPU_GPU_COMPARISON
		//Compute for debug test
		temp = lstFaceVertices[0];
		for(int iVertex=1; iVertex < (int)lstFaceVertices.size(); iVertex++)
			temp = temp + lstFaceVertices[iVertex];
		temp = temp / static_cast<float>(ctSides);
		m_lstAllFacePoints.push_back(temp);	
#endif
	}

	

	HRESULT res;
	res = shaderFP->createStructuredBuffer(sizeof(QUADFACE), ctFaces, vertices, &gpuInputVertices);
	res = shaderFP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctFaces, NULL, &gpuOutputFacePoints);
	
	res = shaderFP->createBufferSRV(gpuInputVertices, &srvInputVertices);
	res = shaderFP->createBufferUAV(gpuOutputFacePoints, &uavOutputFacePoints);

	//Run shader
	ID3D11ShaderResourceView* views[1] = {srvInputVertices};
	shaderFP->run(1, views, NULL, NULL, 0, uavOutputFacePoints, ctFaces, 1, 1);


	//Check results
	D3D11_MAPPED_SUBRESOURCE mapped;
	ID3D11Buffer* debugBuffer = shaderFP->createAndCopyToDebugBuffer(gpuOutputFacePoints);
	m_shaderFP->map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped);
	
	D3DXVECTOR3* p = reinterpret_cast<D3DXVECTOR3*>(mapped.pData);

#ifdef PERFORM_CPU_GPU_COMPARISON
	vec3 correct;
	BOOL bSuccess = TRUE;
	
	for (UINT iFace = 0; iFace < ctFaces; iFace++ )
	{
		correct = m_lstAllFacePoints[iFace];
		if((correct.x != p[iFace].x)||(correct.y != p[iFace].y)||(correct.z != p[iFace].z))
			bSuccess = FALSE;			
	}

	if(!bSuccess)
	{
		ReportError("CS FacePoint returned invalid output.\n");		
	}
#else
	for (UINT i = 0; i < ctFaces; i++ )
		m_lstAllFacePoints.push_back(vec3(p[i].x, p[i].y, p[i].z));
#endif

		
	m_shaderFP->unMap(debugBuffer, 0);

	SAFE_RELEASE(gpuInputVertices);
	SAFE_RELEASE(gpuOutputFacePoints);
	SAFE_RELEASE(srvInputVertices);
	SAFE_RELEASE(uavOutputFacePoints);
	
	SAFE_DELETE(shaderFP);

	SAFE_DELETE_ARRAY(vertices);
	
	return m_lstAllFacePoints.size();
}

size_t CSubDivDX11::computeEdgePoints()
{
	if(m_weMesh == NULL)
		return INVALID_INPUT_PARAMS;
	if(m_lstAllFacePoints.size() == 0)
		return INVALID_INPUT_PARAMS;
	//Clear 
	m_lstAllEdgePoints.resize(0);


	//Start shader
	DXShader11* shaderEP = new DXShader11(L"catmullclarkStages.hlsl", "CSEdgePoints", NULL);
	
	//Buffers needed
	ID3D11Buffer* gpuInputVertices;
	ID3D11Buffer* gpuInputEdges;
	ID3D11Buffer* gpuInputFacePoints;	
	ID3D11Buffer* gpuOutputEdgePoints;

	//Shader ResourceView for defined buffers
	ID3D11ShaderResourceView* srvInputVertices;
	ID3D11ShaderResourceView* srvInputEdges;
	ID3D11ShaderResourceView* srvInputFacePoints;
	ID3D11UnorderedAccessView* uavOutputEdgePoints;

	//**********************************************
	UINT ctEdges    = m_weMesh->countEdges();
	UINT ctVertices = m_weMesh->countVertices();
	UINT ctFaces	= m_weMesh->countFaces();


	UINT* edges = new UINT[ctEdges*4];
	D3DXVECTOR3* vertices	= new D3DXVECTOR3[ctVertices];
	D3DXVECTOR3* facePoints = new D3DXVECTOR3[ctFaces];
	vec3 start, end, temp;
	vec3 fpLeft, fpRight;

	for(UINT iEdge=0; iEdge < ctEdges; iEdge++)
	{
		CWEMesh::EDGE e = m_weMesh->edgeAt(iEdge);
		
		edges[iEdge*4 + 0] = e.start;
		edges[iEdge*4 + 1] = e.end;
		edges[iEdge*4 + 2] = e.lface;
		edges[iEdge*4 + 3] = e.rface;		

#ifdef PERFORM_CPU_GPU_COMPARISON
		//Compute on CPU to compare with GPU results
		if(m_pOnStageProgress)
			m_pOnStageProgress(0, ctEdges-1, iEdge);

		if(m_weMesh->getEdgeEndPoints(iEdge, start, end) <= 0)
			ReportError("Couldnot fetch edge end points. Invalid Indices.");

		fpLeft  = m_lstAllFacePoints[e.lface];
		fpRight = m_lstAllFacePoints[e.rface];

		temp	= start + end + fpLeft + fpRight;
		temp	= temp / 4.0f;
		m_lstAllEdgePoints.push_back(temp);
#endif
	}

	vec3 v;
	for(UINT iVertex=0; iVertex < ctVertices; iVertex++)
	{
		v = m_weMesh->vertexAt(iVertex).pos;
		vertices[iVertex].x = v.x;
		vertices[iVertex].y = v.y;
		vertices[iVertex].z = v.z;
	}

	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		v  = m_lstAllFacePoints[iFace];
		facePoints[iFace].x = v.x;
		facePoints[iFace].y = v.y;
		facePoints[iFace].z = v.z;
	}
	
	//Transfer input buffers to GPU	
	HRESULT res;
	res = shaderEP->createStructuredBuffer(sizeof(UINT) * 4, ctEdges, edges, &gpuInputEdges);
	res = shaderEP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctVertices, vertices, &gpuInputVertices);
	res = shaderEP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctFaces, facePoints, &gpuInputFacePoints);
	res = shaderEP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctEdges, NULL, &gpuOutputEdgePoints);

	
	res = shaderEP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = shaderEP->createBufferSRV(gpuInputVertices, &srvInputVertices);
	res = shaderEP->createBufferSRV(gpuInputFacePoints, &srvInputFacePoints);
	res = shaderEP->createBufferUAV(gpuOutputEdgePoints, &uavOutputEdgePoints);

	//Run shader
	ID3D11ShaderResourceView* views[3] = {srvInputEdges, srvInputVertices, srvInputFacePoints};
	shaderEP->run(3, views, NULL, NULL, 0, uavOutputEdgePoints, ctEdges, 1, 1);

	//Check results
	D3D11_MAPPED_SUBRESOURCE mapped;
	ID3D11Buffer* debugBuffer = shaderEP->createAndCopyToDebugBuffer(gpuOutputEdgePoints);
	m_shaderEP->map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	BOOL bSuccess = TRUE;
	D3DXVECTOR3* p = reinterpret_cast<D3DXVECTOR3*>(mapped.pData);

#ifdef PERFORM_CPU_GPU_COMPARISON
	vec3 correct;
	for (UINT iEdge = 0; iEdge < ctEdges; iEdge++ )
	{
		correct = m_lstAllEdgePoints[iEdge];
		if((correct.x != p[iEdge].x)||(correct.y != p[iEdge].y)||(correct.z != p[iEdge].z))
			bSuccess = FALSE;			
	}

	if(!bSuccess)
		ReportError("CS EdgePoints returned invalid output.\n");
#else
	for (UINT i = 0; i < ctEdges; i++ )
		m_lstAllEdgePoints.push_back(vec3(p[i].x, p[i].y, p[i].z));
#endif

		
	m_shaderEP->unMap(debugBuffer, 0);

	SAFE_RELEASE(gpuInputVertices);
	SAFE_RELEASE(gpuInputEdges);
	SAFE_RELEASE(gpuInputFacePoints);
	SAFE_RELEASE(gpuOutputEdgePoints);

	SAFE_RELEASE(srvInputVertices);
	SAFE_RELEASE(srvInputEdges);
	SAFE_RELEASE(srvInputFacePoints);
	SAFE_RELEASE(uavOutputEdgePoints);
	
	SAFE_DELETE(shaderEP);

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(edges);
	SAFE_DELETE_ARRAY(facePoints);
	
	return m_lstAllEdgePoints.size();
}

size_t CSubDivDX11::computeVertexPoints()
{
	compileAllShaders();

	//Reset all internal data structures
	if(m_weMesh == NULL)
	{
		ReportError("Invalid weMesh input found.");
		return 0;
	}
	m_weMesh = new CWEMesh();	
	m_weMesh->m_pOnOverallProgress = this->m_pOnOverallProgress;
	m_weMesh->m_pOnStageProgress   = this->m_pOnStageProgress;
	m_lstAllVertexPoints.resize(0);


	//Buffers needed	
	ID3D11Buffer* gpuInputFaces;	
	ID3D11Buffer* gpuInputEdges;
	ID3D11Buffer* gpuInputVertices;
	ID3D11Buffer* gpuOutputVertexPoints;

	//Shader ResourceView for defined buffers	
	ID3D11ShaderResourceView* srvInputFaces;
	ID3D11ShaderResourceView* srvInputEdges;
	ID3D11ShaderResourceView* srvInputVertices;			
	ID3D11UnorderedAccessView* uavOutputVertexPoints;
	//**********************************************
	UINT ctFaces	= m_weMesh->countFaces();
	UINT ctEdges    = m_weMesh->countEdges();
	UINT ctVertices = m_weMesh->countVertices();
	UINT ctSides    = m_weMesh->getFaceSides();
	

	UINT* faces		 = new UINT[ctFaces*4];
	GPUEDGE* edges	 = new GPUEDGE[ctEdges];
	VERTEX* vertices = new VERTEX[ctVertices];
	
	//We always output quad faces
	//QUADFACE* outputFaces = new QUADFACE[ctFaces*ctSides];	
	vec3 start, end, temp;
	vec3 fpLeft, fpRight;

	for(UINT iEdge=0; iEdge < ctEdges; iEdge++)
	{
		CWEMesh::EDGE e = m_weMesh->edgeAt(iEdge);
		
		edges[iEdge].selr[0] = e.start;
		edges[iEdge].selr[1] = e.end;
		edges[iEdge].selr[2] = e.lface;
		edges[iEdge].selr[3] = e.rface;

		edges[iEdge].lnrn[0] = e.lnext;
		edges[iEdge].lnrn[1] = e.rnext;
	}

	CWEMesh::VERTEX v;
	for(UINT iVertex=0; iVertex < ctVertices; iVertex++)
	{
		v = m_weMesh->vertexAt(iVertex);
		vertices[iVertex].pos.x = v.pos.x;
		vertices[iVertex].pos.y = v.pos.y;
		vertices[iVertex].pos.z = v.pos.z;
		vertices[iVertex].edge  = v.edge;
	}

	//CWEMesh::FACE f;
	DVec<UINT> lstFaceIndices;
	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		lstFaceIndices.resize(0);
		m_weMesh->getFaceVertices(iFace, lstFaceIndices);
		if(lstFaceIndices.size() != ctSides)
			ReportError("Incorrect number of face vertices returned!");
		else
		{
			for(UINT i=0; i < ctSides; i++)
				faces[iFace*4 + i] = lstFaceIndices[i];
		}
	}

	//Constants
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof( CBCS );

	ID3D11Buffer* pConstBuffer = NULL;
	HRESULT res = m_shaderVP->createBuffer( &desc, NULL, &pConstBuffer);

	CBCS configSettings;
	configSettings.g_ctFaceSides = ctSides;
	configSettings.g_ctInputEdges = ctEdges;
	configSettings.g_ctInputFaces = ctFaces;
	configSettings.g_ctInputVertices = ctVertices;
	
	//Transfer input buffers to GPU		
	res = m_shaderVP->createStructuredBuffer(sizeof(UINT) * 4, ctFaces, faces, &gpuInputFaces);
	res = m_shaderVP->createStructuredBuffer(sizeof(GPUEDGE), ctEdges, edges, &gpuInputEdges);
	res = m_shaderVP->createStructuredBuffer(sizeof(VERTEX), ctVertices, vertices, &gpuInputVertices);		
	res = m_shaderVP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctVertices, NULL, &gpuOutputVertexPoints);

	res = m_shaderVP->createBufferSRV(gpuInputFaces, &srvInputFaces);
	res = m_shaderVP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = m_shaderVP->createBufferSRV(gpuInputVertices, &srvInputVertices);	
	res = m_shaderVP->createBufferUAV(gpuOutputVertexPoints, &uavOutputVertexPoints);

	// For CS constant buffer
	ID3D11ShaderResourceView* views[3] = {srvInputFaces, srvInputEdges, srvInputVertices};
	m_shaderVP->run(3, views, pConstBuffer, &configSettings, sizeof(CBCS), uavOutputVertexPoints, ctFaces, 1, 1);

	//Read FacePoints
	int ires = readFromGPU(m_shaderVP, gpuOutputVertexPoints, ctVertices, m_lstAllVertexPoints);

	SAFE_RELEASE(gpuOutputVertexPoints);
	SAFE_RELEASE(uavOutputVertexPoints);

	SAFE_RELEASE(gpuInputFaces);
	SAFE_RELEASE(gpuInputVertices);
	SAFE_RELEASE(gpuInputEdges);

	SAFE_RELEASE(srvInputFaces);
	SAFE_RELEASE(srvInputVertices);
	SAFE_RELEASE(srvInputEdges);	

	SAFE_DELETE_ARRAY(faces);
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(edges);		

	return ires;
}

void CSubDivDX11::drawComputedPoints(float pointSize /* = 4.0f */ ,
									 bool bFacePoints /* = true*/ , 
									 bool bEdgePoints /*= true*/, 
									 bool bVertexPoints /*= true*/)
{
	size_t i = 0;
	vec3 clrFP(1.0, 0.0, 0.0);
	vec3 clrEP(0.0, 1.0, 0.0);
	vec3 clrVP(0.0, 0.0, 1.0);

	/*
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPointSize(pointSize);

	if(bFacePoints && m_lstAllFacePoints.size() > 0)
	{		
		glDisable(GL_LIGHTING);
		glBegin(GL_POINTS);		
		glColor3fv(clrFP.Ptr());
		for(i=0; i < m_lstAllFacePoints.size(); i++)
		{
			glVertex3fv(m_lstAllFacePoints[i].Ptr());
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}

	if(bEdgePoints && m_lstAllEdgePoints.size() > 0)
	{		
		glDisable(GL_LIGHTING);
		glBegin(GL_POINTS);		
		glColor3fv(clrEP.Ptr());
		for(i=0; i < m_lstAllEdgePoints.size(); i++)
		{
			glVertex3fv(m_lstAllEdgePoints[i].Ptr());
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}

	if(bVertexPoints && m_lstAllVertexPoints.size() > 0)
	{
		glDisable(GL_LIGHTING);
		glBegin(GL_POINTS);		
		glColor3fv(clrVP.Ptr());
		for(i=0; i < m_lstAllVertexPoints.size(); i++)
		{
			glVertex3fv(m_lstAllVertexPoints[i].Ptr());
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}

	glPopAttrib();
	*/
}

bool CSubDivDX11::compileAllShaders()
{
	if(m_bCompiled)
		return true;
	
	//Start shader
	m_shaderFPEPVP = new DXShader11();
	DWideStr wstrPath = PS::findShaderProgram(DAnsiStr("catmullclarkComplete.hlsl"));

	m_bCompiled = m_shaderFPEPVP->tryLoadBinThenCompile(wstrPath.ptr(), "CSMain", NULL);
	if(!m_bCompiled)
	{
		ReportError("Unable to compile shader code!");
		SAFE_DELETE(m_shaderFP);
		SAFE_DELETE(m_shaderEP);
		SAFE_DELETE(m_shaderVP);
		SAFE_DELETE(m_shaderFPEPVP);
		return false;
	}		

	return true;
}

//Worked Perfectly
bool CSubDivDX11::runOneShot()
{
	if(!compileAllShaders())
	{
		CErrorManager::GetInstance().FlushErrors();
		return false;
	}

	//Reset all internal data structures
	if(m_weMesh == NULL)
	{
		ReportError("Invalid weMesh input found.");
		return false;
	}

	m_weMesh->m_pOnOverallProgress = this->m_pOnOverallProgress;
	m_weMesh->m_pOnStageProgress   = this->m_pOnStageProgress;
	m_lstAllFacePoints.resize(0);
	m_lstAllEdgePoints.resize(0);	
	m_lstAllVertexPoints.resize(0);


	//Buffers needed	
	ID3D11Buffer* gpuInputFaces;	
	ID3D11Buffer* gpuInputEdges;
	ID3D11Buffer* gpuInputVertices;
	ID3D11Buffer* gpuOutputPoints;

	//Shader ResourceView for defined buffers	
	ID3D11ShaderResourceView* srvInputFaces;
	ID3D11ShaderResourceView* srvInputEdges;
	ID3D11ShaderResourceView* srvInputVertices;			
	ID3D11UnorderedAccessView* uavOutputPoints;
	//**********************************************
	UINT ctFaces	= m_weMesh->countFaces();
	UINT ctEdges    = m_weMesh->countEdges();
	UINT ctVertices = m_weMesh->countVertices();
	UINT ctSides    = m_weMesh->getFaceSides();
	if(ctSides == 3)
		ctSides = ctSides;
	

	UINT* faces		 = new UINT[ctFaces*4];
	GPUEDGE* edges	 = new GPUEDGE[ctEdges];
	VERTEX* vertices = new VERTEX[ctVertices];
	
	//We always output quad faces
	//QUADFACE* outputFaces = new QUADFACE[ctFaces*ctSides];	
	vec3 start, end, temp;
	vec3 fpLeft, fpRight;

	for(UINT iEdge=0; iEdge < ctEdges; iEdge++)
	{
		CWEMesh::EDGE e = m_weMesh->edgeAt(iEdge);
		
		edges[iEdge].selr[0] = e.start;
		edges[iEdge].selr[1] = e.end;
		edges[iEdge].selr[2] = e.lface;
		edges[iEdge].selr[3] = e.rface;

		edges[iEdge].lnrn[0] = e.lnext;
		edges[iEdge].lnrn[1] = e.rnext;
	}

	CWEMesh::VERTEX v;
	for(UINT iVertex=0; iVertex < ctVertices; iVertex++)
	{
		v = m_weMesh->vertexAt(iVertex);
		vertices[iVertex].pos.x = v.pos.x;
		vertices[iVertex].pos.y = v.pos.y;
		vertices[iVertex].pos.z = v.pos.z;
		vertices[iVertex].edge  = v.edge;
	}

	//CWEMesh::FACE f;
	DVec<UINT> lstFaceIndices;
	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		lstFaceIndices.resize(0);
		m_weMesh->getFaceVertices(iFace, lstFaceIndices);
		if(lstFaceIndices.size() != ctSides)
			ReportError("Incorrect number of face vertices returned!");
		else
		{
			for(UINT i=0; i < ctSides; i++)
				faces[iFace*4 + i] = lstFaceIndices[i];
		}
	}

	//Constants
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof( CBCS );

	ID3D11Buffer* pConstBuffer = NULL;
	HRESULT res = m_shaderFPEPVP->createBuffer( &desc, NULL, &pConstBuffer);

	CBCS configSettings;
	configSettings.g_ctFaceSides = ctSides;
	configSettings.g_ctInputEdges = ctEdges;
	configSettings.g_ctInputFaces = ctFaces;
	configSettings.g_ctInputVertices = ctVertices;		
	//***********************************************************************
	//FP
	//Transfer input buffers to GPU		

	UINT ctOutput = max(max(ctFaces, ctEdges), ctVertices);
	res = m_shaderFPEPVP->createStructuredBuffer(sizeof(UINT) * 4, ctFaces, faces, &gpuInputFaces);
	res = m_shaderFPEPVP->createStructuredBuffer(sizeof(GPUEDGE), ctEdges, edges, &gpuInputEdges);
	res = m_shaderFPEPVP->createStructuredBuffer(sizeof(VERTEX), ctVertices, vertices, &gpuInputVertices);		
	res = m_shaderFPEPVP->createStructuredBuffer(sizeof(FPEPVP), ctOutput, NULL, &gpuOutputPoints);

	res = m_shaderFPEPVP->createBufferSRV(gpuInputFaces, &srvInputFaces);
	res = m_shaderFPEPVP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = m_shaderFPEPVP->createBufferSRV(gpuInputVertices, &srvInputVertices);	
	res = m_shaderFPEPVP->createBufferUAV(gpuOutputPoints, &uavOutputPoints);

	// For CS constant buffer	
	ID3D11ShaderResourceView* views[3] = {srvInputFaces, srvInputEdges, srvInputVertices};

	//Running compute shader
	vec3ui ctThreadGroups(ctFaces, 1, 1);
	if(ctThreadGroups.x > 65535)
	{
		ctThreadGroups.y = ctThreadGroups.x / 32768;
		if(ctThreadGroups.x % 32768 > 0)
			ctThreadGroups.y++;
		ctThreadGroups.x = 32768;		
	}

	m_shaderFPEPVP->run(3, views, pConstBuffer, &configSettings, sizeof(CBCS), uavOutputPoints, ctThreadGroups.x, ctThreadGroups.y, ctThreadGroups.z);

	//Read FacePoints
	D3D11_MAPPED_SUBRESOURCE mapped;
	ID3D11Buffer* debugBuffer = m_shaderFPEPVP->createAndCopyToDebugBuffer(gpuOutputPoints);
	res = m_shaderFPEPVP->map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped);	

	FPEPVP* p = reinterpret_cast<FPEPVP*>(mapped.pData);

	for (UINT i = 0; i < ctOutput; i++ )
	{
		if(i < ctFaces)
			m_lstAllFacePoints.push_back(vec3(p[i].fp.x, p[i].fp.y, p[i].fp.z));
		if(i < ctEdges)
			m_lstAllEdgePoints.push_back(vec3(p[i].ep.x, p[i].ep.y, p[i].ep.z));
		if(i < ctVertices)
			m_lstAllVertexPoints.push_back(vec3(p[i].vp.x, p[i].vp.y, p[i].vp.z));	
	}

	m_shaderFPEPVP->unMap(debugBuffer, 0);

	//Clean
	SAFE_RELEASE(gpuInputFaces);
	SAFE_RELEASE(gpuInputEdges);
	SAFE_RELEASE(gpuInputVertices);
	
	SAFE_RELEASE(srvInputFaces);
	SAFE_RELEASE(srvInputEdges);	
	SAFE_RELEASE(srvInputVertices);
	
	SAFE_RELEASE(gpuOutputPoints);
	SAFE_RELEASE(uavOutputPoints);
	//***********************************************************************
	SAFE_DELETE_ARRAY(faces);
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(edges);		

	m_level++;

	createFinalMesh();

	return true;
}


//====================================================================================================
// Running subdivision code in parallel across multiple cores with TBB.
// This version uses our debug shader code.
//====================================================================================================
bool CSubDivDX11::runOneShotTBB()
{
	QuickPerformanceLog;

	//Reset all internal data structures
	if(m_weMesh == NULL)
	{
		ReportError("Invalid weMesh input found.");
		return false;
	}

	int ctErrors;
	if(!m_weMesh->performCompleteTest(&ctErrors))
	{
		ReportError("Couldnot fix the mesh!");
		return false;
	}


	m_weMesh->m_pOnOverallProgress = this->m_pOnOverallProgress;
	m_weMesh->m_pOnStageProgress   = this->m_pOnStageProgress;
	m_lstAllFacePoints.resize(0);
	m_lstAllEdgePoints.resize(0);	
	m_lstAllVertexPoints.resize(0);
	
	//**********************************************
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("TBB SubDiv: Making input buffers for TBB processing", 20.0f);

	UINT ctFaces	= m_weMesh->countFaces();
	UINT ctEdges    = m_weMesh->countEdges();
	UINT ctVertices = m_weMesh->countVertices();
	UINT ctSides    = m_weMesh->getFaceSides();
	if(ctSides == 3)
		ctSides = ctSides;
	

	DEBUGFACE* faces = new DEBUGFACE[ctFaces];
	GPUEDGE* edges	 = new GPUEDGE[ctEdges];
	VERTEX* vertices = new VERTEX[ctVertices];
		
	//We always output quad faces
	//QUADFACE* outputFaces = new QUADFACE[ctFaces*ctSides];	
	vec3 start, end, temp;
	vec3 fpLeft, fpRight;

	for(UINT iEdge=0; iEdge < ctEdges; iEdge++)
	{
		CWEMesh::EDGE e = m_weMesh->edgeAt(iEdge);
		
		edges[iEdge].selr[0] = e.start;
		edges[iEdge].selr[1] = e.end;
		edges[iEdge].selr[2] = e.lface;
		edges[iEdge].selr[3] = e.rface;

		edges[iEdge].lnrn[0] = e.lnext;
		edges[iEdge].lnrn[1] = e.rnext;
	}

	CWEMesh::VERTEX v;
	for(UINT iVertex=0; iVertex < ctVertices; iVertex++)
	{
		v = m_weMesh->vertexAt(iVertex);
		vertices[iVertex].pos.x = v.pos.x;
		vertices[iVertex].pos.y = v.pos.y;
		vertices[iVertex].pos.z = v.pos.z;
		vertices[iVertex].edge  = v.edge;
	}

	

	//Write Faces
	size_t arrFaceVertices[MAX_FACE_SIDES];
	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		m_weMesh->getFaceVertices(iFace, arrFaceVertices, MAX_FACE_SIDES);
		faces[iFace].x = arrFaceVertices[0];
		faces[iFace].y = arrFaceVertices[1];
		faces[iFace].z = arrFaceVertices[2];
		if(ctSides > 3)
			faces[iFace].w = arrFaceVertices[3];

	}

	//Constants
	CBCS configSettings;
	configSettings.g_ctFaceSides = ctSides;
	configSettings.g_ctInputEdges = ctEdges;
	configSettings.g_ctInputFaces = ctFaces;
	configSettings.g_ctInputVertices = ctVertices;		
	//***********************************************************************
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("TBB SubDiv: Calling TBB Parallel_For", 40.0f);

	UINT ctOutput = max(max(ctFaces, ctEdges), ctVertices);
	FPEPVP* output = new FPEPVP[ctOutput];


	CProcessFaceBody body(configSettings, faces, edges, vertices, output);
	tbb::parallel_for(tbb::blocked_range<UINT>(0, ctOutput), body, tbb::auto_partitioner());
	//body.processAll(ctOutput);

	//************************************************************************
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("TBB SubDiv: Reading output", 60.0f);

	FPEPVP* p = reinterpret_cast<FPEPVP*>(output);
	m_lstAllEdgePoints.resize(0);

	for (UINT i = 0; i < ctOutput; i++ )
	{
		if(i < ctFaces)
			m_lstAllFacePoints.push_back(vec3(p[i].fp.x, p[i].fp.y, p[i].fp.z));
		if(i < ctEdges)
			m_lstAllEdgePoints.push_back(vec3(p[i].ep.x, p[i].ep.y, p[i].ep.z));
		if(i < ctVertices)
			m_lstAllVertexPoints.push_back(vec3(p[i].vp.x, p[i].vp.y, p[i].vp.z));	
	}
	//***********************************************************************
	SAFE_DELETE_ARRAY(faces);
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(edges);		
	SAFE_DELETE_ARRAY(output);		

	m_level++;

	//************************************************************************
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("TBB SubDiv: Creating final output WEMesh.", 80.0f);

	createFinalMesh();
	//createFinalMeshInPlace();

	//Show all errors
	CErrorManager::GetInstance().PopMostRecentThenCleanup();
	//************************************************************************
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("TBB SubDiv: Complete.", 100.0f);

	return true;
}

bool CSubDivDX11::runStageByStage()
{
	compileAllShaders();

	//Reset all internal data structures
	if(m_weMesh == NULL)
	{
		ReportError("Invalid weMesh input found.");
		return false;
	}
	m_weMesh->m_pOnOverallProgress = this->m_pOnOverallProgress;
	m_weMesh->m_pOnStageProgress   = this->m_pOnStageProgress;
	m_lstAllFacePoints.resize(0);
	m_lstAllEdgePoints.resize(0);	
	m_lstAllVertexPoints.resize(0);


	//Buffers needed	
	ID3D11Buffer* gpuInputFaces;	
	ID3D11Buffer* gpuInputEdges;
	ID3D11Buffer* gpuInputVertices;
	ID3D11Buffer* gpuOutputPoints;



	//Shader ResourceView for defined buffers	
	ID3D11ShaderResourceView* srvInputFaces;
	ID3D11ShaderResourceView* srvInputEdges;
	ID3D11ShaderResourceView* srvInputVertices;			
	ID3D11UnorderedAccessView* uavOutputPoints;
	//**********************************************
	UINT ctFaces	= m_weMesh->countFaces();
	UINT ctEdges    = m_weMesh->countEdges();
	UINT ctVertices = m_weMesh->countVertices();
	UINT ctSides    = m_weMesh->getFaceSides();
	

	UINT* faces		 = new UINT[ctFaces*4];
	GPUEDGE* edges	 = new GPUEDGE[ctEdges];
	VERTEX* vertices = new VERTEX[ctVertices];
	
	//We always output quad faces
	//QUADFACE* outputFaces = new QUADFACE[ctFaces*ctSides];	
	vec3 start, end, temp;
	vec3 fpLeft, fpRight;

	for(UINT iEdge=0; iEdge < ctEdges; iEdge++)
	{
		CWEMesh::EDGE e = m_weMesh->edgeAt(iEdge);
		
		edges[iEdge].selr[0] = e.start;
		edges[iEdge].selr[1] = e.end;
		edges[iEdge].selr[2] = e.lface;
		edges[iEdge].selr[3] = e.rface;

		edges[iEdge].lnrn[0] = e.lnext;
		edges[iEdge].lnrn[1] = e.rnext;
	}

	CWEMesh::VERTEX v;
	for(UINT iVertex=0; iVertex < ctVertices; iVertex++)
	{
		v = m_weMesh->vertexAt(iVertex);
		vertices[iVertex].pos.x = v.pos.x;
		vertices[iVertex].pos.y = v.pos.y;
		vertices[iVertex].pos.z = v.pos.z;
		vertices[iVertex].edge  = v.edge;
	}

	//CWEMesh::FACE f;
	DVec<UINT> lstFaceIndices;
	for(UINT iFace=0; iFace < ctFaces; iFace++)
	{
		lstFaceIndices.resize(0);
		m_weMesh->getFaceVertices(iFace, lstFaceIndices);
		if(lstFaceIndices.size() != ctSides)
			ReportError("Incorrect number of face vertices returned!");
		else
		{
			for(UINT i=0; i < ctSides; i++)
				faces[iFace*4 + i] = lstFaceIndices[i];
		}
	}

	//Constants
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof( CBCS );

	ID3D11Buffer* pConstBuffer = NULL;
	HRESULT res =m_shaderFP->createBuffer( &desc, NULL, &pConstBuffer);

	CBCS configSettings;
	configSettings.g_ctFaceSides = ctSides;
	configSettings.g_ctInputEdges = ctEdges;
	configSettings.g_ctInputFaces = ctFaces;
	configSettings.g_ctInputVertices = ctVertices;

	int ires;
	
	//***********************************************************************
	//FP
	//Transfer input buffers to GPU		
	res = m_shaderFP->createStructuredBuffer(sizeof(UINT) * 4, ctFaces, faces, &gpuInputFaces);
	res = m_shaderFP->createStructuredBuffer(sizeof(GPUEDGE), ctEdges, edges, &gpuInputEdges);
	res = m_shaderFP->createStructuredBuffer(sizeof(VERTEX), ctVertices, vertices, &gpuInputVertices);		
	res = m_shaderFP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctFaces, NULL, &gpuOutputPoints);

	res = m_shaderFP->createBufferSRV(gpuInputFaces, &srvInputFaces);
	res = m_shaderFP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = m_shaderFP->createBufferSRV(gpuInputVertices, &srvInputVertices);	
	res = m_shaderFP->createBufferUAV(gpuOutputPoints, &uavOutputPoints);

	// For CS constant buffer	
	ID3D11ShaderResourceView* views[3] = {srvInputFaces, srvInputEdges, srvInputVertices};
	m_shaderFP->run(3, views, pConstBuffer, &configSettings, sizeof(CBCS), uavOutputPoints, ctFaces, 1, 1);

	//Read FacePoints
	ires = readFromGPU(m_shaderFP, gpuOutputPoints, ctFaces, m_lstAllFacePoints);

	//Clean
	SAFE_RELEASE(gpuInputFaces);
	SAFE_RELEASE(gpuInputEdges);
	SAFE_RELEASE(gpuInputVertices);
	
	SAFE_RELEASE(srvInputFaces);
	SAFE_RELEASE(srvInputEdges);	
	SAFE_RELEASE(srvInputVertices);
	
	SAFE_RELEASE(gpuOutputPoints);
	SAFE_RELEASE(uavOutputPoints);
	//***********************************************************************
	//EP
	res = m_shaderEP->createStructuredBuffer(sizeof(UINT) * 4, ctFaces, faces, &gpuInputFaces);
	res = m_shaderEP->createStructuredBuffer(sizeof(GPUEDGE), ctEdges, edges, &gpuInputEdges);
	res = m_shaderEP->createStructuredBuffer(sizeof(VERTEX), ctVertices, vertices, &gpuInputVertices);		
	res = m_shaderEP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctEdges, NULL, &gpuOutputPoints);

	res = m_shaderEP->createBufferSRV(gpuInputFaces, &srvInputFaces);
	res = m_shaderEP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = m_shaderEP->createBufferSRV(gpuInputVertices, &srvInputVertices);	
	res = m_shaderEP->createBufferUAV(gpuOutputPoints, &uavOutputPoints);

	// For CS constant buffer	
	m_shaderEP->run(3, views, pConstBuffer, &configSettings, sizeof(CBCS), uavOutputPoints, ctEdges, 1, 1);

	//Read FacePoints
	ires = readFromGPU(m_shaderEP, gpuOutputPoints, ctEdges, m_lstAllEdgePoints);

	//Clean
	SAFE_RELEASE(gpuInputFaces);
	SAFE_RELEASE(gpuInputEdges);
	SAFE_RELEASE(gpuInputVertices);
	
	SAFE_RELEASE(srvInputFaces);
	SAFE_RELEASE(srvInputEdges);	
	SAFE_RELEASE(srvInputVertices);
	
	SAFE_RELEASE(gpuOutputPoints);
	SAFE_RELEASE(uavOutputPoints);


	//***********************************************************************
	//VP
	res = m_shaderVP->createStructuredBuffer(sizeof(UINT) * 4, ctFaces, faces, &gpuInputFaces);
	res = m_shaderVP->createStructuredBuffer(sizeof(GPUEDGE), ctEdges, edges, &gpuInputEdges);
	res = m_shaderVP->createStructuredBuffer(sizeof(VERTEX), ctVertices, vertices, &gpuInputVertices);		
	res = m_shaderVP->createStructuredBuffer(sizeof(D3DXVECTOR3), ctVertices, NULL, &gpuOutputPoints);

	res = m_shaderVP->createBufferSRV(gpuInputFaces, &srvInputFaces);
	res = m_shaderVP->createBufferSRV(gpuInputEdges, &srvInputEdges);
	res = m_shaderVP->createBufferSRV(gpuInputVertices, &srvInputVertices);	
	res = m_shaderVP->createBufferUAV(gpuOutputPoints, &uavOutputPoints);

	// For CS constant buffer	
	m_shaderVP->run(3, views, pConstBuffer, &configSettings, sizeof(CBCS), uavOutputPoints, ctVertices, 1, 1);

	//Read FacePoints
	ires = readFromGPU(m_shaderVP, gpuOutputPoints, ctVertices, m_lstAllVertexPoints);

	//Clean
	SAFE_RELEASE(gpuInputFaces);
	SAFE_RELEASE(gpuInputVertices);
	SAFE_RELEASE(gpuInputEdges);

	SAFE_RELEASE(srvInputFaces);
	SAFE_RELEASE(srvInputVertices);
	SAFE_RELEASE(srvInputEdges);	

	SAFE_RELEASE(gpuOutputPoints);
	SAFE_RELEASE(uavOutputPoints);
	
	SAFE_DELETE_ARRAY(faces);
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(edges);		

	m_level++;


	return true;
}

size_t CSubDivDX11::countDuplicateVertices(const CWEMesh* lpInMesh)
{
	size_t ctDups = 0;
	if(lpInMesh == NULL)
		return ctDups;

	size_t ctVertices = lpInMesh->countVertices();
	SCANVERTEX* arrScanVertices = new SCANVERTEX[ctVertices];
	CWEMesh::VERTEX v;
	for(size_t i=0; i < ctVertices; i++)
	{
		v = lpInMesh->vertexAt(i);
		arrScanVertices[i].pos = v.pos;
		arrScanVertices[i].state = svsNotVisited;
	}

	//Scan all
	FindDupsVertexWithTBBTaskManager FindDups(arrScanVertices, ctVertices);
	FindDups.processAll();

	//Now count all dups
	size_t ctNotVisited = 0;
	for(size_t i=0; i < ctVertices; i++)
	{
		if(arrScanVertices[i].state == svsDuplicate)
			ctDups++;
		else
		{
			if(arrScanVertices[i].state == svsNotVisited)
				ctNotVisited++;
		}
	}
	SAFE_DELETE_ARRAY(arrScanVertices);
	DASSERT(ctNotVisited == 0);
	

	return ctDups;
}

//Not complete yet
bool CSubDivDX11::createFinalMeshInPlace()
{
	size_t ctFaces = m_weMesh->countFaces();

	size_t arrFaceVertexPointIndices[MAX_FACE_SIDES];
	vec3 arrFaceVertexPoints[MAX_FACE_SIDES];

	size_t arrFaceEdgePointIndices[MAX_FACE_SIDES];
	vec3 arrFaceEdgePoints[MAX_FACE_SIDES];

	int ctVertices;
	int ctEdges;
	int iVertex, iEdge;
	//size_t idxEffective; 

	vec3 start;
	vec3 ep1, ep2;
	vec3 fp;

	DVec<CWEMesh::FACE> lstNewFaces;
	//Each face is getting replaced by 4 new faces
	//Update VertexPoints
	//Update EdgePoints 
	//Update FacePoints
	for(size_t iFace = 0; iFace < ctFaces; iFace++)
	{
		if(m_pOnStageProgress)
			m_pOnStageProgress(0, ctFaces-1, iFace);

		//Get FacePoints for this face
		fp = m_lstAllFacePoints[iFace];

		//Get All Required vertexPoints
		ctVertices = m_weMesh->getFaceVertices(iFace, arrFaceVertexPointIndices, MAX_FACE_SIDES);				
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
			arrFaceVertexPoints[iVertex] = m_lstAllVertexPoints[arrFaceVertexPointIndices[iVertex]];

		//Get All Required edgePoints
		ctEdges	   = m_weMesh->getFaceEdges(iFace, arrFaceEdgePointIndices, MAX_FACE_SIDES);
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
			arrFaceEdgePoints[iEdge] = m_lstAllEdgePoints[arrFaceEdgePointIndices[iEdge]];

		//Update vertex points
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
		{			
			m_weMesh->setVertex(arrFaceVertexPointIndices[iVertex], arrFaceVertexPoints[iVertex]);	
		}

		//Update EdgePoints
		//Get All Required vertexPoints
		//Get All Required edgePoints
		ctEdges	   = m_weMesh->getFaceEdges(iFace, arrFaceEdgePointIndices, MAX_FACE_SIDES);
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
			arrFaceEdgePoints[iEdge] = m_lstAllEdgePoints[arrFaceEdgePointIndices[iEdge]];

		//Add all new vertices
		size_t idxFP = m_weMesh->addVertex(fp);

		CWEMesh::FACE f = m_weMesh->faceAt(iFace);
		CWEMesh::EDGE e1; 
		CWEMesh::EDGE e2; 
		CWEMesh::EDGE eInside; 
		

		size_t arrInsideEdgesIndices[MAX_FACE_SIDES];
		vec2i arrOutsideEdgesIndices[MAX_FACE_SIDES];			
		size_t arrGenFaces[MAX_FACE_SIDES];


		//Add and update edges with no relationships updates
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
		{			
			e1 = m_weMesh->edgeAt(f.edges[iEdge]);
			e2		= e1;  
			eInside = e1;
			
			//Add new edgepoint
			ep1 = arrFaceEdgePoints[iEdge];
			arrFaceEdgePointIndices[iEdge] = m_weMesh->addVertex(ep1);
							
			//OuterEdges=========================================
			//Update edge e1
			e1.end = arrFaceEdgePointIndices[iEdge];
			m_weMesh->setEdge(f.edges[iEdge], e1);

			//Update edge e2
			e2.start = arrFaceEdgePointIndices[iEdge];
			arrOutsideEdgesIndices[iEdge].x = f.edges[iEdge];
			arrOutsideEdgesIndices[iEdge].y = m_weMesh->addEdge(e2);
			
			//InnerEdge===========================================
			//update edge e3			
			eInside.start = arrFaceEdgePointIndices[iEdge];
			eInside.end   = idxFP;		
			arrInsideEdgesIndices[iEdge] = m_weMesh->addEdge(eInside);
		}


		//Now add 4 new faces to Temporary list of faces
		CWEMesh::FACE genFace;		
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
		{
			genFace.edges[0] = arrOutsideEdgesIndices[iEdge].x;
			genFace.edges[1] = arrInsideEdgesIndices[iEdge];
			if(iEdge == 0)
			{
				genFace.edges[2] = arrInsideEdgesIndices[ctEdges-1];
				genFace.edges[3] = arrOutsideEdgesIndices[ctEdges-1].y;
			}
			else
			{
				genFace.edges[2] = arrInsideEdgesIndices[iEdge-1];
				genFace.edges[3] = arrOutsideEdgesIndices[iEdge-1].y;
			}
			
			arrGenFaces[iEdge] = lstNewFaces.size();
			lstNewFaces.push_back(genFace);			
		}
		
		//Update Edges Relationships
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
		{
			e1 = m_weMesh->edgeAt(arrOutsideEdgesIndices[iEdge].x);
			e2 = m_weMesh->edgeAt(arrOutsideEdgesIndices[iEdge].y);
			eInside = m_weMesh->edgeAt(arrInsideEdgesIndices[iEdge]);
	
			e1.rface = arrGenFaces[iEdge];
			
		}

	} 

	return true;
}

bool CSubDivDX11::createFinalMesh()
{
	//Step Four
	//Create a new mesh 
	//Each quad is being divided at face point creating 4 quads
	//Previous mesh vertices are moved at step 3 so we use them now
	//We use face-point, edge-point and vertex-points for the new mesh	
	parsProfilerStart(1);

	int ctFaces = m_weMesh->countFaces();
	
	vec3 start;
	vec3 ep1, ep2;
	vec3 fp;
	int iFace, iVertex, iEdge;
	int ctVertices;
	int ctEdges;

	size_t arrFaceVertexPointIndices[MAX_FACE_SIDES];
	vec3 arrFaceVertexPoints[MAX_FACE_SIDES];


	size_t arrFaceEdgePointIndices[MAX_FACE_SIDES];
	vec3 arrFaceEdgePoints[MAX_FACE_SIDES];

	//CWEMesh subMesh;
	//subMesh.setFaceSides(4);


	//Allocate memory
	SCANVERTEX* arrScanVertices = new SCANVERTEX[MAX_FACE_SIDES*MAX_FACE_SIDES*ctFaces];
	vec4ui* arrScanFaces = new vec4ui[MAX_FACE_SIDES*ctFaces];
	size_t ctScannedVertices = 0;
	size_t ctScannedFaces = 0;

	//For each face in the original mesh we add 4 faces
	for(iFace = 0; iFace < ctFaces; iFace++)
	{
		if(m_pOnStageProgress)
			m_pOnStageProgress(0, ctFaces-1, iFace);
		
		
		//Get All Required vertexPoints
		ctVertices = m_weMesh->getFaceVertices(iFace, arrFaceVertexPointIndices, MAX_FACE_SIDES);				
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
			arrFaceVertexPoints[iVertex] = m_lstAllVertexPoints[arrFaceVertexPointIndices[iVertex]];

		//Get All Required edgePoints
		ctEdges	   = m_weMesh->getFaceEdges(iFace, arrFaceEdgePointIndices, MAX_FACE_SIDES);
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
			arrFaceEdgePoints[iEdge] = m_lstAllEdgePoints[arrFaceEdgePointIndices[iEdge]];

		//For each vertex we will create a quad
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
		{		
			start   = arrFaceVertexPoints[iVertex];
			ep1		= arrFaceEdgePoints[iVertex];
			fp		= m_lstAllFacePoints[iFace];

			
			if(iVertex == 0)			
				ep2 = arrFaceEdgePoints[ctVertices-1];
			else
				ep2 = arrFaceEdgePoints[iVertex-1];


			//=================================================
			vec4ui face;
			arrScanVertices[ctScannedVertices].pos   = start;
			arrScanVertices[ctScannedVertices].state = svsNotVisited;
			face.x = ctScannedVertices;
			ctScannedVertices++;

			arrScanVertices[ctScannedVertices].pos   = ep1;
			arrScanVertices[ctScannedVertices].state = svsNotVisited;
			face.y = ctScannedVertices;
			ctScannedVertices++;

			arrScanVertices[ctScannedVertices].pos   = fp;
			arrScanVertices[ctScannedVertices].state = svsNotVisited;
			face.z = ctScannedVertices;
			ctScannedVertices++;
			
			arrScanVertices[ctScannedVertices].pos   = ep2;
			arrScanVertices[ctScannedVertices].state = svsNotVisited;
			face.w = ctScannedVertices;
			ctScannedVertices++;

			arrScanFaces[ctScannedFaces] = face;
			ctScannedFaces++;
			//Adds a quad face by directly adding its 4 corner points
			//subMesh.addQuadFace(start, ep1, fp, ep2);
		}
	} 

	//=======================================================
	//Now we create a VVMesh with TBB and our new algorithm		
	//Parallel Dups Marker using TaskManager
	//The constructor will default to number of available cores for window size param
	FindDupsVertexWithTBBTaskManager FindDups(arrScanVertices, ctScannedVertices);
	FindDups.processAll();
	
	//Now for each face we need to access unique vertices in the vertex list
	//We do this in parallel using TBB
	FindDupsFaceBody FaceBody(arrScanVertices, arrScanFaces, ctScannedVertices, ctScannedFaces);
	tbb::parallel_for(tbb::blocked_range<size_t>(0, ctScannedFaces), FaceBody, tbb::auto_partitioner());
	//=======================================================

	bool bres;		
	{		
		CMeshVV vvMesh;
		//bres = convertToVVMesh(&subMesh, &vvMesh);
		vvMesh.setModeByFaceSides(4);
		vvMesh.setUnitVertexSize(3);

		size_t ctPacked = 0;
		size_t ctDups = 0;
		for(size_t iVertex = 0; iVertex < ctScannedVertices; iVertex++)
		{
			if(arrScanVertices[iVertex].state == svsOK)
			{
				vvMesh.addVertex(arrScanVertices[iVertex].pos);
				ctPacked++;
			}
			else
				ctDups++;
		}

		for(size_t iFace = 0; iFace < ctScannedFaces; iFace++)
		{			
			vec4ui face = arrScanFaces[iFace];
			//if((face.x >= ctPacked)||(face.y >= ctPacked)||(face.z >= ctPacked)||(face.w >= ctPacked))
			//	face = face;
			vvMesh.addQuad(face.x, face.y, face.z, face.w);
		}

		bres = convertToWingedEdgeMesh(&vvMesh, m_weMesh);
	}
		
	//Cleanup
	SAFE_DELETE_ARRAY(arrScanFaces);
	SAFE_DELETE_ARRAY(arrScanVertices);

	//size_t ctDups = countDuplicateVertices(m_weMesh);
	parsProfilerEnd(1);		
	double t = parsProfilerTime(1);	
	DAnsiStr strMsg = PS::printToAStr("Profiler>>Create Final WEMesh: %f milisec, SubD Level: %d, WEMesh F#: %d, V#: %d\n", t * 1000.0, m_level, m_weMesh->countFaces(), m_weMesh->countVertices());
	OutputDebugStringA(strMsg.ptr());

	return true;
}


int CSubDivDX11::readFromGPU(PS::DXShader11* shader, ID3D11Buffer* gpuOutputBuffer, UINT count, DVec<vec3>& lstToStore)
{
	if((shader == NULL)||(gpuOutputBuffer == NULL))
		return -1;

	lstToStore.resize(0);	
	HRESULT res;
	BOOL bSuccess = TRUE;
	D3D11_MAPPED_SUBRESOURCE mapped;
	ID3D11Buffer* debugBuffer = shader->createAndCopyToDebugBuffer(gpuOutputBuffer);
	res = shader->map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped);	
	if(res != S_OK)
		return -2;

	vec3* p = reinterpret_cast<vec3*>(mapped.pData);

	for (UINT i = 0; i < count; i++ )
	{
		lstToStore.push_back(vec3(p[i].x, p[i].y, p[i].z));
	}

	shader->unMap(debugBuffer, 0);
	return 1;
}

bool isSameEdge(CSubDivDX11::GPUEDGE a, CSubDivDX11::GPUEDGE b)
{
	return ((a.selr[0] == b.selr[0] && a.selr[1] == b.selr[1])||
			(a.selr[0] == b.selr[1] && a.selr[1] == b.selr[0]));
/*
	return ((a.lnrn[0] == b.lnrn[0] && a.lnrn[1] == b.lnrn[1])&&
			(a.selr[0] == b.selr[0] && a.selr[1] == b.selr[1])&&
			(a.selr[2] == b.selr[2] && a.selr[3] == b.selr[3]));
		*/
}

bool CSubDivDX11::run()
{
	m_level++;

	//Reset all internal data structures
	if(m_weMesh == NULL)
	{
		ReportError("Invalid weMesh input found.");
		return false;
	}
	m_weMesh->m_pOnOverallProgress = this->m_pOnOverallProgress;
	m_weMesh->m_pOnStageProgress   = this->m_pOnStageProgress;
	m_lstAllFacePoints.resize(0);
	m_lstAllEdgePoints.resize(0);	
	m_lstAllVertexPoints.resize(0);

	//Step one
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("Computing FacePoints", 10.0f);
	int ctFP = computeFacePoints();

	//Step Two
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("Computing EdgePoints", 30.0f);
	int ctEP = computeEdgePoints();

	//Step Three
	if(m_pOnOverallProgress)
		m_pOnOverallProgress("Computing VertexPoints", 60.0f);
	int ctVP = computeVertexPoints();

	//Step Four
	//Create a new mesh 
	//Each quad is being divided at face point creating 4 quads
	//Previous mesh vertices are moved at step 3 so we use them now
	//We use face-point, edge-point and vertex-points for the new mesh
	CWEMesh subMesh;
	int ctFaces = m_weMesh->countFaces();
	
	vec3 start;
	vec3 ep1, ep2;
	vec3 fp;
	int iFace, iVertex, iEdge;
	int ctVertices;
	int ctEdges;
	DVec<unsigned int> lstFaceVertexPointIndices;
	DVec<vec3> lstFaceVertexPoints;

	DVec<unsigned int> lstFaceEdgePointIndices;
	DVec<vec3> lstFaceEdgePoints;

	if(m_pOnOverallProgress)
		m_pOnOverallProgress("Creating new mesh", 80.0f);

	for(iFace = 0; iFace < ctFaces; iFace++)
	{
		if(m_pOnStageProgress)
			m_pOnStageProgress(0, ctFaces-1, iFace);

		lstFaceVertexPointIndices.resize(0);
		lstFaceVertexPoints.resize(0);

		lstFaceEdgePointIndices.resize(0);
		lstFaceEdgePoints.resize(0);
		
		//Get All Required vertexPoints
		ctVertices = m_weMesh->getFaceVertices(iFace, lstFaceVertexPointIndices);				
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
			lstFaceVertexPoints.push_back(m_lstAllVertexPoints[lstFaceVertexPointIndices[iVertex]]);

		//Get All Required edgePoints
		ctEdges	   = m_weMesh->getFaceEdges(iFace, lstFaceEdgePointIndices);
		for(iEdge = 0; iEdge < ctEdges; iEdge++)
			lstFaceEdgePoints.push_back(m_lstAllEdgePoints[lstFaceEdgePointIndices[iEdge]]);

		//For each vertex we will create a quad
		for(iVertex = 0; iVertex < ctVertices; iVertex++)
		{		
			start   = lstFaceVertexPoints[iVertex];
			ep1		= lstFaceEdgePoints[iVertex];
			fp		= m_lstAllFacePoints[iFace];

			
			if(iVertex == 0)			
				ep2 = lstFaceEdgePoints[ctVertices-1];
			else
				ep2 = lstFaceEdgePoints[iVertex-1];


			subMesh.addQuadFace(start, ep1, fp, ep2);
		}
	} 

	//Push to Vertex Pool
	m_weMesh->copyFrom(subMesh);
	
	//Cleanup
	subMesh.cleanup();
	lstFaceVertexPointIndices.resize(0);
	lstFaceVertexPoints.resize(0);
	lstFaceEdgePointIndices.resize(0);
	lstFaceEdgePoints.resize(0);

	if(m_pOnOverallProgress)
		m_pOnOverallProgress("cleaned up and done.", 100.0f);

	CErrorManager::GetInstance().FlushErrors();


	return true;
}

void CSubDivDX11::CProcessFaceBody::process(UINT iface) const
{
	UINT idxCurFace = iface;	
	if(idxCurFace >= inConstBuf.g_ctInputFaces) 
		return;

	//Get the face		
	UINT ctFaceSides = inConstBuf.g_ctFaceSides;
	DEBUGFACE face = inputFaces[iface];
	//uint4 face	 = inputFaces[idxCurFace];

	//Compute FacePoint
	D3DXVECTOR3 fp;
	if(ctFaceSides == 4)
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos + inputVertices[face.w].pos;
	else
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos;
	fp = fp / (float)ctFaceSides;	
	outputAll[idxCurFace].fp = fp;
	//===================================================================================================
	VERTEX start, end;
	GPUEDGE e, original;

	vec2ui want;
	D3DXVECTOR3 R, Q;
	D3DXVECTOR3 fpo;
	DEBUGFACE otherface;
	UINT ctFaces = 0;
	//UINT ctEdges = 0;
	UINT idxCurEdge = 0;
	UINT idxNextEdge = 0;	
	UINT iVertex = 0;
	//Now We will move each control point to a new position based on Catmull Clark's formula
	// S' = (Q/n) + (2R/n) + (S(n-3)/n)
	// n = valence of a point (number of edges that connect to that point	
	// Q = average of surrounding facepoints
	// R = average of all sorround edg mid-points (Not edge points but MID-POINTS)		
	for(iVertex=0; iVertex < ctFaceSides; iVertex++)
	{		
		want.x = face[iVertex];
		if(iVertex == ctFaceSides - 1)		
			want.y = face[0];
		else
			want.y = face[iVertex + 1];

		start  = inputVertices[want.x];
		end    = inputVertices[want.y];

		idxCurEdge  = start.edge;
		e			= inputEdges[start.edge];


		original = e;
		ctFaces  = 0;
		//ctEdges  = 0;

		//Q = zero();
		//R = zero();
		Q = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		R = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		//Compute Q and R
		//[loop]
		while((ctFaces == 0)||(!isSameEdge(e, original)))
		{					
			//This edge has our vertex as an endpoint of it			
			if((e.selr[0] == want.x)||(e.selr[1] == want.x))
			{
				R = R + (inputVertices[e.selr[0]].pos + inputVertices[e.selr[1]].pos) * 0.5f;

				//If we are right face of this edge
				if(e.selr[0] == want.x)
				{
					//Compute lface FacePoint	
					otherface   = inputFaces[e.selr[2]];
					idxNextEdge = e.lnrn[0];
				}
				else
				{
					otherface   = inputFaces[e.selr[3]];
					idxNextEdge = e.lnrn[1];
				}


				if(ctFaceSides == 4)
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos + inputVertices[otherface.w].pos;
				else
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos;
				fpo = fpo / (float)ctFaceSides;
				//fpo = fpo * 0.25f;
				Q = Q + fpo;
				ctFaces++;
			}

			if((e.selr[0] == want.x || e.selr[1] == want.x)&&
				(e.selr[0] == want.y || e.selr[1] == want.y))
			{
				outputAll[idxCurEdge].ep   = (inputVertices[e.selr[0]].pos + inputVertices[e.selr[1]].pos + fp + fpo) * 0.25f;
			}			

			//Update loop variables
			e  = inputEdges[idxNextEdge];				
			idxCurEdge = idxNextEdge;						
		}//End While

		float valence = (float)(ctFaces);
		Q = Q / valence;
		R = R / valence;

		//Copy to output buffer
		outputAll[want.x].vp = Q / valence + (2.0f / valence) * R + ((valence - 3.0f) / valence) * start.pos;		
	}//End For iVertex
}



}


