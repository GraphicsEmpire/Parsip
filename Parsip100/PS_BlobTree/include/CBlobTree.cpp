#include "CBlobTree.h"
#include "PS_FrameWork/include/PS_String.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"

namespace PS{
namespace BLOBTREE {

bool CBlobNode::saveGenericInfoScript(CSketchConfig *lpSketchConfig)
{
    //First write current operator
    //Use the same id passed in as parameter for writing the operator
    DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID());

    //Type
    lpSketchConfig->writeBool(strNodeName, "IsOperator", this->isOperator());

    //Writing Transformation
    lpSketchConfig->writeVec3f(strNodeName, "AffineScale", this->getTransform().getScale());
    lpSketchConfig->writeVec4f(strNodeName, "AffineRotate", this->getTransform().getRotationVec4());
    lpSketchConfig->writeVec3f(strNodeName, "AffineTranslate", this->getTransform().getTranslate());

    if(this->isOperator())
    {
        lpSketchConfig->writeString(strNodeName, "OperatorType", DAnsiStr(this->getName().c_str()));
        lpSketchConfig->writeInt(strNodeName, "ChildrenCount", this->countChildren());

        //First write all the children
        vector<int> arrayInt;
        for(U32 i=0; i<this->countChildren(); i++)
        {
            arrayInt.push_back(this->getChild(i)->getID());
            this->getChild(i)->saveScript(lpSketchConfig);
        }

        lpSketchConfig->writeIntArray(strNodeName, "ChildrenIDs", arrayInt);
        arrayInt.resize(0);
    }
    else
    {
        CMaterial m = this->getMaterial();
        lpSketchConfig->writeVec4f(strNodeName, "MtrlAmbient", m.ambient);
        lpSketchConfig->writeVec4f(strNodeName, "MtrlDiffused", m.diffused);
        lpSketchConfig->writeVec4f(strNodeName, "MtrlSpecular", m.specular);
        lpSketchConfig->writeFloat(strNodeName, "MtrlShininess", m.shininess);        
        //Write skeleton Name
        lpSketchConfig->writeString(strNodeName, "SkeletonType", DAnsiStr(this->getName().c_str()));
    }

    return true;
}

bool CBlobNode::loadGenericInfoScript(CSketchConfig *lpSketchConfig, int id)
{
    DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);

    if(lpSketchConfig->hasSection(strNodeName) < 0)
        return false;

    //Read Node Transformation First
    vec3f affScale = lpSketchConfig->readVec3f(strNodeName, "AffineScale");
    vec4f affRotate = lpSketchConfig->readVec4f(strNodeName, "AffineRotate");
    quat quatRotation(affRotate);
    vec3f affTranslate = lpSketchConfig->readVec3f(strNodeName, "AffineTranslate");
    this->getTransform().set(affScale, quatRotation, affTranslate);

    if(!this->isOperator())
    {
        CMaterial mtrl;
        mtrl.ambient   = lpSketchConfig->readVec4f(strNodeName, "MtrlAmbient");
        mtrl.diffused  = lpSketchConfig->readVec4f(strNodeName, "MtrlDiffused");
        mtrl.specular  = lpSketchConfig->readVec4f(strNodeName, "MtrlSpecular");
        mtrl.shininess = lpSketchConfig->readFloat(strNodeName, "MtrlShininess");
        this->setMaterial(mtrl);
    }
    return true;
}

void CBlobNode::copyGenericInfo(const CBlobNode* rhs)
{
    /*
    int              m_id;
    COctree          m_octree;
    CMaterial        m_material;
    bool             m_bDeleteChildrenUponCleanup;

    CAffineTransformation m_transform;
    */
    this->m_id = rhs->m_id;
    this->m_octree = rhs->m_octree;
    this->m_material = rhs->m_material;
    this->m_bDeleteChildrenUponCleanup = rhs->m_bDeleteChildrenUponCleanup;
    this->m_transform = rhs->m_transform;
}

}
}


