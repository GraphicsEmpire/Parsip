#include "CompactBlobTree.h"
#include "PS_BlobTree/include/BlobTreeLibraryAll.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "PS_BlobTree/include/CSkeletonTriangle.h"
#include "_GlobalFunctions.h"

#define PS_SUCCESS            1
#define ERR_OPS_OVERFLOW     -1
#define ERR_PRIMS_OVERFLOW   -2
#define ERR_KIDS_OVERFLOW    -3
#define ERR_PARAM_ERROR      -4
#define ERR_NODE_NOT_RECOGNIZED -5

//Initialize all structures
void COMPACTBLOBTREE::init()
{
    m_lpPrims = NULL;
    m_lpOps = NULL;
    m_szAllocatedPrims = 0;
    m_szAllocatedOps = 0;
    m_ctPrims = 0;
    m_ctOps = 0;
    m_ctPCMNodes = 0;
}

int COMPACTBLOBTREE::convert( CBlobNode* root)
{
    if(root == NULL)
    {
        ReportError("Invalid input tree.");
        FlushAllErrors();
        return ERR_PARAM_ERROR;
    }

    //Check if we need to allocate new memory
    m_ctInstances = 0;
    m_lstConvertedIds.resize(0);
    int ctNeededOps = MATHMAX(root->recursive_CountOperators(), MIN_BLOB_NODES);
    int ctNeededPrims = MATHMAX(root->recursive_CountPrimitives(), MIN_BLOB_NODES);

    //Upgrade Ops
    m_ctOps = 0;
    if((m_szAllocatedOps < ctNeededOps)||(m_lpOps == NULL))
    {
        SAFE_DELETE(m_lpOps);
        m_lpOps = new BlobOperator[ctNeededOps];
        m_szAllocatedOps = ctNeededOps;
    }

    //Update Prims
    m_ctPrims = 0;
    if((m_szAllocatedPrims < ctNeededPrims)||(m_lpPrims == NULL))
    {
        SAFE_DELETE(m_lpPrims);
        m_lpPrims = new BlobPrimitive[ctNeededPrims];
        m_szAllocatedPrims = ctNeededPrims;
    }

    //Convert Recursively
    int res = convert(root, -1);
    if(m_ctInstances > 0)
    {
        int ctFixed = updateInstanceNodes();
        if(ctFixed != m_ctInstances)
        {
            ReportError("Problem occured when fixing instanced node ids\n");
            FlushAllErrors();
        }
    }

    //Count PCM Nodes
    m_ctPCMNodes = 0;
    std::vector<int> pcmIDS;
    for(int i=0; i<m_ctOps; i++)
    {
        if(m_lpOps[i].type == bntOpPCM)
        {
            m_ctPCMNodes++;
            pcmIDS.push_back(i);
        }
    }

    //Assign PCM Nodes
    if(m_ctPCMNodes > 0)
    {
        m_pcmCONTEXT.idPCM = pcmIDS[0];
        m_pcmCONTEXT.maxCompressionLeft = ISO_VALUE;
        m_pcmCONTEXT.maxCompressionRight = ISO_VALUE;
        /*
        m_lpPCMCONTEXT = new PCMCONTEXT[m_ctPCMNodes];
        for(int i=0; i<m_ctPCMNodes; i++)
        {
            m_lpPCMCONTEXT[i].idPCM = pcmIDS[i];
            m_lpPCMCONTEXT[i].maxCompressionLeft = ISO_VALUE;
            m_lpPCMCONTEXT[i].maxCompressionRight = ISO_VALUE;
        }
        */
    }

    pcmIDS.resize(0);
    return res;
}

