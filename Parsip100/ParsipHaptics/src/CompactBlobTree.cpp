#include "CompactBlobTree.h"
#include "PS_BlobTree/include/BlobTreeLibraryAll.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "_GlobalFunctions.h"


int COMPACTBLOBTREE::convert( CBlobTree* root)
{
	if(root == NULL)
	{
		ReportError("Invalid input tree.");
		FlushAllErrors();
		return -1;
	}
	return convert(root, -1);
}

int COMPACTBLOBTREE::convert(CBlobTree* root, int parentID/*, const CMatrix& mtxBranch*/)
{
	int curID = -1;
	if(root->isOperator())
	{			
		if(ctOps >= MAX_BLOB_ENTRIES)
		{
			ReportError("Exceeded maximum number of allowed Operators");
			FlushAllErrors();
		}

		curID = ctOps;
		ctOps++;

		ops[curID].type = root->getNodeType();
		ops[curID].params.zero();
		ops[curID].parent = parentID;	

				
                ops[curID].octLo = vec4f(root->getOctree().lower, 0.0f);
                ops[curID].octHi = vec4f(root->getOctree().upper, 0.0f);

	
		int kidID;
		/*
	
		if(root->countChildren() > MAX_COMPACT_KIDS_COUNT)
		{
			CBlobTree* clonned = cloneNode(root);
			for(size_t i=MAX_COMPACT_KIDS_COUNT-1; i<root->countChildren();i++)			
				clonned->addChild(root->getChild(i));
			
			while(root->isChildIndex(MAX_COMPACT_KIDS_COUNT-1))
				root->detachChild(MAX_COMPACT_KIDS_COUNT-1);
			root->addChild(clonned);

			if(root->countChildren() != MAX_COMPACT_KIDS_COUNT)
			{
				DAnsiStr strMsg = printToAStr("Exceeded maximum kids count. Expected at most %d, Recv: %d", MAX_COMPACT_KIDS_COUNT, root->countChildren());
				ReportError(strMsg.ptr());
				FlushAllErrors();
			}
		}
		*/

		//Process kids and add them
		//CMatrix mtxCurrent = mtxBranch;
		//mtxCurrent.multiply(root->getTransform().getBackwardMatrix());		

		ops[curID].ctKids = root->countChildren();
		ops[curID].kidIds.resize(root->countChildren());
		for(size_t i=0;i<root->countChildren(); i++)
		{			
			kidID = convert(root->getChild(i), curID );
			if(root->getChild(i)->isOperator())
				kidID += MAX_BLOB_ENTRIES;
			ops[curID].kidIds[i] = kidID;		
		}

		switch(root->getNodeType())
		{
		case(bntOpRicciBlend):
			{
				CRicciBlend* ricci = dynamic_cast<CRicciBlend*>(root);
				//cfg->writeFloat(strNodeName, "power", ricci->getN());
				float n = ricci->getN();
				ops[curID].params.x = n;
				if(n != 0.0f)
					ops[curID].params.y = 1.0f / n;
				
				break;
			}
		case(bntOpWarpTwist):
			{
				CWarpTwist* twist = dynamic_cast<CWarpTwist*>(root);
				//cfg->writeFloat(strNodeName, "factor", twist->getWarpFactor());
				//cfg->writeInt(strNodeName, "axis", static_cast<int>(twist->getMajorAxis()));
				ops[curID].params.x = twist->getWarpFactor();
				ops[curID].params.y = static_cast<float>(twist->getMajorAxis());
				break;
			}
		case(bntOpWarpTaper):
			{
				CWarpTaper* taper = dynamic_cast<CWarpTaper*>(root);
				//cfg->writeFloat(strNodeName, "factor", taper->getWarpFactor());
				//cfg->writeInt(strNodeName, "base axis", static_cast<int>(taper->getAxisAlong()));
				//cfg->writeInt(strNodeName, "taper axis", static_cast<int>(taper->getAxisTaper()));
				ops[curID].params.x = taper->getWarpFactor();
				ops[curID].params.y = static_cast<float>(taper->getAxisAlong());
				ops[curID].params.z = static_cast<float>(taper->getAxisTaper());
				break; 
			}
		case(bntOpWarpBend):
			{
				CWarpBend* bend = dynamic_cast<CWarpBend*>(root);
				//cfg->writeFloat(strNodeName, "rate", bend->getBendRate());
				//cfg->writeFloat(strNodeName, "center", bend->getBendCenter());
				//cfg->writeFloat(strNodeName, "left bound", bend->getBendRegion().left);
				//cfg->writeFloat(strNodeName, "right bound", bend->getBendRegion().right);
				ops[curID].params.x = bend->getBendRate();
				ops[curID].params.y = bend->getBendCenter();
				ops[curID].params.z = bend->getBendRegion().left;
				ops[curID].params.w = bend->getBendRegion().right;
				break;
			}
		case(bntOpWarpShear):
			{
				CWarpShear* shear = dynamic_cast<CWarpShear*>(root);
				//cfg->writeFloat(strNodeName, "factor", shear->getWarpFactor());
				//cfg->writeInt(strNodeName, "base axis", static_cast<int>(shear->getAxisAlong()));
				//cfg->writeInt(strNodeName, "shear axis", static_cast<int>(shear->getAxisDependent()));
				ops[curID].params.x = shear->getWarpFactor();
				ops[curID].params.y = static_cast<float>(shear->getAxisAlong());
				ops[curID].params.z = static_cast<float>(shear->getAxisDependent());
				break;
			}
			
		default:
			{
				
				//char chrName[MAX_NAME_LEN];
				//root->getName(chrName);
				//DAnsiStr strMsg = printToAStr("Operator %s has not been implemented in compact mode yet!", chrName);
				//ReportError(strMsg.ptr());
				//FlushAllErrors();
			}			
		}

		return curID;
	}
	else
	{				
		if(ctPrims >= MAX_BLOB_ENTRIES)
		{
			ReportError("Exceeded maximum number of allowed primitives");
			FlushAllErrors();
		}

		curID = ctPrims;
		ctPrims++;

		CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
		vec4f c = sprim->getColor();
		COctree oct = sprim->getOctree();		
		
		float row[4];
	
		prims[curID].skelet = sprim->getSkeleton()->getType();
		prims[curID].parent = parentID;
		prims[curID].color.set(c.x, c.y, c.z, c.w);
		
		oct.lower.get(row); 
		prims[curID].octLo.set(row[0], row[1], row[2]);
		
		oct.upper.get(row); 
		prims[curID].octHi.set(row[0], row[1], row[2]);

		//Load Matrix
		//Here we post multiply current primitive transformation matrix to 
		//that of branch
		//CMatrix mtxBackward = mtxBranch;
		//mtxBackward.multiply(sprim->getTransform().getBackwardMatrix());
		CMatrix mtxBackward = sprim->getTransform().getBackwardMatrix();
		mtxBackward.getRow(row, 0);
		prims[curID].mtxBackwardR0.set(row[0], row[1], row[2], row[3]);
		mtxBackward.getRow(row, 1);
		prims[curID].mtxBackwardR1.set(row[0], row[1], row[2], row[3]);
		mtxBackward.getRow(row, 2);
		prims[curID].mtxBackwardR2.set(row[0], row[1], row[2], row[3]);
		mtxBackward.getRow(row, 3);
		prims[curID].mtxBackwardR3.set(row[0], row[1], row[2], row[3]);

		switch(sprim->getSkeleton()->getType())
		{
		case(sktPoint):
			{
				CSkeletonPoint* skeletPoint = reinterpret_cast<CSkeletonPoint*>(sprim->getSkeleton());			
				//cfg->writeVec3f(strNodeName, "position", skeletPoint->getPosition());
				vec3f pos = skeletPoint->getPosition();
				prims[curID].pos.set(pos.x, pos.y, pos.z, 0.0f);
			}
			break;
		case(sktLine):
			{			
				CSkeletonLine* skeletLine = reinterpret_cast<CSkeletonLine*>(sprim->getSkeleton());			
				//cfg->writeVec3f(strNodeName, "start", skeletLine->getStartPosition());
				//cfg->writeVec3f(strNodeName, "end", skeletLine->getEndPosition());
				vec3f s = skeletLine->getStartPosition();
				vec3f e = skeletLine->getEndPosition();
				prims[curID].res1.set(s.x, s.y, s.z);
				prims[curID].res2.set(e.x, e.y, e.z);
			}
			break;			
		case(sktRing):
			{			
				CSkeletonRing* skeletRing = reinterpret_cast<CSkeletonRing*>(sprim->getSkeleton());			
				//cfg->writeVec3f(strNodeName, "position", skeletRing->getPosition());
				//cfg->writeVec3f(strNodeName, "direction", skeletRing->getDirection());
				//cfg->writeFloat(strNodeName, "radius", skeletRing->getRadius());
				vec3f p = skeletRing->getPosition();
				vec3f d = skeletRing->getDirection();
				float r = skeletRing->getRadius();
				prims[curID].pos.set(p.x, p.y, p.z);
				prims[curID].dir.set(d.x, d.y, d.z);
				prims[curID].res1.set(r);
				prims[curID].res2.set(r*r);
			}
			break;
		case(sktDisc):
			{			
				CSkeletonDisc* skeletDisc = reinterpret_cast<CSkeletonDisc*>(sprim->getSkeleton());			
				//cfg->writeVec3f(strNodeName, "position", skeletDisc->getPosition());
				//cfg->writeVec3f(strNodeName, "direction", skeletDisc->getDirection());
				//cfg->writeFloat(strNodeName, "radius", skeletDisc->getRadius());
				vec3f p = skeletDisc->getPosition();
				vec3f d = skeletDisc->getDirection();
				float r = skeletDisc->getRadius();
				prims[curID].pos.set(p.x, p.y, p.z);
				prims[curID].dir.set(d.x, d.y, d.z);
				prims[curID].res1.set(r);
				prims[curID].res2.set(r*r);
			}
			break;
		case(sktCylinder):
			{			
				CSkeletonCylinder* skeletCyl = reinterpret_cast<CSkeletonCylinder*>(sprim->getSkeleton());
				//cfg->writeVec3f(strNodeName, "position", skeletCyl->getPosition());
				//cfg->writeVec3f(strNodeName, "direction", skeletCyl->getDirection());
				//cfg->writeFloat(strNodeName, "radius", skeletCyl->getRadius());
				//cfg->writeFloat(strNodeName, "height", skeletCyl->getHeight());
				vec3f p = skeletCyl->getPosition();
				vec3f d = skeletCyl->getDirection();				
				prims[curID].pos.set(p.x, p.y, p.z);
				prims[curID].dir.set(d.x, d.y, d.z);
				prims[curID].res1.set(skeletCyl->getRadius());
				prims[curID].res2.set(skeletCyl->getHeight());
			}
			break;			

		case(sktCube):
			{			
				CSkeletonCube* skeletCube = reinterpret_cast<CSkeletonCube*>(sprim->getSkeleton());
				//cfg->writeVec3f(strNodeName, "position", skeletCube->getPosition());
				//cfg->writeFloat(strNodeName, "side", skeletCube->getSide());
				vec3f p = skeletCube->getPosition();
				float side = skeletCube->getSide();
				prims[curID].pos.set(p.x, p.y, p.z);
				prims[curID].res1.set(side);
			}
			break;	
		case(sktTriangle):
			{
				CSkeletonTriangle* skeletTriangle = reinterpret_cast<CSkeletonTriangle*>(sprim->getSkeleton());
				//cfg->writeVec3f(strNodeName, "corner0", skeletTriangle->getTriangleCorner(0));
				//cfg->writeVec3f(strNodeName, "corner1", skeletTriangle->getTriangleCorner(1));
				//cfg->writeVec3f(strNodeName, "corner2", skeletTriangle->getTriangleCorner(2));
				vec3f p0 = skeletTriangle->getTriangleCorner(0);
				vec3f p1 = skeletTriangle->getTriangleCorner(1);
				vec3f p2 = skeletTriangle->getTriangleCorner(2);

				prims[curID].pos.set(p0.x, p0.y, p0.z);
				prims[curID].res1.set(p1.x, p1.y, p1.z);
				prims[curID].res2.set(p2.x, p2.y, p2.z);
			}
			break;
		default:
			{
				char chrName[MAX_NAME_LEN];
				root->getName(chrName);
				DAnsiStr strMsg = printToAStr("Primitive %s has not been implemented in compact mode yet!", chrName);
				ReportError(strMsg.ptr());
				FlushAllErrors();
			}
		}
		return curID;
	}
}

