#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "CLayerManager.h"
#include "PS_FrameWork/include/_dataTypes.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"
#include "PS_FrameWork/include/PS_DateTime.h"
#include "PS_FrameWork/include/PS_AppConfig.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "PS_BlobTree/include/BlobTreeBuilder.h"

#include "PS_BlobTree/include/BlobTreeLibraryAll.h"

/*
Layer will group a set of BlobNodes so that they can be polygonized together.
Layer will also holds the resulting mesh. Attributes of Quality and Polygonization Settings
related to the current group
*/

namespace PS{
namespace BLOBTREE{

using namespace PS::DATETIMEUTILS;
using namespace PS::FILESTRINGUTILS;

ConvertToBinaryTree::ConvertToBinaryTree(CLayer* aLayer, bool bPadWithNull, bool bSplitAlways)
{
    assert(aLayer != NULL);

    m_bPadWithNULL = bPadWithNull;
    m_bSplitAlways = bSplitAlways;
    m_lpLayer = aLayer;
}


void ConvertToBinaryTree::cleanup()
{
    m_lstConverted.resize(0);
}

CBlobNode* ConvertToBinaryTree::cloneNode(CBlobNode* node)
{
    CBlobNode* clone = TheBlobNodeCloneFactory::Instance().CreateObject(node);
    clone->setID(m_lpLayer->getIDDispenser().bump());

    //Add converted item to list
    ConvertToBinaryTree::CONVERTED cvt(node, clone);
    m_lstConverted.push_back(cvt);

    return clone;
}

CBlobNode* ConvertToBinaryTree::findConverted(CBlobNode* lpFrom)
{
    for(U32 i=0; i<m_lstConverted.size(); i++)
    {
        if(m_lstConverted[i].lpFrom == lpFrom)
            return m_lstConverted[i].lpTo;
    }
    return NULL;
}

int ConvertToBinaryTree::updateAllInstances()
{
    int ctDone = 0;
    for(U32 i=0; i<m_lstConverted.size(); i++)
    {
        CONVERTED cvt = m_lstConverted[i];
        if(cvt.lpTo->getNodeType() == bntPrimInstance)
        {
            CInstance* lpFrom = reinterpret_cast<CInstance*>(cvt.lpFrom);
            CInstance* lpTo = reinterpret_cast<CInstance*>(cvt.lpTo);
            lpTo->setOriginalNode(findConverted(lpFrom->getOriginalNode()));
            ctDone++;
        }
    }
}

int ConvertToBinaryTree::run()
{
    int ctConverted = 0;
    int ctErrors = CountErrors(m_lpLayer->getBlob(), m_bSplitAlways);
    if(ctErrors > 0)
    {
       m_lpLayer->getIDDispenser().reset();

        //Create a clone of the node
        CBlobNode* root = m_lpLayer->getBlob();
        CBlobNode* replacement = this->cloneNode(root);
        ctConverted = this->run(root, replacement);
        if(ctConverted > 0)
        {
            this->updateAllInstances();
            SAFE_DELETE(root);
            m_lpLayer->setBlob(replacement);
        }
        else
        {
            SAFE_DELETE(replacement);
        }

        //Update data-structures
        m_lpLayer->setOctreeFromBlobTree();
        m_lpLayer->flattenTransformations();
        m_lpLayer->queryBlobTree(true, false);
    }

    this->cleanup();
    return ctConverted;
}

int ConvertToBinaryTree::run(CBlobNode* node, CBlobNode* clonned)
{
    if ((node == NULL)||(clonned == NULL))
    {
        ReportError("Node or its clonned is NULL.");
        FlushAllErrors();
        return -1;
    }

    if(node->getNodeType() != clonned->getNodeType())
    {
        ReportError("Node and its clone are not identical.");
        FlushAllErrors();
        return -2;
    }

    bool bIsOp = node->isOperator();

    //If it is not an instance node then process
    if(bIsOp)
    {
        int res = 0;
        bool bSplit = true;
        if((!m_bSplitAlways)&&(node->isAllChildrenPrims()))
        {
            bSplit = false;
        }

        //Replace nodes with more than 2 non-primitive kids
        CBlobNode* clonnedChild = NULL;
        if(node->countChildren() > 2)
        {
            if(bSplit)
            {
                CBlobNode* cur = NULL;
                CBlobNode* prev = NULL;

                for(U32 i=0; i<node->countChildren() -1; i++)
                {
                    if(i == 0)
                        cur = clonned;
                    else
                        cur = cloneNode(clonned);

                    //Clone child
                    clonnedChild = cloneNode(node->getChild(i));

                    //Add clonned child to parent
                    cur->addChild(clonnedChild);

                    if(prev)
                        prev->addChild(cur);
                    prev = cur;

                    //Recurse to child
                    res += run(node->getChild(i), clonnedChild);
                }

                if(prev)
                {
                    clonnedChild = cloneNode(node->getLastChild());

                    //Last node has 2 children
                    prev->addChild(clonnedChild);

                    //Recurse to child
                    res += run(node->getLastChild(), clonnedChild);
                }
            }
            else //NO SPLIT
            {
                for(U32 i=0; i<node->countChildren(); i++)
                {
                    //Clone child
                    clonnedChild = cloneNode(node->getChild(i));
                    clonned->addChild(clonnedChild);
                }
            }
        }
        //Pad with NULL if it is a non-unary node
        else if(node->countChildren() <= 2)
        {
            for(size_t i=0; i<2; i++)
            {
                if(node->getChild(i))
                {
                    //Clone child
                    clonnedChild = this->cloneNode(node->getChild(i));

                    //Add clone child to clonned parent
                    clonned->addChild(clonnedChild);

                    //Recurse to child
                    res += run(node->getChild(i), clonnedChild);
                }
                else
                {
                    //If it's not a unary node then pad it with NULL
                    if(!node->isUnary() && m_bPadWithNULL)
                    {
                        clonnedChild = TheBlobNodeFactoryName::Instance().CreateObject("NULL");
                        clonnedChild->setID(m_lpLayer->getIDDispenser().bump());

                        //Add clone child to clonned parent
                        clonned->addChild(clonnedChild);

                        ReportError("Found a non-unary operator with 1 child. Added a NULL primitive.");
                        FlushAllErrors();
                    }

                    res++;
                }

            }
        }
        return res + 1;
    }
    else
        return 1;
}

int ConvertToBinaryTree::CountErrors(CBlobNode* lpNode, bool bAlwaysSplit)
{
    if(lpNode == NULL) return 0;
    if(lpNode->isOperator())
    {
        int res = 0;
        for (U32 i=0; i<lpNode->countChildren(); i++)
            res += CountErrors(lpNode->getChild(i), bAlwaysSplit);

        if(lpNode->countChildren() > 2)
        {
            if(bAlwaysSplit)
                res++;
            else
            {
                if(!lpNode->isAllChildrenPrims())
                    res++;
            }
        }
        else if((lpNode->countChildren() == 1) && (!lpNode->isUnary()))
        {
            res++;
            DAnsiStr strMsg = printToAStr("Found a non-unary node with 1 child in tree! Name:%s, ID:%d", lpNode->getName().c_str(), lpNode->getID());
            ReportError(strMsg.ptr());
            FlushAllErrors();
        }

        return res;
    }
    else
        return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
CLayer::CLayer()
{
    init(NULL);
}

CLayer::CLayer(CBlobNode * root)
{
    init(root);
}

CLayer::~CLayer()
{	
    cleanup();
}

void CLayer::init(CBlobNode* root)
{
    //BlobTree
    m_lpBlobTree = root;
    m_lpCompactBlobTree = new COMPACTBLOBTREE();

    //Properties
    m_bVisible   = true;
    m_lpMesh	 = NULL;
}

void CLayer::cleanup()
{
    selRemoveItem();
    m_lstQuery.clear();
    m_lstSelected.clear();
    removeAllSeeds();

    m_polygonizer.removeAllMPUs();
    SAFE_DELETE(m_lpBlobTree);
    SAFE_DELETE(m_lpCompactBlobTree);
    SAFE_DELETE(m_lpMesh);
}


int CLayer::queryBlobTree(bool bIncludePrims, bool bIncludeOps)
{
    if((!bIncludePrims) && (!bIncludeOps)) return 0;
    m_lstQuery.clear();
    return recursive_QueryBlobTree(bIncludePrims, bIncludeOps, m_lpBlobTree);
}

int CLayer::queryHitOctree(const CRay& ray, float t0, float t1) const
{
    COctree aOctree;

    int idxClosest = -1;
    float min_dist = FLT_MAX;
    float d;
    //Return closest query item
    for(size_t i=0; i<m_lstQuery.size(); i++)
    {
        aOctree = m_lstQuery[i]->getOctree();
        if(aOctree.intersect(ray, t0, t1))
        {
            vec3f c = aOctree.center();
            d = ray.start.distance(c);
            if(d < min_dist)
            {
                idxClosest = (int)i;
                min_dist = d;
            }
        }
    }

    return idxClosest;
}
//==================================================================
CBlobNode* CLayer::selGetItem(int index) const
{
    if(index >= 0 && index < m_lstSelected.size())
        return static_cast<CBlobNode*>(m_lstSelected[index].first);
    else
        return NULL;
}
//==================================================================
CMeshVV* CLayer::selGetMesh(int index) const
{
    if(index >= 0 && index < m_lstSelected.size())
        return static_cast<CMeshVV*>(m_lstSelected[index].second);
    else
        return NULL;
}
//==================================================================
void CLayer::selRemoveItem(int index)
{
    if(index == -1)
    {
        for(size_t i=0; i<m_lstSelected.size(); i++)
        {
            CMeshVV* lpMesh = m_lstSelected[i].second;
            SAFE_DELETE(lpMesh);
        }
        m_lstSelected.clear();
    }
    else if(index >= 0 && index < m_lstSelected.size())
    {
        CMeshVV* lpMesh  =  m_lstSelected[index].second;
        SAFE_DELETE(lpMesh);
        m_lstSelected.erase(m_lstSelected.begin() + index);
    }
}
//==================================================================
bool CLayer::selAddItem(CBlobNode* lpNode)
{
    if(lpNode == NULL) return false;

    //CMeshVV* lpMesh = new CMeshVV();
    //Run Polygonizer and export mesh for selection
    //bool bres = Run_PolygonizerExportMesh(lpNode, lpMesh, 0.2f, ISO_VALUE - 0.2f);
    PAIR_NODEMESH entry(lpNode, NULL);
    m_lstSelected.push_back(entry);
    return true;
}
//==================================================================
bool CLayer::hasMesh() const
{
    if(m_lpMesh)
        if(m_lpMesh->countFaces() > 0)
            return true;
    return false;
}

void CLayer::removeAllSeeds()
{
    m_lstSeeds.resize(0);
}

bool CLayer::getSeed(size_t index, PAIR_NODESEED &seed)
{
    if((index >=0)&&(index < m_lstSeeds.size()))
    {
        seed = m_lstSeeds[index];
        return true;
    }
    else return false;
}

size_t CLayer::countSeedPoints()
{
    return m_lstSeeds.size();
}

vec3f CLayer::getSeed(size_t index)
{
    if((index >=0)&&(index < m_lstSeeds.size()))
        return static_cast<vec3f>(m_lstSeeds[index].second);
    else
        throw "Invalid index passed to fetch a seedpoint";
    //return vec3f(0.0f, 0.0f, 0.0f);
}

size_t CLayer::getAllSeeds(size_t bufLen, vec3f arrSeeds[])
{
    size_t ctTotal = m_lstSeeds.size();
    if(bufLen < ctTotal) return -1;
    for(size_t i=0; i< ctTotal; i++)
        arrSeeds[i] = m_lstSeeds[i].second;
    return ctTotal;
}

size_t CLayer::getAllSeeds(vector<vec3f>& lstAllSeeds)
{
    size_t ctTotal = m_lstSeeds.size();
    lstAllSeeds.resize(ctTotal);
    for(size_t i=0; i< ctTotal; i++)
        lstAllSeeds[i] = m_lstSeeds[i].second;
    return ctTotal;
}


void CLayer::setOctreeFromBlobTree()
{
    if(m_lpBlobTree == NULL) return;

    CMatrix mtx;
    recursive_RecomputeAllOctrees(m_lpBlobTree, mtx);
    m_octree = m_lpBlobTree->getOctree();
}

CBlobNode* CLayer::recursive_FindNodeByID(int id, CBlobNode* root)
{
    if(root == NULL) return NULL;

    if(root->getID() == id)
        return root;

    if(root->isOperator())
    {
        for(size_t i=0; i<root->countChildren(); i++)
        {
            CBlobNode* found = recursive_FindNodeByID(id, root->getChild(i));
            if(found != NULL)
                return found;
        }
    }

    return NULL;
}

int CLayer::recursive_MaxNodeID(int maxID, CBlobNode* root)
{
    if(root == NULL)
        return maxID;

    if(root->getID() > maxID)
        maxID = root->getID();

    if(root->isOperator())
    {
        for(size_t i=0; i<root->countChildren(); i++)
        {
            int curID = recursive_MaxNodeID(maxID, root->getChild(i));
            if(curID > maxID)
                maxID = curID;
        }
    }

    return maxID;
}

bool CLayer::actBlobFindParent(CBlobNode* lpQueryNode, CBlobNode*& lpParent)
{
    lpParent = NULL;

    //Init Structure for recursion
    CmdBlobTreeParams param;
    param.lpOutParent = NULL;
    param.depth     = 0;
    param.idxChild  = -1;

    if(this->recursive_ExecuteCmdBlobtreeNode(this->getBlob(),
                                              lpQueryNode,
                                              cbtFindParent,
                                              &param))
    {
        lpParent = param.lpOutParent;
        return true;
    }

    return false;
}

bool CLayer::recursive_ExecuteCmdBlobtreeNode(CBlobNode* root,
                                              CBlobNode* lpQueryNode,
                                              cmdBlobTree command,
                                              CmdBlobTreeParams* lpParam)
{
    if(root == NULL) return false;
    if((command != cbtDelete)&&(lpParam == NULL)) return false;

    //If query node is the root
    if(root == lpQueryNode)
    {
        if(command == cbtDelete)
        {
            SAFE_DELETE(lpQueryNode);
        }
        else if(command == cbtTransformOperator)
        {
            if(lpParam->lpReplacementNode)
            {
                CBlobNode* replacement = lpParam->lpReplacementNode;
                //Prepare Replacement
                replacement->addChild(root->getChildren());
                replacement->setID(root->getID());

                //Replace now
                //root->setDeleteChildrenUponCleanup(false);
                SAFE_DELETE(root);

                this->setBlob(replacement);
                return true;
            }

        }
        else
        {
            if(lpParam)
            {
                lpParam->lpOutParent = NULL;
                lpParam->depth = 0;
                lpParam->idxChild = -1;
            }
        }
        return true;
    }

    /////////////////////////////////////////////////
    if(root->isOperator())
    {
        size_t ctKids = root->countChildren();
        if(lpParam)
            lpParam->depth++;

        //Try current children
        CBlobNode* kid = NULL;
        for(size_t i=0; i<ctKids; i++)
        {
            kid = root->getChild(i);
            if(kid != lpQueryNode)
                continue;

            //Switch
            switch(command)
            {
            case cbtDelete:
            {
                root->removeChild(i);
                return true;
            }
                break;
            case cbtFindParent:
            {
                if(lpParam)
                {
                    lpParam->lpOutParent = root;
                    lpParam->idxChild = i;
                    return true;
                }
            }
                break;
            case cbtTransformOperator:
            {
                if(lpParam->lpReplacementNode)
                {
                    CBlobNode* replacement = lpParam->lpReplacementNode;
                    //Prepare Replacement
                    replacement->addChild(kid->getChildren());
                    replacement->setID(kid->getID());

                    //Replace now
                    //kid->setDeleteChildrenUponCleanup(false);
                    root->removeChild(i);
                    root->addChild(replacement);
                    return true;
                }
                break;
            }

            }
        }//End For

        //Recurse
        for(size_t i=0; i<ctKids; i++)
        {
            if(recursive_ExecuteCmdBlobtreeNode(root->getChild(i), lpQueryNode, command, lpParam) == true)
                return true;
        }
    }

    return false;
}

void CLayer::recursive_FlattenTransformations(CBlobNode* node, const CAffineTransformation& transformBranch)
{
    if(node == NULL) return;
    if(node->isOperator() && (!node->isUnary()))
    {
        CAffineTransformation t = transformBranch;
        t.add(node->getTransform());
        node->getTransform().init();

        for(size_t i=0; i<node->countChildren(); i++)
        {
            recursive_FlattenTransformations(node->getChild(i), t);
        }
    }
    else
    {
        CAffineTransformation t = transformBranch;
        t.add(node->getTransform());
        node->getTransform().set(t);
    }
}

void CLayer::recursive_RecomputeAllOctrees(CBlobNode* node, const CMatrix& mtxBranch)
{
    if(node == NULL) return;

    CMatrix mtxCurrent = mtxBranch;
    mtxCurrent.multiply(node->getTransform().getForwardMatrix());
    if(node->isOperator())
    {
        for(size_t i=0; i<node->countChildren(); i++)        
            recursive_RecomputeAllOctrees(node->getChild(i), mtxCurrent);
        node->computeOctree();
    }
    else
    {
        //Compute Primitive BBOX and transform it
        COctree oct = node->computeOctree();
        oct.transform(mtxCurrent);
        node->setOctree(oct.lower, oct.upper);
    }
}


int CLayer::recursive_QueryBlobTree(bool bIncludePrim, bool bIncludeOps, CBlobNode* node)
{
    if(node)
    {
        if(node->isOperator())
        {
            int res = 0;
            CBlobNode* kid = NULL;
            size_t ctKids = node->countChildren();
            if(bIncludeOps)
            {
                m_lstQuery.push_back(node);
            }

            for(size_t i=0; i<ctKids; i++)
            {
                kid = node->getChild(i);
                res += recursive_QueryBlobTree(bIncludePrim, bIncludeOps, kid);
            }
            return res;
        }
        else
        {
            if(bIncludePrim)
            {
                m_lstQuery.push_back(node);
            }
            return 1;
        }
    }
    else
        return -1;

}

//////////////////////////////////////////////////////////////////////////
int CLayer::recursive_ReadBlobNode(CSketchConfig* cfg, CBlobNode* parent, int id)
{
    if(cfg == NULL) return 0;

    DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
    if(cfg->hasSection(strNodeName) < 0)
        return 0;


    //Read Node Transformation First
    bool bOperator = cfg->readBool(strNodeName, "IsOperator", false);
    if(bOperator)
    {        
        DAnsiStr strOp = cfg->readString(strNodeName, "OperatorType");
        strOp.toUpper();
        CBlobNode* opNode = NULL;

        try{
            opNode = TheBlobNodeFactoryName::Instance().CreateObject(strOp.cptr());
        }
        catch(exception e)
        {
            DAnsiStr strMsg = printToAStr("[BlobTree Script] %s [%s]", e.what(), strOp.ptr());
            ReportError(strMsg.ptr());
            FlushAllErrors();

            SAFE_DELETE(cfg);
            SAFE_DELETE(parent);
            return -1;
        }

        //Set parent relationships
        if(parent == NULL)
            m_lpBlobTree = opNode;
        else
            parent->addChild(opNode);


        //Read Operator params
        opNode->loadScript(cfg, id);
        opNode->setID(id);

        //Build Children
        int res = 0;
        {
            vector<int> arrayInt;
            int ctKids = cfg->readInt(strNodeName, "ChildrenCount", 0);
            bool bIsRange = cfg->readBool(strNodeName, "ChildrenIDsUseRange");
            if(bIsRange)
            {
                std::vector<int> vRange;
                cfg->readIntArray(strNodeName, "ChildrenIDsRange", 2, vRange);
                if(vRange.size() == 2)
                {
                    for(int i=vRange[0]; i<=vRange[1]; i++)
                        arrayInt.push_back(i);
                }
            }

            if(arrayInt.size() == 0)
                cfg->readIntArray(strNodeName, "ChildrenIDs", ctKids, arrayInt);

            //Find all the children for this operator
            for(U32 i=0; i<arrayInt.size(); i++)
                res += recursive_ReadBlobNode(cfg, opNode, arrayInt[i]);
            arrayInt.clear();
        }

        return res+1;
    }
    else
    {        
        DAnsiStr strPrimitive = cfg->readString(strNodeName, "PrimitiveType");
        CBlobNode* primNode = NULL;

        try{
            primNode = TheBlobNodeFactoryName::Instance().CreateObject(strPrimitive.cptr());
        }
        catch(exception e)
        {
            DAnsiStr strMsg = printToAStr("[BlobTree Script] %s [%s]", e.what(), strPrimitive.ptr());
            ReportError(strMsg.ptr());
            FlushAllErrors();

            return 0;
        }

        //Set parent relationship
        if(parent == NULL)
            m_lpBlobTree = primNode;
        else
            parent->addChild(primNode);


        //Set Params
        primNode->loadScript(cfg, id);
        if(primNode->isSkeletal())
            reinterpret_cast<CSkeletonPrimitive*>(primNode)->getSkeleton()->loadScript(cfg, id);
        else if(primNode->getNodeType() == bntPrimInstance)
        {
            int idxOrigin = cfg->readInt(strNodeName, "OriginalNodeIndex");
            CBlobNode* lpOrigin = this->findNodeByID(idxOrigin);
            if(lpOrigin == NULL)
            {
                DAnsiStr strMsg = printToAStr("Reading Instance node id:%d. Unable to find Origin Node id:%d", id, idxOrigin);
                ReportError(strMsg.ptr());
                FlushAllErrors();
            }
            reinterpret_cast<CInstance*>(primNode)->setOriginalNode(lpOrigin);
        }

        primNode->setID(id);
        return 1;
    }

}

int CLayer::recursive_GetBlobTreeSeedPoints(CBlobNode* node, stack<CBlobNode*> &stkOperators)
{		
    if(node)
    {
        if(node->isOperator())
        {
            CBlobNode* kid = NULL;
            size_t ctKids = 0;

            stkOperators.push(node);
            ctKids = node->countChildren();
            int res = 0;


            for(size_t i=0; i<ctKids; i++)
            {
                kid = node->getChild(i);
                stack<CBlobNode*> stkLocal(stkOperators);
                res += recursive_GetBlobTreeSeedPoints(kid, stkLocal);
            }
            return res;
        }
        else
        {
            //It is an Skeletal Primitive
            CSkeletonPrimitive * prim = dynamic_cast<CSkeletonPrimitive*>(node);
            //vec3f seed = prim->getPolySeedPoint();
            ReportError("Seed Point is not standard");

            vec3f seed;
            CBlobNode* op = NULL;

            //Apply All operators on the seed point
            while(!stkOperators.empty())
            {
                op = stkOperators.top();
                stkOperators.pop();

                seed = op->getTransform().applyForwardTransform(seed);

                switch(op->getNodeType())
                {
                case(bntOpWarpTwist):
                    seed = dynamic_cast<CWarpTwist*>(op)->warp(seed);
                    break;
                case(bntOpWarpTaper):
                    seed = dynamic_cast<CWarpTaper*>(op)->warp(seed);
                    break;
                case(bntOpWarpBend):
                    seed = dynamic_cast<CWarpBend*>(op)->warp(seed);
                    break;
                case(bntOpWarpShear):
                    seed = dynamic_cast<CWarpShear*>(op)->warp(seed);
                    break;
                default:
                {

                }

                }
            }

            //Add seed point to list of seeds
            PAIR_NODESEED MyNewPair(node, seed);
            m_lstSeeds.push_back(MyNewPair);
            return 1;
        }
    }
    else
        return -1;
}

//Finds all Seed Points
bool CLayer::setPolySeedPointAuto()
{
    removeAllSeeds();

    stack<CBlobNode*> stkOperators;
    CBlobNode *node = getBlob();
    int ctRes = recursive_GetBlobTreeSeedPoints(node, stkOperators);
    if(ctRes > 0)
    {
        PAIR_NODESEED e = m_lstSeeds[0];
        m_ptStart = e.second;
        return true;
    }
    else
        return false;
}

int CLayer::recursive_TranslateSkeleton(CBlobNode* node, vec3f d)
{
    if(node == NULL) return 0;

    if(node->isOperator())
    {
        CBlobNode* kid = NULL;
        size_t ctKids = node->countChildren();
        int res = 0;

        for(size_t i=0; i<ctKids; i++)
        {
            kid = node->getChild(i);
            res += recursive_TranslateSkeleton(kid, d);
        }
        return res;
    }
    else
    {
        //It is an Skeletal Primitive
        CSkeletonPrimitive * prim = dynamic_cast<CSkeletonPrimitive*>(node);
        prim->getSkeleton()->translate(d);
        return 1;
    }
}

int CLayer::skeletTranslate(vec3f d)
{
    return recursive_TranslateSkeleton(m_lpBlobTree, d);
}

void CLayer::setOctree(vec3f lower, vec3f upper)
{	
    m_octree.lower = lower;
    m_octree.upper = upper;
}

void CLayer::setOctreeFromMesh()
{	
    if(m_lpMesh == NULL) return;
    vec3f lo, hi;
    m_lpMesh->getExtremes(lo, hi);
    m_octree.lower = lo;
    m_octree.upper = hi;
}

bool CLayer::calcPolyBounds()
{	
    if(m_lpBlobTree == NULL) return false;
    if(m_polyCellSize <= 0.0f) return false;

    COctree oct = m_lpBlobTree->getOctree();
    m_polyBounds.x = static_cast<int>(ceil((oct.upper.x - oct.lower.x) / m_polyCellSize));
    m_polyBounds.y = static_cast<int>(ceil((oct.upper.y - oct.lower.y) / m_polyCellSize));
    m_polyBounds.z = static_cast<int>(ceil((oct.upper.z - oct.lower.z) / m_polyCellSize));

    return true;
}


void CLayer::setMesh()
{
    if(m_lpMesh == NULL)
        m_lpMesh = new CMeshVV();
    else
        m_lpMesh->removeAll();
}

//Copy from another mesh
void CLayer::setMesh(const CMeshVV& other) 
{	
    if(m_lpMesh == NULL)
        m_lpMesh = new CMeshVV(other);
    else
        m_lpMesh->copyFrom(other);
}


//load from file
void CLayer::setMesh(const DAnsiStr& strFileName)
{	
    setMesh();
    m_lpMesh->open(strFileName);
}

bool CLayer::saveAsVolumeData(const char* chrFileName, int w, int h, int d)
{
    ofstream ofs(chrFileName, ofstream::binary);
    if(!ofs.is_open())
        return false;

    char* buffer = new char[w*h*d];
    if(saveAsVolumeData(buffer, w, h, d))
    {
        ofs.write(buffer, w*h*d);
        ofs.close();

        delete [] buffer; buffer = NULL;
        return true;
    }
    else
    {
        delete [] buffer; buffer = NULL;
        return false;
    }

}

bool CLayer::saveAsVolumeData(U8* buffer, int w, int h, int d)
{
    CBlobNode* root = getBlob();
    if((root == NULL) || (m_lpMesh == NULL) || (buffer == NULL))
    {

        return false;
    }


    vec3f lower(0.0f, 0.0f, 0.0f);
    vec3f upper(0.5f, 0.5f, 0.5f);

    m_lpMesh->fitTo(lower, upper);
    m_lpMesh->getExtremes(lower, upper);

    float stepX = (upper.x - lower.x) / w;
    float stepY = (upper.y - lower.y) / h;
    float stepZ = (upper.z - lower.z) / d;
    vec3f pt;

    for (int i=0; i < w; i++)
    {
        for (int j=0; j < h; j++)
        {
            for (int k=0; k < d; k++)
            {
                pt = lower + vec3f(i*stepX, j*stepY, k*stepZ);
                float fv = root->fieldValue(pt);
                if(fv > 1.0f)
                    throw "FieldValue overflow!";
                U8 val = static_cast<U8>(fv * 255);
                //data[j*d*w + k*w + i] = val;
                //data[k*h*w + j*w + i ] = val;
                buffer[i*h*d + j*d + k] = val;
            }
        }
    }

    return true;
}

DAnsiStr CLayer::getMeshInfo() const
{
    DAnsiStr strMesh;
    if(hasMesh())
        strMesh = PS::printToAStr("F#%i, V#%i", m_lpMesh->countFaces(), m_lpMesh->countVertices());
    else
        strMesh = "Empty";
    return strMesh;
}

void CLayer::getMeshInfo( size_t& ctVertices, size_t& ctFaces )
{
    ctVertices = 0;
    ctFaces = 0;
    if(m_lpMesh)
    {
        ctVertices = m_lpMesh->countVertices();
        ctFaces = m_lpMesh->countFaces();
    }
}

int CLayer::convertToBinaryTree(bool bPadWithNull, bool bAlwaysSplit)
{
    ConvertToBinaryTree* lpConverter = new ConvertToBinaryTree(this, bPadWithNull, bAlwaysSplit);
    int res = lpConverter->run();
    SAFE_DELETE(lpConverter);
    return res;
}

int CLayer::queryGetAllOctrees( vector<vec3f>& los, vector<vec3f>& his ) const
{	
    size_t ctOctrees = m_lstQuery.size();
    if(ctOctrees == 0)
        return 0;

    COctree oct;
    //Resize once for better performance
    los.resize(ctOctrees);
    his.resize(ctOctrees);
    for(size_t i=0; i<ctOctrees; i++)
    {
        oct = m_lstQuery[i]->getOctree();
        los[i] = oct.lower;
        his[i] = oct.upper;
    }
    return (int)ctOctrees;
}

void CLayer::flattenTransformations()
{
    if(m_lpBlobTree == NULL) return;
    CAffineTransformation transformBranch;
    recursive_FlattenTransformations(m_lpBlobTree, transformBranch);
}

/*!
  * @brief Finds a BlobNode with its id
  * @return pointer to the BlobNode
  */
CBlobNode* CLayer::findNodeByID( int id )
{
    return recursive_FindNodeByID(id, m_lpBlobTree);
}

int CLayer::recursive_ReassignIDs(CBlobNode* root)
{
    if(root == NULL)
        return 0;

    int ctProcessed = 1;
    root->setID(this->getIDDispenser().get());
    this->getIDDispenser().bump();

    if(root->isOperator())
    {
        for(int i=0; i<root->countChildren(); i++)
            ctProcessed += recursive_ReassignIDs(root->getChild(i));
    }

    return ctProcessed;
}

/*!
  * @brief Assigns node IDs
  *
  */
bool CLayer::reassignBlobNodeIDs()
{
    this->getIDDispenser().reset();
    int ctProcessed = recursive_ReassignIDs(this->getBlob());
    return (ctProcessed > 0);
}

//////////////////////////////////////////////////////////////////////////
CLayerManager::CLayerManager(CLayer * aLayer)
{
    m_idxActiveLayer = -1;
    m_lstLayers.push_back(aLayer);
}

void CLayerManager::removeAllLayers()
{		
    for(size_t i=0; i< m_lstLayers.size(); i++)
        delete m_lstLayers[i];
    m_lstLayers.clear();
    m_idxActiveLayer = -1;
}

size_t CLayerManager::countLayers() const {return m_lstLayers.size();}

bool CLayerManager::isLayerIndex(int index) const
{ 
    return ((index >=0)&&(index < (int)m_lstLayers.size()));
}

CLayer* CLayerManager::getLayer(int index) const
{
    return (isLayerIndex(index)) ? m_lstLayers[index] : NULL;
}

CLayer* CLayerManager::getLast() const
{
    int ctSize = (int)m_lstLayers.size();
    if(ctSize > 0)
        return m_lstLayers[ctSize - 1];
    else
        return NULL;
}

bool CLayerManager::calcPolyBounds()
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        if(!m_lstLayers[i]->calcPolyBounds())
            return false;
    }
    return true;
}

void CLayerManager::setCellSize(float side)
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        m_lstLayers[i]->setCellSize(side);
    }
}

