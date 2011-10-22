#ifndef POLYDATASTRUCTURES_H
#define POLYDATASTRUCTURES_H

#include <math.h>
#include <iostream>
#include <vector>
#include <list>
#include <sys/types.h>
#include "_GlobalSettings.h"

#include "DSystem/include/DContainers.h"
#include "PS_FrameWork/include/PS_Vector.h"
#include "PS_FrameWork/include/PS_String.h"
#include "PS_BlobTree/include/CBlobTree.h" 

using namespace PS;

//Suggest Which method to use for converting cubes to 
//Triangles. Tetrahedra or direct cubes 
enum CellShape
{
	csTetrahedra = 0,  
	csCube		= 1  
};

enum VertexState {vsCold = 0, vsHot = 1};

struct SUBVOL_RAY_TEST_RESULT
{
	float fieldValue;
	vec3 pos;
	int  ctIterations;
	bool bSuccess;
};


//test the function for a signed value
//check if the given position is inside or outside
//OK. if value is of correct sign
struct TEST 
{		   
	vec3 pos;			   
	float value;		   
	int ok;			   
};

struct CORNER {		   
	int i, j, k;		   
	vec3 pos;		   
	float value;		   
};

struct CELL 
{		   
	int i, j, k;		   
	CORNER *corners[8];		   
};
//////////////////////////////////////////////////////////////////////////
struct MPUCORNER{
	int i, j, k;		   
	vec3 pos;		   
	float value;		   	
};

struct MPUCELL{
	int i, j, k;
	MPUCORNER corners[8];
};


//Structure of a center location
//i,j,k is the address of that location
struct CENTERELEMENT {	   
	int i, j, k;		   
};
typedef DVec<CENTERELEMENT> CENTERLIST;

//Structure of a corner
//i,j,k are the address of the corner
//value is the field value at that corner
struct CORNERELEMENT {	   
	int i, j, k;		   
	float value;		   
};
typedef DVec<CORNERELEMENT> CORNER_ELEMENT_LIST;


//Structure to represent a single edge
//Holds address of the 2 endpoints of an edge
//Holds edge id
struct EDGEELEMENT {	   
	int i1, j1, k1, i2, j2, k2;	   
	int vid;			   
};
typedef DVec<EDGEELEMENT> EDGELIST;


typedef DVec<int> INTLIST;
typedef DVec<INTLIST> INTLISTS;


inline float RAND() 
{
	return (rand()&32767)/32767.0f;
}

const int HASHSIZE = (size_t)(1<<(3*HASHBIT));   

const int MASK = ((1<<HASHBIT)-1);

inline int HASH( int i, int j,int k) 
{ 
	return (((((i&MASK)<<HASHBIT)|j&MASK)<<HASHBIT)|k&MASK);
} 

inline int BIT(int i, int bit) 
{ 
	return (i>>bit)&1; 
}

// flip the given bit of i 
inline int FLIP(int i, int bit) 
{
	return i^1<<bit;
}



#endif
