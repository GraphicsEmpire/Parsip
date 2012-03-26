#ifndef BLOBTREEBUILDER_H
#define BLOBTREEBUILDER_H

#include "CBlobTree.h"
#include "_constSettings.h"
#include <string.h>

#include <loki/Function.h>
#include <loki/Factory.h>


using namespace Loki;
using namespace std;
using namespace PS;

namespace PS{
namespace BLOBTREE{

//FactoryByName to access BlobNode with their names
typedef SingletonHolder
<
    Factory<CBlobNode, std::string>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryName;

//FactoryByIndex to access BlobNodes with their indices
typedef SingletonHolder
<
    Factory<CBlobNode, BlobNodeType>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryIndex;

//Clone Factory to create a clone of a BlobNode
typedef SingletonHolder
<
    CloneFactory<CBlobNode>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeCloneFactory;


//Actions and ActionManager are the only channel to modify a Blobtree
//1.Actions are all reversible for propert Undo/Redo
//2.Actions are serializeable for replay and record.
//3.Actions are persistent and (Can be queued for future execution)
//enum BlobNodeActions {bnaAdd, bnaDelete, bnaTranslate, bnaRotate, bnaScale, bnaSetParam};
typedef Functor<bool, TL::MakeTypelist<BlobNodeType, vec3f> > FuncAdd;

/*!
  * SketchAction is an action performed on the BlobTree
  */
class SketchAction{
public:
    SketchAction();
    virtual ~SketchAction() {}

    virtual int type() const = 0;
    virtual std::string name() const = 0;

    virtual void execute() = 0;
    //std::ostream& operator<<(std::ostream&);
private:
    U32 m_id;
};

/*!
  * SketchAction is a reversible action
  */
class ReversibleAction : public SketchAction
{
public:
    ReversibleAction() {}
    virtual ~ReversibleAction() {}


    virtual void unexecute() = 0;
};

//Enumaration for all possible BlobNode Actions
enum BlobNodeActions {bnaScale, bnaRotate, bnaTranslate, bnaInsert, bnaDelete};

/*!
  * Container class which can store, playback, Undo and Redo Actions
  * On BlobTree
  */
class CBlobActionManager{
public:
    CBlobActionManager() {m_idxCurrent = -1;}
    virtual~CBlobActionManager() { cleanup();}

    /*!
      * Adds a reversible action into the list of all actions.
      */
    void add(ReversibleAction* lpAction);

    void cleanup();
    void undo();
    void redo();

    //Saves All Actions to script file
    bool save(const std::string& strFilePath) const;

    //Loads All Actions from script file
    bool load(const std::string& strFilePath);

    bool canUndo() const {return m_idxCurrent > 0;}
    bool canRedo() const {return m_idxCurrent < (m_lstActions.size() - 1);}
    U32 count() const {return m_lstActions.size();}

    ReversibleAction* getCurAction() const {
        assert(m_idxCurrent >= 0 && m_idxCurrent < m_lstActions.size());
        return m_lstActions[m_idxCurrent];
    }
private:
    std::vector<ReversibleAction*> m_lstActions;
    int m_idxCurrent;
};



//Affine Actions are all reversible
/*!
 * Scale Action
 */
class ActionScale : public ReversibleAction
{
public:
    ActionScale(CBlobNode* aNode, const vec3f& s):m_lpNode(aNode), m_scale(s) {}

    void execute();
    void unexecute();

    int type() const {return bnaScale;}
    std::string name() const {return "scale";}
private:
    vec3f m_scale;
    CBlobNode* m_lpNode;
};

/*!
  * Rotate Action
  */
class ActionRotate : public ReversibleAction
{
public:
    ActionRotate(CBlobNode* aNode, const quat& q):m_lpNode(aNode), m_quat(q) {}

    void execute();
    void unexecute();

    int type() const {return bnaRotate;}
    std::string name() const {return "rotate";}
private:
    quat m_quat;
    CBlobNode* m_lpNode;
};

/*!
  * Translate Action
  */
class ActionTranslate : public ReversibleAction
{
public:
    ActionTranslate(CBlobNode* aNode, const vec3f& t):m_lpNode(aNode), m_translate(t) {}

    void execute();
    void unexecute();

    int type() const {return bnaTranslate;}
    std::string name() const {return "translate";}
private:
    vec3f m_translate;
    CBlobNode* m_lpNode;
};

/*!
 * Insert Node. Unexecute will remove the node from its parent
 */
class ActionInsert : public ReversibleAction
{
public:
    ActionInsert(CBlobNode* aParent, BlobNodeType nType);

    void execute();
    void unexecute();

    int type() const {return bnaInsert;}
    std::string name() const {return "insert";}

private:
    CBlobNode* m_lpParent;

    BlobNodeType m_nodeType;
    CBlobNode* m_lpNode;
};

/*!
 * Delete Node
 */
class ActionDelete : public ReversibleAction
{
public:
    ActionDelete(CBlobNode* aParent, CBlobNode* aNode);

    void execute();
    void unexecute();

    int type() const {return bnaDelete;}
    std::string name() const {return "delete";}

private:
    CBlobNode* m_lpParent;
    CBlobNode* m_lpNode;
};

}
}
#endif
