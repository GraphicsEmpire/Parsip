#ifndef CPCM_H
#define CPCM_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

//***********************************************************
//PCM: Precise Contact Modeling
class  CPcm : public CBlobNode
{
private:
    float m_wPropagateLeft;
    float m_wPropagateRight;

    //Attenuation value for propagation region
    float m_alphaLeft;
    float m_alphaRight;

public:
    CPcm() { resetParams(); }

    float getPropagateLeft() const {return m_wPropagateLeft;}
    void  setPropagateLeft(float val) { m_wPropagateLeft = val;}

    float getPropagateRight() const {return m_wPropagateRight;}
    void  setPropagateRight(float val) { m_wPropagateRight = val;}

    float getAlphaLeft() const {return m_alphaLeft;}
    void  setAlphaLeft(float val) { m_alphaLeft = val;}

    float getAlphaRight() const {return m_alphaRight;}
    void  setAlphaRight(float val) { m_alphaRight = val;}

    void resetParams()
    {
        m_wPropagateLeft = m_wPropagateRight = PCM_PROPAGATION_WIDTH;
        m_alphaLeft = m_alphaRight = PCM_ATTENUATION;
    }

    void setParams(float pl, float pr, float al, float ar)
    {
        m_wPropagateLeft = pl;
        m_wPropagateRight = pr;
        m_alphaLeft = al;
        m_alphaRight = ar;
    }

    //FieldValue computation
    float fieldValue(vec3f p)
    {
        if(m_children.size() == 2)
            return MATHMAX(m_children[0]->fieldValue(p), m_children[1]->fieldValue(p));
        else
            return 0.0;
    }

    float curvature(vec3f p)
    {
        return m_children[0]->curvature(p);
    }

    string getName()
    {
        return "PCM";
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        for(size_t i = 1; i < m_children.size(); i++)
            m_octree.csgUnion(m_children[i]->getOctree());
        return m_octree;
    }

    bool isOperator() { return true;}

    BlobNodeType getNodeType() { return bntOpPCM;}

    bool saveScript(CSketchConfig *lpSketchScript, int idOffset)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID() + idOffset);
        lpSketchScript->writeFloat(strNodeName, "Propagate Left", this->getPropagateLeft());
        lpSketchScript->writeFloat(strNodeName, "Propagate Right", this->getPropagateRight());

        lpSketchScript->writeFloat(strNodeName, "Attenuate Left", this->getAlphaLeft());
        lpSketchScript->writeFloat(strNodeName, "Attenuate Right", this->getAlphaRight());
        return true;
    }

    bool loadScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_wPropagateLeft = lpSketchScript->readFloat(strNodeName, "Propagate Left");
        m_wPropagateRight = lpSketchScript->readFloat(strNodeName, "Propagate Right");
        m_alphaLeft = lpSketchScript->readFloat(strNodeName, "Attenuate Left");
        m_alphaRight = lpSketchScript->readFloat(strNodeName, "Attenuate Right");
        return true;
    }

    int getProperties(PropertyList &outProperties)
    {
        outProperties.resize(0);
        outProperties.add(m_wPropagateLeft, "Propagate Left");
        outProperties.add(m_wPropagateRight, "Propagate Right");
        outProperties.add(m_alphaLeft, "Attenuate Right");
        outProperties.add(m_alphaRight, "Attenuate Right");
        return 4;
    }

    int setProperties(const PropertyList &inProperties)
    {        
        //m_wPropagateLeft = inProperties.findProperty("Propagate Left", ttFLT).value.valFloat;
        return 4;

        /*
            CPcm* pcm = reinterpret_cast<CPcm*>(lpNode);
            lstRow.push_back(new QStandardItem(QString("Propagate Left")));
            lstRow.push_back(new QStandardItem(printToQStr("%.2f", pcm->getPropagateLeft())));
            m_modelBlobNodeProperty->appendRow(lstRow);
            lstRow.clear();

            lstRow.push_back(new QStandardItem(QString("Propagate Right")));
            lstRow.push_back(new QStandardItem(printToQStr("%.2f", pcm->getPropagateRight())));
            m_modelBlobNodeProperty->appendRow(lstRow);
            lstRow.clear();

            lstRow.push_back(new QStandardItem(QString("Attenuate Left")));
            lstRow.push_back(new QStandardItem(printToQStr("%.2f", pcm->getAlphaLeft())));
            m_modelBlobNodeProperty->appendRow(lstRow);
            lstRow.clear();

            lstRow.push_back(new QStandardItem(QString("Attenuate Right")));
            lstRow.push_back(new QStandardItem(printToQStr("%.2f", pcm->getAlphaRight())));
            m_modelBlobNodeProperty->appendRow(lstRow);
            */
    }





    };

}
}


#endif
