#include "ConversionToActions.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"

//////////////////////////////////////////////////////////////////////////
CBlobTreeToActions::CBlobTreeToActions( CLayer* input )
{
    m_Layer = input;
}

CBlobTreeToActions::~CBlobTreeToActions()
{
    m_lstActions.clear();
    m_Layer = NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBlobTreeToActions::convert( const char* chrOutFileName /*= NULL*/ )
{
    if(m_Layer == NULL) return false;

    m_lstActions.clear();

    m_Layer->convertToBinaryTree(true, false);
    CBlobNode* root = m_Layer->getBlob();

    //Fixed BlobTree now create the actions list
    int res = recursive_nodeToAction(root);
    if(res != root->recursive_CountAllNodes())
        return false;

    if(chrOutFileName && (m_lstActions.size() > 0))
    {
        QString strFN(chrOutFileName);
        QFile fOut(strFN);
        if (fOut.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream s(&fOut);
            for (int i = 0; i < m_lstActions.size(); ++i)
                s << m_lstActions.at(i) << '\n';
        }
        fOut.close();
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
bool CBlobTreeToActions::appendCommand( SKETCHCMD command, CBlobNode* node, vec4f param)
{
    SKETCHCMDPACKET txMsg;

    //Message ID field will be filled before sending the packet
    //by member
    txMsg.cmd	 = command;
    txMsg.param  = param;
    txMsg.msgID  = m_lstActions.count();

    if(node)
    {
        txMsg.blobnodeID = node->getID();
        if(command == cmdAdd)
        {         
            txMsg.nodeType = reinterpret_cast<PS::BLOBTREE::CSkeletonPrimitive*>(node)->getSkeleton()->getType();
            txMsg.param = node->getMaterial().diffused;
        }
        else if(command == cmdOperator)
        {
            txMsg.nodeType = node->getNodeType();
            txMsg.leftChild  = -1;
            txMsg.rightChild = -1;
            if(node->countChildren() == 1)
                txMsg.leftChild = node->getChild(0)->getID();
            else if(node->countChildren() >= 2)
            {
                txMsg.leftChild = node->getChild(0)->getID();
                txMsg.rightChild= node->getChild(1)->getID();
            }
        }
    }


    QString strOutput;
    CSketchNetCommandTranslator::GetCmdTranslator()->translatePacketToStr(txMsg, strOutput);
    m_lstActions.push_back(strOutput);

    return true;
}

int CBlobTreeToActions::recursive_nodeToAction( CBlobNode* node )
{
    int res = 0;
    if(node->isOperator())
    {
        //First all kids should be written
        for(size_t i=0; i< node->countChildren(); i++)
            res += recursive_nodeToAction(node->getChild(i));

        //Now write the real thing
        appendCommand(cmdOperator, node, node->getMaterial().diffused);
    }
    else
    {
        appendCommand(cmdAdd, node, node->getMaterial().diffused);
    }

    //All transformations should be written
    vec3f v = node->getTransform().getTranslate();
    if(v != vec3f(0.0f, 0.0f, 0.0f))
        appendCommand(cmdMove, node, vec4f(v, 0.0f));

    v = node->getTransform().getScale();
    v = v - vec3f(1.0f, 1.0f, 1.0f);
    if(v != vec3f(0.0f, 0.0f, 0.0f))
        appendCommand(cmdScale, node, vec4f(v, 1.0f));

    vec4f q = node->getTransform().getRotation().getAsVec4f();
    if(q != vec4f(0.0f, 0.0f, 0.0f, 1.0f))
        appendCommand(cmdRotate, node, q);

    return res + 1;
}