void CLayerManager::setCellShape(CellShape poly)
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        m_lstLayers[i]->setCellShape(poly);
    }
}

void CLayerManager::setPolyBounds(int bounds)
{
    setPolyBounds(vec3i(bounds, bounds, bounds));
}

void CLayerManager::setPolyBounds(vec3i bounds)
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        m_lstLayers[i]->setPolyBounds(bounds);
    }
}

void CLayerManager::setMeshQuality(int quality)
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        m_lstLayers[i]->setMeshQuality(quality);
    }
}

void CLayerManager::addLayer(const CBlobNode* blob, const char* chrLayerName)
{
    addLayer(blob, NULL, chrLayerName);
}

void CLayerManager::addLayer(const CBlobNode* blob,
                             const char* chrMeshFileName,
                             const char* chrLayerName)
{
    CLayer * aLayer = new CLayer(const_cast<CBlobNode*>(blob));
    aLayer->setCellSize(DEFAULT_CELL_SIZE);
    aLayer->setPolyBounds(DEFAULT_BOUNDS);
    aLayer->setPolySeedPoint(vec3f(0.0f, 0.0f, 0.0f));
    aLayer->setVisible(true);

    //Load Mesh
    if(chrMeshFileName != NULL)
        if(PS::FILESTRINGUTILS::FileExists(chrMeshFileName))
            aLayer->setMesh(DAnsiStr(chrMeshFileName));

    //Set Mesh FileName
    if(chrLayerName == NULL)
        aLayer->setGroupName("Blob Layer");
    else
        aLayer->setGroupName(DAnsiStr(chrLayerName));

    addLayer(aLayer);
}

