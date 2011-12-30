#ifndef CLAYERMANAGER_H
#define CLAYERMANAGER_H

#include <stack>

#include "_PolygonizerStructs.h"
#include "PS_SketchConfig.h"
#include "PS_FrameWork/include/PS_MeshVV.h"
#include "PS_FrameWork/include/PS_Octree.h"
#include "PS_FrameWork/include/PS_String.h"
#include "PS_FrameWork/include/PS_Tracker.h"


#include "PS_BlobTree/include/CVolumeBox.h"
#include "PS_BlobTree/include/CBlobTree.h"
#include "PS_BlobTree/include/CSkeleton.h"
#include "PS_BlobTree/include/CSkeletonPrimitive.h"


#include "CompactBlobTree.h"
#include "CPolyParsipOptimized.h"
#include "_GlobalFunctions.h"
#include "loki/Singleton.h"

using namespace std;
using namespace Loki;
//using namespace PS::SIMDPOLY;
/*
 Layer will group a set of BlobNodes so that they can be polygonized together.
 Layer will also holds the resulting mesh. Attributes of Quality and Polygonization Settings
 related to the current group
*/
namespace PS{
namespace BLOBTREE{

typedef pair<CBlobNode*, vec3f> PAIR_NODESEED;
typedef pair<CBlobNode*, CMeshVV*> PAIR_NODEMESH;
enum cmdBlobTree{cbtDelete, cbtFindParent, cbtFindDepth, cbtTransformOperator};

struct CmdBlobTreeParams
{	
    int idxChild;
    int depth;
    CBlobNode* lpOutParent;
    CBlobNode* lpReplacementNode;
};


/*!
  * Each layer is a separate BlobTree with its own parameters for rendering.
  *
  */
class CLayer
{
public:
    friend class CParsipOptimized;

private:
    //Pointer to a blobTree parent node
    CBlobNode* m_lpBlobTree;

    //Compact Blobtree
    COMPACTBLOBTREE m_cptBlobTree;

    //Layer Polygonizer
    CParsipOptimized m_polygonizer;

    //List of seed points and the primitives that they originated from
    vector<PAIR_NODESEED> m_lstSeeds;

    //Selected pairs of BlobTree Nodes and their corresponding meshes
    vector<PAIR_NODEMESH> m_lstSelected;

    //Keeping a list of nodes
    vector<CBlobNode*> m_lstQuery;

    //Octree for Collision Detection
    COctree m_octree;

    //Mesh
    CMeshVV* m_lpMesh;

    //Group Name
    DAnsiStr m_strGroupName;

    CellShape m_polyCellShape;
    float m_polyCellSize;
    float m_adaptiveParam;

    //Identify changes for next polygonization
    RevisionTracker m_tracker;

    //ID Assignment to new nodes
    RevisionTracker m_idDispenser;

    //Is layer visible?
    bool  m_bVisible;
    bool  m_bUseParallel;
    vec3i m_polyBounds;
    vec3f m_ptStart;
    int   m_meshQuality;

    void recursive_FlattenTransformations(CBlobNode* node, const CAffineTransformation& transformBranch);
    void recursive_RecomputeAllOctrees(CBlobNode* node, const CMatrix& mtxBranch);
    int recursive_QueryBlobTree(bool bIncludePrim, bool bIncludeOps, CBlobNode* node);

    int recursive_GetBlobTreeSeedPoints(CBlobNode* node, stack<CBlobNode*> &stkOperators);
    int recursive_TranslateSkeleton(CBlobNode*node, vec3f d);    
    int recursive_countBinaryTreeErrors( CBlobNode* node );

public:
    CLayer();
    CLayer(CBlobNode * node);
    CLayer(const CMeshVV& mesh);

    ~CLayer();


    //Get Polygonizer
    //CParsipOptimized& getPolygonizer() { return m_optParsip;}
    int convertToBinaryTree();


