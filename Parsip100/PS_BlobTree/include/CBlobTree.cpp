#include "CBlobTree.h"
#include "PS_FrameWork/include/PS_String.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
#include "PS_ErrorManager.h"

namespace PS{
namespace BLOBTREE {

bool CBlobNode::isIDRange(const vector<int>& ids) const
{
    if(ids.size()  < 2)
        return false;

    int prev = ids[0];
    for(int i=1; i<ids.size(); i++)
    {
        if(ids[i] != prev+1)
            return false;
        prev = ids[i];
    }
    return true;
}

bool CBlobNode::isAllChildrenPrims()
{
    U32 ctPrims = 0;
    for(U32 i=0; i<this->countChildren(); i++)
    {
        if(!this->getChild(i)->isOperator())
            ctPrims++;
    }
    return (ctPrims == this->countChildren());
}

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
        U32 ctPrimChildren = 0;
        for(U32 i=0; i<this->countChildren(); i++)
        {
            arrayInt.push_back(this->getChild(i)->getID());
            this->getChild(i)->saveScript(lpSketchConfig);

            if(!this->getChild(i)->isOperator())
                ctPrimChildren++;
        }

        //If children IDs are really large then compress them
        lpSketchConfig->writeBool(strNodeName, "ChildrenIDsUseRange", false);

        //If all children are primitives and their IDs are in range
        bool bInRange = isIDRange(arrayInt);
        if(arrayInt.size() > 2)
        {
            if(!bInRange)
            {
                ReportError("SAVESCRIPT: Consider reassigning IDs to have them in order!");
                FlushAllErrors();
            }

            if(ctPrimChildren != this->countChildren())
            {
                ReportError("SAVESCRIPT: All children have to be primitives to make a range!");
                FlushAllErrors();
            }
        }

        //Perform Range
        if((arrayInt.size() > 2)&&(bInRange)&&(ctPrimChildren == this->countChildren()))
        {
            printf("SAVESCRIPT: Making a range from:%d to:%d\n", arrayInt[0], arrayInt[arrayInt.size() -1]);

            lpSketchConfig->writeBool(strNodeName, "ChildrenIDsUseRange", true);
            std::vector<int> vRange;
            vRange.push_back(arrayInt[0]);
            vRange.push_back(arrayInt[arrayInt.size() - 1]);
            lpSketchConfig->writeIntArray(strNodeName, "ChildrenIDsRange", vRange);

            vRange.resize(0);
        }
        else
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
        lpSketchConfig->writeString(strNodeName, "PrimitiveType", DAnsiStr(this->getName().c_str()));
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
    this->m_id = rhs->m_id;
    this->m_octree = rhs->m_octree;
    this->m_material = rhs->m_material;    
    this->m_transform = rhs->m_transform;
}


int CBlobNode::getGenericProperties(PropertyList& outProperties)
{
    outProperties.resize(0);
    outProperties.add(m_transform.getScale(), "scale");
    outProperties.add(m_transform.getTranslate(), "translate");

    vec3f axis;
    float angleDeg;
    m_transform.getRotation().getAxisAngle(axis, angleDeg);
    outProperties.add(vec4f(axis, angleDeg), "rotate");
    return 3;
}

int CBlobNode::setGenericProperties(const PropertyList& inProperties)
{
    if(inProperties.size() == 0)
        return 0;

    int idxProp = PropertyList::FindProperty(inProperties, "scale");
    if(idxProp >= 0)
        this->getTransform().setScale(inProperties[idxProp].asVec3());

    idxProp = PropertyList::FindProperty(inProperties, "translate");
    if(idxProp >= 0)
        this->getTransform().setTranslate(inProperties[idxProp].asVec3());


    idxProp = PropertyList::FindProperty(inProperties, "rotate");
    if(idxProp >= 0)
    {
        vec4f rot = inProperties[idxProp].asVec4();
        this->getTransform().setRotation(rot.w, rot.xyz());
    }
    return 3;
}

}
}


