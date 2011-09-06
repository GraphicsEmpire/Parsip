#ifndef CEDGETABLE_H
#define CEDGETABLE_H

#include "_PolygonizerStructs.h"

//////////////////////////////////////////////////////////////////////////////////
class CEdgeTable
{
	//Edge and Vertex ID HashTable
	DVec<EDGELIST> m_table;		   

public:
	CEdgeTable()
	{
		m_table.resize(2*HASHSIZE);
	}

	~CEdgeTable()
	{
		clearAll();
	}

	void clearAll()
	{
		for(size_t i=0; i < m_table.size(); i++)
		{			
			m_table[i].clear();
		}		
		m_table.clear();
	}

	void reset()
	{
		clearAll();
		m_table.resize(2*HASHSIZE);
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
		index = HASH(i1, j1, k1) + HASH(i2, j2, k2);

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

	int getEdge(const CORNER &c1, const CORNER &c2)
	{
		return getEdge(c1.i, c1.j, c1.k, c2.i, c2.j, c2.k);
	}

	int getEdge(vec3i start, vec3i end)
	{
		return getEdge(start.x, start.y, start.z, end.x, end.y, end.z);
	}

	//Return vertex id for edge; return -1 if not set
	int getEdge (int i1, int j1, int k1, int i2, int j2, int k2)
	{
		if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2)))) 
		{
			int t;
			t=i1; i1=i2; i2=t; 
			t=j1; j1=j2; j2=t; 
			t=k1; k1=k2; k2=t;
		}
		int hashval = HASH(i1, j1, k1) + HASH(i2, j2, k2);

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

#endif

