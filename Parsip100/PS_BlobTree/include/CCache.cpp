#include "CCache.h"

namespace PS{
namespace BLOBTREE{
/*
class FieldWithFrame : public rmsimplicit::ScalarField
{
public:
	FieldWithFrame( rmsimplicit::ScalarField * pField )
		{ m_pField = pField; }
	virtual ~FieldWithFrame() {}

	Wml::Frame & Frame() { return m_frame; }

	virtual float Value( float x, float y, float z )
	{
		vec3 v(x,y,z);
		v = m_frame.FrameMatrix() * v;
		v += m_frame.Origin();
		return m_pField->Value( v.X(), v.Y(), v.Z() );
	}

	virtual void Gradient( float x, float y, float z, float & gradX, float & gradY, float & gradZ, float * pFieldValue) { abort(); }
	virtual void Color( float x, float y, float z, float & colorR, float & colorG, float & colorB ){abort();}
	virtual void FieldBounds( Wml::AxisAlignedBox3f & aaBox ) { abort(); m_pField->FieldBounds(aaBox); }
	virtual void SurfaceBounds( float isoValue, Wml::AxisAlignedBox3f & aaBox ) { abort(); m_pField->SurfaceBounds(isoValue, aaBox); }
	virtual void GetSeedPoints( std::vector<vec3> & seedPoints ) {abort();}

protected:
	rmsimplicit::ScalarField * m_pField;
	Wml::Frame m_frame;
};

*/


CFieldCache::CFieldCache()
{
	m_cacheType = AdaptiveUniformGrid;
}

/*
 * CAdaptiveUniformGrid3D cache
 */

CFieldCache::CFieldCache(CBlobTree * child, CacheType type)
{
	//First copy all parameters inside
	addChild(child);
	m_cacheType = type;

	//Compute bounding box and center
	vec3 center = child->getOctree().center();	
	m_pCache = new CAdaptiveUniformGrid3D(1.0f, center);
	m_pCache->SetOrigin(vec3(0.0f, 0.0f, 0.0f));

	//FieldValue and Gradient Sample Modes
	m_eFieldValueSampleMode = CFieldCacheManager::Get()->GetDefaultValueMode();
	m_eGradientSampleMode   = CFieldCacheManager::Get()->GetDefaultGradientMode();
}

CFieldCache::~CFieldCache()
{
	if(m_pCache)
		delete m_pCache;
}

float CFieldCache::fieldValue(vec3 p)
{
	if(m_cacheType == PassThrough)
	{	
		return m_children[0]->fieldValue(p);
	}
	else
	{
		switch (m_eFieldValueSampleMode) 
		{
		case TriLinear:
			return m_pCache->SampleTriLinear(p.x, p.y, p.z, m_children[0]);
		case TriQuadratic:
			return m_pCache->SampleTriQuadratic(p.x, p.y, p.z, m_children[0]);
		default:
			return m_pCache->SampleTriLinear(p.x, p.y, p.z, m_children[0]);
		}		
	}	
}

vec3 CFieldCache::normal(vec3 p, float delta)
{	
	if(m_cacheType == PassThrough)
	{
		return m_children[0]->normal(p, delta);
	}
	else
	{
		float fGradX, fGradY, fGradZ;
		float pFieldValue; 
		CBlobTree* frameField =  m_children[0];
		switch(m_eGradientSampleMode)
		{
		case TriLinear:
			m_pCache->SampleGradientTriLinear( p.x, p.y, p.z, fGradX, fGradY, fGradZ, &pFieldValue, frameField );
			break;
		case TriQuadratic:
			m_pCache->SampleGradientTriQuadratic( p.x, p.y, p.z, fGradX, fGradY, fGradZ, &pFieldValue, frameField );
			break;
		default:
			m_pCache->SampleGradientTriLinear( p.x, p.y, p.z, fGradX, fGradY, fGradZ, &pFieldValue, frameField );
			break;
		}

		vec3 grad(fGradX, fGradY, fGradX);
		return grad;		
	}	
}

void CFieldCache::invalidate(Vol::CVolumeBox &box)
{
	vec3 lo = box.lower();
	vec3 hi = box.upper();
		
	Vol::CVolumeBox TransformBox(lo - m_origin, hi - m_origin);
	m_pCache->Invalidate(TransformBox);
}

void CFieldCache::invalidate()
{
	m_pCache->Invalidate(false);
}


void CFieldCache::NotifyFieldBoundsChanged( const COctree& bounds )
{
	if (bounds.isValid()) {
		m_pCache->SetVoxelSize( bounds, CFieldCacheManager::Get()->GetDefaultCacheResolution() );
	} 
	else 
	{		
		m_pCache->SetVoxelSize(m_children[0]->getOctree(), CFieldCacheManager::Get()->GetDefaultCacheResolution() );
	}
}

void CFieldCache::UpdateCacheResolution( unsigned int nResolution ) 
{
	m_pCache->Invalidate(true);	
	m_pCache->SetVoxelSize( m_children[0]->getOctree(), nResolution, true );
}

void * CFieldCache::GetCache() 
{
	return m_pCache;
}

//CFieldCacheManager
CFieldCacheManager * CFieldCacheManager::m_pCacheManager = NULL;

CFieldCacheManager * CFieldCacheManager::Get()
{
	//Create am instance of it there is no manager
	if (CFieldCacheManager::m_pCacheManager == NULL) 	
		CFieldCacheManager::m_pCacheManager = new CFieldCacheManager();
	
	return CFieldCacheManager::m_pCacheManager;
}

CFieldCacheManager::CFieldCacheManager()
{
	m_eDefaultValueMode = TriLinear;
	m_eDefaultGradientMode = TriQuadratic;
	m_nDefaultCacheResolution = 128;
	m_bCachingEnabled = true;
}

CFieldCacheManager::~CFieldCacheManager()
{
	size_t nSize = m_vCaches.size();
	for (unsigned int i = 0; i < nSize; ++i)
		delete m_vCaches[i];
}


CFieldCache * CFieldCacheManager::CreateCache(CBlobTree * pField, CacheType eType )
{
	CFieldCache * pNewCache = new CFieldCache(pField, eType);
	if (pNewCache)
		m_vCaches.push_back(pNewCache);
	return pNewCache;
}


void CFieldCacheManager::FreeCache( CFieldCache * pCache ) 
{
	// search for cache in vector
	size_t nCount = m_vCaches.size();
	unsigned int i;
	for ( i = 0; i < nCount; ++i ) {
		if ( m_vCaches[i] == pCache )
			break;
	}

	if ( i != nCount ) {
		while ( i < nCount ) {
			m_vCaches[i] = m_vCaches[i+1];
			++i;
		}
		m_vCaches.resize( nCount-1 );
		delete pCache;
	}
}

void CFieldCacheManager::SetDefaultValueMode( SampleMode eMode, bool bSetAllCaches )
{
	m_eDefaultValueMode = eMode;
	if (bSetAllCaches) 
	{
		size_t nSize = m_vCaches.size();
		for (unsigned int i = 0; i < nSize; ++i)
			m_vCaches[i]->setFieldValueSampleMode( m_eDefaultValueMode );
	}
}

void CFieldCacheManager::SetDefaultGradientMode( SampleMode eMode, bool bSetAllCaches )
{
	m_eDefaultGradientMode = eMode;
	if (bSetAllCaches) {
		size_t nSize = m_vCaches.size();
		for (unsigned int i = 0; i < nSize; ++i)
			m_vCaches[i]->setGradientSampleMode( m_eDefaultGradientMode );
	}
}


void CFieldCacheManager::SetDefaultCacheResolution( unsigned int nResolution, bool bSetAllCaches )
{
	m_nDefaultCacheResolution = nResolution;
	if (bSetAllCaches) {
		size_t nSize = m_vCaches.size();
		for (unsigned int i = 0; i < nSize; ++i)
			m_vCaches[i]->UpdateCacheResolution( m_nDefaultCacheResolution );
	}
}
/*

void CFieldCacheManager::OptimizeCacheTree(CBlobTree * pRootNode )
{
	OptimizeCacheTree_Sketching(pRootNode);
	// root node should not have caching enabled

	// top-level composition children should not have caching enabled (??)
	//		only if they have few children & one is selected...

	// nested caches only make sense if the contained tree is large / expensive
}


void CFieldCacheManager::OptimizeCacheTree_Sketching( CBlobTree * pRootNode )
{
	// disable cache on root node
	pRootNode->EnableCaching(false);

	// enable caching on all children up to depth = 2
	RecursiveDisableCaching( pRootNode, 3, 0 );
}

void CFieldCacheManager::RecursiveDisableCaching( CBlobTree * pNode, int nMaxCacheDepth, int nCurDepth )
{
	unsigned int nChildren = pNode->GetNumChildren();
	for ( unsigned int i = 0; i < nChildren; ++i ) {
		if ( ! pNode->GetChild(i)->IsCompositionNode() ) 
			continue;

		rmsbt::CompositionNode * pCompNode = (rmsbt::CompositionNode *)pNode->GetChild(i);

		// do recursion test. Only want to cache *below* children of root (during sketching
		// we add push new prim as child of compop on top of tree. If we are going to edit
		// anything, it's the new prim, so no sense caching it's parent
		bool bEnableCache = (nCurDepth > 0 && nCurDepth < nMaxCacheDepth ? true : false);

		// do children test (if are all cached or primitives, do not cache...)
		if ( bEnableCache ) {
			bEnableCache = false;
			unsigned int nChildren = pCompNode->GetNumChildren();
			for ( unsigned int k = 0; k < nChildren; ++k ) {
				rmsbt::BlobTreeNode * pChild = pCompNode->GetChild( k );
				if ( pChild->IsCompositionNode() && ! ((rmsbt::CompositionNode *)pChild)->IsCachingEnabled() ) {
					bEnableCache = true;
					break;
				}
			}
		}

		pCompNode->EnableCaching( bEnableCache );

		// [MS] chache control for deformation
		// If deformation is involved in a node, we insert a cache.
		if (pCompNode->GetType() == pCompNode->PullDeformationNode || pCompNode->GetParent()->GetType() == pCompNode->PullDeformationNode
			|| pCompNode->GetType() == pCompNode->CurveEditingDeformationNode || pCompNode->GetParent()->GetType() == pCompNode->CurveEditingDeformationNode)
			pCompNode->EnableCaching( true );
				

		// disable caching on all children of this node
		RecursiveDisableCaching( pCompNode, nMaxCacheDepth, nCurDepth+1 );
	}

}
*/
void CFieldCacheManager::EnableCaching( bool bEnable )
{
	m_bCachingEnabled = bEnable;
}

inline SampleMode CFieldCacheManager::GetDefaultValueMode()
{
	return m_eDefaultValueMode;
}

inline SampleMode CFieldCacheManager::GetDefaultGradientMode()
{
	return m_eDefaultGradientMode;
}

inline unsigned int CFieldCacheManager::GetDefaultCacheResolution()
{
	return m_nDefaultCacheResolution;
}


}
}