    void setupCompactTree(CBlobNode* root)
    {
        m_cptBlobTree.reset();
        m_cptBlobTree.convert(root);
    }

    COMPACTBLOBTREE* getCompactTree() {return &m_cptBlobTree;}
    CParsipOptimized* getPolygonizer() {return &m_polygonizer;}

    void cleanup();

    CBlobNode* findNodeByID(int id);    
    CBlobNode* recursive_FindNodeByID(int id, CBlobNode* root);
    int recursive_convertToBinaryTree(CBlobNode* node, CBlobNode* clonned);
    int recursive_MaxNodeID(int maxID, CBlobNode* root);
    bool recursive_ExecuteCmdBlobtreeNode(CBlobNode* root, CBlobNode* lpQueryNode, cmdBlobTree command, CmdBlobTreeParams* lpParam = NULL);

    /*!
      * Recursively reads a BlobTree from Disk
      */
    int recursive_ReadBlobNode(CSketchConfig* cfg, CBlobNode* parent, int id);

    //Implicit BlobTree
    CBlobNode* getBlob() const { return m_lpBlobTree; }
    void setBlob(CBlobNode * blob) {m_lpBlobTree = blob;}
    bool hasBlob() const { return (m_lpBlobTree != NULL);}

    //Track changes
    RevisionTracker& getTracker() {return m_tracker;}
    RevisionTracker& getIDDispenser() {return m_idDispenser;}


    //Visibility
    void setVisible(bool bVisible) { m_bVisible = bVisible;}
    bool isVisible() const { return m_bVisible;}


    DAnsiStr getMeshInfo() const;
    DAnsiStr getGroupName() const 	{return m_strGroupName;}
    void setGroupName(const DAnsiStr& strGroupName) { m_strGroupName = strGroupName;}

    //Octree for whole model
    bool	hasOctree() const {return m_octree.isValid();}
    COctree getOctree() const {return m_octree;}
    void	setOctree(vec3f lower, vec3f upper);
    void	setOctreeFromMesh();
    void	setOctreeFromBlobTree();

    //Post multiply all branch transformations and apply them to leaf nodes
    //Other nodes with have identity matrices
    void	flattenTransformations();

    //Query BlobTree, this will fill query list with prims or ops or both
    int	 queryBlobTree(bool bIncludePrims, bool bIncludeOps);
    size_t queryCountItems() const { return m_lstQuery.size();}

    //Actions For All Nodes in the Query List
    CBlobNode* queryGetItem(int index) const
    {
        if(index >= 0 && index < m_lstQuery.size())
            return m_lstQuery[index];
        else
            return NULL;
    }

    int queryGetAllOctrees(vector<vec3f>& los, vector<vec3f>& his) const;
    int queryHitOctree(const CRay& ray, float t0, float t1) const;

    //Selection
    CBlobNode* selGetItem(int index = 0) const;
    CMeshVV* selGetMesh(int index = 0) const;
    size_t selCountItems() const {return m_lstSelected.size();}
    void selRemoveItem(int index = -1);
    bool selAddItem(CBlobNode* lpNode);

    //Seed Points
    size_t countSeedPoints();
    vec3f getSeed(size_t index);
    bool getSeed(size_t index, PAIR_NODESEED &seed);
    size_t getAllSeeds(size_t bufLen, vec3f arrSeeds[]);
    size_t getAllSeeds(vector<vec3f>& lstAllSeeds);
    void removeAllSeeds();
    vec3f getPolySeedPoint() const { return m_ptStart;}
    void setPolySeedPoint(vec3f ptStart) { m_ptStart = ptStart;}
    bool setPolySeedPointAuto();
    int  skeletTranslate(vec3f d);

    //Polygonization
    bool getParallelMode() const { return m_bUseParallel;}
    void setParallelMode(bool bParallel) { m_bUseParallel = bParallel;}

