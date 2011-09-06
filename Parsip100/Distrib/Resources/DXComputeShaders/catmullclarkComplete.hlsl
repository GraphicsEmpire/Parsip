//========================================================================================
//  File: catmullclarkComplete.hlsl
//  Pourya Shirazian
//
//  Intel Corp.
//========================================================================================

//Complete Catmull Clark Subdivision on GPU

struct VERTEX{
	float3 pos;
	uint edge;
};

struct EDGE{
	uint4 selr;
	uint2 lnrn;
};

struct QUADFACE
{
	float3 p1;
	float3 p2;
	float3 p3;
	float3 p4;
};

struct FPEPVP
{
	float3 fp;
	float3 ep;
	float3 vp;		
};


cbuffer cbCS : register( b0 )
{
	uint g_ctFaceSides;
	uint g_ctInputFaces;
	uint g_ctInputEdges;
	uint g_ctInputVertices;
};


bool isSameEdge(EDGE a, EDGE b)
{
	return ((a.selr[0] == b.selr[0] && a.selr[1] == b.selr[1])||
			(a.selr[0] == b.selr[1] && a.selr[1] == b.selr[0]));
}

float3 zero()
{
	float3 zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;
	
	return zero; 
}

//Input buffers
StructuredBuffer<uint4>  inputFaces : register(t0);
StructuredBuffer<EDGE>  inputEdges : register(t1);
StructuredBuffer<VERTEX> inputVertices : register(t2);

RWStructuredBuffer<float3> outputFP : register(u0);
[numthreads(1,1,1)]
void CSFacePoints( uint3 DTid : SV_DispatchThreadID )
{
	uint4 face = inputFaces[DTid.x];
	float3 fp;
	if(g_ctFaceSides == 4)
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos + inputVertices[face.w].pos;
	else
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos;
	fp = fp / (float)g_ctFaceSides;	
	
	outputFP[DTid.x] = fp;
		
}

RWStructuredBuffer<float3> outputEP : register(u0);
[numthreads(1,1,1)]
void CSEdgePoints( uint3 DTid : SV_DispatchThreadID )
{
	//Get the face	
	uint4 face	 = inputFaces[DTid.x];
	
	//Compute FacePoint
	float3 fp;
	if(g_ctFaceSides == 4)
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos + inputVertices[face.w].pos;
	else
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos;
	fp = fp / (float)g_ctFaceSides;		
	//===================================================================================================
	VERTEX start, end;
	EDGE e, original;
	
	uint2 want;
	float3 R, Q;
	float3 arrEdgePoints[4];
	float3 arrVertexPoints[4];
	float3 fpo;
	uint4 otherface;
	uint ctFaces = 0;
	uint idxCurEdge = 0;
	uint idxNextEdge = 0;	
	uint iVertex = 0;
	//Now We will move each control point to a new position based on Catmull Clark's formula
	// S' = (Q/n) + (2R/n) + (S(n-3)/n)
	// n = valence of a point (number of edges that connect to that point	
	// Q = average of surrounding facepoints
	// R = average of all sorround edge mid-points (Not edge points but MID-POINTS)	
	[loop]
	for(iVertex=0; iVertex < g_ctFaceSides; iVertex++)
	{		
		want.x = face[iVertex];
		if(iVertex == g_ctFaceSides - 1)		
			want.y = face[0];
		else
			want.y = face[iVertex + 1];
			
		start  = inputVertices[want.x];
		end    = inputVertices[want.y];
		
		idxCurEdge  = start.edge;
		e			= inputEdges[start.edge];
		
		
		original = e;
		ctFaces  = 0;
				
		//Q = zero();
		//R = zero();
		Q = float3(0.0f, 0.0f, 0.0f);
		R = float3(0.0f, 0.0f, 0.0f);

		//Compute Q and R
		[loop]
		while((ctFaces == 0)||(!isSameEdge(e, original)))
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

			//This edge has our vertex as an endpoint of it			
			if((e.selr[0] == want.x)||(e.selr[1] == want.x))
			{
				if(g_ctFaceSides == 4)
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos + inputVertices[otherface.w].pos;
				else
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos;
				fpo = fpo / (float)g_ctFaceSides;
				//fpo = fpo * 0.25f;
				Q = Q + fpo;
			}

			if((e.selr[0] == want.x || e.selr[1] == want.x)&&
			   (e.selr[0] == want.y || e.selr[1] == want.y))
			{
				arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) * 0.25f;
				
				//arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) / 3.0f;
				
				//Compute EdgeMid point
				//outputEP[idxCurEdge]	   = (start.pos + end.pos) * 0.5f;
				outputEP[idxCurEdge]   = arrEdgePoints[iVertex];
				
			}			
			
			//Update loop variables
			e  = inputEdges[idxNextEdge];				
			idxCurEdge = idxNextEdge;
			ctFaces++;
		}//End While

		float valence = (float)(ctFaces + 1);
		Q = Q / valence;
		R = R / valence;
			
		arrVertexPoints[iVertex] = Q / valence + (2.0f / valence) * R + ((valence - 3.0f) / valence) * start.pos;		
	}//End For iVertex
}