void CLayerManager::addLayer(CLayer * alayer)
{
    if(alayer != NULL) m_lstLayers.push_back(alayer);
}

void CLayerManager::eraseAllMeshes()
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        if(m_lstLayers[i]->hasMesh())
            m_lstLayers[i]->getMesh()->removeAll();
    }
}

void CLayerManager::resetAllMeshes()
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
        m_lstLayers[i]->setMesh();
}

bool CLayerManager::saveAsVolumeData(const char* strDir, int w, int h, int d)
{

    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        DAnsiStr strPath = DAnsiStr(strDir) + printToAStr("//LAYERNUM_%d.raw", i);
        if(!m_lstLayers[i]->saveAsVolumeData(strPath.ptr(), w, h, d))
            return false;
    }
    return true;
}

bool CLayerManager::setPolySeedPointAuto()
{
    for(size_t i=0; i < m_lstLayers.size(); i++)
    {
        if(!getLayer(i)->setPolySeedPointAuto())
            return false;
    }
    return true;
}

void CLayerManager::setOctreesFromBlobTree()
{
    for(size_t i=0; i < m_lstLayers.size(); i++)
        getLayer(i)->setOctreeFromBlobTree();
}

void CLayerManager::setOctreesFromMeshes()
{
    for(size_t i=0; i < m_lstLayers.size(); i++)
        getLayer(i)->setOctreeFromMesh();
}