PS::MATH::vec4f COMPACTBLOBTREE::normal( const vec4f& p, float inFieldValue, float delta )
{
	float arrN[4];
	vec4f n;
	vec4f insFieldValue(inFieldValue, inFieldValue, inFieldValue, 0.0f);	
	vec4f invsDelta(-1.0f / delta);

	arrN[0] = fieldvalue(p + vec4f(delta, 0.0f, 0.0f, 0.0f));
	arrN[1] = fieldvalue(p + vec4f(0.0f, delta, 0.0f, 0.0f));
	arrN[2] = fieldvalue(p + vec4f(0.0f, 0.0f, delta, 0.0f));
	arrN[3] = 0.0f;	
	n.set(arrN);

	n -= insFieldValue;
	n *= invsDelta;
	n.normalizeXYZ();
	return n;
}

PS::MATH::vec4f COMPACTBLOBTREE::fieldValueAndGradient(const vec4f& p, float delta )
{
	float arrN[4];
	float fp = fieldvalue(p);

	vec4f res;
	vec4f insFieldValue(fp, fp, fp, 0.0f);	
	vec4f invsDelta(1.0f / delta);
	
	//arrN[0] = fieldvalue(p + _mm_shuffle_ps(sDelta, sDelta, _MM_SHUFFLE(3, 3, 3, 0)));
	//arrN[1] = fieldvalue(p + _mm_shuffle_ps(sDelta, sDelta, _MM_SHUFFLE(3, 3, 0, 3)));
	//arrN[2] = fieldvalue(p + _mm_shuffle_ps(sDelta, sDelta, _MM_SHUFFLE(3, 0, 3, 3)));
	arrN[0] = fieldvalue(p + vec4f(delta, 0.0f, 0.0f, 0.0f));
	arrN[1] = fieldvalue(p + vec4f(0.0f, delta, 0.0f, 0.0f));
	arrN[2] = fieldvalue(p + vec4f(0.0f, 0.0f, delta, 0.0f));
	arrN[3] = 0.0f;	

	res.set(arrN[0], arrN[1], arrN[2], arrN[3]);
	res -= insFieldValue;
	res *= invsDelta;
	res.w = fp;
	return res;
}

