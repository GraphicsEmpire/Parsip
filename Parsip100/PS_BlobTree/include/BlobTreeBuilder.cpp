#include "BlobTreeBuilder.h"

namespace PS{
namespace BLOBTREE{

CBlobTree* createBlobOperator( BlobNodeType opType )
{
    CBlobTree* op = NULL;
    switch(opType)
    {
    case bntOpUnion:
        op = new CUnion();
        break;
    case bntOpIntersect:
        op = new CIntersection();
        break;
    case bntOpDif:
        op = new CDifference();
        break;
    case bntOpSmoothDif:
        op = new CSmoothDifference();
        break;
    case bntOpBlend:
        op = new CBlend();
        break;
    case bntOpRicciBlend:
        op = new CRicciBlend();
        break;
    case bntOpGradientBlend:
        op = new CGradientBlend();
        break;

    case bntOpWarpTwist:
        op = new CWarpTwist();
        break;
    case bntOpWarpTaper:
        op = new CWarpTaper();
        break;
    case bntOpWarpBend:
        op = new CWarpBend();
        break;
    case bntOpWarpShear:
        op = new CWarpShear();
        break;
    case(bntOpPCM):
        op = new CPcm();
        break;
    }

    return op;
}

}
}
