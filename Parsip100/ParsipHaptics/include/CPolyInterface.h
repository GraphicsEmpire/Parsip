#ifndef CPARALLELPOLYGONIZER_H
#define CPARALLELPOLYGONIZER_H

#include <vector>

#include "_PolygonizerStructs.h"
#include "PS_BlobTree/include/CBlobTree.h"

//#include "CPolyMCUsingTBB.h"
#include "CPolyContinuation.h"
#include "CPolyParsip.h"
#include "CPolyParsipServer.h"

#include "CLayerManager.h"
#include <QTime>
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"

class QTime;

using namespace tbb;
using namespace BLOBTREE;

class CPolygonizerInterface{

private:
	CLayerManager* m_lm;
	bool m_bForceSerial;
	CParsipServer m_parsipServer;

public:
	CPolygonizerInterface(CLayerManager* lm, bool bForceSerial = false)
	{
		m_lm = lm;
		m_bForceSerial = bForceSerial;
	}

	void setForceSerial(bool bForceSerial){ m_bForceSerial = bForceSerial; }

	void operator() (const blocked_range<size_t>& range) const 
	{
		process(range.begin(), range.end());
	}

	void process() const
	{
		process(0, m_lm->countLayers());
	}

	//TBB uses Half-open Intervals so start is inclusive 
	//and end is exclusive
	void process(size_t startInclusive, size_t endExclusive) const
	{
		for(int i = startInclusive; i != endExclusive; i++)
			process(i);
	}

	void process(size_t idxLayer) const
	{
		CLayer *alayer = m_lm->getLayer(idxLayer);				
		//alayer->getMesh()->removeAll();			
		//renderFrame(alayer);
		m_parsipServer.setup(alayer);
		m_parsipServer.run();
		//runParallelReduceContinuation(alayer);
	}

	void drawMesh()
	{
		m_parsipServer.drawMesh();
	}
};

#endif
