#pragma once
#ifndef CPOLYHASHGRIDEDGE_H
#define CPOLYHASHGRIDEDGE_H

#define DEFAULT_GRIM_DIM 8

#include "PS_Framework/include/PS_Vector.h"

namespace PS{

	struct HASHFUNC
	{
		unsigned int dim;
		unsigned int cellid_shift_x;
		unsigned int cellid_shift_y;
		unsigned int cellid_shift_z;
		unsigned int cellid_bitmask;
		unsigned int cellid_hashsize;

		HASHFUNC()
		{
			dim = DEFAULT_GRIM_DIM;
			cellid_shift_x = 0;
			cellid_shift_y = 3;
			cellid_shift_z = 6;			
			cellid_bitmask = 0x07;
			cellid_hashsize = (unsigned int)(1 << (3*cellid_shift_y));
		}

		void setup(unsigned int d)
		{
			dim = d;
			cellid_shift_x = 0;
			cellid_shift_y = log2i(dim);
			cellid_shift_z = 2*cellid_shift_y;			
			cellid_bitmask = (unsigned int)((1 << cellid_shift_y) - 1);
			cellid_hashsize = (unsigned int)(1 << (3*cellid_shift_y));
		}

		__inline unsigned int hash(int i, int j, int k)
		{
			//#define CELLID_FROM_IDX(i,j,k) (((k) & CELLID_BITMASK) << CELLID_SHIFT_Z) | (((j) & CELLID_BITMASK) << CELLID_SHIFT_Y) | ((i) & CELLID_BITMASK)
			return ((k & cellid_bitmask) << cellid_shift_z) | ((j & cellid_bitmask) << cellid_shift_y) | (i & cellid_bitmask);
		}
	};
	//////////////////////////////////////////////////////////////////////////
	//Grid is used inside each MPU
	class Grid
	{
	public:
		bool bValid;
		int nDim;
		static const int CACHE_DIM = 32;
		__declspec(align(16)) float pValues[CACHE_DIM*CACHE_DIM*CACHE_DIM];	

		Grid()
		{ 
			nDim = 0; 
			bValid = false;
			//pValues = NULL;
		}

		Grid(int gridDim)
		{
			setup(gridDim);
		}

		~Grid()
		{
			clearAll();
		}

		void clearAll()
		{	
			bValid = false;
			/*
			if(pValues)
			{
			aligned_free(pValues);
			pValues = NULL;
			}
			*/
		}

		Grid(const Grid& rhs)
		{
			setup(rhs.nDim, rhs.pValues);
		}

		void setup(int gridDim, const float* lpSrc)
		{
			//clearAll();
			nDim = gridDim;
			//pValues = static_cast<float*>(aligned_malloc(nDim*nDim*nDim*sizeof(float), 32));		
			memcpy(pValues, lpSrc, sizeof(float)*nDim*nDim*nDim);
			bValid = true;
		}

		void setup(int gridDim)
		{
			//clearAll();
			nDim = gridDim;
			//pValues = static_cast<float*>(aligned_malloc(nDim*nDim*nDim*sizeof(float), 32));
			//memset(pValues, (int)FLT_MIN, nDim*nDim*nDim);
			for(int i=0; i<nDim*nDim*nDim; i++)
				pValues[i] = FLT_MIN;	
			//pValues[i] = 0.0f;	
			bValid = true;
		}	
	};
	//////////////////////////////////////////////////////////////////////////
	class CProcessedEdges
	{
	public:
		//Edge and Vertex ID HashTable
		DVec<EDGELIST> m_table;		   		
		HASHFUNC m_hashFunc;
		bool m_bAllocated;

	public:
		CProcessedEdges()	{	m_bAllocated = false;}
		~CProcessedEdges()	{	clearAll();	}

		void clearAll()
		{
			if(m_bAllocated)
			{
				for(size_t i=0; i < m_table.size(); i++)
				{			
					m_table[i].clear();
				}		
				m_table.clear();
				m_bAllocated = false;
			}
		}

		void setup(unsigned int dim)
		{
			clearAll();
			m_hashFunc.setup(dim);
			m_table.resize(2*m_hashFunc.cellid_hashsize);
			m_bAllocated = true;
		}

		void setEdge(vec3i start, vec3i end, int vid)
		{
			setEdge(start.x, start.y, start.z, end.x, end.y, end.z, vid);
		}

		void setEdge (int i1, int j1, int k1, 
			int i2, int j2, int k2, int vid)

		{
			unsigned int index;

			if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2)))) 
			{
				//Swap i1, j1, k1 with i2, j2, k2
				int t;
				t=i1; i1=i2; i2=t; 
				t=j1; j1=j2; j2=t; 
				t=k1; k1=k2; k2=t;
			}

			//Compute Hash Address
			index = m_hashFunc.hash(i1, j1, k1) + m_hashFunc.hash(i2, j2, k2);

			//Add new Edge to appropriate EdgeList
			EDGEELEMENT edge;
			edge.i1 = i1; 
			edge.j1 = j1; 
			edge.k1 = k1;
			edge.i2 = i2; 
			edge.j2 = j2; 
			edge.k2 = k2;
			edge.vid = vid;
			m_table[index].push_back(edge);
		}

		int getEdge(vec3i start, vec3i end)
		{
			return getEdge(start.x, start.y, start.z, end.x, end.y, end.z);
		}

		//Return vertex id for edge; return -1 if not set
		int getEdge (int i1, int j1, int k1, int i2, int j2, int k2)
		{
			static int MAX_EDGES = 0;
			if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2)))) 
			{
				int t;
				t=i1; i1=i2; i2=t; 
				t=j1; j1=j2; j2=t; 
				t=k1; k1=k2; k2=t;
			}
			int hashval = m_hashFunc.hash(i1, j1, k1) + m_hashFunc.hash(i2, j2, k2);

			size_t ctEdges = m_table[hashval].size();
			EDGEELEMENT e;

			for(size_t iEdge=0; iEdge < ctEdges; iEdge++)
			{
				e = m_table[hashval][iEdge];
				if((e.i1 == i1 && e.j1 == j1 && e.k1 == k1)&&
					(e.i2 == i2 && e.j2 == j2 && e.k2 == k2))
				{
					return e.vid;
				}
			}
			return -1;
		}
	};

}

#endif