    float getCellSize() const { return m_polyCellSize;}
    void setCellSize(float side) { m_polyCellSize = side;}
    float getAdaptiveParam() const {return m_adaptiveParam;}
    void setAdaptiveParam(float param) { m_adaptiveParam = param;}

    CellShape getCellShape() const { return m_polyCellShape;}
    void setCellShape(CellShape poly) { m_polyCellShape = poly;}

    bool calcPolyBounds();
    vec3i getPolyBounds() const { return m_polyBounds;}
    void setPolyBounds(vec3i bounds) { m_polyBounds = bounds;}
    void setPolyBounds(int bound)   { m_polyBounds.set(bound, bound, bound);}

    //Mesh for other graphics objects imported
    bool hasMesh() const;
    int getMeshQuality() const  { return m_meshQuality;}
    void setMeshQuality(int quality) { m_meshQuality = quality;}
    void getMeshInfo(size_t& ctVertices, size_t& ctFaces);
    CMeshVV* getMesh() const {return m_lpMesh;}
    void setMesh();
    void setMesh(const CMeshVV& other);
    void setMesh(const DAnsiStr& strFileName);


    bool saveAsVolumeData(U8* buffer, int w, int h, int d);
    bool saveAsVolumeData(const char* strFileName, int w, int h, int d);
};

//LayerManager is the entire Data-Structure for the Scene
//Holds a set of layers which will constitute a scene
class CLayerManager
{
private:
    std::vector<CLayer*> m_lstLayers;
    int m_idxActiveLayer;

public:
    CLayerManager() { m_idxActiveLayer = -1;}
    CLayerManager(CLayer * aLayer);

    void removeAllLayers();

    size_t countLayers() const;


    //Layers Versioning
    void	 bumpRevisions();
    void	 resetRevisions();

    //Access to layers
    bool	 isLayerIndex(int index) const;
    CLayer*      getLayer(int index) const;
    int		 getActiveLayerIndex() const {return m_idxActiveLayer;}
    bool	 hasActiveLayer() const {return isLayerIndex(m_idxActiveLayer);}

    CLayer*  getActiveLayer() const;
    bool	 hasActiveSelOctree() const;
    COctree  getActiveSelOctree() const;
    void	 setActiveLayer(int idxLayer);
    CLayer*  getLast() const;

    void setAllVisible(bool bVisible);

    bool calcPolyBounds();

    //Octree Management
    size_t countOctrees() const;
    void setOctreesFromBlobTree();
    void setOctreesFromMeshes();
    int  hitLayerOctree(const CRay& ray, float t0, float t1) const;

    int	 computeAllPrimitiveOctrees();
    bool queryHitOctree(const CRay& ray, float t0, float t1, int& idxLayer, int& idxPrimitive);
    void selRemoveItems();

    void setAdaptiveParam(float param);
    void setCellSize(float side);
    void setCellShape(CellShape poly);

    void setPolyBounds(int bounds);

    void setPolyBounds(vec3i bounds);

    bool setPolySeedPointAuto();

    void setMeshQuality(int quality);

    void reduceAllBlobTrees();
    void resetAllMeshes();

    void addLayer(const CBlobNode* blob, const char* chrLayerName = NULL);
    void addLayer(const CBlobNode* blob, const char* chrMeshFileName, const char* chrLayerName);
    void addLayer(CLayer * aLayer);

    void removeLayer(int idxLayer);


    void eraseAllMeshes();

    bool saveAsVolumeData(const char* strDir, int w, int h, int d);

    //Serialization    
    bool saveScript(const DAnsiStr& strFN);
    bool saveScript(int idxLayer, CSketchConfig* lpSketchConfig);

    bool loadScript(const DAnsiStr& strFN);
    bool loadScript(CSketchConfig* lpSketchConfig);


    CLayer* operator[](int index)
    {
        return (isLayerIndex(index)) ? m_lstLayers[index] : NULL;
    }

};

typedef SingletonHolder<CLayerManager, CreateUsingNew, PhoenixSingleton> TheLayerManager;

}
}
#endif 
