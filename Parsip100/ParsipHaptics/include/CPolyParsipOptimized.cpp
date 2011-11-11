#include "CPolyParsipOptimized.h"
#include "PS_FrameWork/include/PS_PerfLogger.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"
#include <map>

#ifdef linux
    #include <pthread.h>
#endif

namespace PS{

	//Start Processing MPU
	void CSIMDMPU::start()
	{
		m_bReady = false;
		m_statStartTime = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());
		
		//Init		
		lpOutputMesh->removeAll();
		lpOutputMesh->initMesh(CMeshVV::TRIANGLES, 3, 4, 0, 0);				

		//Thread ID
                #ifdef WIN32
		m_threadID = GetCurrentThreadId();
                #else
                m_threadID = pthread_self();
                #endif


		//Do All Pre-fetchings
		//PREFETCH(m_fvCache);
		//PREFETCH(m_cptBlob);
		//PREFETCH(g_triTableCache);
	}

	void CSIMDMPU::finish(size_t ctFieldEvals, size_t ctIntersectedCells)
	{
		double e = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());
		m_statProcessTime	   = e - m_statStartTime;
		m_statFieldEvaluations = ctFieldEvals;
		m_statIntersectedCells = ctIntersectedCells;

		//Update Flags
		m_bReady = true;		
	}

	//////////////////////////////////////////////////////////////////////////
	int CSIMDMPURunBody::getEdge(vec3i start, vec3i end) const
	{
		int i1 = start.x;
		int j1 = start.y;
		int k1 = start.z;
		int i2 = end.x;
		int j2 = end.y;
		int k2 = end.z;

		if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2)))) 
		{
			int t;
			t=i1; i1=i2; i2=t; 
			t=j1; j1=j2; j2=t; 
			t=k1; k1=k2; k2=t;
			start.set(i1, j1, k1);
			end.set(i2, j2, k2);
		}

		int hashval = CELLID_FROM_IDX(i1, j1, k1) + CELLID_FROM_IDX(i2, j2, k2);

		size_t ctEdges = m_edgeTableSizes[hashval];
		SIMDEDGEELEMENTS e;

		for(size_t iEdge=0; iEdge < ctEdges; iEdge++)
		{
			e = m_edgeTable[hashval];			
			if((e.start[iEdge] == start)&&(e.end[iEdge] == end))
			{
				return e.vid[iEdge];
			}
		}
		return -1;
	}

	void CSIMDMPURunBody::setEdge(vec3i start, vec3i end, int vid) const
	{
		int i1 = start.x;
		int j1 = start.y;
		int k1 = start.z;
		int i2 = end.x;
		int j2 = end.y;
		int k2 = end.z;

		if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2)))) 
		{
			int t;
			t=i1; i1=i2; i2=t; 
			t=j1; j1=j2; j2=t; 
			t=k1; k1=k2; k2=t;
			start.set(i1, j1, k1);
			end.set(i2, j2, k2);
		}
		int hashval = CELLID_FROM_IDX(i1, j1, k1) + CELLID_FROM_IDX(i2, j2, k2);

		size_t idx = m_edgeTableSizes[hashval];
		if(idx >= EDGETABLE_DEPTH)
		{
			ReportError("I wanted to add an edge in the edgetable but table is full!");
			FlushAllErrors();
			return;
		}
		m_edgeTableSizes[hashval] = idx + 1;
		
		m_edgeTable[hashval].start[idx] = start;
		m_edgeTable[hashval].end[idx] = end;
		m_edgeTable[hashval].vid[idx] = vid;
	}

	bool CSIMDMPURunBody::intersects( const vec4f& lo1, const vec4f& hi1, const vec4f& lo2, const vec4f& hi2 ) const
	{
		if ((lo1.x >= hi2.x) || (hi1.x <= lo2.x))
			return false;
		if ((lo1.y >= hi2.y) || (hi1.y <= lo2.y))
			return false;
		if ((lo1.z >= hi2.z) || (hi1.z <= lo2.z))
			return false;

		return true;
	}


	bool CSIMDMPURunBody::doMarchingCubes(CSIMDMPU* aMPU) const
	{			
		//Start Processing aMPU
		aMPU->start();
				
		//Vars
		float fp;		
		size_t ctInside = 0;			
		int	  cellCornerKey[8];
		vec4f cellCornerPos[8];		
		vec3i  cellCornerIDX[8];
		float  cellCornerFields[8];

		int icase;
		int idxCellConfig;		
		size_t ctFieldEvals = 0;
		size_t ctIntersectedCells = 0;
		
		//Init
                memset(m_edgeTableSizes, 0, 2*CELLID_HASHSIZE*sizeof(int));
		int m = GRID_DIM*GRID_DIM*GRID_DIM;
		for(int i=0; i<m; i++)
			m_fvCache[i] = FLT_MIN;



		//Check if we really need to find field-values for all splats in this grid				
		float invCellSize = 1.0f / m_cellsize;
		vec3f origin = aMPU->getOrigin();		
		vec3f sides  = (m_modelUpperCorner - origin) * invCellSize;

		//Bounds
		vec3i bounds;
		bounds.x = MATHMIN(GRID_DIM, static_cast<int>(ceil(sides[0])));
		bounds.y = MATHMIN(GRID_DIM, static_cast<int>(ceil(sides[1])));
		bounds.z = MATHMIN(GRID_DIM, static_cast<int>(ceil(sides[2])));	
		sides.set(m_cellsize * (bounds.x-1), m_cellsize * (bounds.y-1), m_cellsize * (bounds.z-1));
		aMPU->setSides(sides);

		//Check if intersects?		
		vec4f org4(origin.x, origin.y, origin.z);
		vec4f side4(sides.x, sides.y, sides.z);
		vec4f hi = org4 + side4;		
		bool bIntersectsPrimOctree = false;					
		for(int i=0; i < m_cptBlob->ctPrims; i++)
		{			
			if(intersects(m_cptBlob->prims[i].octLo, 
						  m_cptBlob->prims[i].octHi, 
						  org4, 
						  hi))
			{
				bIntersectsPrimOctree = true;
				break;
			}
		}

		//Add now
		if(!bIntersectsPrimOctree)
		{
			aMPU->finish(0, 0);
			return false;
		}		


		//Process all cubes 
		for(int i=0; i<bounds.x-1; i++)
		{
			for(int j=0; j<bounds.y-1; j++)
			{
				for(int k=0; k<bounds.z-1; k++)
				{						
					cellCornerKey[0] = CELLID_FROM_IDX(i, j, k);					
					cellCornerIDX[0] = vec3i(i, j, k);
					cellCornerPos[0] = org4 + vec4f(m_cellsize*i, m_cellsize*j, m_cellsize*k);					

					cellCornerKey[1] = CELLID_FROM_IDX(i, j, k+1);
					cellCornerIDX[1] = vec3i(i, j, k+1);
					cellCornerPos[1] = org4 + vec4f(m_cellsize*i, m_cellsize*j, m_cellsize*(k+1));					

					cellCornerKey[2] = CELLID_FROM_IDX(i, j+1, k);
					cellCornerIDX[2] = vec3i(i, j+1, k);
					cellCornerPos[2] = org4 + vec4f(m_cellsize*i, m_cellsize*(j+1), m_cellsize*k);					

					cellCornerKey[3] = CELLID_FROM_IDX(i, j+1, k+1);
					cellCornerIDX[3] = vec3i(i, j+1, k+1);
					cellCornerPos[3] = org4 + vec4f(m_cellsize*i, m_cellsize*(j+1), m_cellsize*(k+1));

					cellCornerKey[4] = CELLID_FROM_IDX(i+1, j, k);
					cellCornerIDX[4] = vec3i(i+1, j, k);
					cellCornerPos[4] = org4 + vec4f(m_cellsize*(i+1), m_cellsize*j, m_cellsize*k);					

					cellCornerKey[5] = CELLID_FROM_IDX(i+1, j, k+1);
					cellCornerIDX[5] = vec3i(i+1, j, k+1);
					cellCornerPos[5] = org4 + vec4f(m_cellsize*(i+1), m_cellsize*j, m_cellsize*(k+1));					

					cellCornerKey[6] = CELLID_FROM_IDX(i+1, j+1, k);
					cellCornerIDX[6] = vec3i(i+1, j+1, k);
					cellCornerPos[6] = org4 + vec4f(m_cellsize*(i+1), m_cellsize*(j+1), m_cellsize*k);					

					cellCornerKey[7] = CELLID_FROM_IDX(i+1, j+1, k+1);
					cellCornerIDX[7] = vec3i(i+1, j+1, k+1);
					cellCornerPos[7] = org4 + vec4f(m_cellsize*(i+1), m_cellsize*(j+1), m_cellsize*(k+1));					


					//Get the field for the cube from cache
					//Find cube-case 
					idxCellConfig = 0;
					for(icase=0; icase<8; icase++)
					{
						fp = m_fvCache[cellCornerKey[icase]];

						//If not set then evaluate
						if(fp == FLT_MIN)
						{
							ctFieldEvals++;
							fp = m_cptBlob->fieldvalue(cellCornerPos[icase]);
							m_fvCache[cellCornerKey[icase]] = fp;
						}

						cellCornerFields[icase] = fp;
						if(fp > m_isovalue)
							idxCellConfig += (1 << icase);
					}

					if((idxCellConfig != 0)&&(idxCellConfig != 255))
					{
						//Increment number of processed cells
						ctIntersectedCells++;

						//Compute surface vertex, normal and material on each crossing edge
						int idxMeshVertex[16];
						int idxEdgeStart, idxEdgeEnd;
						int candidates[16];

						//Read case
						size_t ctTotalPolygons = 0;
						size_t ctEdges = 0;
						vec4f p;
						vec4f n;
						vec4f diffused;
						float fp;

						for(icase=0; icase<16; icase++)
						{							
							candidates[icase] = g_triTableCache[idxCellConfig][icase];
							if(candidates[icase] != -1)
								ctEdges++;
						}
						ctTotalPolygons = ctEdges / 3;

						
						for(icase = 0; icase < (int)ctEdges; icase++)
						{							
							idxEdgeStart = corner1[candidates[icase]];
							idxEdgeEnd	 = corner2[candidates[icase]];
							idxMeshVertex[icase] = getEdge(cellCornerIDX[idxEdgeStart], cellCornerIDX[idxEdgeEnd]);
							if(idxMeshVertex[icase] == -1)
							{		
								float* lpStoreFVPrims = new float[m_cptBlob->ctPrims];
								float* lpStoreFVOps   = new float[m_cptBlob->ctOps];
								ctFieldEvals += ComputeRootNewtonRaphsonVEC4(m_cptBlob,
																			 lpStoreFVOps,
																			 lpStoreFVPrims,
																			 cellCornerPos[idxEdgeStart], 
																			 cellCornerPos[idxEdgeEnd], 
																			 m_fvCache[cellCornerKey[idxEdgeStart]], 
																			 m_fvCache[cellCornerKey[idxEdgeEnd]], 																					   
																			 p, fp, m_isovalue);	

								//Setup mesh
								ctFieldEvals += 3;
								n		 = m_cptBlob->normal(p, fp, NORMAL_DELTA);
								diffused = m_cptBlob->baseColor(p, lpStoreFVOps, lpStoreFVPrims);

								//Cleanup
								SAFE_DELETE(lpStoreFVOps);
								SAFE_DELETE(lpStoreFVPrims);

								aMPU->lpOutputMesh->addVertex(p[0], p[1], p[2]);
								aMPU->lpOutputMesh->addNormal(n[0], n[1], n[2]);
								aMPU->lpOutputMesh->addColor(vec4f(diffused[0], diffused[1], diffused[2], diffused[3]));

								//Save PolyVertex
								//Get vertex v index from list. It is the last one
								idxMeshVertex[icase] = aMPU->lpOutputMesh->countVertices() - 1;		
								setEdge(cellCornerIDX[idxEdgeStart], cellCornerIDX[idxEdgeEnd], idxMeshVertex[icase]);
							}
							
						}

						for(icase = 0; icase < (int)ctTotalPolygons; icase++)
						{
							aMPU->lpOutputMesh->addTriangle(idxMeshVertex[icase*3+0], 
														 idxMeshVertex[icase*3+1], 
														 idxMeshVertex[icase*3+2]);
						}	
						
					}
				}
			}
		}
	
		//Process Finished
		aMPU->finish(ctFieldEvals, ctIntersectedCells);		
		return (ctInside > 0);
	}

	//////////////////////////////////////////////////////////////////////////
	void CParsipOptimized::setup(COMPACTBLOBTREE* lpCompactBlobTree, 
  	 							 const COctree& oct, 
								 int id /* = 0 */, 
								 float cellsize /* = DEFAULT_CELL_SIZE */, 
								 float isovalue /*= ISO_VALUE */)

	{
		I64 tsStart = CPerfLogger::getPerfCounter();

		//Process BlobTree		
		removeAllMPUs();
		
		m_inlpCompactTree = lpCompactBlobTree;
		m_inOctree		  = oct;
		m_inAssignedID	  = id;
		m_inCellSize	  = cellsize;
		m_inIsoValue      = isovalue;

		vec3f allSides		= oct.upper - oct.lower;
		vec3i ctCellsNeeded = vec3i(static_cast<int>(ceil(allSides.x / cellsize)), 
									static_cast<int>(ceil(allSides.y / cellsize)), 
									static_cast<int>(ceil(allSides.z / cellsize)));


		//m_gridDim = griddim;
		int cellsPerMPU = GRID_DIM - 1;
		vec3i ctMPUNeeded;
		ctMPUNeeded.x = ctCellsNeeded.x / cellsPerMPU;
		ctMPUNeeded.y = ctCellsNeeded.y / cellsPerMPU;
		ctMPUNeeded.z = ctCellsNeeded.z / cellsPerMPU;
		if(ctCellsNeeded.x % cellsPerMPU != 0)
			ctMPUNeeded.x++;
		if(ctCellsNeeded.y % cellsPerMPU != 0)
			ctMPUNeeded.y++;
		if(ctCellsNeeded.z % cellsPerMPU != 0)
			ctMPUNeeded.z++;

		//Create all MPUs		
		float side = static_cast<float>(cellsPerMPU) * cellsize;
		vec3f mpuSides(side, side, side);		

		//Create all intersecting MPUs
		for(int i=0; i<ctMPUNeeded.x; i++)
		{
			for(int j=0; j<ctMPUNeeded.y; j++)
			{
				for(int k=0; k<ctMPUNeeded.z; k++)
				{	
					vec3f origin = oct.lower + side * vec3f((float)i, (float)j, (float)k);

					CSIMDMPU* aMPU = new CSIMDMPU(origin, mpuSides);						
					m_lstMPUs.push_back(aMPU);					

				}
			}
		}

		//Compute Time
		I64 tsEnd = CPerfLogger::getPerfCounter();
		m_tsSetup = CPerfLogger::convertTimeTicksToMS(tsEnd - tsStart);
	}

	void CParsipOptimized::run()
	{
		if(m_inlpCompactTree == NULL) return;
		m_tsStart = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());

		CSIMDMPURunBody body(m_inlpCompactTree,
							 m_inOctree.upper,
							 m_lstMPUs,
							 m_inCellSize,
							 m_inIsoValue);
		parallel_reduce(blocked_range<int>(0, m_lstMPUs.size()), body, tbb::auto_partitioner());		
		size_t extra = body.getExtraCount();
		if(extra > 0)
			removeExtraPUs();
		//parallel_for(blocked_range<int>(0, m_lstMPUs.size()), body, tbb::affinity_partitioner());		

		double e = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());
		m_tsPolygonize = e - m_tsStart;		
	}

	void CParsipOptimized::removeAllMPUs()
	{
		if(m_lstMPUs.size() == 0) return;
		CSIMDMPU* ampu;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			ampu = m_lstMPUs[i];
			SAFE_DELETE(ampu);
		}
		m_lstMPUs.resize(0);		
		m_inAssignedID = -1;
	}

	void CParsipOptimized::drawMesh()
	{
		CSIMDMPU* ampu = NULL;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			ampu = m_lstMPUs[i];				
			if(ampu->isReady())
				m_lstMPUs[i]->lpOutputMesh->drawBuffered();
		}
	}

	void CParsipOptimized::drawNormals(int normalLength)
	{
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			if(m_lstMPUs[i]->isReady())				
				m_lstMPUs[i]->lpOutputMesh->drawNormals(normalLength);				
		}
	}

	bool CParsipOptimized::getMPUExtent(size_t idxMPU, vec3f& lo, vec3f& hi) const
	{
		lo = vec3f(0.0f, 0.0f, 0.0f);
		hi = lo;
		vec3f gridSide;
		if((idxMPU >= 0) && (idxMPU < m_lstMPUs.size()))
		{					
			lo			= m_lstMPUs[idxMPU]->getOrigin();
			gridSide	= m_lstMPUs[idxMPU]->getSides();
			hi = lo + gridSide;
			return true;
		}

		return false;
	}

	CSIMDMPU* CParsipOptimized::statsLatestMPU() const
	{
		double tsProcess = 0.0;
		CSIMDMPU* latestMPU = NULL;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			if(tsProcess < m_lstMPUs[i]->statsProcessTime())
			{
				tsProcess = m_lstMPUs[i]->statsProcessTime(); 
				latestMPU = m_lstMPUs[i];
			}
		}
		return latestMPU;
	}

	double CParsipOptimized::statsLatestMPUTime() const
	{
		double tsProcess = 0.0;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			if(tsProcess < m_lstMPUs[i]->statsProcessTime())
				tsProcess = m_lstMPUs[i]->statsProcessTime(); 
		}
		return tsProcess;
	}

	size_t CParsipOptimized::statsIntersectedMPUs() const
	{
		size_t ctIntersected = 0;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			if(m_lstMPUs[i]->hasSurface())
				ctIntersected++;
		}
		return ctIntersected;
	}

	void CParsipOptimized::statsMeshInfo( size_t& ctVertices, size_t& ctFaces ) const
	{
		ctVertices = 0;
		ctFaces = 0;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			ctVertices += m_lstMPUs[i]->lpOutputMesh->countVertices();
			ctFaces += m_lstMPUs[i]->lpOutputMesh->countFaces();			
		}
	}

	size_t CParsipOptimized::statsTotalFieldEvals() const
	{
		size_t ctFieldEvals = 0;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			ctFieldEvals += m_lstMPUs[i]->statsFieldEvals();
		}
		return ctFieldEvals;
	}

	size_t CParsipOptimized::statsIntersectedCellsCount() const
	{
		size_t ctProcessedCells = 0;
		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			ctProcessedCells += m_lstMPUs[i]->statsIntersectedCells();
		}
		return ctProcessedCells;
	}

	int CParsipOptimized::statsCoreUtilizations( DVec<size_t>& arrOutThreadIDs, DVec<double>& arrOutUtilization )
	{
		size_t id;

		typedef pair<size_t,double> threadpair;
		std::map<size_t, double> m;
		std::map<size_t, double>::iterator mIter;

		double t;
		int ctActiveCores = 0;

		for(size_t i=0; i<m_lstMPUs.size(); i++)
		{
			if(m_lstMPUs[i]->isReady() && (m_lstMPUs[i]->statsProcessTime() > 0.0))
			{			
				id = m_lstMPUs[i]->getThreadId();
				if(m.find(id) == m.end())
				{
					m.insert(threadpair(id, m_lstMPUs[i]->statsProcessTime()));				
					ctActiveCores++;
				}
				else
				{
					t = m[id];
					m[id] = t + m_lstMPUs[i]->statsProcessTime();
				}
			}
		}

		//Resize output
		arrOutThreadIDs.resize(ctActiveCores);
		arrOutUtilization.resize(ctActiveCores);

		int iCore = 0;
		for(mIter = m.begin(); mIter != m.end(); mIter++)
		{
			arrOutThreadIDs[iCore] = mIter->first;
			arrOutUtilization[iCore] = mIter->second / m_tsPolygonize;
			iCore++;			
		}
		m.clear();
		return ctActiveCores;
	}

	size_t CParsipOptimized::removeExtraPUs()
	{
		size_t i=0;
		size_t ctRemoved = 0;
		CSIMDMPU* ampu = NULL;
		while(i < m_lstMPUs.size())
		{
			ampu = m_lstMPUs[i];			
			if(ampu->hasSurface() == false)
			{
				m_lstMPUs.erase(m_lstMPUs.begin() + i);
				SAFE_DELETE(ampu);
				ctRemoved++;
			}
			else
				i++;
		}

		return ctRemoved;
	}

	bool CParsipOptimized::exportMesh( CMeshVV* lpOutputMesh )
	{
		if(lpOutputMesh == NULL) 
			return false;

		size_t ctUnits = m_lstMPUs.size();
		if(ctUnits == 0)
			return false;
		
		CSIMDMPU* ampu = m_lstMPUs[0];
		lpOutputMesh->copyFrom(*ampu->lpOutputMesh);
		for (size_t i=1; i<ctUnits; i++)
		{
			ampu = m_lstMPUs[i];
			if(ampu->hasSurface())
				lpOutputMesh->appendFrom(*ampu->lpOutputMesh);
		}		

		return (lpOutputMesh->countFaces() > 0);
	}

	CParsipOptimized* Run_Polygonizer( CBlobTree* input, float cellSize, float isovalue )
	{
		if(input == NULL) return NULL;

		COMPACTBLOBTREE cptBlob;
		cptBlob.convert(input);

		CParsipOptimized* parsip = new CParsipOptimized();
		//parsip->setup(cellSize, input->getOctree(), &cptBlob, input->getID());
		parsip->setup(&cptBlob, input->getOctree(), input->getID(), cellSize, isovalue);
		parsip->run();

		return parsip;
	}

	bool Run_PolygonizerExportMesh( CBlobTree* input, 
									CMeshVV* lpMesh, 
									float cellSize /*= DEFAULT_CELL_SIZE*/,
									float isovalue /*= ISO_VALUE */)
	{
		if((input == NULL)&&(lpMesh == NULL)) return false;

		COMPACTBLOBTREE cptBlob;
		cptBlob.convert(input);

		CParsipOptimized* parsip = new CParsipOptimized();
		//cellSize, input->getOctree(), &cptBlob, input->getID(
		parsip->setup(&cptBlob, input->getOctree(), input->getID(), cellSize, isovalue);
		parsip->run();
		bool bres = parsip->exportMesh(lpMesh);
		SAFE_DELETE(parsip);

		return bres;
	}

}