CLayer* CLayerManager::getActiveLayer() const
{
    if(isLayerIndex(m_idxActiveLayer))
        return m_lstLayers[m_idxActiveLayer];
    else
        return NULL;
}

void CLayerManager::setActiveLayer( int idxLayer )
{
    if(isLayerIndex(idxLayer))
        m_idxActiveLayer = idxLayer;
    else
        m_idxActiveLayer = -1;
}

bool CLayerManager::hasActiveSelOctree() const
{
    CLayer* aLayer = getActiveLayer();
    if(aLayer)
        if(aLayer->selCountItems() > 0)
            return aLayer->selGetItem(0)->getOctree().isValid();
    return false;
}

COctree CLayerManager::getActiveSelOctree() const
{
    COctree result;
    CLayer* aLayer = getActiveLayer();
    if(aLayer)
        if(aLayer->selCountItems() > 0)
            result = aLayer->selGetItem(0)->getOctree();
    return result;
}

void CLayerManager::removeLayer( int idxLayer )
{
    if(isLayerIndex(idxLayer))
    {
        CLayer* alayer = getLayer(idxLayer);
        m_lstLayers.erase(m_lstLayers.begin() + idxLayer);
        SAFE_DELETE(alayer);
    }
}

size_t CLayerManager::countOctrees() const
{
    size_t ctOcts = 0;
    for(size_t i=0; i<m_lstLayers.size(); i++)
        if(getLayer(i)->hasOctree()) ctOcts++;
    return ctOcts;
}