RWStructuredBuffer<float3> outputVP : register(u0);
[numthreads(1,1,1)]
void CSVertexPoints( uint3 DTid : SV_DispatchThreadID )
{
	//Get the face	
	uint4 face	 = inputFaces[DTid.x];
	
	//Compute FacePoint
	float3 fp;
	if(g_ctFaceSides == 4)
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos + inputVertices[face.w].pos;
	else
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos;
	fp = fp / (float)g_ctFaceSides;	
	//outputFP[DTid.x] = fp;
	//===================================================================================================
	VERTEX start, end;
	EDGE e, original;
	
	uint2 want;
	float3 R, Q;
	float3 arrEdgePoints[4];
	float3 arrVertexPoints[4];
	float3 fpo;
	uint4 otherface;
	uint ctFaces = 0;
	uint idxCurEdge = 0;
	uint idxNextEdge = 0;	
	//Now We will move each control point to a new position based on Catmull Clark's formula
	// S' = (Q/n) + (2R/n) + (S(n-3)/n)
	// n = valence of a point (number of edges that connect to that point	
	// Q = average of surrounding facepoints
	// R = average of all sorround edge mid-points (Not edge points but MID-POINTS)	
	[loop]
	for(uint iVertex=0; iVertex < g_ctFaceSides; iVertex++)
	{		
		want.x = face[iVertex];
		if(iVertex == g_ctFaceSides - 1)		
			want.y = face[0];
		else
			want.y = face[iVertex + 1];
			
		start  = inputVertices[want.x];
		end    = inputVertices[want.y];
		
		idxCurEdge  = start.edge;
		e			= inputEdges[start.edge];
		
		
		original = e;
		ctFaces  = 0;
				
		//Q = zero();
		//R = zero();
		Q = float3(0.0f, 0.0f, 0.0f);
		R = float3(0.0f, 0.0f, 0.0f);

		//Compute Q and R
		[loop]
		while((ctFaces == 0)||(!isSameEdge(e, original)))
		{
			R = R + (inputVertices[e.selr[0]].pos + inputVertices[e.selr[1]].pos) * 0.5f;			

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

			//This edge has our vertex as an endpoint of it			
			if((e.selr[0] == want.x)||(e.selr[1] == want.x))
			{
				if(g_ctFaceSides == 4)
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos + inputVertices[otherface.w].pos;
				else
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos;
				fpo = fpo / (float)g_ctFaceSides;
				//fpo = fpo * 0.25f;
				Q = Q + fpo;
			}

			if((e.selr[0] == want.x || e.selr[1] == want.x)&&
			   (e.selr[0] == want.y || e.selr[1] == want.y))
			{
				arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) * 0.25f;
				//arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) / 3.0f;
				
				//Compute EdgeMid point
				//outputEP[idxCurEdge]	   = (start.pos + end.pos) * 0.5f;
				//outputEP[idxCurEdge]	   = arrEdgePoints[iVertex];
				
			}			
			
			//Update loop variables
			e  = inputEdges[idxNextEdge];				
			idxCurEdge = idxNextEdge;
			ctFaces++;
		}//End While

		float valence = (float)(ctFaces + 1);
		Q = Q / valence;
		R = R / valence;
			
		arrVertexPoints[iVertex] = Q / valence + (2.0f / valence) * R + ((valence - 3.0f) / valence) * start.pos;		
		//outputVP[want.x] = start.pos;
		outputVP[want.x] = arrVertexPoints[iVertex];
	}//End For iVertex
}


