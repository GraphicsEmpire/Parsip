#include "CCubeTable.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"

CMarchCubeTable* CMarchCubeTable::sm_pCubeTable = NULL;


CMarchCubeTable* CMarchCubeTable::getInstance()
{
	if(sm_pCubeTable == NULL)
	{
		sm_pCubeTable = new CMarchCubeTable();
	}
	
	return sm_pCubeTable;
}

CMarchCubeTable::CMarchCubeTable()
{
	ctable.resize(256);
	int iConfig, iEdge, iVertex;
	int arrDone[12], arrHotCold[8];

	int ctMaxTriangles = 0;
	int ctCurTriangles = 0;
	int ctMaxLists = 0;
	//Looping through all possible 256 configurations
	for (iConfig = 0; iConfig < 256; iConfig++) 
	{
		//Set done flag to false for all edges in this 
		//Cube configuration
		for (iEdge = 0; iEdge < 12; iEdge++) 
			arrDone[iEdge] = 0;
		
		//arrPos will show us if a vertex is hot or cold
		//Remember we have 8 vertices inside a cube
		//Bit = 1 means inside or hot otherwise outside or cold
		for (iVertex = 0; iVertex < 8; iVertex++) 
			arrHotCold[iVertex] = BIT(iConfig, iVertex);

		ctCurTriangles = 0;

		//Processing edges and check if they are crossing (Oppositely signed) 
		for (iEdge = 0; iEdge < 12; iEdge++)
		{			
			if (!arrDone[iEdge] && (arrHotCold[corner1[iEdge]] != arrHotCold[corner2[iEdge]])) 
			{
				INTLIST triangleFan;
				int start = iEdge;
				int edge = iEdge;

				//Get the face that is to the right of edge from pos to neg corner:
				int face = arrHotCold[corner1[iEdge]]? rightface[iEdge] : leftface[iEdge];
				while (1) 
				{
					edge = nextClockWiseEdge(edge, face);
					arrDone[edge] = 1;
					if (arrHotCold[corner1[edge]] != arrHotCold[corner2[edge]]) 
					{
						triangleFan.push_back(edge);
						if (edge == start) 
							break;
						face = otherface(edge, face);
					}
				}

				ctCurTriangles += triangleFan.size() - 2;
				ctable[iConfig].push_back(triangleFan);
			}
		}

		if(ctCurTriangles > ctMaxTriangles)
			ctMaxTriangles = ctCurTriangles;

		if((int)ctable[iConfig].size() > ctMaxLists)
			ctMaxLists = ctable[iConfig].size();
	}

	ctMaxTriangles = ctMaxTriangles;
	ctMaxLists = ctMaxLists;
}


CMarchCubeTable::~CMarchCubeTable()
{
	for(size_t i=0; i<ctable.size(); i++)
		ctable[i].resize(0);
	ctable.resize(0);
}

//nextClockWiseEdge: return next clockwise edge from given edge around given face
int CMarchCubeTable::nextClockWiseEdge (int edge, int face)
{
	switch (edge) 
	{
	case LB: return (face == L)? LF : BN;
	case LT: return (face == L)? LN : TF;
	case LN: return (face == L)? LB : TN;
	case LF: return (face == L)? LT : BF;
	case RB: return (face == R)? RN : BF;
	case RT: return (face == R)? RF : TN;
	case RN: return (face == R)? RT : BN;
	case RF: return (face == R)? RB : TF;
	case BN: return (face == B)? RB : LN;
	case BF: return (face == B)? LB : RF;
	case TN: return (face == T)? LT : RN;
	case TF: return (face == T)? RT : LF;
	default:
		return -1;
	}
}

//otherface: return face adjoining edge that is not the given face
int CMarchCubeTable::otherface (int edge, int face)
{
	int left = leftface[edge];
	int right = rightface[edge];
	
	return face == left ? right : left;
}

void CMarchCubeTable::writeLookupTable()
{
//{-1, -1, -1, -1, -1, -1, -1, -1},
	DVec<DAnsiStr> content;
	INTLISTS lstPolygons;
	INTLIST triangleFans;
	int a, b, c;

	DAnsiStr str;
	for(int i=0; i<256; i++)
	{
		lstPolygons = getCubeCase(i);
		size_t ctPolygons = lstPolygons.size();

		str = "{";
		int ctTris = 0;
		for(size_t iPolygon=0; iPolygon<ctPolygons; iPolygon++)
		{
			triangleFans = lstPolygons[iPolygon];
			for(size_t iEdge=0; iEdge<triangleFans.size(); iEdge++)
			{
				c = triangleFans[iEdge];
				if (iEdge >= 2) 				
				{
					str += printToAStr("%d, %d, %d, ", c, b, a);			
					ctTris++;
				}
				if (iEdge < 2) 
					a = b;
				b = c;
			}
		}

		if(ctTris < 5)
		{
			for(size_t iPolygon=ctTris; iPolygon<5; iPolygon++)
				str += printToAStr("-1, -1, -1, ");			
		}
		

		str += "-1},";
		content.push_back(str);
	}

	PS::FILESTRINGUTILS::WriteTextFile("c:\\lookupTable.h", content);
}