float COMPACTBLOBTREE::fieldvalue(const vec4f& p, float* lpStoreFVOp, float* lpStoreFVPrim)
{	
	vec4f pp = p;
	pp[3] = 0.0f;

	if(ctOps > 0)	
		return fieldvalueOp(pp, 0, lpStoreFVOp, lpStoreFVPrim);		
	else if(ctPrims > 0)	
		return fieldvaluePrim(pp, 0, lpStoreFVPrim);		
	else
		return 0.0f;
}

float COMPACTBLOBTREE::fieldvalueOp(const vec4f& p, int id, float* lpStoreFVOp, float* lpStoreFVPrim)
{
	float fv[MAX_BLOB_ENTRIES];
	float res  = 0.0f;	
	int ctKids = ops[id].ctKids;
	int kidID  = 0;
	vec4f pWarped = p;
	//////////////////////////////////////////////////////////////////////////
	if(lpStoreFVOp)
		lpStoreFVOp[id] = 0.0f;
	vec4f lo = ops[id].octLo;
	vec4f hi = ops[id].octHi;
	if((p.x < lo.x)||(p.y < lo.y)||(p.z < lo.z))
		return 0.0f;
	if((p.x > hi.x)||(p.y > hi.y)||(p.z > hi.z))
		return 0.0f;
	//////////////////////////////////////////////////////////////////////////
	//Check if we can satisfy from fieldvalue from internal cache
	/*
	if(ops[id].fvCache.ctFilled > 0)
	{
		float hashVal = p.x + p.y + p.z;
		for(int i=0;i<ops[id].fvCache.ctFilled;i++)
		{
			if(ops[id].fvCache.hashVal[i] == hashVal)
			{
				if(ops[id].fvCache.xyzf[i].xyz() == p.xyz())
					return ops[id].fvCache.xyzf[i].w;
			}
		}
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	//Process Warp unary ops first since we need to warp the space before affine 
	//transformations effect
	BlobNodeType nodetype = ops[id].type;
	bool bWarpNode =((nodetype == bntOpWarpBend) || (nodetype == bntOpWarpShear) || 
		(nodetype == bntOpWarpTwist) || (nodetype == bntOpWarpTaper));

	if(bWarpNode)
	{
		
		//Warp the space
		switch(nodetype)
		{
		case(bntOpWarpBend):
			{
				vec4f param = ops[id].params;
				pWarped = warpBend(pWarped, param.x, param.y, CInterval(param.z, param.w));			
			}
			break;
		case(bntOpWarpTwist):
			{
				vec4f param = ops[id].params;
				pWarped = warpTwist(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)));			
			}
			break;
		case(bntOpWarpTaper):
			{
				vec4f param = ops[id].params;
				pWarped = warpTaper(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)), static_cast<MajorAxices>((int)(param.z)));			
			}
			break;
		case(bntOpWarpShear):
			{
				vec4f param = ops[id].params;
				pWarped = warpShear(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)), static_cast<MajorAxices>((int)(param.z)));			
			}
			break;
		}
	}

	//Processing other operators
	for(int i=0; i<ctKids; i++)
	{
		kidID = ops[id].kidIds[i];
		if(kidID >= MAX_BLOB_ENTRIES)
			fv[i] = fieldvalueOp(pWarped, kidID - MAX_BLOB_ENTRIES, lpStoreFVOp, lpStoreFVPrim);
		else
			fv[i] = fieldvaluePrim(pWarped, kidID, lpStoreFVPrim);
	}

	//Processing FieldValues 
	switch(ops[id].type)
	{
	case(bntOpBlend):
		{
			for(int i=0; i<ctKids; i++)
				res += fv[i];		
		}
		break;
	case(bntOpRicciBlend):
		{
			for(int i=0; i<ctKids; i++)
				res += powf(fv[i], ops[id].params.x);		
			res = powf(res, ops[id].params.y);
		}
		break;
	case(bntOpUnion):
		{
			res = fv[0];
			for(int i=1; i<ctKids; i++)
			{
				if(fv[i] > res)
					res = fv[i];					
			}
		}
		break;
	case(bntOpIntersect):
		{
			res = fv[0];
			for(int i=1; i<ctKids; i++)
			{
				if(fv[i] < res)
					res = fv[i];					
			}
		}
		break;	
	case(bntOpDif):	
		{
			res = fv[0];
			for(int i=1; i<ctKids; i++)
			{
				res = MATHMIN(res, MAX_FIELD_VALUE - fv[i]);
			}
		}
		break;	
	case(bntOpSmoothDif):	
		{
			res = fv[0];
			for(int i=1; i<ctKids; i++)
			{
				res *= (MAX_FIELD_VALUE - fv[i]);
			}
		}
		break;	
	case(bntOpWarpBend):
		{
			res = fv[0];
		}
		break;
	case(bntOpWarpTwist):
		{
			res = fv[0];
		}
		break;
	case(bntOpWarpTaper):
		{
			res = fv[0];
		}
		break;
	case(bntOpWarpShear):
		{
			res = fv[0];
		}	
		break;
	default:
		{
			DAnsiStr strMsg = printToAStr("That operator is not been implemented yet! Op = %d", ops[id].type);
			ReportError(strMsg.ptr());
			FlushAllErrors();
		}
	}

	//Save fieldvalue here for reference
	/*
	if(res > TREENODE_CACHE_STORETHRESHOLD)
	{
		int pos = (ops[id].fvCache.ctFilled % MAX_TREENODE_FVCACHE);
		ops[id].fvCache.xyzf[pos]	  = vec4f(p.x, p.y, p.z, res);
		ops[id].fvCache.hashVal[pos]  = p.x + p.y + p.z; 
		ops[id].fvCache.ctFilled++;
	}
	*/
	if(lpStoreFVOp)
		lpStoreFVOp[id] = res;

	return res;
}

