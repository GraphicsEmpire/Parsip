#ifndef CFieldCache_H
#define CFieldCache_H

#include <math.h>
#include "CBlobTree.h"
#include "CVolumeBox.h"
#include "CAdaptiveUniformGrid3d.h"

namespace PS{
namespace BLOBTREE{

typedef enum CacheType {PassThrough, AdaptiveUniformGrid};
typedef enum SampleMode {TriLinear, TriQuadratic};

class  CFieldCache : public CBlobTree
{
public:	
	CFieldCache();
	CFieldCache(CBlobTree * child, CacheType type = AdaptiveUniformGrid);
	~CFieldCache();

	float fieldValue(vec3f p);
	
	vec3 normal(vec3f p, float delta);


	void invalidate(BLOBTREE::Vol::CVolumeBox& box);
	void invalidate();

	float curvature(vec3f p)
	{
		return m_children[0]->curvature(p);
	}

	vec4f baseColor(vec3f p)
	{
		return m_children[0]->baseColor(p);
	}

	CMaterial baseMaterial(vec3f p)
	{
		return m_children[0]->baseMaterial(p);
	}

	//Cache Resolution
	int getCacheResolution() const {return m_cacheResolution;}
	void setCacheResolution(int res) { m_cacheResolution = res;}

	//Origin of the Grid
	vec3 getOrigin() const {return m_origin;}
	void setOrigin(const vec3 origin) {m_origin = origin;}

	//Cache Type default is Adaptive
	CacheType getCacheType() {return m_cacheType;}
	void setCacheType(CacheType type) { m_cacheType = type;}

	//Sample mode is either Linear or Quadratic
	SampleMode getFieldValueSampleMode() const {return m_eFieldValueSampleMode;}
	void setFieldValueSampleMode(SampleMode mode) { m_eFieldValueSampleMode = mode;}
	
	//Sample mode is either Linear or Quadratic
	SampleMode getGradientSampleMode() const {return m_eGradientSampleMode;}
	void setGradientSampleMode(SampleMode mode) { m_eGradientSampleMode = mode;}	

	virtual void * GetCache();

	virtual void NotifyFieldBoundsChanged( const COctree& bounds );

	virtual void UpdateCacheResolution( unsigned int nResolution );


	void getName(char * chrName)
	{
            strncpy(chrName, "CACHE", MAX_NAME_LEN);
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();		
		return m_octree;
	}


	bool isOperator() { return true;}


	BlobNodeType getNodeType() {return bntOpCache;}

protected:
	SampleMode m_eFieldValueSampleMode;
	SampleMode m_eGradientSampleMode;

	vec3 m_origin;
	CacheType m_cacheType;

	CAdaptiveUniformGrid3D *m_pCache;

	unsigned int m_cacheResolution;
};


//CFieldCacheManager manages and sets all cache nodes at once.
class CFieldCacheManager
{
public:
	CFieldCacheManager();
	virtual ~CFieldCacheManager();

	static CFieldCacheManager * Get();

	CFieldCache * CreateCache(CBlobTree * pField, CacheType eType );
	void FreeCache( CFieldCache * pCache );

	void SetDefaultValueMode( SampleMode eMode, bool bSetAllCaches );
	SampleMode GetDefaultValueMode();

	void SetDefaultGradientMode( SampleMode eMode, bool bSetAllCaches );
	SampleMode GetDefaultGradientMode();

	void SetDefaultCacheResolution( unsigned int nResolution, bool bSetAllCaches );
	unsigned int GetDefaultCacheResolution();

	//void OptimizeCacheTree( CBlobTree * pRootNode );

	void EnableCaching( bool bEnable );
	bool isCachingEnabled() { return m_bCachingEnabled; }

	// optimize cache tree for sketch mode
	//void OptimizeCacheTree_Sketching(CBlobTree * pRootNode  );

	//void RecursiveDisableCaching(CBlobTree * pNode, int nMaxCacheDepth, int nCurDepth );

protected:
	static CFieldCacheManager * m_pCacheManager;

	SampleMode m_eDefaultValueMode;
	SampleMode m_eDefaultGradientMode;

	unsigned int m_nDefaultCacheResolution;
	bool m_bCachingEnabled;

	//EXPORT_STL_VECTOR( RMSBLOBTREE_API, CFieldCache *);
	std::vector<CFieldCache *> m_vCaches;
};

}
}

#endif