int COMPACTBLOBTREE::convert(CBlobNode* root, int parentID)
{
    int curID = -1;
    if(root->isOperator())
    {
        if(m_ctOps >= m_szAllocatedOps)
        {
            DAnsiStr strError = printToAStr("Not enough memory for operators. Allocated=%d, Needed > %d",
                                            m_szAllocatedOps,
                                            m_ctOps);
            ReportError(strError.cptr());
            FlushAllErrors();
            return ERR_OPS_OVERFLOW;
        }

        curID = m_ctOps;
        m_ctOps++;

        m_lpOps[curID].type = root->getNodeType();
        m_lpOps[curID].orgID = root->getID();
        m_lpOps[curID].params.zero();

        m_lpOps[curID].octLo = vec4f(root->getOctree().lower, 0.0f);
        m_lpOps[curID].octHi = vec4f(root->getOctree().upper, 0.0f);



        if(root->countChildren() > MAX_COMPACT_KIDS_COUNT)
        {
            DAnsiStr strError = printToAStr("MAX Number of kids reached. Allowed=%d, Had > %d",
                                            MAX_COMPACT_KIDS_COUNT,
                                            root->countChildren());
            ReportError(strError.cptr());
            FlushAllErrors();
            return ERR_KIDS_OVERFLOW;
        }

        int kidID;
        m_lpOps[curID].ctKids = root->countChildren();
        //m_lpOps[curID].kidIds.resize(root->countChildren());
        //m_lpOps[curID].kidIsOp.resize(root->countChildren());
        for(size_t i=0;i<root->countChildren(); i++)
        {
            kidID = convert(root->getChild(i), curID );
            if(kidID < 0)
                return kidID;

            m_lpOps[curID].kidIds[i] = kidID;
            m_lpOps[curID].kidIsOp[i] = root->getChild(i)->isOperator();
        }

        //Convert operator
        switch(root->getNodeType())
        {
        case(bntOpUnion): case(bntOpBlend): case(bntOpDif): case(bntOpSmoothDif):
        case(bntOpIntersect):
        {

        }
        break;
        case(bntOpPCM):
        {
            vec4f param;
            CPcm* lpPCM = dynamic_cast<CPcm*>(root);
            param.x = lpPCM->getPropagateLeft();
            param.y = lpPCM->getPropagateRight();
            param.z = lpPCM->getAlphaLeft();
            param.w = lpPCM->getAlphaRight();
            m_lpOps[curID].params = param;
        }
            break;
        case(bntOpRicciBlend):
        {
            CRicciBlend* ricci = dynamic_cast<CRicciBlend*>(root);
            //cfg->writeFloat(strNodeName, "power", ricci->getN());
            float n = ricci->getN();
            m_lpOps[curID].params.x = n;
            if(n != 0.0f)
                m_lpOps[curID].params.y = 1.0f / n;
        }
            break;
        case(bntOpWarpTwist):
        {
            CWarpTwist* twist = dynamic_cast<CWarpTwist*>(root);
            //cfg->writeFloat(strNodeName, "factor", twist->getWarpFactor());
            //cfg->writeInt(strNodeName, "axis", static_cast<int>(twist->getMajorAxis()));
            m_lpOps[curID].params.x = twist->getWarpFactor();
            m_lpOps[curID].params.y = static_cast<float>(twist->getMajorAxis());
        }
            break;
        case(bntOpWarpTaper):
        {
            CWarpTaper* taper = dynamic_cast<CWarpTaper*>(root);
            //cfg->writeFloat(strNodeName, "factor", taper->getWarpFactor());
            //cfg->writeInt(strNodeName, "base axis", static_cast<int>(taper->getAxisAlong()));
            //cfg->writeInt(strNodeName, "taper axis", static_cast<int>(taper->getAxisTaper()));
            m_lpOps[curID].params.x = taper->getWarpFactor();
            m_lpOps[curID].params.y = static_cast<float>(taper->getAxisAlong());
            m_lpOps[curID].params.z = static_cast<float>(taper->getAxisTaper());
        }
            break;
        case(bntOpWarpBend):
        {
            CWarpBend* bend = dynamic_cast<CWarpBend*>(root);
            //cfg->writeFloat(strNodeName, "rate", bend->getBendRate());
            //cfg->writeFloat(strNodeName, "center", bend->getBendCenter());
            //cfg->writeFloat(strNodeName, "left bound", bend->getBendRegion().left);
            //cfg->writeFloat(strNodeName, "right bound", bend->getBendRegion().right);
            m_lpOps[curID].params.x = bend->getBendRate();
            m_lpOps[curID].params.y = bend->getBendCenter();
            m_lpOps[curID].params.z = bend->getBendRegion().left;
            m_lpOps[curID].params.w = bend->getBendRegion().right;
        }
            break;
        case(bntOpWarpShear):
        {
            CWarpShear* shear = dynamic_cast<CWarpShear*>(root);
            //cfg->writeFloat(strNodeName, "factor", shear->getWarpFactor());
            //cfg->writeInt(strNodeName, "base axis", static_cast<int>(shear->getAxisAlong()));
            //cfg->writeInt(strNodeName, "shear axis", static_cast<int>(shear->getAxisDependent()));
            m_lpOps[curID].params.x = shear->getWarpFactor();
            m_lpOps[curID].params.y = static_cast<float>(shear->getAxisAlong());
            m_lpOps[curID].params.z = static_cast<float>(shear->getAxisDependent());
        }
            break;
        default:
        {
            DAnsiStr strMsg = printToAStr("Operator %s has not been implemented in compact mode yet!", root->getName().c_str());
            ReportError(strMsg.ptr());
            FlushAllErrors();
            return ERR_NODE_NOT_RECOGNIZED;
        }
        }
    }
    else
    {
        if(m_ctPrims >= m_szAllocatedPrims)
        {
            DAnsiStr strError = printToAStr("Not enough memory for primitives. Allocated=%d, Needed > %d",
                                            m_szAllocatedPrims,
                                            m_ctPrims);
            ReportError(strError.cptr());
            FlushAllErrors();
            return ERR_PRIMS_OVERFLOW;
        }

        curID = m_ctPrims;
        m_ctPrims++;

        //
        m_lpPrims[curID].type  = root->getNodeType();
        m_lpPrims[curID].orgID = root->getID();
        m_lpPrims[curID].color = root->getMaterial().diffused;

        //Bounding Box
        m_lpPrims[curID].octLo = vec4f(root->getOctree().lower, 0.0f);
        m_lpPrims[curID].octHi = vec4f(root->getOctree().upper, 0.0f);

        //Transformation Matrix
        /*
        CMatrix mtxForward = root->getTransform().getForwardMatrix();
        mtxForward.getRow(row, 0);
        */

        float row[4];        
        CMatrix mtxForward = root->getTransform().getForwardMatrix();
        mtxForward.getRow(row, 0);
        m_lpPrims[curID].mtxForwardR0.set(row[0], row[1], row[2], row[3]);
        mtxForward.getRow(row, 1);
        m_lpPrims[curID].mtxForwardR1.set(row[0], row[1], row[2], row[3]);
        mtxForward.getRow(row, 2);
        m_lpPrims[curID].mtxForwardR2.set(row[0], row[1], row[2], row[3]);
        mtxForward.getRow(row, 3);
        m_lpPrims[curID].mtxForwardR3.set(row[0], row[1], row[2], row[3]);

        CMatrix mtxBackward = root->getTransform().getBackwardMatrix();
        mtxBackward.getRow(row, 0);
        m_lpPrims[curID].mtxBackwardR0.set(row[0], row[1], row[2], row[3]);
        mtxBackward.getRow(row, 1);
        m_lpPrims[curID].mtxBackwardR1.set(row[0], row[1], row[2], row[3]);
        mtxBackward.getRow(row, 2);
        m_lpPrims[curID].mtxBackwardR2.set(row[0], row[1], row[2], row[3]);
        mtxBackward.getRow(row, 3);
        m_lpPrims[curID].mtxBackwardR3.set(row[0], row[1], row[2], row[3]);

        switch(root->getNodeType())
        {
        case(bntPrimPoint):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonPoint* skeletPoint = reinterpret_cast<CSkeletonPoint*>(sprim->getSkeleton());
            vec3f pos = skeletPoint->getPosition();
            m_lpPrims[curID].pos.set(pos.x, pos.y, pos.z, 0.0f);
        }
            break;
        case(bntPrimLine):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonLine* skeletLine = reinterpret_cast<CSkeletonLine*>(sprim->getSkeleton());
            vec3f s = skeletLine->getStartPosition();
            vec3f e = skeletLine->getEndPosition();
            m_lpPrims[curID].res1.set(s.x, s.y, s.z);
            m_lpPrims[curID].res2.set(e.x, e.y, e.z);
        }
            break;
        case(bntPrimRing):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonRing* skeletRing = reinterpret_cast<CSkeletonRing*>(sprim->getSkeleton());
            vec3f p = skeletRing->getPosition();
            vec3f d = skeletRing->getDirection();
            float r = skeletRing->getRadius();
            m_lpPrims[curID].pos.set(p.x, p.y, p.z);
            m_lpPrims[curID].dir.set(d.x, d.y, d.z);
            m_lpPrims[curID].res1.set(r);
            m_lpPrims[curID].res2.set(r*r);
        }
            break;
        case(bntPrimDisc):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonDisc* skeletDisc = reinterpret_cast<CSkeletonDisc*>(sprim->getSkeleton());
            vec3f p = skeletDisc->getPosition();
            vec3f d = skeletDisc->getDirection();
            float r = skeletDisc->getRadius();
            m_lpPrims[curID].pos.set(p.x, p.y, p.z);
            m_lpPrims[curID].dir.set(d.x, d.y, d.z);
            m_lpPrims[curID].res1.set(r);
            m_lpPrims[curID].res2.set(r*r);
        }
            break;
        case(bntPrimCylinder):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonCylinder* skeletCyl = reinterpret_cast<CSkeletonCylinder*>(sprim->getSkeleton());
            vec3f p = skeletCyl->getPosition();
            vec3f d = skeletCyl->getDirection();
            m_lpPrims[curID].pos.set(p.x, p.y, p.z);
            m_lpPrims[curID].dir.set(d.x, d.y, d.z);
            m_lpPrims[curID].res1.set(skeletCyl->getRadius());
            m_lpPrims[curID].res2.set(skeletCyl->getHeight());
        }
            break;

        case(bntPrimCube):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonCube* skeletCube = reinterpret_cast<CSkeletonCube*>(sprim->getSkeleton());
            vec3f p = skeletCube->getPosition();
            float side = skeletCube->getSide();
            m_lpPrims[curID].pos.set(p.x, p.y, p.z);
            m_lpPrims[curID].res1.set(side);
        }
            break;
        case(bntPrimTriangle):
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonTriangle* skeletTriangle = reinterpret_cast<CSkeletonTriangle*>(sprim->getSkeleton());
            vec3f p0 = skeletTriangle->getTriangleCorner(0);
            vec3f p1 = skeletTriangle->getTriangleCorner(1);
            vec3f p2 = skeletTriangle->getTriangleCorner(2);

            m_lpPrims[curID].pos.set(p0.x, p0.y, p0.z);
            m_lpPrims[curID].res1.set(p1.x, p1.y, p1.z);
            m_lpPrims[curID].res2.set(p2.x, p2.y, p2.z);
        }
            break;
        case(bntPrimQuadricPoint):
        {
            CQuadricPoint* lpPrim = reinterpret_cast<CQuadricPoint*>(root);
            vec3f  p = lpPrim->getPosition();
            m_lpPrims[curID].pos.set(p.x, p.y, p.z);
            m_lpPrims[curID].res1 = vec4f(lpPrim->getFieldRadius());
            m_lpPrims[curID].res2 = vec4f(lpPrim->getFieldScale());
        }
            break;
        case(bntPrimNull):
        {
            m_lpPrims[curID].pos.set(0, 0 ,0);
        }
        break;
        case(bntPrimInstance):
        {
            CInstance* lpInst = reinterpret_cast<CInstance*>(root);
            m_lpPrims[curID].res1.x = 0;
            m_lpPrims[curID].res1.y = static_cast<float>(lpInst->getOriginalNode()->getID());
            m_lpPrims[curID].res1.z = static_cast<float>(lpInst->getOriginalNode()->isOperator());
            m_lpPrims[curID].res1.w = static_cast<float>(lpInst->getOriginalNode()->getNodeType());
            m_ctInstances++;
        }
        break;
        default:
        {
            DAnsiStr strMsg = printToAStr("Primitive %s has not been implemented in compact mode yet!", root->getName().c_str());
            ReportError(strMsg.ptr());
            FlushAllErrors();
            return ERR_NODE_NOT_RECOGNIZED;
        }
        }
    }

    //Store
    m_lstConvertedIds.push_back(std::make_pair(root->getID(), curID));

    //Return Current ID for this NODE
    return curID;
}