void CLayerManager::bumpRevisions()
{
    for(size_t i=0; i<m_lstLayers.size(); i++)
        getLayer(i)->getTracker().bump();
}

void CLayerManager::resetRevisions()
{
    for(size_t i=0; i<m_lstLayers.size(); i++)
        getLayer(i)->getTracker().reset();
}

int CLayerManager::hitLayerOctree( const CRay& ray, float t0, float t1 ) const
{
    for(size_t i=0; i<m_lstLayers.size(); i++)
    {
        if(getLayer(i)->hasOctree() && getLayer(i)->isVisible())
        {
            if(getLayer(i)->getOctree().intersect(ray, t0, t1))
            {
                return i;
            }
        }
    }

    return -1;
}

//First hit a layer then if successful hit a primitive within that layer
bool CLayerManager::queryHitOctree(const CRay& ray, float t0, float t1, int& idxLayer, int& idxFound)
{
    idxFound = -1;
    idxLayer = hitLayerOctree(ray, t0, t1);
    if(isLayerIndex(idxLayer))
    {
        idxFound = getLayer(idxLayer)->queryHitOctree(ray, t0, t1);
        if(idxFound >= 0)
            return true;
    }
    return false;
}

void CLayerManager::selRemoveItems()
{	
    for(size_t i=0; i<m_lstLayers.size(); i++)
        getLayer(i)->selRemoveItem(-1);
}