RWStructuredBuffer<FPEPVP> outputAll : register(u0);
//Instead of computing each stage individually and reading back results. Let's do 
//Everything on GPU
[numthreads(1,1,1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	uint idxCurFace;	
	if(g_ctInputFaces > 65535)
		idxCurFace = DTid.y * 32768 + DTid.x;	
	else
		idxCurFace = DTid.x;
	if(idxCurFace >= g_ctInputFaces) 
		return;

	//Get the face	
	uint4 face	 = inputFaces[idxCurFace];
	
	//Compute FacePoint
	float3 fp;
	if(g_ctFaceSides == 4)
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos + inputVertices[face.w].pos;
	else
		fp = inputVertices[face.x].pos + inputVertices[face.y].pos + inputVertices[face.z].pos;
	fp = fp / (float)g_ctFaceSides;	
	outputAll[idxCurFace].fp = fp;
	//===================================================================================================
	VERTEX start, end;
	EDGE e, original;
	
	uint2 want;
	float3 R, Q;
	float3 fpo;
	uint4 otherface;
	uint ctFaces = 0;
	//uint ctEdges = 0;
	uint idxCurEdge = 0;
	uint idxNextEdge = 0;	
	uint iVertex = 0;
	//Now We will move each control point to a new position based on Catmull Clark's formula
	// S' = (Q/n) + (2R/n) + (S(n-3)/n)
	// n = valence of a point (number of edges that connect to that point	
	// Q = average of surrounding facepoints
	// R = average of all sorround edge mid-points (Not edge points but MID-POINTS)	
	[loop]
	for(iVertex=0; iVertex < g_ctFaceSides; iVertex++)
	{		
		want.x = face[iVertex];
		if(iVertex == g_ctFaceSides - 1)		
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
		Q = float3(0.0f, 0.0f, 0.0f);
		R = float3(0.0f, 0.0f, 0.0f);

		//Compute Q and R
		[loop]
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


				if(g_ctFaceSides == 4)
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos + inputVertices[otherface.w].pos;
				else
					fpo = inputVertices[otherface.x].pos + inputVertices[otherface.y].pos + inputVertices[otherface.z].pos;
				fpo = fpo / (float)g_ctFaceSides;
				//fpo = fpo * 0.25f;
				Q = Q + fpo;
				ctFaces++;
			}

			if((e.selr[0] == want.x || e.selr[1] == want.x)&&
			   (e.selr[0] == want.y || e.selr[1] == want.y))
			{
				//arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) * 0.25f;
				
				//arrEdgePoints[iVertex] = (start.pos + end.pos + fp + fpo) / 3.0f;
				
				//Compute EdgeMid point
				//outputEP[idxCurEdge]	   = (start.pos + end.pos) * 0.5f;			
				//outputAll[idxCurEdge].ep   = (start.pos + end.pos + fp + fpo) * 0.25f;
				outputAll[idxCurEdge].ep   = (inputVertices[e.selr[0]].pos + inputVertices[e.selr[1]].pos + fp + fpo) * 0.25f;
				
				//R = R + (inputVertices[e.selr[0]].pos + inputVertices[e.selr[1]].pos) * 0.5f;
				//ctEdges++;
				
				//R = R + (start.pos + end.pos) * 0.5f;				
				
				//R = R + outputAll[idxCurEdge].ep;				
			}			
			
			//Update loop variables
			e  = inputEdges[idxNextEdge];				
			idxCurEdge = idxNextEdge;						
		}//End While

		float valence = (float)(ctFaces);
		Q = Q / valence;
		R = R / valence;
			
		//arrVertexPoints[iVertex] = Q / valence + (2.0f / valence) * R + ((valence - 3.0f) / valence) * start.pos;		
		//outputVP[want.x] = start.pos;
		outputAll[want.x].vp = Q / valence + (2.0f / valence) * R + ((valence - 3.0f) / valence) * start.pos;		
	}//End For iVertex


/*
	//For each vertex we will create a quad	
	[loop]
	for(iVertex = 0; iVertex < g_ctFaceSides; iVertex++)
	{		
		outputFaces[idxCurFace*g_ctFaceSides + iVertex].p1 = arrVertexPoints[iVertex];
		outputFaces[idxCurFace*g_ctFaceSides + iVertex].p2 = arrEdgePoints[iVertex];
		outputFaces[idxCurFace*g_ctFaceSides + iVertex].p3 = fp;
		if(iVertex == 0)
			outputFaces[idxCurFace*g_ctFaceSides + iVertex].p4 = arrEdgePoints[g_ctFaceSides-1];
		else
			outputFaces[idxCurFace*g_ctFaceSides + iVertex].p4 = arrEdgePoints[iVertex-1];
	}	
	*/
}