int COMPACTBLOBTREE::updateInstanceNodes()
{
    int ctFixed = 0;
    for(U32 i=0; i<m_ctPrims; i++)
    {
        if(m_lpPrims[i].type == bntPrimInstance)
        {
            for(int j=0; j < m_lstConvertedIds.size(); j++)
            {
                int idxScript = static_cast<int>(m_lpPrims[i].res1.y);
                if(idxScript == m_lstConvertedIds[j].first)
                {
                    m_lpPrims[i].res1.x = m_lstConvertedIds[j].second;
                    ctFixed ++;
                    break;
                }
            }
        }
    }
    return ctFixed;
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

    if(m_ctOps > 0)
        return fieldvalueOp(pp, 0, lpStoreFVOp, lpStoreFVPrim);
    else if(m_ctPrims > 0)
        return fieldvaluePrim(pp, 0, lpStoreFVPrim);
    else
        return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
float COMPACTBLOBTREE::computePCM(const vec4f& p,
                                  const vec4f& pcmParam,
                                  const vec4f& oct1Lo,
                                  const vec4f& oct1Hi,
                                  const vec4f& oct2Lo,
                                  const vec4f& oct2Hi,
                                  int idSelf,
                                  int idChild1, int idChild2,
                                  U8 isOpChild1, U8 isOpChild2,
                                  float fp1, float fp2)
{
    assert(m_ctPCMNodes > 0);

    bool bCrossed = intersects(oct1Lo, oct1Hi, oct2Lo, oct2Hi);
    //Boxes have intersection: Find interpenetration and propagation regions
    if(bCrossed)
    {
        //Interpenetration
        if(fp1 >= ISO_VALUE && fp2 >= ISO_VALUE)
        {
            if(fp1 > fp2)
            {
                if(fp2 > m_pcmCONTEXT.maxCompressionLeft)
                    m_pcmCONTEXT.maxCompressionLeft = fp2;
                return fp1 + (ISO_VALUE - fp2);
            }
            else
            {
                if(fp1 > m_pcmCONTEXT.maxCompressionRight)
                    m_pcmCONTEXT.maxCompressionRight = fp1;
                return fp2 + (ISO_VALUE - fp1);
            }
        }
        //Propagation Left Child
        else if(fp1 >= ISO_VALUE && fp2 > FIELD_VALUE_EPSILON)
        {
            //Compute P0: closest point to fp2
            vec3f grad = gradientAtNode(isOpChild2, idChild2, p, fp2, NORMAL_DELTA).xyz();
            vec3f pp = p.xyz();

            vec3f p0 = marchTowardNode(isOpChild2, idChild2, pp, grad, fp2);

            float k;
            {
                vec4f p00 = vec4f(p0, 0.0f);
                vec3f gradP0 = gradientAtNode(isOpChild2, idChild2, p00, fp2, NORMAL_DELTA).xyz();
                k = gradP0.length();
            }

            float a0 = pcmParam.z * m_pcmCONTEXT.maxCompressionLeft;
            float d = pp.distance(p0);

            return fp1 + computePropagationDeformation(d, k, a0, pcmParam.x);

        }
        //Propagation Right Child
        else if(fp2 >= ISO_VALUE && fp1 > FIELD_VALUE_EPSILON)
        {
            //Compute P0: closest point to fp2
            vec3f grad = gradientAtNode(isOpChild1, idChild1, p, fp1, NORMAL_DELTA).xyz();
            vec3f pp = p.xyz();
            vec3f p0 = marchTowardNode(isOpChild1, idChild1, pp, grad, fp1);

            float k;
            {
                vec4f p00 = vec4f(p0, 0.0f);
                vec3f gradP0 = gradientAtNode(isOpChild1, idChild1, p00, fp1, NORMAL_DELTA).xyz();
                k = gradP0.length();
            }

            float a0 = pcmParam.w * m_pcmCONTEXT.maxCompressionRight;
            float d = pp.distance(p0);

            return fp2 + computePropagationDeformation(d, k, a0, pcmParam.y);
        }
        else
            return MATHMAX(fp1, fp2);
    }
    else
        return MATHMAX(fp1, fp2);
}

vec3f COMPACTBLOBTREE::marchTowardNode(bool isOp, int id, const vec3f& p, const vec3f& grad, float& fp)
{
    int dir = (fp < ISO_VALUE)?1:-1;
    int dir2;
    float step = ISO_DISTANCE;

    //Uses shrink-wrap method to converge to surface
    //float d = (ISO_VALUE - fp2) / (grad.dot(grad));
    vec3f q = p;
    while(!FLOAT_EQ(fp, ISO_VALUE, FIELD_VALUE_EPSILON))
    {
        q += step * dir * grad;
        fp = fieldAtNode(isOp, id, vec4f(q, 0.0f));
        dir2 = (fp < ISO_VALUE)?1:-1;
        if(dir != dir2)
            step *= 0.5f;
        dir = dir2;
    }

    return q;
}

float COMPACTBLOBTREE::computePropagationDeformation(float dist, float k, float a0, float w)
{
    float wh = 0.5f * w;
    float w2 = w * w;
    float w3 = w2 * w;

    if(dist >= 0 && dist < wh)
    {
        float p1 = (4.0f * (w*k - 4*a0))/w3;
        float p2 = (4.0f * (3.0f*a0 - w*k))/ w2;
        float d2 = dist * dist;
        float d3 = dist * d2;
        return p1 * d3 + p2 * d2 + k*dist;
    }
    else if(dist >= wh && dist < w)
    {
        return (4*a0 + (dist - w)*(dist - w) * (4*dist - w)) / w3;
    }
    else
        return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
vec4f COMPACTBLOBTREE::gradientAtNode(bool isOp, int id, const vec4f& p, float fp, float delta )
{
    vec4f res;
    vec4f insFieldValue(fp, fp, fp, 0.0f);
    vec4f invsDelta(1.0f / delta);
    float arrN[4];

    if(isOp)
    {
        arrN[0] = fieldvalueOp(p + vec4f(delta, 0.0f, 0.0f, 0.0f), id);
        arrN[1] = fieldvalueOp(p + vec4f(0.0f, delta, 0.0f, 0.0f), id);
        arrN[2] = fieldvalueOp(p + vec4f(0.0f, 0.0f, delta, 0.0f), id);
        arrN[3] = 0.0f;
    }
    else
    {
        arrN[0] = fieldvaluePrim(p + vec4f(delta, 0.0f, 0.0f, 0.0f), id);
        arrN[1] = fieldvaluePrim(p + vec4f(0.0f, delta, 0.0f, 0.0f), id);
        arrN[2] = fieldvaluePrim(p + vec4f(0.0f, 0.0f, delta, 0.0f), id);
        arrN[3] = 0.0f;
    }

    res.set(arrN);
    res -= insFieldValue;
    res *= invsDelta;
    return res;
}

//////////////////////////////////////////////////////////////////////////
float COMPACTBLOBTREE::fieldAtNode(bool isOp, int id, const vec4f& p)
{
    if(isOp)
        return fieldvalueOp(p, id);
    else
        return fieldvaluePrim(p, id);
}

//////////////////////////////////////////////////////////////////////////
int   COMPACTBLOBTREE::getID(bool isOp, int treeOrgID)
{
    if(isOp)
    {
        for(int i=0; i<m_ctOps; i++)
        {
            if(m_lpOps[i].orgID == treeOrgID)
                return i;
        }
    }
    else
    {
        for(int i=0; i<m_ctPrims; i++)
        {
            if(m_lpPrims[i].orgID == treeOrgID)
                return i;
        }
    }

    return -1;
}
//////////////////////////////////////////////////////////////////////////
float COMPACTBLOBTREE::fieldvalueOp(const vec4f& p, int id, float* lpStoreFVOp, float* lpStoreFVPrim)
{
    float lpKidsFV[MAX_COMPACT_KIDS_COUNT];
    float res  = 0.0f;
    int ctKids = m_lpOps[id].ctKids;
    int kidID  = 0;
    vec4f pWarped = p;
    //////////////////////////////////////////////////////////////////////////
    if(lpStoreFVOp)
        lpStoreFVOp[id] = 0.0f;
    vec4f lo = m_lpOps[id].octLo;
    vec4f hi = m_lpOps[id].octHi;
    if((p.x < lo.x)||(p.y < lo.y)||(p.z < lo.z))
        return 0.0f;
    if((p.x > hi.x)||(p.y > hi.y)||(p.z > hi.z))
        return 0.0f;
    //////////////////////////////////////////////////////////////////////////
    //Process Warp unary ops first since we need to warp the space before affine
    //transformations effect
    BlobNodeType nodetype = m_lpOps[id].type;
    bool bWarpNode =((nodetype == bntOpWarpBend) || (nodetype == bntOpWarpShear) ||
                     (nodetype == bntOpWarpTwist) || (nodetype == bntOpWarpTaper));

    if(bWarpNode)
    {

        //Warp the space
        switch(nodetype)
        {
        case(bntOpWarpBend):
        {
            vec4f param = m_lpOps[id].params;
            pWarped = warpBend(pWarped, param.x, param.y, CInterval(param.z, param.w));
        }
            break;
        case(bntOpWarpTwist):
        {
            vec4f param = m_lpOps[id].params;
            pWarped = warpTwist(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)));
        }
            break;
        case(bntOpWarpTaper):
        {
            vec4f param = m_lpOps[id].params;
            pWarped = warpTaper(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)), static_cast<MajorAxices>((int)(param.z)));
        }
            break;
        case(bntOpWarpShear):
        {
            vec4f param = m_lpOps[id].params;
            pWarped = warpShear(pWarped, param.x, static_cast<MajorAxices>((int)(param.y)), static_cast<MajorAxices>((int)(param.z)));
        }
            break;
        }
    }

    //Processing other operators
    for(int i=0; i<ctKids; i++)
    {
        kidID = m_lpOps[id].kidIds[i];
        if(m_lpOps[id].kidIsOp[i])
            lpKidsFV[i] = fieldvalueOp(pWarped, kidID, lpStoreFVOp, lpStoreFVPrim);
        else
            lpKidsFV[i] = fieldvaluePrim(pWarped, kidID, lpStoreFVPrim);
    }

    //Processing FieldValues
    switch(m_lpOps[id].type)
    {

    //Precise Contact modeling will be done here:
    case(bntOpPCM):
    {
        if(ctKids == 2)
        {
            int kidID1 = m_lpOps[id].kidIds[0];
            int kidID2 = m_lpOps[id].kidIds[1];
            U8 isOpKid1 = m_lpOps[id].kidIsOp[0];
            U8 isOpKid2 = m_lpOps[id].kidIsOp[1];

            vec4f oct1Lo, oct1Hi, oct2Lo, oct2Hi;
            if(isOpKid1)
            {
                oct1Lo = m_lpOps[kidID1].octLo;
                oct1Hi = m_lpOps[kidID1].octHi;
            }
            else
            {
                oct1Lo = m_lpPrims[kidID1].octLo;
                oct1Hi = m_lpPrims[kidID1].octHi;
            }

            if(isOpKid2)
            {
                oct2Lo = m_lpOps[kidID2].octLo;
                oct2Hi = m_lpOps[kidID2].octHi;
            }
            else
            {
                oct2Lo = m_lpPrims[kidID2].octLo;
                oct2Hi = m_lpPrims[kidID2].octHi;
            }

            res = computePCM(pWarped, m_lpOps[id].params,
                             oct1Lo, oct1Hi,
                             oct2Lo, oct2Hi,
                             id,
                             kidID1, kidID2,
                             isOpKid1, isOpKid2,
                             lpKidsFV[0], lpKidsFV[1]);
        }
        else
            return 0.0f;
    }
        break;
    case(bntOpBlend):
    {
        for(int i=0; i<ctKids; i++)
            res += lpKidsFV[i];
    }
        break;
    case(bntOpRicciBlend):
    {
        for(int i=0; i<ctKids; i++)
            res += powf(lpKidsFV[i], m_lpOps[id].params.x);
        res = powf(res, m_lpOps[id].params.y);
    }
        break;
    case(bntOpUnion):
    {
        res = lpKidsFV[0];
        for(int i=1; i<ctKids; i++)
        {
            if(lpKidsFV[i] > res)
                res = lpKidsFV[i];
        }
    }
        break;
    case(bntOpIntersect):
    {
        res = lpKidsFV[0];
        for(int i=1; i<ctKids; i++)
        {
            if(lpKidsFV[i] < res)
                res = lpKidsFV[i];
        }
    }
        break;
    case(bntOpDif):
    {
        res = lpKidsFV[0];
        for(int i=1; i<ctKids; i++)
        {
            res = MATHMIN(res, MAX_FIELD_VALUE - lpKidsFV[i]);
        }
    }
        break;
    case(bntOpSmoothDif):
    {
        res = lpKidsFV[0];
        for(int i=1; i<ctKids; i++)
        {
            res *= (MAX_FIELD_VALUE - lpKidsFV[i]);
        }
    }
        break;
    case(bntOpWarpBend):
    {
        res = lpKidsFV[0];
    }
        break;
    case(bntOpWarpTwist):
    {
        res = lpKidsFV[0];
    }
        break;
    case(bntOpWarpTaper):
    {
        res = lpKidsFV[0];
    }
        break;
    case(bntOpWarpShear):
    {
        res = lpKidsFV[0];
    }
        break;
    default:
    {
        DAnsiStr strMsg = printToAStr("That operator is not been implemented yet! Op = %d", m_lpOps[id].type);
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
    vec3f pn;    
    //Apply Affine Matrix transformation
    pn.x = m_lpPrims[id].mtxBackwardR0.dot(pp);
    pn.y = m_lpPrims[id].mtxBackwardR1.dot(pp);
    pn.z = m_lpPrims[id].mtxBackwardR2.dot(pp);

    float fvRes;
    switch(m_lpPrims[id].type)
    {
    case bntPrimPoint:
    {
        fvRes = ComputeWyvillFieldValueSquare(pn.dist2(m_lpPrims[id].pos.xyz()));
    }
        break;
    case bntPrimCylinder:
    {
        vec3f pos = pn - m_lpPrims[id].pos.xyz();

        float y = pos.dot(m_lpPrims[id].dir.xyz());
        //float x = MATHMAX(0.0f, sqrtf(pos.length2() - y*y) - prims[id].res1.x);
        float x = maxf(0.0f, sqrtf(pos.length2() - y*y) - m_lpPrims[id].res1.x);

        //Make y 0.0 if it is positive and less than height
        // For Hemispherical caps
        if(y > 0.0f)
            y = maxf(0.0f, y - m_lpPrims[id].res2.x);

        fvRes = ComputeWyvillFieldValueSquare(x*x + y*y);
    }
        break;
    case bntPrimTriangle:
    {
        vec3f vertices[3];
        vertices[0] = m_lpPrims[id].pos.xyz();
        vertices[1] = m_lpPrims[id].res1.xyz();
        vertices[2] = m_lpPrims[id].res2.xyz();
        vec3f outClosest, outBaryCoords;
        float dd = ComputeTriangleSquareDist(vertices, pn, outClosest, outBaryCoords);
        fvRes = ComputeWyvillFieldValueSquare(dd);
    }
        break;

    case bntPrimCube:
    {
        vec3f center = m_lpPrims[id].pos.xyz();
        float side   = m_lpPrims[id].res1.x;

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

        fvRes = ComputeWyvillFieldValueSquare(dist2);
    }
        break;
    case bntPrimDisc:
    {
        vec3f n = m_lpPrims[id].dir.xyz();
        vec3f c = m_lpPrims[id].pos.xyz();
        float r = m_lpPrims[id].res1[0];
        vec3f dir = pn - c - (n.dot(pn - c))*n;

        float dd;
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
        fvRes = ComputeWyvillFieldValueSquare(dd);
    }
        break;

    case bntPrimRing:
    {
        vec3f n = m_lpPrims[id].dir.xyz();
        vec3f c = m_lpPrims[id].pos.xyz();
        float r = m_lpPrims[id].res1[0];
        vec3f dir = pn - c - (n.dot(pn - c))*n;

        float dd;
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
        fvRes = ComputeWyvillFieldValueSquare(dd);
    }
        break;
    case bntPrimLine:
    {
        vec3f s = m_lpPrims[id].res1.xyz();
        vec3f e = m_lpPrims[id].res2.xyz();
        vec3f nearestPoint = NearestPointInLineSegment(pn, s, e);        
        fvRes = ComputeWyvillFieldValueSquare(nearestPoint.dist2(pn));
    }
    break;

    case(bntPrimQuadricPoint):
    {
        vec3f c = m_lpPrims[id].pos.xyz();
        float fDist2 = (pn - c).length2();
        float fRadius = m_lpPrims[id].res1.x;

        float fValue = (1.0f - (fDist2 / (fRadius * fRadius)));
        if(fValue <= 0.0f)
            fvRes = 0.0f;
        else
            //Scale * Field * Field
            fvRes = m_lpPrims[id].res2.x * fValue * fValue;

    }
        break;
    case bntPrimNull:
        fvRes = 0;
        break;
    case bntPrimInstance:
    {
        bool isOriginOp = static_cast<bool>(m_lpPrims[id].res1.z);
        int idxOrigin = static_cast<int>(m_lpPrims[id].res1.x);
        vec4f myp = vec4f(pn, 1.0f);
        if(isOriginOp)
            this->fieldvalueOp(myp, idxOrigin);
        else
        {
            myp.x = m_lpPrims[idxOrigin].mtxForwardR0.dot(pp);
            myp.y = m_lpPrims[idxOrigin].mtxForwardR1.dot(pp);
            myp.z = m_lpPrims[idxOrigin].mtxForwardR2.dot(pp);
            this->fieldvaluePrim(myp, idxOrigin);
        }
    }
        break;
    default:
    {
        ReportError("I don't know how to compute fieldvalue for this primitive!");
        FlushAllErrors();
    }
    }



    if(lpStoreFVPrim)
        lpStoreFVPrim[id] = fvRes;
    return fvRes;
}

//////////////////////////////////////////////////////////////////////////
PS::MATH::vec4f COMPACTBLOBTREE::baseColor( const vec4f& p, float* lpStoreFVOp, float* lpStoreFVPrim )
{	
    if(m_ctOps > 0)
        return baseColorOp(p, 0, lpStoreFVOp, lpStoreFVPrim);
    else if(m_ctPrims > 0)
        return m_lpPrims[0].color;
    else
    {
        static vec4f black;
        return black;
    }
}

vec4f COMPACTBLOBTREE::baseColorOp(const vec4f& p, int id, float* lpStoreFVOp, float* lpStoreFVPrim )
{
    float arrFV[MAX_COMPACT_KIDS_COUNT];
    vec4f arrCL[MAX_COMPACT_KIDS_COUNT];

    float resFV = 0.0f;
    vec4f resCL;
    int kidID = 0;
    int ctKids = m_lpOps[id].ctKids;
    if(ctKids == 0)
        return resCL;

    if((lpStoreFVOp != NULL)&&(lpStoreFVPrim != NULL))
    {
        //Get them from Cached values
        for(int i=0; i<ctKids; i++)
        {
            kidID = m_lpOps[id].kidIds[i];
            if(m_lpOps[id].kidIsOp[i])
            {
                arrCL[i] = baseColorOp(p, kidID, lpStoreFVOp, lpStoreFVPrim);
                arrFV[i] = lpStoreFVOp[kidID];
            }
            else
            {
                arrCL[i] = m_lpPrims[kidID].color;
                arrFV[i] = lpStoreFVPrim[kidID];
            }
        }
    }
    else
    {
        for(int i=0; i<ctKids; i++)
        {
            kidID = m_lpOps[id].kidIds[i];
            if(m_lpOps[id].kidIsOp[i])
            {
                arrCL[i] = baseColorOp(p, kidID, lpStoreFVOp, lpStoreFVPrim);
                arrFV[i] = fieldvalueOp(p, kidID, lpStoreFVOp, lpStoreFVPrim);
            }
            else
            {
                arrCL[i] = m_lpPrims[kidID].color;
                arrFV[i] = fieldvaluePrim(p, kidID, lpStoreFVPrim);
            }
        }
    }

    float temp = 0.0f;

    switch(m_lpOps[id].type)
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
    case(bntOpPCM):
    {
        kidID = 0;
        if(arrFV[0] > arrFV[1])
            kidID = 0;
        else
            kidID = 1;
        resCL = arrCL[kidID];
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
    //Ops
    this->m_ctOps = rhs.m_ctOps;
    this->m_szAllocatedOps = rhs.m_szAllocatedOps;
    m_lpOps = new BlobOperator[rhs.m_ctOps];
    memcpy(m_lpOps, rhs.m_lpOps, sizeof(BlobOperator) * rhs.m_ctOps);

    //Prims
    this->m_ctPrims = rhs.m_ctPrims;
    this->m_szAllocatedPrims = rhs.m_szAllocatedPrims;
    m_lpPrims = new BlobPrimitive[rhs.m_ctPrims];
    memcpy(m_lpPrims, rhs.m_lpPrims, sizeof(BlobPrimitive) * rhs.m_ctPrims);

    //PCM
    this->m_ctPCMNodes = rhs.m_ctPCMNodes;
    memcpy(&this->m_pcmCONTEXT, &rhs.m_pcmCONTEXT, sizeof(m_pcmCONTEXT));

    //Copy Ops
    /*
    for(int i=0;i<rhs.m_ctOps;i++)
    {
        this->m_lpOps[i].type  = rhs.m_lpOps[i].type;
        this->m_lpOps[i].params = rhs.m_lpOps[i].params;
        this->m_lpOps[i].orgID = rhs.m_lpOps[i].orgID;
        this->m_lpOps[i].octLo = rhs.m_lpOps[i].octLo;
        this->m_lpOps[i].octHi = rhs.m_lpOps[i].octHi;

        this->m_lpOps[i].ctKids = rhs.m_lpOps[i].ctKids;
        this->m_lpOps[i].kidIds.assign(rhs.m_lpOps[i].kidIds.begin(),
                                   rhs.m_lpOps[i].kidIds.end());
        this->m_lpOps[i].kidIsOp.assign(rhs.m_lpOps[i].kidIsOp.begin(),
                                   rhs.m_lpOps[i].kidIsOp.end());
    }
    */

    //Copy Prims
    /*
    for(int i=0;i<rhs.m_ctPrims;i++)
    {
        this->m_lpPrims[i].type = rhs.m_lpPrims[i].type;
        this->m_lpPrims[i].orgID  = rhs.m_lpPrims[i].orgID;
        this->m_lpPrims[i].color  = rhs.m_lpPrims[i].color;
        this->m_lpPrims[i].pos    = rhs.m_lpPrims[i].pos;
        this->m_lpPrims[i].dir    = rhs.m_lpPrims[i].dir;
        this->m_lpPrims[i].res1   = rhs.m_lpPrims[i].res1;
        this->m_lpPrims[i].res2   = rhs.m_lpPrims[i].res2;
        this->m_lpPrims[i].octLo  = rhs.m_lpPrims[i].octLo;
        this->m_lpPrims[i].octHi  = rhs.m_lpPrims[i].octHi;

        this->m_lpPrims[i].mtxBackwardR0 = rhs.m_lpPrims[i].mtxBackwardR0;
        this->m_lpPrims[i].mtxBackwardR1 = rhs.m_lpPrims[i].mtxBackwardR1;
        this->m_lpPrims[i].mtxBackwardR2 = rhs.m_lpPrims[i].mtxBackwardR2;
        this->m_lpPrims[i].mtxBackwardR3 = rhs.m_lpPrims[i].mtxBackwardR3;
    }
    */

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