void CLayerManager::setAllVisible( bool bVisible )
{
    for(size_t i=0; i<m_lstLayers.size(); i++)
        getLayer(i)->setVisible(bVisible);
}

bool CLayerManager::saveScript( const DAnsiStr& strFN )
{
    if(m_lstLayers.size() == 0) return false;

    bool bres;
    if(this->countLayers() > 1)
    {
        bres = true;

        DAnsiStr strPath = PS::FILESTRINGUTILS::ExtractFilePath(strFN);
        DAnsiStr strTitle = PS::FILESTRINGUTILS::ExtractFileTitleOnly(strFN);
        for(int i=0; i<this->countLayers(); i++)
        {
            DAnsiStr strActualPath = strPath + strTitle;
            strActualPath += printToAStr("_L%d.scene", i+1);
            CSketchConfig* lpSketchConfig = new CSketchConfig(strActualPath, CAppConfig::fmWrite);
            bres &= saveScript(i, lpSketchConfig);
            SAFE_DELETE(lpSketchConfig);
        }
    }
    else
    {
        CSketchConfig* lpSketchConfig = new CSketchConfig(strFN, CAppConfig::fmWrite);
        bres = saveScript(0, lpSketchConfig);
        SAFE_DELETE(lpSketchConfig);
    }

    return bres;
}

