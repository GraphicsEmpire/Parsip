#include "BlobTreeBuilder.h"
#include "BlobTreeLibraryAll.h"

namespace PS{
namespace BLOBTREE{

void CBlobActionManager::cleanup()
{
    m_idxCurrent = -1;
    for(U32 i=0; i<m_lstActions.size(); i++)
        SAFE_DELETE(m_lstActions[i]);
    m_lstActions.resize(0);
}

void CBlobActionManager::undo()
{
    if(canUndo())
    {
        m_lstActions[m_idxCurrent]->unexecute();
        m_idxCurrent--;
    }
}

void CBlobActionManager::redo()
{
    if(canRedo())
    {
        m_lstActions[m_idxCurrent]->execute();
        m_idxCurrent++;
    }
}

bool CBlobActionManager::save(const std::string& strFilePath) const
{
    return false;
}

bool CBlobActionManager::load(const std::string& strFilePath)
{
    return false;
}



}
}