float COMPACTBLOBTREE::fieldvaluePrim(const vec4f& p, int id, float* lpStoreFVPrim)
{	
	vec4f pp = p;
	pp.w = 1.0f;

	//////////////////////////////////////////////////////////////////////////
	//Check if we can satisfy from fieldvalue from internal cache
	/*
	if(prims[id].fvCache.ctFilled > 0)
	{
		float hashVal = p.x + p.y + p.z;
		for(int i=0;i<prims[id].fvCache.ctFilled;i++)
		{
			if(prims[id].fvCache.hashVal[i] == hashVal)
			{
				if(prims[id].fvCache.xyzf[i].xyz() == p.xyz())
					return prims[id].fvCache.xyzf[i].w;
			}
		}
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	vec3f pn;
	//Apply Affine Matrix transformation	
	pn.x = prims[id].mtxBackwardR0.dot(pp);
	pn.y = prims[id].mtxBackwardR1.dot(pp);
	pn.z = prims[id].mtxBackwardR2.dot(pp);
	
	//switch(prims)
	float dd;
	switch(prims[id].skelet)
	{
	case sktPoint:
		{
			dd = pn.dist2(prims[id].pos.xyz());
		}
		break;
	case sktCylinder:
		{							
			vec3f pos = pn - prims[id].pos.xyz();
			
			float y = pos.dot(prims[id].dir.xyz());
			//float x = MATHMAX(0.0f, sqrtf(pos.length2() - y*y) - prims[id].res1.x);
			float x = maxf(0.0f, sqrtf(pos.length2() - y*y) - prims[id].res1.x);

			//Make y 0.0 if it is positive and less than height 
			// For Hemispherical caps
			if(y > 0.0f)				
				y = maxf(0.0f, y - prims[id].res2.x);

			dd = x*x + y*y;
		}
		break;
	case sktTriangle:
		{
			vec3f vertices[3];
			vertices[0] = prims[id].pos.xyz();
			vertices[1] = prims[id].res1.xyz();
			vertices[2] = prims[id].res2.xyz();			
			vec3f outClosest, outBaryCoords;
			dd = ComputeTriangleSquareDist(vertices, pn, outClosest, outBaryCoords);			
		}
		break;

	case sktCube:
		{			
			vec3f center = prims[id].pos.xyz();
			float side   = prims[id].res1.x;
		
			vec3f dif = pn - center;
			float dist2 = 0.0f;
			float delta;

			float projected;

			//Along X
			projected = dif.dot(vec3f(1.0f, 0.0f, 0.0f));
			if(projected < -1.0f * side)
			{
				delta = projected + side;
				dist2 += delta*delta;
			}
			else if (projected > side)
			{
				delta = projected - side;
				dist2 += delta*delta;					
			}

			//Along Y
			projected = dif.dot(vec3f(0.0f, 1.0f, 0.0f));
			if(projected < -1.0f * side)
			{
				delta = projected + side;
				dist2 += delta*delta;					
			}
			else if (projected > side)
			{
				delta = projected - side;
				dist2 += delta*delta;					
			}

			//Along Z
			projected = dif.dot(vec3f(0.0f, 0.0f, 1.0f));
			if(projected  < -1.0f * side)
			{
				delta = projected + side;
				dist2 += delta*delta;					
			}
			else if (projected > side)
			{
				delta = projected - side;
				dist2 += delta*delta;					
			}

			dd = dist2;
		}
		break;
	case sktDisc:
		{
			vec3f n = prims[id].dir.xyz();
			vec3f c = prims[id].pos.xyz();		
			float r = prims[id].res1[0];
			vec3f dir = pn - c - (n.dot(pn - c))*n;

			//Check if Q lies on center or p is just above center		
			if(dir.length() <= r)
			{
				dd = Absolutef((pn - c).length2() - dir.length2());
			}
			else
			{
				dir.normalize();
				vec3f x = c + r * dir;
				dd = (x - pn).length2();		
			}
		}
		break;

	case sktRing:
		{
			vec3f n = prims[id].dir.xyz();
			vec3f c = prims[id].pos.xyz();			
			float r = prims[id].res1[0];
			vec3f dir = pn - c - (n.dot(pn - c))*n;

			//Check if Q lies on center or p is just above center
			if(dir.isZero())
			{
				//r^2 + |p-c|^2
				dd = r*r + (pn - c).length2();
			}
			else
			{
				dir.normalize();
				vec3f x = c + r * dir;
				dd = (x - pn).length2();		
			}
		}
		break;
	case sktLine:
		{
			vec3f s = prims[id].res1.xyz();
			vec3f e = prims[id].res2.xyz();
			vec3f nearestPoint = NearestPointInLineSegment(pn, s, e);
			dd = nearestPoint.dist2(pn);
		}
		break;
	default:
		ReportError("That skeleton is not been implemented yet!");
		FlushAllErrors();
	}

	float res = ComputeWyvillFieldValueSquare(dd);

	//Save fieldvalue here for reference
	/*
	if(res > TREENODE_CACHE_STORETHRESHOLD)
	{	
		int pos = (prims[id].fvCache.ctFilled % MAX_TREENODE_FVCACHE);
		prims[id].fvCache.xyzf[pos]		 = vec4f(p.x, p.y, p.z, res);
		prims[id].fvCache.hashVal[pos]   = p.x + p.y + p.z; 
		prims[id].fvCache.ctFilled++;
	}
	*/
	if(lpStoreFVPrim)
		lpStoreFVPrim[id] = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////
PS::MATH::vec4f COMPACTBLOBTREE::baseColor( const vec4f& p, float* lpStoreFVOp, float* lpStoreFVPrim )
{	
	if(ctOps > 0)	
		return baseColorOp(p, 0, lpStoreFVOp, lpStoreFVPrim);		
	else if(ctPrims > 0)	
		return prims[0].color;	
	else
	{
		static vec4f black;
		return black;
	}	
}

vec4f COMPACTBLOBTREE::baseColorOp(const vec4f& p, int id, float* lpStoreFVOp, float* lpStoreFVPrim )
{
	float arrFV[MAX_BLOB_ENTRIES];
	vec4f arrCL[MAX_BLOB_ENTRIES];
	
	float resFV = 0.0f;
	vec4f resCL;
	int kidID = 0;
	int ctKids = ops[id].ctKids;
	if(ctKids == 0) 
		return resCL;

	if((lpStoreFVOp != NULL)&&(lpStoreFVPrim != NULL))
	{
		//Get them from Cached values
		for(int i=0; i<ctKids; i++)
		{
			kidID = ops[id].kidIds[i];
			if(kidID >= MAX_BLOB_ENTRIES)
			{
				arrCL[i] = baseColorOp(p, kidID - MAX_BLOB_ENTRIES, lpStoreFVOp, lpStoreFVPrim);
				arrFV[i] = lpStoreFVOp[kidID - MAX_BLOB_ENTRIES];
			}
			else
			{
				arrCL[i] = prims[kidID].color;
				arrFV[i] = lpStoreFVPrim[kidID];
			}
		}
	}
	else
	{
		for(int i=0; i<ctKids; i++)
		{
			kidID = ops[id].kidIds[i];
			if(kidID >= MAX_BLOB_ENTRIES)
			{
				arrCL[i] = baseColorOp(p, kidID - MAX_BLOB_ENTRIES, lpStoreFVOp, lpStoreFVPrim);
				arrFV[i] = fieldvalueOp(p, kidID - MAX_BLOB_ENTRIES, lpStoreFVOp, lpStoreFVPrim);			
			}
			else
			{
				arrCL[i] = prims[kidID].color;
				arrFV[i] = fieldvaluePrim(p, kidID, lpStoreFVPrim);			
			}
		}
	}

	float temp = 0.0f;
	vec4f stemp;

	switch(ops[id].type)
	{
	case(bntOpBlend):
		{
			for(int i=0; i<ctKids; i++)
			{
				temp = arrFV[i];
				if(temp > 0.0f)
				{
					resCL += arrCL[i] * vec4f(temp);
					resFV += arrFV[i];							
				}
			}

			if(resFV == 0.0f)
				resCL = arrCL[0];
			else							
				resCL = resCL * vec4f(1.0f / resFV);			
		}
		break;
	case(bntOpRicciBlend):
		{
			for(int i=0; i<ctKids; i++)
			{
				temp = arrFV[i];
				if(temp > 0.0f)
				{
					resCL += arrCL[i] * vec4f(temp);
					resFV += arrFV[i];							
				}
			}

			if(resFV == 0.0f)
				resCL = arrCL[0];
			else							
				resCL = resCL * vec4f(1.0 / resFV);			
		}
		break;
	case(bntOpUnion):
		{
			kidID = 0;
			temp = arrFV[0];
			for(int i=1; i<ctKids; i++)
			{				
				if(arrFV[i] > temp)
				{
					temp = arrFV[i];
					kidID = i;
				}
			}
			resCL = arrCL[kidID];
		}
		break;
	case(bntOpIntersect):
		{
			kidID = 0;
			temp = arrFV[0];
			for(int i=1; i<ctKids; i++)
			{				
				if(arrFV[i] < temp)
				{
					temp = arrFV[i];
					kidID = i;
				}
			}
			resCL = arrCL[kidID];
		}
		break;	
	case(bntOpDif):	
		{
			kidID = 0;
			temp = arrFV[0];
			
			float curField;
			for(int i = 1; i < ctKids; i++)
			{
				curField = MAX_FIELD_VALUE - arrFV[i];
				if(curField < temp)
				{
					temp = curField;
					kidID = i;
				}
			}
			resCL = arrCL[kidID];
		}
		break;	
	case(bntOpSmoothDif):	
		{
			kidID = 0;
			temp = arrFV[0];

			float curField;
			for(int i = 1; i < ctKids; i++)
			{
				curField = MAX_FIELD_VALUE - arrFV[i];
				if(curField < temp)
				{
					temp = curField;
					kidID = i;
				}
			}
			resCL = arrCL[kidID];
		}
		break;	
	case(bntOpWarpBend):
		{
			resCL = arrCL[0];
		}
		break;
	case(bntOpWarpTwist):
		{
			resCL = arrCL[0];
		}
		break;
	case(bntOpWarpTaper):
		{
			resCL = arrCL[0];
		}
		break;
	case(bntOpWarpShear):
		{
			resCL = arrCL[0];
		}	
		break;
	default:
		ReportError("That operator is not been implemented yet!");
		FlushAllErrors();
	}

	return resCL;

}

void COMPACTBLOBTREE::copyFrom( const COMPACTBLOBTREE& rhs )
{
	this->ctOps = rhs.ctOps;
	this->ctPrims = rhs.ctPrims;
		
	//Copy Ops
	for(int i=0;i<rhs.ctOps;i++)
	{
		this->ops[i].params = rhs.ops[i].params;
		this->ops[i].parent = rhs.ops[i].parent;
		this->ops[i].type  = rhs.ops[i].type;
		this->ops[i].octLo = rhs.ops[i].octLo;
		this->ops[i].octHi = rhs.ops[i].octHi;

		this->ops[i].ctKids = rhs.ops[i].ctKids;
		this->ops[i].kidIds.copyFrom(rhs.ops[i].kidIds);
		//for(int j=0; j<8;j++)
			//this->ops[i].kidIds[j] = rhs.ops[i].kidIds[j];
	}

	//Copy Prims
	for(int i=0;i<rhs.ctPrims;i++)
	{
		this->prims[i].skelet = rhs.prims[i].skelet;
		this->prims[i].parent = rhs.prims[i].parent;
		this->prims[i].color  = rhs.prims[i].color;
		this->prims[i].pos	  = rhs.prims[i].pos;
		this->prims[i].dir   = rhs.prims[i].dir;
		this->prims[i].res1  = rhs.prims[i].res1;
		this->prims[i].res2  = rhs.prims[i].res2;
		this->prims[i].octLo = rhs.prims[i].octLo;
		this->prims[i].octHi = rhs.prims[i].octHi;

		this->prims[i].mtxBackwardR0 = rhs.prims[i].mtxBackwardR0;
		this->prims[i].mtxBackwardR1 = rhs.prims[i].mtxBackwardR1;
		this->prims[i].mtxBackwardR2 = rhs.prims[i].mtxBackwardR2;
		this->prims[i].mtxBackwardR3 = rhs.prims[i].mtxBackwardR3;
	}

}

PS::MATH::vec4f COMPACTBLOBTREE::warpBend( const vec4f& pin, float bendRate, float bendCenter, const CInterval& bendRegion)
{
	vec4f pout;
	float k = bendRate;
	float kDiv = 1.0f/k;
	float y0 = bendCenter;


	//Compute where yhat is:
	float yh = 0.0f;
	if(pin.y <= bendRegion.left)
		yh = bendRegion.left;
	else if((pin.y > bendRegion.left)&&(pin.y < bendRegion.right))
		yh = pin.y;
	else if(pin.y >= bendRegion.right)
		yh = bendRegion.right;

	float theta = k*(yh - y0);
	float ct = cos(theta);
	float st = sin(theta);

	pout.x = pin.x;
	if(bendRegion.isInside(pin.y))		
		pout.y = -st*(pin.z - kDiv) + y0;		
	else if(pin.y < bendRegion.left)		
		pout.y = -st*(pin.z - kDiv) + y0 + ct*(pin.y - bendRegion.left);		
	else if(pin.y > bendRegion.right)
		pout.y = -st*(pin.z - kDiv) + y0 + ct*(pin.y - bendRegion.right);


	if(bendRegion.isInside(pin.y))		
		pout.z = ct*(pin.z - kDiv) + kDiv;		
	else if(pin.y < bendRegion.left)		
		pout.z = ct*(pin.z - kDiv) + kDiv + st*(pin.y - bendRegion.left);		
	else if(pin.y > bendRegion.right)
		pout.z = ct*(pin.z - kDiv) + kDiv + st*(pin.y - bendRegion.right);

	return pout;
}

PS::MATH::vec4f COMPACTBLOBTREE::warpTwist( const vec4f& pin, float factor, MajorAxices axis )
{
	float theta = 0.0f;
	vec4f pout;

	switch(axis)
	{
	case(xAxis):		
		theta = pin.x * factor;
		pout.x = pin.x;
		pout.y = pin.y*cos(theta) - pin.z*sin(theta);
		pout.z = pin.y*sin(theta) + pin.z*cos(theta);
		break;

	case(yAxis):
		theta = pin.y * factor;
		pout.x = pin.x*cos(theta) - pin.z*sin(theta);
		pout.y = pin.y;
		pout.z = pin.x*sin(theta) + pin.z*cos(theta);
		break;

	case(zAxis):
		theta = pin.z * factor;
		pout.x = pin.x*cos(theta) - pin.y*sin(theta);
		pout.y = pin.x*sin(theta) + pin.y*cos(theta);
		pout.z = pin.z;
		break;
	}

	return pout;
}

vec3f COMPACTBLOBTREE::taperAlongX(vec3f p, float factor, MajorAxices axisTaper)
{
	vec3f result;
	switch(axisTaper)
	{
	case(yAxis):
		result = vec3f(p.x, p.y * (1 + p.x * factor), p.z);
		break;
	case(zAxis):
		result = vec3f(p.x, p.y, p.z * (1 + p.x * factor));
		break;
	default:
		result = vec3f(p.x, p.y * (1 + p.x * factor), p.z);
		break;
	}

	return result;
}

vec3f COMPACTBLOBTREE::taperAlongY(vec3f p, float factor, MajorAxices axisTaper)
{
	vec3f result;
	switch(axisTaper)
	{
	case(xAxis):
		result = vec3f(p.x * (1 + p.y * factor), p.y, p.z);
		break;
	case(zAxis):
		result = vec3f(p.x, p.y, p.z * (1 + p.y * factor));
		break;
	default:
		result = vec3f(p.x * (1 + p.y * factor), p.y, p.z);
		break;
	}

	return result;
}

vec3f COMPACTBLOBTREE::taperAlongZ(vec3f p, float factor, MajorAxices axisTaper)
{
	vec3f result;
	switch(axisTaper)
	{
	case(xAxis):
		result = vec3f(p.x * (1 + p.z * factor), p.y, p.z);
		break;
	case(zAxis):
		result = vec3f(p.x, p.y * (1 + p.z * factor), p.z);
		break;
	default:
		result = vec3f(p.x * (1 + p.z * factor), p.y, p.z);
		break;
	}

	return result;
}



PS::MATH::vec4f COMPACTBLOBTREE::warpTaper( const vec4f& pin, float factor, MajorAxices axisAlong, MajorAxices axisTaper )
{
	vec3f res = pin.xyz();
	switch(axisAlong)
	{
	case(xAxis):
		res = taperAlongX(res, factor, axisTaper);
		break;
	case(yAxis):
		res = taperAlongY(res, factor, axisTaper);
		break;
	case(zAxis):
		res = taperAlongZ(res, factor, axisTaper);
	}		
	return vec4f(res.x, res.y, res.z);

}
//////////////////////////////////////////////////////////////////////////
vec3f COMPACTBLOBTREE::shearAlongX(vec3f p, float factor, MajorAxices axisDependent)
{
	vec3f result;
	switch(axisDependent)
	{
	case(yAxis):
		return result = vec3f(p.x + factor * p.y, p.y, p.z);
		break;
	case(zAxis):
		return result = vec3f(p.x + factor * p.z, p.y, p.z);
		break;
	default:
		return result = vec3f(p.x + factor * p.y, p.y, p.z);
		break;
	}		
	return result;
}

vec3f COMPACTBLOBTREE::shearAlongY(vec3f p, float factor, MajorAxices axisDependent)
{
	vec3f result;
	switch(axisDependent)
	{
	case(xAxis):
		return result = vec3f(p.x, p.y + factor * p.x, p.z);
		break;
	case(zAxis):
		return result = vec3f(p.x, p.y + factor * p.z, p.z);
		break;
	default:
		return result = vec3f(p.x, p.y + factor * p.x, p.z);
		break;
	}		
	return result;
}

vec3f COMPACTBLOBTREE::shearAlongZ(vec3f p, float factor, MajorAxices axisDependent)
{
	vec3f result;
	switch(axisDependent)
	{
	case(xAxis):
		return result = vec3f(p.x, p.y, p.z + factor * p.x);
		break;
	case(yAxis):
		return result = vec3f(p.x, p.y, p.z + factor * p.y);
		break;
	default:
		return result = vec3f(p.x, p.y, p.z + factor * p.x);
		break;
	}		
	return result;
}

PS::MATH::vec4f COMPACTBLOBTREE::warpShear( const vec4f& pin, float factor, MajorAxices axisAlong, MajorAxices axisDependent )
{
	vec3f res = pin.xyz();
	switch(axisAlong)
	{
	case(xAxis):
		res = shearAlongX(res, factor, axisDependent);			
		break;
	case(yAxis):
		res = shearAlongY(res, factor, axisDependent);			
		break;
	case(zAxis):
		res = shearAlongZ(res, factor, axisDependent);			
		break;
	default:
		res = shearAlongX(res, factor, axisDependent);			
	}		
	return vec4f(res.x, res.y, res.z);
}
//////////////////////////////////////////////////////////////////////////
/*
int ComputeRootNewtonRaphsonSIMD(COMPACTBLOBTREE* cptBlob, 
								 const svec4f& p1, const svec4f& p2, 
								 float fp1, float fp2, 
								 svec4f& output, float& outputField,
								 float target_field, int iterations)
{
	
	svec4f grad, x, d;
	float f;

	if(iterations <= 0) return -1;	

	if(fabsf(fp1 - target_field) < fabsf(fp2 - target_field))
		x = p1;
	else
		x = p2;

	int i=0;
	for(i=0; i<iterations; i++)
	{
		//Get gradient for direction of displacement
		//grad = m_root->gradient(x, FIELD_VALUE_EPSILON);			
		//f = m_root->fieldValue(x);
		//Use faster method to compute fieldvalue and gradient at once
		grad = cptBlob->fieldValueAndGradient(x, FIELD_VALUE_EPSILON);

		d.set(target_field - grad[3]);
				
		//Uses shrink-wrap method to converge to surface		
		x = x + (d*grad) * simd_rcp(simd_dot(grad, grad));

		outputField = cptBlob->fieldvalue(x);
		output = x;		
		if(fabsf(outputField - target_field) < FIELD_VALUE_EPSILON)				
			break;				
	}

	output[3] = 0.0f;
	return (i+1)*4;
}
*/

int ComputeRootNewtonRaphsonVEC4( COMPACTBLOBTREE* cptBlob, 
								 float* lpStoreFVOps, 
								 float* lpStoreFVPrims,
								 const vec4f& p1, const vec4f& p2, 
								 float fp1, float fp2, 
								 vec4f& output, float& outputField, 
								 float target_field /*= ISO_VALUE*/, int iterations /*= DEFAULT_ITERATIONS*/ )
{
	vec4f grad, x, d;
	float f, g;

	if(iterations <= 0) return -1;	

	if(fabsf(fp1 - target_field) < fabsf(fp2 - target_field))
		x = p1;
	else
		x = p2;

	int i=0;
	for(i=0; i<iterations; i++)
	{
		//Get gradient for direction of displacement
		//grad = m_root->gradient(x, FIELD_VALUE_EPSILON);			
		//f = m_root->fieldValue(x);
		//Use faster method to compute fieldvalue and gradient at once
		grad = cptBlob->fieldValueAndGradient(x, FIELD_VALUE_EPSILON);

		d.set(target_field - grad[3]);

		//Uses shrink-wrap method to converge to surface	
		g = 1.0f / (grad.dot(grad));
		x = x + (d*grad) * g;

		outputField = cptBlob->fieldvalue(x, lpStoreFVOps, lpStoreFVPrims);
		output = x;		
		if(fabsf(outputField - target_field) < FIELD_VALUE_EPSILON)				
			break;				
	}

	output[3] = 0.0f;
	return (i+1)*4;
}