bool CLayerManager::saveScript(int idxLayer, CSketchConfig* lpSketchConfig )
{
    if(lpSketchConfig == NULL) return false;
    if(!isLayerIndex(idxLayer)) return false;

    //Save Stats from Version 4
    int ctPrims = 0;
    int ctOps = 0;
    int depth = 0;

    CLayer* aLayer = getLayer(idxLayer);
    vector<int> layersRoot;
    if(aLayer->hasBlob())
    {
        layersRoot.push_back(aLayer->getBlob()->getID());
        aLayer->getBlob()->saveScript(lpSketchConfig);
        ctOps   = aLayer->getBlob()->recursive_CountOperators();
        ctPrims = aLayer->getBlob()->recursive_CountPrimitives();
        depth   = aLayer->getBlob()->recursive_Depth();
    }
    else
        layersRoot.push_back(-1);

    //Save Global Info for the BlobTree
    lpSketchConfig->writeInt("Global", "FileVersion", SCENE_FILE_VERSION);
    lpSketchConfig->writeInt("Global", "NumLayers", m_lstLayers.size());
    lpSketchConfig->writeInt("Global", "CurrentLayer", idxLayer + 1);
    lpSketchConfig->writeInt("Global", "CountPrimitives", ctPrims);
    lpSketchConfig->writeInt("Global", "CountOperators", ctOps);
    lpSketchConfig->writeInt("Global", "Depth", depth);
    lpSketchConfig->writeIntArray("Global", "RootIDs", layersRoot);
    return true;
}

bool CLayerManager::loadScript( const DAnsiStr& strFN )
{
    if(!PS::FILESTRINGUTILS::FileExists(strFN.ptr())) return false;
    //DAnsiStr strTitle = PS::FILESTRINGUTILS::ExtractFileTitleOnly(strFN);

    CSketchConfig* cfg = new CSketchConfig(strFN, CAppConfig::fmRead);
    bool bres = loadScript(cfg);
    SAFE_DELETE(cfg);
    return bres;
}

bool CLayerManager::loadScript( CSketchConfig* lpSketchConfig )
{
    removeAllLayers();
    int ctLayers = lpSketchConfig->readInt("Global", "NumLayers");
    if(ctLayers <= 0)
        return false;

    int fileVersion = lpSketchConfig->readInt("Global", "FileVersion");
    cout << "Scene File Version = " << fileVersion << endl;

    vector<int> layersRoot;
    lpSketchConfig->readIntArray("Global", "RootIDs", ctLayers, layersRoot);

    CLayer* aLayer = NULL;
    for(U32 i=0; i<layersRoot.size(); i++)
    {
        aLayer = new CLayer();
        aLayer->setCellSize(DEFAULT_CELL_SIZE);
        aLayer->setPolyBounds(DEFAULT_BOUNDS);
        aLayer->setPolySeedPoint(vec3f(0.0f, 0.0f, 0.0f));
        aLayer->setVisible(true);
        aLayer->setGroupName(printToAStr("Layer %d", i));
        if(layersRoot[i] >= 0)
        {
            aLayer->recursive_ReadBlobNode(lpSketchConfig, NULL, layersRoot[i]);
            aLayer->getTracker().bump();
        }

        //Update lastNodeID
        int lastUsedID = aLayer->recursive_MaxNodeID(-1, aLayer->getBlob());
        aLayer->getIDDispenser().set(lastUsedID + 1);

        addLayer(aLayer);
    }

    //Set Active Layer
    setActiveLayer(0);

    layersRoot.clear();
    return true;
}

void CLayerManager::setAdaptiveParam( float param )
{
    for(size_t i=0; i< m_lstLayers.size(); i++)
    {
        m_lstLayers[i]->setAdaptiveParam(param);
    }
}

}
}


